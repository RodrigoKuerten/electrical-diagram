# Architecture

FioVivo is an educational electrical control diagram editor and simulator. This document
describes the target architecture and the rules that implementation code must follow. It
complements [`plan.md`](plan.md), which is the decision log, and
[`simulation-semantics.md`](simulation-semantics.md), which defines the scan-cycle behavior.

The project is built phase by phase. Paths and class names below describe the intended
structure; some of them do not exist yet. Statements about layers, dependencies, ownership,
threading and determinism are normative rules for code as it lands. Features that are not
implemented are marked **planned**.

All documentation, source identifiers, comments, log keys and commit messages are written in
English. User-facing text is translated; Brazilian Portuguese is the default UI language. The
only Portuguese in the documentation is in the glossary and in quoted domain or UI terms.

## 1. Layers

The system has six conceptual layers. The symbol library is one layer but is built as two
CMake targets so that manifest code stays free of Qt, as required by `plan.md`.

| Layer | CMake target(s) | Responsibility | May depend on | Qt |
|---|---|---|---|---|
| `core` | `fiovivo_core` | Strong IDs, `Result`, `Error`, diagram model, routing geometry, validation and problem reporting | C++ standard library only | No |
| `sim` | `fiovivo_sim` | Scan-cycle engine, connectivity and source groups, device semantics, timers, observer boundary | `core` only | No |
| `io` | `fiovivo_io` | `.dcj` load/save, schema subset validation, migrations, BOM and SVG export helpers | `core` and nlohmann/json | No |
| `library` | `fiovivo_symbols`, `fiovivo_library` | Symbol manifests and built-in definitions; Qt SVG rendering of those manifests | `fiovivo_symbols`: `core` only. `fiovivo_library`: `core`, `fiovivo_symbols`, Qt Gui/Svg | Only in `fiovivo_library` |
| `ui` | `fiovivo_ui` | Qt Widgets editor, canvas, docks, dialogs, problems panel, simulation adapter | All lower layers | Qt Widgets |
| `app` | `fiovivo` | `main()`, composition root, settings, translator installation, window wiring | `ui`, `io`, `library` | Qt |

The six layers remain the mental model even though `library` is split:

- `fiovivo_symbols` owns the Qt-free symbol manifest model and the built-in symbol
  definitions. It is the part that can be tested headlessly.
- `fiovivo_library` owns the SVG renderer. It consumes manifests and produces pixmaps or
  paints into a Qt paint device. It does not own domain logic.

## 2. Dependency rules

```text
app
 |-- ui
 |    |-- sim ---------> core
 |    |-- io ----------> core + nlohmann/json
 |    `-- library -----> symbols -----> core
 |                   `-> Qt Gui/Svg
 |-- io
 `-- library
```

The graph is acyclic. The following rules are mandatory:

1. `core` does not include Qt headers, does not perform file I/O, and does not depend on any
   other project layer. It uses only the C++ standard library.
2. `sim` depends on `core` only. It includes no Qt type, no filesystem access, no settings,
   no translation and no QObject. The engine can be constructed and stepped in a plain
   GoogleTest process without an event loop.
3. `io` depends on `core` and nlohmann/json only. It does not depend on `sim`, `library`,
   `ui` or Qt. Loading a file never starts a simulation.
4. `fiovivo_symbols` depends on `core` only and includes no Qt. Manifests and built-in symbol
   definitions are plain data.
5. `fiovivo_library` may use Qt Gui and Qt Svg for rendering. It does not use Qt Widgets and
   does not mutate the diagram model.
6. `ui` may use all lower layers and Qt Widgets. It reaches the simulation engine only
   through the simulation adapter described in section 4. It does not implement electrical
   semantics of its own.
7. `app` is the composition root. It constructs concrete services, installs translators and
   connects the main window and simulation controller. Domain rules do not live in `app`.
8. Tests may depend on the layer under test and GoogleTest. Test-only helpers are never
   linked into production targets.
9. Lower layers never include headers from `ui` or `app`. Includes use the
   `"core/..."`, `"sim/..."`, `"io/..."`, `"library/..."`, `"ui/..."` form, never relative
   paths such as `../../core/...`.
10. CMake target links express the same rules. A dependency that is only needed by an
    implementation file is `PRIVATE`; a type that appears in a public header is `PUBLIC`.
    Review rejects an include that bypasses the target graph.

The headless build (`FIOVIVO_BUILD_UI=OFF`) must continue to build `core`, `sim`, `io` and
`symbols`. This is how the no-Qt rule is checked in practice.

## 3. Target directory layout

The tree below is the intended layout. File names may be split as the implementation grows;
the layer boundaries and dependency direction are the normative part.

```text
src/
  core/
    ids.hpp                    strong ID template and aliases
    result.hpp                 Result<T>, Error, ErrorCode
    model/                     diagram, sheet, component, pin, wire values
    routing/                   orthogonal routing geometry
    validation/                validation rules and problem values
  sim/
    simulation_engine.hpp      scan-cycle entry point
    connectivity/              nets, source groups, union-find
    devices/                   contactor, timer, selector, protection behavior
    observer.hpp               SimulationObserver and state/event values
  io/
    dcj/                       reader, writer, schema subset validator
      migrations/              version-to-version migration functions
    export/                    BOM and SVG text export helpers
  library/
    symbols/                   fiovivo_symbols, Qt-free manifests and built-ins
    render/                    fiovivo_library, Qt SVG renderer
  ui/
    main_window.hpp
    canvas/                    scene/view, selection, grid, wiring tools
    docks/                     properties, problems, simulation log
    dialogs/                   settings, about, export
    simulation/                SimulationController, SimulationWorker
  app/
    main.cpp                   composition root
  i18n/
    fiovivo_pt_BR.ts
    fiovivo_en.ts
