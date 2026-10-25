# FioVivo

> **Working title.** FioVivo is a provisional project name. The application identifier,
> namespace and file extension are kept in a small number of places so the name can be
> changed without touching the domain model.

FioVivo is an educational electrical control diagram editor and simulator written in C++20
with Qt 6 Widgets. It targets Brazilian NBR 5280-style command diagrams. The current symbol
library and electrical conventions were rebuilt from the shipped **CADe_SIMU 4.0** application:
the library geometry, terminal numbering and state language were recovered with REA and Ghidra
and reconstructed as original SVG assets. The project is not a copy of CADe_SIMU and is not a
certified reproduction of IEC 60617.

The application is a **prototyping and teaching tool**. It is **not** a certified electrical
design, protection coordination or safety tool. It does not simulate voltage, current,
heating, mechanical motion or electrical safety. Never use it to validate a real installation.

## Status

The implementation covers the editor, the scan-cycle simulator and most of the persistence
and export scope. The table below is honest about the remaining gaps.

| Phase | Scope | Status |
|---|---|---|
| 1 | Editor core | implemented |
| 2 | Scan-cycle simulator | implemented |
| 3 | Persistence, export and printing | implemented, with the gaps listed below |
| 4 | Extras | partial: multi-sheet navigation, settings; light/dark theme completed in Track A |

See [`docs/report.md`](docs/report.md) for the final delivery report and
[`docs/plan.md`](docs/plan.md) for the decision log.

## Features

- Multi-sheet diagram model with strong component, pin, wire, sheet and annotation IDs.
- **215 CADe_SIMU-style component types** in 18 categories, with SVG artwork, JSON pin
  manifests, terminal labels and state variants. The 20 original FioVivo types remain
  loadable for existing projects and examples.
- Multi-pole contactors and breakers, group-controlled relay/contactor contacts, timed and
  thermal auxiliary contacts, selector positions, sensors, motors, logic gates, PLC blocks,
  pneumatic symbols, terminals and measurement/GRAFCET symbols.
- Grid snapping, orthogonal auto-routing, wiring drag, zoom, pan, selection, rotation,
  copy/paste, delete, duplicate and snapshot undo/redo.
- **Independent workspace dimensions** in addition to zoom: the sheet can grow beyond the
  printed page, persisted in the project file, with menu/toolbar controls and scrollbars.
- **Zoom** from 25% to 400% with mouse-wheel anchoring under the cursor, keyboard/toolbar
  controls and fit-to-page.
- Properties panel with tag uniqueness checks, description, position, rotation, typed
  parameters, pin table and simulation status.
- Searchable symbol library with drag and drop.
- Validation rules and a clickable problems panel.
- Deterministic scan-cycle simulator in a worker thread:
  - contactor self-holding ("selo"), NC interlocks and first-wins simultaneous starts;
  - multi-pole contactors/breakers, relay and contactor groups, changeover contacts,
    timed and thermal auxiliary contacts;
  - TON/TOF and ON/OFF/pulse/flasher timers, emergency-stop latch, thermal relay,
    fuse/breaker, selector, lamp, buzzer/siren and three-phase motor;
  - digital sensors, switches and limit switches as canvas inputs;
  - basic logic gates (AND/OR/NOT/XOR/NAND/NOR), RS latch, edge detection, logic timer
    and counter driving switched outputs;
  - fixed 16-pass convergence guard with oscillation detection;
  - input interaction on the canvas, step/step-N, speed control and simulated time;
  - highlight of energized paths and active devices;
  - simulation log with CSV export.
- `.dcj` UTF-8 JSON project files:
  - versioned format, embedded schema validation, referential integrity checks and
    unknown-field preservation;
  - migration framework for future format versions, refusing newer files with a clear error;
  - autosave every two minutes and startup recovery.
- Export to PNG (1x/2x/4x, transparent option), SVG, PDF and BOM CSV; print and print preview.
- Brazilian Portuguese (default) and English UI through Qt Linguist (`tr()`, `.ts`, `.qm`).
- Palette-driven light and dark themes with runtime switching, adjustable font size,
  configurable grid, cycle period and default simulation speed. All canvas, grid, sheet,
  title-block, symbol, wire, dock and dialog colours come from named theme tokens.
- Examples for direct-on start, star-delta, timer sequence, two-motor interlock and
  emergency stop, exercised by the test suite.

## Known limitations and deliberate gaps

These are documented rather than hidden:

- Only presence/absence of energy is simulated. Voltage levels, current, power factor,
  impedance, phase angle, heating, mechanical motion and safety functions are not modelled.
- **CADe_SIMU artwork is reconstructed, not byte-identical.** The 215 symbols follow the
  recovered geometry, terminal numbering and visual language, but the original uses GDI
  primitives and ClearType text; FioVivo uses antialiased SVG/Qt text. See
  [`docs/cadesimu/report.md`](docs/cadesimu/report.md).
