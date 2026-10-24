# Implementation plan and decision log

This document is the written plan required before coding starts, and is kept up to date as
decisions are made.

## 1. Working name

The working title is **FioVivo** ("live wire"). The user has not fixed a final product name.
The CMake project name, application display name, translation context and asset identifiers
are centralized so that renaming is a mechanical change. Internal C++ namespaces use
`fiovivo` and the file extension remains `.dcj` (DiagramaComandos JSON), as required by the
project specification.

## 2. Architecture

```
core   <-- sim   (sim depends on core only)
  ^       ^
  |       |
  io <----+          io depends on core only; JSON is a third-party header library
  ^
library ----------- Qt only in the SVG renderer
  ^
ui  --> all of the above
app --> ui, io, library
```

- `fiovivo_core`: strong IDs, diagram model, routing geometry, validation and result types.
  No Qt, no exceptions in normal control flow.
- `fiovivo_sim`: pure C++ scan-cycle engine. It observes the model through const references
  and reports through a plain observer interface.
- `fiovivo_io`: `.dcj` load/save, schema validation, migrations and BOM CSV export.
  Depends on `core` and nlohmann/json only. PNG/SVG/PDF rendering lives in the Qt UI layer.
- `fiovivo_symbols`: symbol manifests and built-in definitions, no Qt. Depends on `core` and
  nlohmann/json for manifest parsing.
- `fiovivo_library`: Qt SVG renderer that consumes the manifests. Qt is used only for
  rendering, not for the model.
- `fiovivo_ui`: Qt Widgets editor, canvas, docks, dialogs and the thin simulation adapter.
- `fiovivo`: `main()` and wiring.

Ownership uses `std::unique_ptr` for top-level objects and value types inside the model.
Graph links use strong IDs, never raw pointers. `std::shared_ptr` is avoided unless a shared
immutable Qt resource needs it.

Error handling uses `fiovivo::Result<T>` with an `Error { code, message, context }`. Core and
sim never throw to the UI. JSON parsing errors are converted to `Result`.

The simulation runs in a worker `QThread` owned by a `SimulationController` QObject. The
worker communicates with UI through queued Qt signals; it never touches widgets. Commands
(start, pause, step, input changes) are delivered with `QMetaObject::invokeMethod`. The sim
engine itself has no Qt dependency and is tested synchronously.

## 3. Deliberate deviations and conflict resolutions

### 3.1 More than one wire per pin

The specification says a pin may connect to at most one wire (§3.3) while also requiring
parallel circuits, self-holding, indicator lamps in parallel and bundled examples. That is
not constructible with strictly one wire per pin. The implementation permits multiple
wires on a pin and draws a junction dot when three or more conductors meet. There is no hard
cap in this build. This is the only coherent way to satisfy the examples and tests. The
conflict is reported in the final report for owner confirmation.

### 3.2 Strong IDs and file IDs

Internal IDs are strong integer types. `.dcj` files store them as prefixed strings (`c1`,
`p1`, `w1`, `s1`, `a1`) for readability and stable round-trips. IDs are allocated
sequentially per file and never reused after a deletion.

### 3.3 Symbol state rendering

Each symbol is an SVG plus a JSON manifest. Dynamic states that change geometry are rendered
by swapping to a preloaded active-state SVG (for example `contactor_no_active.svg`), as
allowed by the manifest. The SVG is parsed once when the library is loaded, never during a
simulation cycle. Coils, lamps and motors additionally receive a colored state overlay.

### 3.4 Schema validation

The repository ships `docs/file-format.schema.json`. The loader embeds it at configure time
and validates structural rules through a small in-tree JSON Schema subset validator (types,
required fields, enums, items, patterns, additional properties). This avoids adding an
unlisted dependency and is unit-tested against valid and invalid fixtures. Referential
integrity is checked separately.

### 3.4.1 Internal stacking parameter

The context-menu actions **Bring to Front** and **Send to Back** store an integer `z_order`
parameter in the `.dcj` file. The properties dock hides this internal key. It is a small UI
extension to the original component field list and has no electrical meaning.

### 3.5 Format version 1 only

Only format version 1 exists now. The migration framework already dispatches per version so
that version 2 migrations can be added without changing the loader contract. Migration
fixtures for version 1 are committed.

## 4. Electrical convention ambiguities and defaults