tests/
  core/
  sim/
  io/
  library/
  ui/
  integration/
  golden/
  fixtures/
assets/
  symbols/                     SVG artwork and manifests
examples/                      .dcj example projects
docs/                          plan, architecture, semantics, glossary, contributing
cmake/                         warning and dependency helpers
```

The project include root is `src/`, so a public header is included as
`#include "core/model/diagram.hpp"`, not with a path relative to the including file.

## 4. Threading model

The simulation is paced by a worker thread. The UI thread never runs scan logic, and the
worker thread never touches widgets.

| Thread | Owns | Runs |
|---|---|---|
| UI thread | `QApplication`, `MainWindow`, canvas, docks, dialogs, `DiagramDocument`, undo stack, `SimulationController` | All widget painting and user input |
| Simulation worker thread | `SimulationWorker` QObject, `SimulationEngine`, immutable diagram snapshot, cycle `QTimer` | Scan cycles, connectivity, timers, event generation |

`SimulationController` is a QObject that lives in the UI thread and owns the `QThread`. It
creates a `SimulationWorker`, moves it to the thread and starts the thread. The worker owns
the engine. The engine itself is a plain C++ object and has no Qt dependency.

### 4.1 Model handoff

A running simulation must not read a document that the user can edit concurrently. At
session start, the controller takes an immutable snapshot of the diagram and gives it to the
worker. The engine observes that snapshot through `const` references for the whole session.
The live `DiagramDocument` remains owned by the editor and is never accessed by the worker.

The planned editing policy is:

- Structural edits (adding, deleting, moving, rewiring, changing parameters) are blocked
  while a simulation session is running or paused.
- Input changes (pushbuttons, selector positions, emergency-stop trigger/reset, thermal
  trip/reset, fuse/breaker state) are allowed during a session.
- Stopping the session releases the snapshot. The next start takes a fresh snapshot, so it
  sees the edited document.

This policy avoids data races and keeps every scan reproducible. It can be relaxed later
only by versioning the snapshot and documenting the behavior in
`simulation-semantics.md`.

### 4.2 Commands and state

Commands travel from the UI to the worker as queued invocations. The worker never reads UI
state directly.

| Direction | Mechanism | Applied |
|---|---|---|
| UI to worker | `QMetaObject::invokeMethod(worker, ..., Qt::QueuedConnection)` | At the next command-processing point, normally a scan boundary |
| Worker to UI | Queued Qt signal carrying an immutable `SimulationState` value | On the UI thread's event loop |
| Worker to UI | Queued Qt signal carrying `SimulationEvent` values, in order | On the UI thread's event loop |

Commands include start, pause, resume, step, reset, stop, input changes and, when the
snapshot feature lands, restore. Because the worker has a single event loop and a single
engine, command order is the order in which the worker processes them. The UI must not call
engine methods directly.

