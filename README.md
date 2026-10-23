# FioVivo

> **Working title.** FioVivo is a provisional project name. The application identifier,
> namespace and file extension are kept in one place so the name can be changed without
> touching the domain model.

FioVivo is an educational electrical control diagram editor and simulator written in C++20
with Qt 6 Widgets. It targets Brazilian NBR 5280-style command diagrams and uses graphical
conventions inspired by IEC 60617. Every symbol in this repository was drawn from scratch.

The application is a **prototyping and teaching tool**. It is **not** a certified electrical
design, protection coordination or safety tool. It does not simulate voltage, current,
heating, mechanical motion or electrical safety. Never use it to validate a real installation.

## Status

This repository is under active development. The roadmap follows the phases in
[`docs/plan.md`](docs/plan.md):

| Phase | Scope | Status |
|---|---|---|
| 1 | Editor core | in progress |
| 2 | Scan-cycle simulator | in progress |
| 3 | Persistence, export, printing | planned |
| 4 | Extras | planned |

## Features (current development build)

- Multi-sheet diagram model with strong component, pin and wire identifiers.
- Orthogonal wiring, grid snapping, selection, rotation, copy/paste and snapshot undo/redo.
- Symbol library with SVG artwork and JSON pin manifests.
- Validation rules with a clickable problems panel.
- Deterministic scan-cycle simulator: contactor self-holding, interlocks, timers,
  emergency-stop latch, thermal relay, lamps and motors.
- `.dcj` JSON project files with format-version checks and unknown-field preservation.
- Brazilian Portuguese (default) and English UI.
- Export to PNG, SVG, PDF and BOM CSV.

## Build requirements

- CMake >= 3.21 and Ninja (recommended) or another generator.
- A C++20 compiler: GCC 11+, Clang 14+ or MSVC 19.30+.
- Qt 6.5+ (LTS recommended) with Widgets, Svg, PrintSupport and LinguistTools.
- GoogleTest and nlohmann/json. They are fetched automatically when not installed.

### Windows (MSYS2 / MinGW-w64 UCRT)

```powershell
winget install MSYS2.MSYS2
C:\msys64\usr\bin\bash -lc "pacman -S --needed mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-qt6-base mingw-w64-ucrt-x86_64-qt6-svg \
  mingw-w64-ucrt-x86_64-qt6-tools mingw-w64-ucrt-x86_64-gtest \
  mingw-w64-ucrt-x86_64-nlohmann-json"
```

Open an MSYS2 UCRT64 shell (`C:\msys64\ucrt64.exe`) in the repository and run:

```bash
cmake --preset windows-msys2-debug
cmake --build --preset windows-msys2-debug
ctest --preset windows-msys2-debug
./build/windows-msys2-debug/src/app/fiovivo.exe
```

### Windows (MSVC)

Install Visual Studio 2022 with the "Desktop development with C++" workload and Qt 6 with
the MSVC 2019/2022 kit. From a Developer PowerShell:

```powershell
cmake --preset windows-debug -DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/msvc2019_64
cmake --build --preset windows-debug
ctest --preset windows-debug
```

### Linux (Ubuntu/Debian)

```bash
sudo apt install build-essential cmake ninja-build qt6-base-dev qt6-svg-dev \
  qt6-tools-dev qt6-tools-dev-tools libgtest-dev nlohmann-json3-dev
cmake --preset linux-debug
cmake --build --preset linux-debug
ctest --preset linux-debug
```

### Linux (MSYS2 is not used)

The `linux-debug` and `linux-release` presets use Ninja. Install a Qt 6.5+ LTS package from
your distribution or the Qt online installer. CMake's `CMAKE_PREFIX_PATH` can point at a
custom Qt installation.

## Run tests

```bash
ctest --preset windows-msys2-debug --output-on-failure
```

The `core` and `sim` libraries do not include Qt and their tests are headless. UI tests are
built when Qt is available.

## Open the examples

Use **File > Open** and choose a file in [`examples/`](examples/). The examples are also
loaded by the headless integration tests.

## Known limitations

See [`docs/simulation-semantics.md`](docs/simulation-semantics.md) and the final report in
[`docs/report.md`](docs/report.md). In short:

- Only presence/absence of energy is simulated. Voltage, current, impedance, phase angle,
  protection coordination and thermal behaviour are not modelled.
- Loads in series are not represented. Each load is expected to have a supply path and a
  return path through distribution wiring.
- Phase 1 allows more than one wire per pin so that parallel circuits can be drawn. This is
  a deliberate deviation from the original one-wire-per-pin draft; it is documented in
  [`docs/decisions.md`](docs/decisions.md).
- Symbol geometry is original and follows common IEC/NBR conventions, but it is not a
  certified reproduction of any standard.

## Third-party libraries

| Library | License | Purpose |
|---|---|---|
| Qt 6 | LGPL-3.0 (or commercial) | GUI, SVG rendering, printing, translations |
| nlohmann/json | MIT | JSON parsing and serialization |
| GoogleTest | BSD-3-Clause | Unit and integration tests |

No other runtime dependencies are used. See `LICENSE` for the project license (MIT) and the
About dialog for a complete third-party notice list.

## License

MIT. See [LICENSE](LICENSE).

## Safety disclaimer

**FioVivo is an educational and prototyping tool. It is not a certified safety tool and
must not be used to design, verify or commission safety-critical electrical systems.**
Always follow local standards, manufacturer data and applicable law when working on real
electrical installations.
