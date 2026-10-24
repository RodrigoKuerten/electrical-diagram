# Contributing

FioVivo is an educational electrical control diagram editor and simulator. Contributions
should improve the editor, the simulator or the documentation without turning the project
into a certification, protection-coordination or safety tool.

Read these first:

- [`plan.md`](plan.md): scope, decisions and open questions.
- [`architecture.md`](architecture.md): layers, ownership, threading and determinism.
- [`simulation-semantics.md`](simulation-semantics.md): scan-cycle behavior and limitations.
- [`glossary.md`](glossary.md): Portuguese/English terminology.

All documentation, source identifiers, comments, log keys and commit messages are in English.
User-facing text is translated. Brazilian Portuguese is the default UI language.

## 1. Before you start

- Check that the change belongs to the current phase or is an explicitly approved
  improvement. `docs/plan.md` is the decision log; record a new architectural or electrical
  decision there before implementing it.
- Keep the change small enough to review. Split refactors from behavior changes.
- Do not add a feature that implies voltage, current, protection coordination, heating,
  mechanical motion or safety analysis unless the plan and
  `docs/simulation-semantics.md` are updated first.
- Preserve the educational disclaimer. Do not describe the application as certified,
  compliant, safe or suitable for validating a real installation.
- Do not copy artwork, symbol libraries or text from a standards document or another
  application. Symbol artwork in this repository is original.

## 2. Language and naming

Code identifiers, file names, comments, commit messages, branch names, JSON keys, log codes
and documentation are English. Portuguese is used only in translated UI strings, in the
glossary and in quoted domain or UI terms.

The project uses a consistent C++ naming scheme, enforced by `.clang-tidy` where possible.

| Item | Convention | Example |
|---|---|---|
| Namespace | `lower_case` | `fiovivo::sim` |
| Class, struct, enum | `CamelCase` | `SimulationEngine`, `ErrorCode` |
| Enum constant | `CamelCase` | `ErrorCode::ParseError` |
| Function, method | `camelBack` | `loadDiagram`, `onStateChanged` |
| Private data member | `lower_case_` | `cycle_ms_` |
| Local variable, parameter | `lower_case` | `source_group`, `sheet_id` |
| Local constant | `lower_case` | `max_passes` |
| `constexpr` / global constant | `k` + `CamelCase` | `kMaxPasses`, `kDefaultCycleMs` |
| File | `snake_case` | `simulation_engine.cpp` |
| Header | `snake_case.hpp` | `simulation_engine.hpp` |
| Test file | `snake_case_test.cpp` | `engine_test.cpp` |
| Test suite / test name | `CamelCase` | `TEST(Contactor, HoldsAfterStartRelease)` |

Additional rules:

- Use ASCII file names. Avoid accents, spaces and platform-specific characters in paths.
- Use `id`, `svg`, `ui`, `io`, `json`, `cmake` as lowercase abbreviations. Do not invent
  Hungarian prefixes such as `pComponent` or `m_wire`.
- Prefer full words over unclear abbreviations. `component`, not `cmp`; `temperature`, not
  `tmp`, unless the short form is the domain term.
- Boolean names read as predicates: `is_energized`, `has_value`, `can_start`.
- Names of persisted fields match the `.dcj` schema. Do not translate JSON keys.
- Comments explain why a decision was made, not what the next line does. Keep them in
  English and update them when the code changes.
- UI strings use `tr()`. Do not concatenate translated fragments. Use `%1` placeholders and
  `%n` for plurals. Add a translator comment for ambiguous terms.
- Do not put translated text in core, sim, io or symbol manifest data.

## 3. C++20 and ownership

The project is C++20, built with warnings enabled. Write code that is simple, value-oriented
and deterministic.

### 3.1 Resource management

- Follow RAII. Every resource has an owner with a destructor.
- Use `std::unique_ptr` for top-level objects and long-lived services.
- Store model objects by value in `std::map` keyed by strong ID.
- Use Qt parent-child ownership for QObject children. Do not call `delete`; use
  `deleteLater()` only for a parentless QObject that cannot be owned otherwise.
- No owning raw pointers. Any raw `new` required by a Qt parented-child API belongs in a
  small UI factory that immediately establishes the parent; it is never stored as an owning
  pointer.
- `std::shared_ptr` is only for genuinely shared immutable resources, such as cached parsed
  symbol artwork. Do not use it for graph links, documents, undo snapshots or to avoid
  deciding which object owns something.
- Do not store pointers or references to model elements across mutations. Store strong IDs
  and resolve them again when needed.