- **PLC, pneumatic, GRAFCET and measurement are visual/limited.** No LADDER/STL program is
  executed, no pressure/flow/cylinder position is simulated, there is no GRAFCET token engine,
  and meters do not calculate values.
- Protection is commanded, not calculated: a thermal relay trips by user action rather than a
  current/time overload integral; an RCD does not detect residual current; short-circuit and
  protection coordination are out of scope.
- Loads in series are not supported; each load must reach a supply and a return path.
- A load is energized when it sees two distinct source groups. A 24 V and a 220 V coil are
  indistinguishable in the current model.
- The star-delta example models the control logic and contactor interlocks; the optional
  six-terminal three-phase motor symbol is available, but its winding ends are not tied to the
  star/delta power contactors in the simulator.
- **Multiple wires per pin** are allowed (with a junction dot for three or more) because the
  original one-wire-per-pin draft makes parallel circuits and the required examples
  impossible. This is the main deviation from the original specification; see
  [`docs/plan.md`](docs/plan.md).
- Wire segments cannot be dragged after routing yet; re-route by deleting and redrawing.
- Mirroring (`M`) is not implemented; rotation (`R`, context menu and properties) is.
- Simulation snapshots exist in memory for tests but cannot yet be saved to/loaded from a
  snapshot file.
- Shortcut customization UI is not implemented; the standard shortcuts are fixed.
- The title block uses the active UI language rather than a separate selectable title-block
  language.
- The JSON Schema validator is a small in-tree subset covering every keyword used by the
  committed schema, not a general-purpose validator.
- Only format version 1 exists today. The migration framework and refusal of newer versions
  are tested; no historical version has been invented.
- Golden-file coverage is committed for BOM. SVG export is tested for structural validity
  and byte-identical repeated output instead of a cross-Qt-version golden file.
- The symbol editor (Phase 4) is not implemented.
- Autosave uses a single recovery slot in the user data directory. It never overwrites the
  main project.

## Build requirements

- CMake >= 3.21 and Ninja (recommended) or another generator.
- A C++20 compiler: GCC 11+, Clang 14+ or MSVC 19.30+.
- Qt 6.5+ with Widgets, Svg, PrintSupport and LinguistTools. Qt 6.8 LTS is the recommended
  LTS line; local development used Qt 6.12.
- GoogleTest and nlohmann/json. They are fetched automatically when not installed.
- Optional: Qt Test for the UI tests, Python only for the deliberate BOM golden-file helper.

### Windows (MSYS2 UCRT64, tested)

```powershell
winget install MSYS2.MSYS2
C:\msys64\usr\bin\bash -lc "pacman -S --needed mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-qt6-base mingw-w64-ucrt-x86_64-qt6-svg \
  mingw-w64-ucrt-x86_64-qt6-tools mingw-w64-ucrt-x86_64-qt6-declarative \
  mingw-w64-ucrt-x86_64-gtest mingw-w64-ucrt-x86_64-nlohmann-json"
```

Open an MSYS2 UCRT64 shell (`C:\msys64\ucrt64.exe`), then:

```bash
cmake --preset windows-msys2-debug
cmake --build --preset windows-msys2-debug
ctest --preset windows-msys2-debug
./build/windows-msys2-debug/bin/fiovivo.exe
```

Add `-DFIOVIVO_WARNINGS_AS_ERRORS=ON` (the preset already does this for Debug) to enforce
the zero-warning policy. The Release preset is `windows-msys2-release`.

### Windows (MSVC)

Install Visual Studio 2022 with the "Desktop development with C++" workload and Qt 6 with
the MSVC 2019/2022 x64 kit. From a Developer PowerShell:

```powershell
cmake --preset windows-debug -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/msvc2022_64
cmake --build --preset windows-debug
ctest --preset windows-debug
```

`windows-release` uses the same Visual Studio generator.

### Linux (Ubuntu/Debian)

```bash
sudo apt install build-essential cmake ninja-build qt6-base-dev qt6-svg-dev \
  qt6-tools-dev qt6-tools-dev-tools libgtest-dev nlohmann-json3-dev
cmake --preset linux-debug
cmake --build --preset linux-debug
ctest --preset linux-debug
```

The `linux-debug` and `linux-release` presets use Ninja. If Qt is installed through the Qt
online installer, pass `-DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64`.

### Headless and sanitizer builds

```bash
cmake -S . -B build/headless -G Ninja -DFIOVIVO_BUILD_UI=OFF -DFIOVIVO_BUILD_TESTS=ON
cmake --build build/headless
ctest --test-dir build/headless --output-on-failure

cmake --preset asan        # Linux ASan + UBSan, UI disabled
cmake --build --preset asan
ctest --preset asan
```

