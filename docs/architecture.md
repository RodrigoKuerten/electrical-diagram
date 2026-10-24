# Architecture

FioVivo is split into layers so that the electrical model and the simulator can be tested
without Qt. This document describes the implementation that exists in the repository; where
an extension is only planned, it says so explicitly.

All source identifiers, comments and documentation are in English. User-facing text is
translated, with Brazilian Portuguese as the default UI language.

## 1. Layers and targets

| Layer | CMake target | Responsibility | Depends on | Qt |
|---|---|---|---|---|
| `core` | `fiovivo_core` | Strong IDs, `Result<T>`, diagram model, geometry/routing, validation | C++ standard library | no |
| `sim` | `fiovivo_sim` | Scan-cycle engine, connectivity, timers, device behaviour, observer boundary | `core` | no |
| `io` | `fiovivo_io` | `.dcj` read/write, embedded schema validation, migrations, BOM CSV | `core`, nlohmann/json | no |
| `library` | `fiovivo_symbols` | Symbol manifests and built-in instantiation | `core`, nlohmann/json | no |
| `library` | `fiovivo_library` | Preloaded SVG renderer and icons | `fiovivo_symbols`, Qt Gui/Svg | renderer only |
| `ui` | `fiovivo_ui` | Main window, canvas, docks, dialogs, exports, simulation adapter | all lower layers, Qt Widgets | yes |
| `app` | `fiovivo` | `main()`, settings, translator installation, composition root | `ui` | yes |

The dependency graph is acyclic:

```text
app -> ui -> sim, io, library, core
            library -> symbols -> core
            io -> core
            sim -> core
```

Mandatory rules:

1. `core` includes no Qt header, performs no file I/O and depends on no other project layer.
2. `sim` includes no Qt type, does no I/O and has no translation/settings dependency. It can
   be stepped from a plain GoogleTest process.
3. `io` depends on `core` and nlohmann/json only. It does **not** depend on `sim`, Qt or UI.
   SVG/PNG/PDF rendering lives in `ui/export_utils.cpp` because it needs Qt.
4. `fiovivo_symbols` is Qt-free. `fiovivo_library` is the only symbol target that uses Qt,
   and only for SVG/pixmap rendering.
5. `ui` may depend on all lower layers. It reaches the simulator only through
   `SimulationController` and never touches a worker object directly.
6. `app` constructs settings, installs the translator and creates the main window. Business
   rules stay out of `main.cpp`.
7. Lower layers never include `ui/` or `app/` headers. Includes use the `src/` root, for
   example `#include "core/model.hpp"`.
8. The headless build `-DFIOVIVO_BUILD_UI=OFF` builds and tests `core`, `sim`, `io` and
   `fiovivo_symbols`; this is how the no-Qt rule is enforced in CI.

## 2. Directory map

```text
src/
  core/        result, ids, types, model, geometry/routing, validation
  sim/         simulation_types, simulator (scan cycle, connectivity, devices)
  io/          json_io, schema_validator, bom; embedded schema generated from docs/
  library/     symbol_definition, svg_renderer
  ui/          main_window, document, sheet_scene/view, component/pin/wire/annotation items,
               docks, dialogs, simulation controller/worker, export helpers, settings
  app/         main.cpp
  i18n/        fiovivo_pt_BR.ts, fiovivo_en.ts, lrelease build integration
tests/
  unit/        core, sim, io, library
  integration/ the five bundled examples, determinism, BOM golden
  ui/          canvas, exporter, translation and worker-thread tests
  data/        golden files and fixtures
assets/
  symbols/     original SVG artwork + JSON manifests
  fiovivo.svg  application mark
examples/      .dcj example projects
tools/         example generator and deliberate BOM golden helper
docs/          this document and the other deliverables
```

## 3. Ownership and identifiers

- Model values are held by value in `Diagram`, `Sheet`, `Component`, `Pin`, `Wire` and
  `Annotation`.
- Cross-references use strong typed IDs (`ComponentId`, `PinId`, `WireId`, `SheetId`,
  `AnnotationId`), never raw pointers or integers.
- `Diagram` owns an `IdAllocator` that is advanced on insertion and rebuilt after loading.
  IDs are stable, sequential and never reused.
- Top-level UI objects are owned by Qt parents or `std::unique_ptr`. The simulation worker is
  owned by `SimulationController` and moved to its `QThread`; Qt widgets never see raw
  simulator pointers.
- `shared_ptr` is used only for cached, immutable SVG renderers shared by canvas items.
- Undo is snapshot-based: `Document::mutate` captures a `Diagram` before and after an edit and
  pushes one `QUndoCommand`. This is deliberately simple and makes every editing operation
  undoable, including multi-item moves, paste and parameter edits.

## 4. Errors

- `fiovivo::Result<T>` and `Result<void>` carry an `Error { code, message, context }`.
- Core, sim and io never throw across a library boundary. JSON parsing exceptions are caught
  and converted. `assert` is reserved for programmer errors such as calling `value()` on an
  error result.
- Schema errors include the JSON pointer of the offending field.
- Wire creation errors are returned to the canvas and shown in the status bar/output.
- Oscillation is returned as `ErrorCode::Oscillation`; the simulation stops and the UI keeps
  the last stable frame for inspection.