- Prefer `const`, `constexpr`, `[[nodiscard]]` and `noexcept` where they express the contract.
- Avoid static mutable state and singletons. Pass dependencies explicitly.

### 3.2 Errors and boundaries

- Expected failures return `fiovivo::Result<T>`; they do not throw.
- `Result<T>` is `[[nodiscard]]`. Check both success and error paths.
- Use `std::optional` when absence is a normal query result. Use `Result` when absence is an
  error for the operation.
- No exception crosses a layer boundary. `core`, `sim` and `io` do not throw for invalid
  input, malformed JSON, unsupported versions or missing IDs.
- Catch nlohmann/json exceptions inside `io` and convert them to `ParseError`.
- Catch unexpected exceptions only at the Qt application boundary, log them and return to the
  event loop.
- Error messages are English developer diagnostics. UI text is translated at the edge from a
  stable error code.
- Do not use exceptions for control flow, including validation.

### 3.3 Determinism

- Use `std::map` or a sorted `std::vector` for anything that affects state, logs or files.
- Do not iterate a hash container to produce output or drive evaluation order.
- Never use pointer addresses, memory-address order or hash values as a tie-breaker. The
  documented `ComponentId` order is stable per file.
- Use integer milliseconds for simulated time. Do not read the wall clock in the engine.
- Do not use random values in the engine.
- Do not make connectivity or energization decisions with floating-point comparisons.
- Apply user inputs at scan boundaries. Do not let thread scheduling change the logical scan
  sequence.
- Simulation tests run a fixed number of scans with an explicit `cycle_ms` and compare
  complete state/log sequences.

### 3.4 Qt and threading

- Qt Widgets code lives only in `ui` and `app`. `core`, `sim`, `io` and `fiovivo_symbols`
  contain no Qt types or includes.
- The simulation engine is plain C++. It is tested synchronously without a `QApplication`.
- The worker thread owns the engine and its model snapshot. It never accesses widgets,
  pixmaps, SVG renderers, settings, dialogs, the undo stack or the canvas.
- Commands to the worker use queued `QMetaObject::invokeMethod`; state and events come back
  through queued signals.
- Register non-QObject value types used in queued connections as Qt metatypes.
- Do not block the UI thread waiting for the worker. Do not call `QThread::terminate()`.
- Shut down by stopping the engine, quitting the thread event loop and waiting for the thread.
- Structural document edits are blocked while a simulation session is active, as described
  in `architecture.md`. Do not bypass that policy with direct model access.

## 4. Formatting and static analysis

Formatting is defined by `.clang-format`:

- LLVM base style, C++20.
- 2-space indentation, no tabs.
- 100-column limit.
- Attached braces.
- Pointers and references aligned left (`Type* name`, `Type& name`).
- Includes regrouped; first-party includes (`"core/..."`, `"sim/..."`, `"io/..."`,
  `"library/..."`, `"ui/..."`, `"app/..."`) come before Qt, GoogleTest, standard library and
  local includes.
- Namespace comments are fixed.

Naming and selected static checks are defined by `.clang-tidy`. The configuration enables
`bugprone-*`, `cert-*`, `clang-analyzer-*`, `cppcoreguidelines-*`, `misc-*`, `modernize-*`,
`performance-*` and `readability-*`, with a small list of deliberate exclusions.

Before opening a pull request:

```bash
clang-format -i path/to/changed_file.cpp path/to/changed_file.hpp
clang-format --dry-run --Werror path/to/changed_file.cpp path/to/changed_file.hpp
```

Run `clang-tidy` with the compilation database:

```bash
clang-tidy -p build/windows-msys2-debug src/core/model/diagram.cpp
```

On MSVC, `CMAKE_EXPORT_COMPILE_COMMANDS` may not produce a usable database. Use the
`windows-msys2-debug` or `linux-debug` preset for static analysis, or generate a Ninja
compile database separately. Do not commit `compile_commands.json`.

Do not add `// NOLINT` to silence a warning without a comment explaining why the check is
wrong for that line. Prefer fixing the code. New warnings are treated as failures even when
the local build does not set `FIOVIVO_WARNINGS_AS_ERRORS`.

### 4.1 Build and test commands

Use a preset rather than hand-written CMake flags.

Windows (MSYS2 UCRT64):

```bash
cmake --preset windows-msys2-debug
cmake --build --preset windows-msys2-debug
ctest --preset windows-msys2-debug --output-on-failure
```

Linux:

```bash
cmake --preset linux-debug
cmake --build --preset linux-debug
ctest --preset linux-debug --output-on-failure
```

Sanitizers (Linux, headless core/sim):

```bash
cmake --preset asan
cmake --build --preset asan
ctest --preset asan --output-on-failure
```

The `asan` preset disables the UI. UI tests are built only when Qt is available. The headless
libraries must always build and test with `FIOVIVO_BUILD_UI=OFF`.

## 5. Test-first expectation

New behavior starts with a test that fails for the right reason.

1. Write or update a test that expresses the behavior.
2. Run it and confirm that it fails because the behavior is missing, not because of a build
   error.
3. Implement the smallest change that makes it pass.
4. Run the surrounding suite.
5. Refactor with the test still passing.

Rules:

- Every bug fix includes a regression test.
- Simulation tests use fixed `cycle_ms`, explicit input sequences and a fixed number of
  scans. Do not use `sleep()` or wall-clock timing.
- Test the scan semantics, not implementation details: nets, load energization, first-wins,
  timer outputs, contact latency, latches, logs and snapshots.
- `core`, `sim`, `io` and `fiovivo_symbols` tests are headless and use GoogleTest. They must
  not need a display server or a `QApplication`.
- UI tests use Qt's test facilities and, where possible, the offscreen platform
  (`QT_QPA_PLATFORM=offscreen`). Prefer checking model/view state over pixel-perfect
  screenshots.
- IO tests use temporary directories and committed fixtures. Do not depend on the current
  working directory.
- Determinism tests should run the same scenario twice and compare the full state and log
  sequence.
- A flaky test is a defect. Fix the source of nondeterminism or quarantine the test with an
  issue; do not retry it in CI.
- Do not weaken assertions, skip tests or update golden files just to make a red build green.

## 6. Commits and branches

Commits follow Conventional Commits.

Format:

```text
<type>(<scope>): <imperative summary>

<optional body explaining why>

<optional footer with issue references>
```

Common types:

| Type | Use |
|---|---|
| `feat` | New user-visible or domain behavior |
| `fix` | Bug fix |
| `test` | Tests only |
| `docs` | Documentation only |
| `build` | CMake, dependencies or packaging |
| `ci` | Continuous integration |
| `refactor` | Behavior-preserving restructuring |
| `perf` | Performance improvement |
| `style` | Formatting or whitespace only |
| `chore` | Maintenance that fits no other type |

Scopes are optional and usually match a layer or area: `core`, `sim`, `io`, `library`, `ui`,
`app`, `i18n`, `cmake`, `docs`, `tests`.

Examples:

```text
feat(sim): add TON and TOF timing
fix(io): preserve unknown fields on round-trip
test(core): cover duplicate pin detection
docs: document source group energization
build(cmake): add Qt Svg to the library target
ci: run the headless sanitizer preset
refactor(ui): move canvas selection into its own class
```

Commit rules:

- Use the imperative mood in the subject: "add", not "added" or "adds".
- Keep the subject at or below 72 characters. Do not end it with a period.
- Explain why in the body when the change is not obvious. Do not narrate the diff.
- One logical change per commit. Separate formatting-only commits from behavior changes.
- Reference the issue or plan item when one exists.
- Do not commit build output, `.qm` files, `compile_commands.json`, IDE metadata, screenshots
  of local debugging or large binary fixtures without review.
- Phase ends are tagged `v0.1.0-phaseN` by a maintainer, following `docs/plan.md`.

Branch names:

```text
feat/sim-ton-timer
fix/io-unknown-fields
docs/architecture
test/core-strong-ids
chore/ci-sanitizers
```

Keep branches focused. Rebase or merge according to the maintainer's current workflow; do not
rewrite a shared branch.

## 7. Pull request checklist

Copy this checklist into the pull request description and fill it in.

- [ ] The change is in scope for the current phase or was discussed and recorded in
      `docs/plan.md`.
- [ ] The change is small enough to review, or the description explains why it is not.
- [ ] Tests were written first and fail without the change.
- [ ] `ctest` passes locally for the relevant preset.
- [ ] New behavior has tests; bug fixes have regression tests.
- [ ] `clang-format` and `clang-tidy` are clean for changed files.
- [ ] No Qt include or Qt type leaked into `core`, `sim`, `io` or `fiovivo_symbols`.
- [ ] Ownership follows the rules: no raw `new`/`delete` in application or model code, no
      owning raw pointers, no unnecessary `shared_ptr`, no stored pointers into the model.