The worker uses a `QTimer` with the configured `cycle_ms` period. The timer is created in the
worker's thread, after `moveToThread`, so it belongs to the worker event loop. Each timeout
performs at most one scan. There is no catch-up loop: if the event loop is delayed, simulated
time still advances by exactly one cycle per scan, and wall-clock time may diverge from
simulated time. This is intentional. The engine's state sequence depends on the scan sequence
and the input snapshot, not on host scheduling.

Shutdown is orderly: pause, stop the engine, quit the thread event loop, wait for the
thread, then release the worker. `QThread::terminate()` is never used.

### 4.3 Observer boundary

`sim` exposes a plain observer interface. It reports state and events without knowing that
Qt exists.

```cpp
namespace fiovivo::sim {

class SimulationObserver {
public:
  virtual ~SimulationObserver() = default;

  // Called on the simulation worker thread after a committed scan.
  virtual void onStateChanged(const SimulationState& state) = 0;

  // Called in deterministic order for each log event.
  virtual void onEvent(const SimulationEvent& event) = 0;

  // Called once when the session stops.
  virtual void onStopped(SimulationStopReason reason) = 0;
};

}  // namespace fiovivo::sim
```

The names are the planned names; the contract is what matters:

- Callbacks are synchronous and run on the worker thread.
- The engine does not block waiting for the UI and does not call back into itself from the
  observer.
- An observer that wants to retain state copies it. A `const SimulationState&` is valid only
  for the duration of the callback.
- Observer implementations must not throw. The UI adapter catches unexpected exceptions at
  its own boundary and reports a failure.
- State is emitted only after a scan commits. The UI never receives a partially evaluated
  fixed point.
- Events are delivered in deterministic order. The UI may coalesce state repaints, but it
  must not reorder events.

The UI-side adapter implements `SimulationObserver`, copies the state, translates it into a
Qt signal and emits it. `SimulationState`, `SimulationEvent` and `SimulationStopReason` are
plain value types; if they cross a queued connection they are registered as Qt metatypes. No
Qt type appears in the `sim` headers.

### 4.4 What the worker must not touch

The worker and the engine must never access:

- `QWidget` or any other widget class;
- `QPixmap`, `QImage`, `QPainter` or `QSvgRenderer`;
- the live `DiagramDocument` or the undo stack;
- `QSettings`, translation objects or file dialogs;
- the canvas item map or selection state.

The engine has no Qt dependency at all. The worker uses QtCore for its event loop, timer,
signals and queued invocations only.

## 5. Ownership and identifiers

### 5.1 Ownership rules

| Object kind | Ownership | Notes |
|---|---|---|
| Top-level windows, controllers and services | `std::unique_ptr` held by `app` or the owning UI object | One clear owner |
| QObject children | Qt parent-child ownership | No manual delete |
| Diagram document | Value owned by the editor/document controller | Copied for sessions and undo snapshots |
| Sheets, components, pins, wires | Values in `std::map` keyed by strong ID | The model is a value graph, not a pointer graph |
| Links between model objects | Strong IDs | Non-owning; a link never keeps an object alive |
| Simulation snapshot | Value or `std::unique_ptr` owned by the worker | Immutable during a session |
| Parsed SVG artwork and immutable symbol data | `std::shared_ptr<const T>` cache where sharing is genuine | Never used for graph links or to avoid designing ownership |

Additional rules:

- Application code does not use owning raw pointers. No code calls `delete`; top-level
  objects use `std::make_unique` and QObject children are owned by their parent. Any raw
  `new` required by a Qt parented-child API is confined to a small UI factory that
  immediately establishes parent ownership; it is never stored as an owning raw pointer.
- Model objects are held by value. Copying a document copies its values; it does not copy
  pointers into another document.
- A function may take a non-owning `const T&` or `T*` for the duration of a call, but no
  object stores such a pointer or reference across a mutation. Resolve an element again
  from its ID after any command that can change the model.
- `std::shared_ptr` is reserved for genuinely shared immutable resources, such as cached
  parsed symbol artwork. It is not used for parent/child relations, graph edges, undo
  snapshots or "someone else may need it" convenience.
- Undo snapshots are values. They do not share mutable model state with the live document.

### 5.2 Strong typed IDs

Every persistent entity has a strong ID. IDs are not interchangeable integers.