## 5. Simulation threading

The UI thread never executes scan logic. The worker thread never touches a widget.

| Thread | Owns | Activity |
|---|---|---|
| UI | `QApplication`, `MainWindow`, `Document`, `SheetScene/View`, docks, `SimulationController`, export code | Painting, editing, input, queued command emission |
| Worker | `SimulationWorker`, `sim::Simulator`, the immutable diagram copy, `QTimer`, `QElapsedTimer` | Scan cycles, timers, observer callbacks |

`SimulationController::loadDiagram` copies the diagram, creates a worker, moves it to a
`QThread` and starts it. The worker creates its `QTimer` inside the worker thread during
`initialize()`.

Commands (`start`, `pause`, `stepOnce`, `stepCycles`, `reset`, `setInput`, `setTripped`,
`setSelector`, `setSpeed`) are delivered with queued `QMetaObject::invokeMethod` calls.
Frames and log entries return through queued signals carrying Qt-friendly value types
(`SimFrame`, `SimLogEntry`, registered as metatypes). The observer is implemented by the
worker; the engine has no Qt dependency.

Structural edits while a simulation session is loaded stop and discard the session from the
UI (the diagram-change signal triggers `discardSimulation`). The next Start takes a fresh
snapshot. Input edits remain available while running or paused. This is safer than trying to
mutate a diagram observed by the worker.

Pacing is wall-clock best effort: the worker accumulates wall time, runs `cycle_ms` of
simulated time per step and may batch several steps at 2x-10x speed. The logical state
sequence is independent of host load; tests drive `sim::Simulator::step()` directly.

## 6. Determinism

- All model collections used for simulation are vectors with stable insertion order or
  `std::map`/`std::set`; no simulation decision depends on hash iteration or pointer values.
- Coils are evaluated in `(tag, id)` order. This gives deterministic "first wins" behavior
  for two interlocked coils requested in the same scan.
- Fixed-point convergence is bounded at 16 passes. Every pass that changes a coil state
  updates the shared contactor-group snapshot so later coils see the change.
- Source groups are strings derived from supply type and pin name. They are stable and
  ordered.
- The simulator owns its own `Diagram` copy; an observer cannot mutate it.
- The simulation log, snapshots and `SimulationFrame` are deterministic for identical input
  sequences and cycle period.

## 7. UI structure

- `Document` owns the `Diagram`, current sheet and `QUndoStack`. All edits go through
  `mutate(...)`; loading and new-project creation replace the diagram and clear the stack.
- `SheetScene` rebuilds items when the document changes, preserving selection by ID. It
  draws the page, grid, frame and title block, handles selection, wiring drag/click mode,
  drop from the library and context menus.
- `ComponentItem` preloads the state SVG, paints the tag/status/timer overlay and hosts
  `PinItem` children with a 5 mm hit circle. Pin tooltips show name, role and connection
  count.
- `WireItem` paints orthogonal routes and highlights live wires. Junction dots are drawn by
  pins with three or more wires.
- `PropertiesDock` edits typed parameters, tags, descriptions, positions, rotations and wire
  styles. It hides the internal `z_order` parameter.
- `SimulationController`/`SimulationWorker` bridge the engine and the canvas. `MainWindow`
  applies `SimFrame` updates to the current scene without rebuilding the document.
- `io` is used for load/save/BOM; `ui/export_utils` renders PNG/SVG/PDF and print previews
  with Qt.

## 8. Internationalization

- Source strings are English and all user-visible strings use `tr()` or
  `QCoreApplication::translate`. Dynamic simulation state strings use explicit translation
  contexts.
- `src/i18n/fiovivo_pt_BR.ts` and `fiovivo_en.ts` are committed. CMake invokes `lrelease`
  during the build and copies the `.qm` files into `assets/i18n` next to the executable;
  installation places them under `share/fiovivo/assets/i18n`.
- `main()` installs the translator selected by `AppSettings` before constructing the window.
  The default is `pt_BR`. A language change takes effect on restart, as indicated in the
  preferences dialog.
- Manifest `display_name_pt` and parameter labels provide domain terminology outside the Qt
  translation catalog. Number and date formatting use `QLocale`.

## 9. Extension points

- **Voltage/current model:** source groups are already distinct conductor identities. A future
  version can attach a voltage value to a source group and an impedance to loads without
  changing graph references. Fault-current, protection-coordination and thermal models are
  explicitly out of scope.
- **Simulation snapshots on disk:** `SimulationSnapshot` is a plain value type with
  deterministic maps. A future io writer can serialize it next to the project without
  changing the engine.
- **Symbol editing:** manifests carry `symbol_version`, `states` and `element_ids`. A Phase 4
  editor can modify manifests/renderers without touching the domain model.
- **Additional devices:** add a manifest, a behaviour branch in the simulator and tests.
  Unknown component types load and round-trip but have no simulated behaviour.

## 10. Safety boundary

The architecture is intentionally not a protection-coordination engine. There is no current,
voltage magnitude, impedance, heating or mechanical model. The About dialog and README carry
the same disclaimer: the tool is educational and must not be used for safety-critical design
or commissioning.