- [ ] Expected failures use `Result`; no exception crosses a layer boundary.
- [ ] Deterministic ordering is preserved; no pointer/hash order, wall clock or randomness in
      the engine.
- [ ] The simulation worker does not touch widgets or the live document.
- [ ] User-facing strings use `tr()`; `pt_BR` and `en` catalogs are updated where needed.
- [ ] The architecture, simulation semantics, glossary or plan were updated if behavior or
      terminology changed.
- [ ] No safety, certification or protection-coordination claim was added.
- [ ] UI changes include a short description or screenshot, and were checked at 100% and
      200% scaling where relevant.
- [ ] No generated files, secrets, large binaries or unrelated formatting changes are
      included.
- [ ] CI is green.

## 8. Golden-file policy

Golden files protect deterministic output: `.dcj` serialization, BOM CSV, SVG export,
validation reports and simulation logs. They are compared byte-for-byte unless a test
explicitly documents a normalized comparison.

Policy:

- A normal test run never rewrites a golden file. If a golden test fails, the test reports a
  diff and exits.
- Golden updates are a deliberate, separate action. The harness will expose a dedicated
  explicit helper command, never automatically by CI. CI and the normal `ctest` invocation
  use comparison mode only. The BOM golden is generated with:

  ```bash
  python tools/make_bom_golden.py
  git diff tests/data/golden/
  ```

- Run the update command only after deciding that the new output is correct. Review the diff
  before committing.
- Include the update command and the reason in the pull request. Commit the code change and
  the golden change in the same pull request; a separate commit for the regenerated files is
  preferred.
- Never hand-edit a generated golden to match a broken serializer. Fix the serializer or the
  expected behavior first.
- Keep goldens small and focused. Do not embed absolute paths, timestamps, host names,
  locale-dependent numbers or random IDs. Use fixed fixtures and normalized line endings.
- Version golden directories when the format changes. Keep the old fixtures for migration
  tests.
- If a golden change is unintentional, treat it as a regression even if the new output looks
  plausible.

Until the update target exists, maintainers update goldens with an explicit command recorded
in the pull request, not by changing test code to write files on failure.

## 9. Adding a format migration

Only `.dcj` format version 1 exists today. The loader already dispatches migrations by
version so that a future version can be added without changing the loading contract.

To add version N+1:

1. Record the decision and compatibility impact in `docs/plan.md`.
2. Bump the format version in the schema, the writer and the version constants. Update
   `docs/file-format.schema.json` and any format documentation.
3. Add a pure migration function in `io`, for example `migrateV1ToV2`, returning
   `Result<Json>` or the project's migration value type. It must not use Qt, sim or the UI.
4. Register the migration in the version dispatch chain. Loading an older file runs each
   migration in order until the current version is reached.
5. Preserve IDs, unknown fields and data that the new version does not understand. Fill new
   fields with documented defaults. Never reuse a deleted ID.
6. Validate the migrated document against the new schema and then against referential
   integrity rules. Report errors with the file path and object context.
7. Add fixtures: a version N input, the expected version N+1 output, a round-trip test and an
   invalid migration case. Add or update golden files.
8. Keep the old version loadable. Never edit a released migration in place; add a new
   migration for the next version.
9. Run the headless IO suite and the integration examples.

Migration code must be deterministic and locale-independent. It may depend only on `core`
and nlohmann/json. If a migration needs another dependency or electrical behavior, stop and
discuss the design before implementing it.

## 10. Adding a symbol

Symbols are original SVG artwork plus a JSON manifest. The manifest code is Qt-free; only the
renderer in `fiovivo_library` uses Qt Svg.

A typical addition follows this shape (the exact paths are established by the symbol
library):

```text
assets/symbols/<symbol_name>/
  <symbol_name>.svg
  <symbol_name>_active.svg      # optional active state
  manifest.json
```

Steps:

1. Draw the symbol from scratch. Do not trace or copy a standards document, a manufacturer
   library or another application's artwork.
2. Define the manifest: stable symbol type, display name, pins with numbers and positions,
   connection direction, default parameters, `label_group` behavior and optional state SVG
   files. The SVG is parsed once when the library is loaded, never during a simulation cycle.
3. Keep the SVG simple and self-contained. No scripts, external references, embedded raster
   images or translated text baked into the artwork. Text that must be localized belongs in
   the UI, keyed by a stable manifest identifier; manifests contain identifiers, not
   translated sentences.
4. Use the manifest to express dynamic states. A state that changes geometry swaps to a
   preloaded active-state SVG, as described in `docs/plan.md`. Coils, lamps and motors may
   also receive a colored state overlay.