```cpp
template <typename Tag>
class StrongId {
public:
  using Value = std::uint32_t;

  constexpr StrongId() = default;
  constexpr explicit StrongId(Value value) : value_(value) {}

  constexpr Value value() const noexcept { return value_; }

  friend constexpr bool operator==(StrongId lhs, StrongId rhs) = default;
  friend constexpr auto operator<=>(StrongId lhs, StrongId rhs) = default;

private:
  Value value_ = 0;
};

using ComponentId = StrongId<struct ComponentTag>;
using PinId = StrongId<struct PinTag>;
using WireId = StrongId<struct WireTag>;
using SheetId = StrongId<struct SheetTag>;
using AnnotationId = StrongId<struct AnnotationTag>;
```

The exact underlying width may change, but the following properties do not:

- Value `0` is invalid and is rejected by the loader.
- There is no implicit conversion to an integer and no arithmetic on IDs.
- `operator<` provides the deterministic order used by maps, vectors and tie-breakers.
- `std::hash` may be specialized for lookup. It is never used to order output.
- IDs are allocated per file, sequentially. Deleted IDs are never reused.
- `.dcj` stores IDs as prefixed strings (`c1`, `p1`, `w1`, `s1`, `a1`). Parsing and
  formatting live in `io`; `core` never sees the string form.
- An ID is a non-owning reference. Storing an ID does not keep an entity alive. Lookups
  return an optional value/reference or a `Result`, not a silently dereferenced pointer.

The undo stack keeps the ID high-water mark outside the document snapshot. Undoing a
creation removes the entity, and redoing it restores the same ID; the allocator still never
hands that ID to a different object.

## 6. Error handling

Expected failures are values, not exceptions. `core` defines:

```cpp
enum class ErrorCode {
  InvalidArgument,
  NotFound,
  ParseError,
  ValidationError,
  UnsupportedVersion,
  IoError,
  Internal,
};

struct Error {
  ErrorCode code;
  std::string message;
  std::string context;
};

template <typename T>
class [[nodiscard]] Result {
  // hasValue(), value(), error(), and a Result<void> specialization
};
```

Rules:

- Fallible APIs return `Result<T>`. `Result` is `[[nodiscard]]`; callers must handle both
  branches.
- `Error::message` is an English developer diagnostic. `Error::context` identifies the
  object, file or operation, for example `"wire w7"` or the path being loaded. Neither is a
  translated UI string.
- The UI maps stable error codes to translated text where possible and appends the context
  for diagnosis. Core, sim and io never call `tr()`.
- No exception crosses a layer boundary. `core`, `sim` and `io` do not throw for expected
  conditions such as invalid input, malformed JSON, unsupported versions or missing IDs.
- `io` catches nlohmann/json exceptions and converts them to `Result` with a parse error.
  A JSON exception must never escape the loader.
- Qt slots catch unexpected exceptions only at the application boundary, report a fatal
  error and return to the event loop. Expected validation and I/O failures still use
  `Result`.
- Programmer contract violations (for example, dereferencing an invalid iterator) are
  assertions in debug builds. They are not modelled as recoverable `ErrorCode` values.
- Destructors and move operations are `noexcept`. No destructor throws.
- Lookup by ID returns `std::optional` when absence is normal, and `Result` when absence is
  an error for the operation.
- Validation returns a collection of problems with stable codes; it does not throw and does
  not open dialogs.

## 7. Determinism

Simulation results and persisted output must be reproducible. The engine is a deterministic
state machine.

| Concern | Rule |
|---|---|
| Containers | Use `std::map` keyed by strong ID or a sorted `std::vector`. Do not iterate an unordered container to produce output or drive evaluation order. |
| Hash containers | `std::unordered_map` / `QHash` may be used for lookup only. Sort keys before any iteration that affects behavior, logs or files. |
| Tie-breakers | Use IDs, tags and stable indexes. Never use pointer addresses, hash values or container insertion order. The documented `ComponentId` order is the only creation-order effect. |
| Coil order | Ascending `ComponentId`, as defined in `simulation-semantics.md`. Sheet order is not consulted for electrical results. |
| Source groups | Allocated in sorted source-pin order, not in scene or pointer order. |
| Time | Simulated time is integer milliseconds derived from the scan index and `cycle_ms`. The engine does not read the wall clock. |
| Randomness | None in the engine. No random seeds, no randomized tie-breaks. |
| Locale | JSON and file output are locale-independent. UI number formatting uses `QLocale` only at the display edge. |
| Floating point | Geometry may use floating point. Connectivity, timing and energization decisions do not depend on floating-point comparisons. |
| Threading | The worker is single-threaded. Host scheduling may delay scans; it must not change their logical sequence. |
| Input | Input levels are sampled at scan boundaries. The same per-scan input sequence produces the same state sequence. |
| Logs | Event order and event payloads are deterministic. No wall-clock timestamps. |
| Tests | Simulation tests run a fixed number of scans with an explicit `cycle_ms` and compare complete state/log sequences. |