Every default below is stored as a component parameter or application setting where the
model allows it.

| Topic | Default | Configurability / notes |
|---|---|---|
| Coil/contact pairing | `label_group` parameter; if absent, the component tag is used | Editable per contact and coil; validation warns on unmatched contacts |
| NC rest state | NC contacts are closed at rest and open when actuated | `normally_open` parameter is read-only metadata per type |
| Contactor auxiliary numbering | NO 13/14, NC 21/22 | Symbol manifests can be versioned with other numbering |
| Thermal relay | 95/96 NC, 97/98 NO; trip is a latched user input | `trip_current`, `reset_mode`; heating is not modelled |
| Emergency stop | Latching NC, opens when triggered, explicit reset required | `latching=true` by default |
| Selector switch | Common pin 1, position pins 2..n; one position closed at a time | Number of positions and maintained/momentary mode are parameters |
| Timer contact response | Timer output changes at the end of the scan that reaches the delay; the graph is rebuilt and contactors re-solved in the same scan | `delay_ms`, `time_base`, `cycle_ms`; documented in simulation-semantics.md |
| Motor energization | Running when at least two of U1/V1/W1 reach distinct source groups; the symbol has no PE or winding-end pins | `rated_voltage`, `power_kw`, `connection`, `min_live_phases` (default 2) |
| Supply return | A load needs two distinct source groups (for example L1 and N); PE is never a return | `rated_voltage`, `frequency` |
| Phase identity | Tracked as source groups even though voltage magnitude is not simulated | Future voltage extension point |
| Wire junction | Junction dot when 3 or more wires share a pin | Not part of Phase 1 file format beyond `wires` |
| Grid | 5 mm; symbol grid 10 mm | Preferences and `.dcj` sheet settings |
| Units | One scene unit = 1 mm; 100% zoom = 96 dpi (3.7795 px/mm) | Zoom range 25-400% |
| Auto-routing | Orthogonal deterministic route with obstacle avoidance; segments are not draggable yet | Router has no Qt dependency and is unit-tested |

## 5. Questions that need an answer from the project owner

These do not block implementation; defaults are in place.

1. Confirm the final product name and application ID. The working title FioVivo is used
   throughout.
2. Confirm that multiple wires per pin / explicit junctions are acceptable, since the
   one-wire-per-pin rule makes the required examples impossible.
3. Which phase should be considered the delivery target for the first public release? The
   implementation covers Phase 1, Phase 2 and most of Phase 3. Simulation snapshot files and
   shortcut remapping remain open.
4. Should the About dialog link to the project repository and issue tracker at a final URL?
5. Should the default paper be A4 landscape (current) or A3?
6. Is MIT the intended project license?

## 6. Work breakdown and status

1. Repository, toolchain, docs, licensing, formatting and CI — done.
2. Core model, strong IDs, result type, routing and validation, with tests — done.
3. Symbol manifests and SVG artwork, with consistency tests — done.
4. JSON persistence, schema subset validator, migrations, examples, golden tests — done
   (BOM golden; SVG is tested structurally and for repeated-output determinism).
5. Simulation engine, all Phase 1/2 behaviours, determinism and timing tests — done.
6. Qt UI: main window, docks, canvas, wiring, undo, properties, problems — done.
7. Simulation UI and worker thread, logs, snapshots — worker, logs and in-memory snapshots
   done; snapshot files not done.
8. Export and printing, autosave/recovery, settings and i18n — done.
9. UI tests, sanitizer/coverage CI, documentation and final report — done in the repository;
   sanitizer/coverage execution is delegated to the GitHub Actions workflow.

Commits follow Conventional Commits. Phase ends are tagged `v0.1.0-phaseN`.

## 7. Handoff summary

- 134 CTest tests pass in the local Debug and Release (MinGW-w64 UCRT) configurations and
  in the local Clang build; 122 headless tests run in the coverage configuration.
- Line coverage measured with gcovr: 88% overall for `core`+`sim` (core 88%, sim 90%).

- Measured on the reference machine (AMD Ryzen 5 5500, Windows 11, GCC 16.2, Qt 6.12,
  Release): **0.62 ms per scan** for 500 components / 800 wires and **52.5 ms** to open and
  render a 500-component sheet.
- Known deviations and remaining gaps are listed in `README.md` and `docs/report.md`.