## Running and opening examples

Open a project with **File > Open**, drag a symbol from the left dock, and use the toolbar
to start the simulation. You can also launch a file directly:

```bash
fiovivo examples/partida-direta.dcj
```

Bundled examples:

| File | Content |
|---|---|
| `examples/partida-direta.dcj` | Direct-on start with self-holding, thermal relay and pilot lamps |
| `examples/partida-estrela-triangulo.dcj` | Star-delta control with timer, transition relay and cross interlocks |
| `examples/sequencia-temporizada.dcj` | TON and TOF lamp sequence |
| `examples/intertravamento-dois-motores.dcj` | Two-motor NC interlock |
| `examples/parada-emergencia.dcj` | Latching emergency stop and reset button |

All examples are regenerated deliberately with `fiovivo_make_examples` (built when
`FIOVIVO_BUILD_TOOLS=ON`, the default) and are validated by the integration tests.

## Tests

```bash
ctest --preset windows-msys2-debug --output-on-failure
```

The suite includes headless core/sim/io tests, the five example scenarios, determinism checks,
timer timing, UI canvas tests, theme palette/contrast/golden tests, export checks,
translation loading, a 500-component/800-wire performance test and a 500-component
rendering test. UI tests run with `QT_QPA_PLATFORM=offscreen`. Theme goldens are updated
only with `--update-golden`, and the light-theme investigation is recorded in
[`docs/theme-audit/report.md`](docs/theme-audit/report.md).

Current local results: **154/154 tests pass** in Debug with the UI enabled; in the local
Release build all tests pass except the 500-component rendering budget, which also exceeds
its limit on the pre-theme baseline at this Qt version (see
`docs/theme-audit/report.md`). The headless suite includes the same core/sim/io tests; the
coverage configuration previously reported **88% line coverage** across `core`+`sim`.

Golden files live in `tests/data/golden/`. Updating one is deliberate: edit the exporter,
review the diff, run the generator, and commit both together. See the README in that folder.

### Reference machine and measured performance

Reference machine for the numbers below:

- AMD Ryzen 5 5500, 6 cores / 12 threads, 16 GB RAM.
- Windows 11 Pro.
- GCC 16.2.0 (MinGW-w64 UCRT), Qt 6.12.0, Release build.

Measured on that machine with the tests in this repository:

- 500 components / 800 wires: **0.62 ms per scan cycle** (budget: 2 ms).
- Opening and rendering a 500-component sheet: **52.5 ms** (budget: 100 ms).
- Debug builds are slower and use looser test budgets; the release numbers are the ones
  compared against the specification.

## Install / deployment notes

- Linux: `cmake --install build/linux-release --prefix /usr/local`; assets, examples,
  translations and the desktop entry are installed under the prefix.
- Windows: after a Release build, run
  `windeployqt --release --no-translations path\to\fiovivo.exe` and copy the `assets`,
  `examples` and compiled translation files next to the executable (the build already does
  this in the build tree). Packaging can be added with CPack/NSIS when a release is cut.
- Qt deployment follows the standard `windeployqt` (Windows) and distribution Qt packages
  (Linux) approach.

## Dependencies and licenses

| Library | License | Purpose |
|---|---|---|
| Qt 6 | LGPL-3.0 (or commercial) | GUI, SVG, printing, translations |
| nlohmann/json | MIT | JSON parsing and serialization |
| GoogleTest | BSD-3-Clause | Unit and integration tests only |

The About dialog repeats the safety disclaimer and third-party notices. FioVivo itself is
MIT-licensed; see [`LICENSE`](LICENSE).

## Documentation

- [`docs/plan.md`](docs/plan.md) — architecture plan, decisions and open questions
- [`docs/architecture.md`](docs/architecture.md) — layers, ownership, threading
- [`docs/file-format.md`](docs/file-format.md) — `.dcj` format and worked example
- [`docs/cadesimu/report.md`](docs/cadesimu/report.md) - CADe_SIMU migration report, evidence and limits
- [`docs/cadesimu/inventory.md`](docs/cadesimu/inventory.md) - 215-type catalog and recovered CADe_SIMU IDs
- [`docs/simulation-semantics.md`](docs/simulation-semantics.md) - scan model and limits
- [`docs/symbol-library.md`](docs/symbol-library.md) — symbols, pins, parameters, states
- [`docs/glossary.md`](docs/glossary.md) — Portuguese/English electrical glossary
- [`docs/contributing.md`](docs/contributing.md) — coding, commits and review
- [`docs/report.md`](docs/report.md) — final implementation report

## Safety disclaimer

**FioVivo is an educational and prototyping tool. It is not a certified safety tool and
must not be used to design, verify or commission safety-critical electrical systems.**
Always follow local standards, manufacturer data and applicable law when working on real
electrical installations.