`docs/simulation-semantics.md` defines the exact scan order, fixed-point bound and
first-wins rule. Any change to those rules requires updating that document and the
corresponding golden tests.

## 8. Canvas and undo (planned)

The canvas is a projection of the document, not an independent source of truth. The planned
design is:

- `DiagramDocument` holds sheets, components, pins, wires and annotations as values.
- The canvas maps strong IDs to view items. A view item may cache geometry for painting, but
  it does not own domain state. Moving or editing an item ultimately changes the document.
- The editor mutates the document only through commands. A command validates its
  preconditions, applies the change and reports a `Result`.
- Undo is snapshot-based. At a command boundary, the editor captures an immutable snapshot
  of the affected document state and pushes it with a human-readable command label. Undo
  restores a snapshot; redo restores the newer one. No inverse-operation logic is required
  for each command type.

### 8.1 Snapshot boundaries

- A drag or wire-rubber-band operation paints a preview and commits one snapshot on release,
  not one snapshot per mouse-move event.
- A property edit commits when editing finishes, not on every keystroke. Consecutive edits
  of the same field may be coalesced.
- Selection, hover state, zoom, scroll position and panel layout are UI state and are not
  part of the document snapshot.
- Simulation state, simulation logs and the running session are not part of the undo stack.
- Structural edits while a simulation session is active are blocked by the planned policy
  in section 4.1. After a session stops, undo/redo changes the document and invalidates any
  stored simulation state.

### 8.2 Memory and history

The first implementation may snapshot the whole document for simplicity. A later
optimization may snapshot only the affected sheet, or use copy-on-write for unchanged
sheets, without changing the command API. The history is bounded by a configurable limit.
Loading a file or creating a new document clears the history.

The ID allocator high-water mark is not rolled back. This preserves the format rule that an
ID is never reused for a different entity, including across undo and redo.

The canvas rebuilds or refreshes items from the restored document by ID. It may preserve
the current selection and camera when the entities still exist. It must tolerate entities
that disappeared during undo.

Snapshot-based undo is **planned**; this document does not claim it is already implemented.
The current design intent is recorded here so that commands, the document model and the
canvas are built with snapshotting in mind.

## 9. Translations

The application source language is English. The default UI language is Brazilian Portuguese
(`pt_BR`); English is also shipped.

### 9.1 Source strings

- Every user-visible string is wrapped in `tr()` inside a QObject subclass, or in
  `QCoreApplication::translate("Context", "text")` in a non-QObject helper.
- Source strings are complete English sentences or phrases. Do not build a sentence by
  concatenating translated fragments.
- Use `%1`, `%2`, ... placeholders for values and `%n` with the plural form for counts.
- Add a translator comment for ambiguous words. For example, "sheet" in the drawing sense is
  not a spreadsheet.
- Do not translate identifiers, file extensions, log codes, JSON keys, `label_group` values
  or command-line options.
- Do not put translated text in SVG artwork, symbol manifests or `.dcj` files.

### 9.2 Catalogs and loading

Translation sources live in `src/i18n/`:

```text
src/i18n/fiovivo_pt_BR.ts
src/i18n/fiovivo_en.ts
```

The planned i18n build uses Qt LinguistTools (`lupdate`, `lrelease`) to compile `.ts` files
into `.qm` catalogs and embeds them in the application resources under `:/i18n/`. The current
top-level CMake already locates LinguistTools optionally; release packaging must include the
compiled catalogs.

At startup, before the main window is constructed, the planned loader performs these steps:

1. Read the saved language preference.
2. If there is no preference, select `pt_BR`.
3. Load `:/i18n/fiovivo_<language>.qm` with `QTranslator`.
4. Install the translator on `QCoreApplication` and keep it alive for the application
   lifetime.
5. If the catalog cannot be loaded, fall back to the English source strings and report the
   problem in the diagnostic log, not in a modal dialog before the UI exists.