5. Put manifest and built-in definitions in `fiovivo_symbols`, with no Qt include. Put
   rendering code in `fiovivo_library`, which may use Qt Gui/Svg but not Widgets.
6. Add tests: manifest schema validation, unique pin numbers, SVG parse success, state file
   existence and a golden render at documented sizes and zoom levels.
7. Add the symbol to the catalog and, if useful, to an example project. Update the glossary
   when the symbol introduces a new Portuguese/English term.
8. Do not claim that the symbol is a certified reproduction of NBR, IEC or any other standard.
   The project uses conventions inspired by common practice.

If a symbol needs new simulation behavior, update `docs/simulation-semantics.md` and its tests
first. A new drawing alone must not silently change energization semantics.

## 11. Review checklist

Reviewers check the following, in addition to the pull request checklist:

### Architecture and ownership

- Layer boundaries are respected. No lower layer includes a higher layer.
- `core`, `sim`, `io` and `fiovivo_symbols` are free of Qt.
- `sim` depends only on `core`; `io` depends only on `core` and nlohmann/json.
- The UI does not implement electrical rules that belong in `sim`.
- Ownership is clear. No owning raw pointer, no hidden `shared_ptr`, no stored model pointer.
- Strong IDs are used for persistent links. `0` and invalid IDs are rejected.
- The model remains a value graph; no pointer graph was introduced.

### Errors and determinism

- Fallible APIs return `Result` and are checked.
- No exception crosses a layer boundary.
- Error codes are stable; messages are English developer diagnostics.
- Iteration order is deterministic.
- No wall clock, random value, locale dependency or pointer-order tie-break was added to the
  engine.
- Simulation changes have tests with fixed cycles and complete state/log comparisons.
- The 16-pass bound and first-wins rule are unchanged unless the semantics document was
  updated deliberately.

### Threading and UI

- The worker does not touch widgets, pixmaps, SVG renderers, settings or the live document.
- Commands and state use queued connections.
- Shutdown waits for the thread; no `terminate()`.
- Structural edits cannot race with a running simulation.
- UI strings are in `tr()`, with placeholders and translator comments where needed.
- Portuguese and English catalogs are updated together.
- The UI displays state from the engine rather than recomputing it.

### Tests and documentation

- Tests fail without the change and pass with it.
- Regression tests cover the reported bug.
- Golden files are updated only through the deliberate update path and are reviewed.
- Migrations preserve IDs and unknown fields, and old fixtures still load.
- New symbols have manifest and render tests, and their artwork is original.
- Documentation matches the behavior and marks planned features as planned.
- No claim of certification, compliance or real-installation safety was introduced.

## 12. Safety and scope boundaries

FioVivo is a teaching and prototyping tool. It models the logical presence or absence of
energy in a command diagram. It is not a certified electrical design, protection
coordination or safety tool.

Contributions must not:

- imply that a simulated result is a real installation verification, commissioning or safety
  assessment;
- add voltage, current, heating, mechanical or protection behavior without an explicit plan
  change and a documented model;
- present NBR 5280, IEC 60617 or any other standard as "implemented" or "compliant";
- describe the application as "safe", "certified", "approved" or "validated" for real work;
- hide the limitations listed in `docs/simulation-semantics.md`;
- remove or weaken the safety disclaimer in the README, the About dialog or the
  documentation.

When in doubt, prefer the smaller educational model with a clear limitation over a
convincing-looking simulation that is not physically justified.

## 13. Documentation changes

- Keep documentation in English. The glossary is the only bilingual reference; quoted UI
  labels may be Portuguese.
- Use present tense for architecture rules and behavior that exists. Mark planned features
  explicitly.
- Update `docs/architecture.md` for layer, ownership, threading or extension-point changes.
- Update `docs/simulation-semantics.md` for any change to scan order, energization, timers,
  latches, logs, snapshots or limitations.
- Update `docs/plan.md` for decisions and scope changes. `README.md` is the project entry
  point; propose changes there when the public status or build instructions change.
- Keep tables and examples concrete. Avoid marketing language and emojis.
- If a code change makes a document wrong, fix the document in the same pull request.

## 14. Definition of done

A change is done when:

- the behavior is implemented and covered by tests;
- the relevant preset builds without new warnings;
- formatting and static analysis are clean;
- architecture and simulation documentation match the behavior;
- translations are updated for user-visible text;
- the pull request checklist and review checklist are satisfied;
- the change remains within the educational scope and does not imply certification.
