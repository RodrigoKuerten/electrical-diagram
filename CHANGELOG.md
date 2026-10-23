# Changelog

All notable changes to FioVivo are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this
project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

Phase tags follow the convention `v0.1.0-phaseN` described in
[`docs/plan.md`](docs/plan.md).

## [Unreleased]

The entries below are **planned** for the first milestone. Nothing in this section has
been released, and phase status is tracked in the table in [`README.md`](README.md).

### Planned - Phase 1: Editor core

- Multi-sheet diagram model with strong component, pin and wire identifiers.
- Orthogonal wiring, grid snapping, selection, rotation, copy/paste and snapshot
  undo/redo.
- Symbol library with original SVG artwork and JSON pin manifests.
- Validation rules with a clickable problems panel.
- Brazilian Portuguese (default) and English user interface.

### Planned - Phase 2: Scan-cycle simulator

- Deterministic scan-cycle engine with no Qt dependency.
- Contactor self-holding, interlocks, timers, emergency-stop latch, thermal relay,
  lamps and motors.
- Simulation worker thread with queued signals to the UI.
- Determinism, timing and example-based simulation tests.

### Planned - Phase 3: Persistence, export and printing

- `.dcj` JSON project files with format-version checks, schema validation, migrations
  and unknown-field preservation.
- Golden-file and round-trip tests for the file format and bundled examples.
- Export to PNG, SVG, PDF and BOM CSV, plus printing through Qt.
- Autosave and recovery, application settings and completed translations.

### Planned - Phase 4: Extras

- Additional components, conveniences and documentation improvements after Phases 1-3.
- The Phase 4 scope will be defined by the project owner; no extra scope is committed
  yet.

### Planned - Documentation, safety and licensing

- Repository toolchain documentation, formatting rules and CI (format, Linux GCC/Clang,
  Windows MSVC, ASan/UBSan and coverage jobs).
- The README safety disclaimer stating that FioVivo is an educational and prototyping
  tool, not a certified electrical design, protection coordination or safety tool, and
  must not be used to validate a real installation.
- The third-party license table below and the matching notices in the About dialog.

## [0.1.0] - Planned

First planned public milestone. There is no release date or tagged build yet; the planned
scope is listed under [Unreleased](#unreleased). Entries will move here when the release
is tagged. The project version is `0.1.0` in `CMakeLists.txt`.

### Third-party licenses

FioVivo itself is licensed under the MIT License (see [`LICENSE`](LICENSE)). The planned
dependencies and their licenses are:

| Library | License | Purpose |
|---|---|---|
| Qt 6 | LGPL-3.0 (or commercial) | GUI, SVG rendering, printing, translations |
| nlohmann/json | MIT | JSON parsing and serialization |
| GoogleTest | BSD-3-Clause | Unit and integration tests |

No other runtime dependencies are planned. The About dialog will carry the same notices,
and any new dependency must be added to this table before it is merged.

**Safety notice:** FioVivo is an educational and prototyping tool. It is not a certified
safety tool and must not be used to design, verify or commission safety-critical
electrical systems.

[Unreleased]: https://github.com/RodrigoKuerten/electrical-diagram/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/RodrigoKuerten/electrical-diagram/releases/tag/v0.1.0