A language change installs the new translator, removes the old one and sends
`QEvent::LanguageChange`. Windows call `retranslateUi()` and refresh visible text without
requiring a restart. This dynamic switch is **planned** with the settings UI.

Qt's own strings (standard dialogs, line edits, file dialogs) are localized by installing
the matching Qt translation catalog from `QLibraryInfo::path(QLibraryInfo::TranslationsPath)`
when available.

### 9.3 Locale-sensitive data

- Display numbers, dates and units through `QLocale`.
- `.dcj` JSON always uses a locale-independent format with `.` as the decimal separator.
- Simulation time and timer presets are integer milliseconds internally. The UI may display
  seconds according to the user's locale.
- Sorting for display uses a documented collation; sorting for simulation and file output
  uses IDs and byte order.

A translation-completeness check is **planned**: CI will fail when a `tr()` string has no
`pt_BR` translation, excluding strings explicitly marked as source-only.

## 10. Extension points for voltage modelling

The current engine models the presence or absence of energy, not voltage. Source groups are
identities, not voltage levels. A 24 V coil and a 220 V coil behave identically if each sees
two distinct source groups. This is deliberate and is stated in
`simulation-semantics.md`.

The architecture keeps the following seams so that voltage can be added later without
rewriting the editor:

| Seam | Current contract | Future extension |
|---|---|---|
| Supply metadata | Each supply terminal creates a deterministic source group; optional labels identify L1, L2, L3, N or PE | Add nominal voltage, phase angle and frequency as optional metadata on the source/group; keep group identity stable |
| Network solver | Connectivity is computed by union-find over wires and closed pass-through elements; loads are evaluated by distinct source groups | Replace or extend the energy evaluator with a nodal/potential solver. Device pairing, timers and the scan loop remain separate |
| Load data | Loads carry `rated_voltage`, `frequency` and related parameters; the current solver ignores them | Use the ratings to decide whether a load can operate at the computed potential difference |
| Motor data | `voltage`, `power`, `connection` and `min_live_phases` are metadata; energization uses distinct groups | Compute phase voltages, current and starting behavior in a separately scoped model |
| Observer state | State exposes per-component energization, contact position, timer state and motor running flag | Add optional voltage/current fields with a versioned state contract; the UI renders fields it understands |
| File format | Format version 1; optional fields are preserved where the schema allows | Add fields through a new format version and a migration; old files remain loadable |
| Symbols | SVG geometry and manifest pins; no electrical values are baked into artwork | Add manifest parameters for voltage/phase labels without changing geometry |

Rules for any voltage extension:

- Do not add voltage, current or protection behavior by implication. A change to the
  semantics requires an update to `docs/plan.md`, `docs/simulation-semantics.md`, the schema
  and the tests.
- Keep source-group identity deterministic. A voltage solver may compute potentials, but it
  must not introduce hash-order or pointer-order dependence.
- Keep the solver behind a narrow sim-internal boundary. The UI must continue to consume
  state values, not reach into the solver.
- Preserve the educational disclaimer. A voltage model is still not a certification,
  protection-coordination or safety tool.
- Use a format migration for new persisted fields; never silently reinterpret version 1
  data.

## 11. Enforcement and review

The architecture is enforced by build configuration and review:

- `fiovivo_core`, `fiovivo_sim`, `fiovivo_io` and `fiovivo_symbols` are built and tested with
  `FIOVIVO_BUILD_UI=OFF`. Their tests must pass without a display server.
- CMake target links follow the dependency graph. A lower layer cannot link a higher one.
- Compiler warnings are enabled for all targets. Debug presets treat warnings as errors.
- `.clang-format` and `.clang-tidy` define formatting and naming; see
  [`contributing.md`](contributing.md).
- Review checks for Qt includes in headless layers, exceptions crossing boundaries,
  `shared_ptr` misuse, raw owning pointers, unordered iteration in deterministic paths and
  direct widget access from the worker.
- A planned CI check searches the headless layers for Qt includes and for forbidden
  dependencies.

## 12. Related documents

- [`plan.md`](plan.md): scope, decisions, conflict resolutions and open questions.
- [`simulation-semantics.md`](simulation-semantics.md): exact scan cycle, device behavior,
  determinism, snapshots and limitations.
- [`glossary.md`](glossary.md): Portuguese/English terminology.
- [`contributing.md`](contributing.md): coding standards, tests, commits, migrations and
  symbol workflow.
