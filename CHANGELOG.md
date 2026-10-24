# Changelog

All notable changes to FioVivo are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and the
project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2026-10-23

First integrated development milestone. The editor, simulator and persistence/export scope
are present; the Phase 4 symbol editor and simulation snapshot files remain future work.

### Added

- C++20 CMake project with `core`, `sim`, `io`, `symbols`, `library`, `ui` and `app` targets,
  dependency rules that keep Qt out of `core`/`sim`/`io`, install rules and desktop metadata.
- Strong typed identifiers, `Result<T, Error>`, deterministic ordered model containers,
  orthogonal routing, validation and problem reports.
- Original SVG symbol set for all 20 Phase 1 types, each with a JSON pin/parameter manifest
  and preloaded active/state artwork.
- Qt Widgets editor: multi-sheet document, grid and snap, zoom/pan, drag and click wiring,
  pin tooltips, selection, move, rotate, copy/paste/duplicate/delete, context menus,
  snapshot undo/redo, searchable symbol library and properties/problems/log docks.
- Deterministic scan-cycle simulator:
  - source-group connectivity over wires and closed pass-throughs;
  - contactor coils, NO/NC/changeover-style contacts, self-holding and first-wins interlocks;
  - TON/TOF timers with exact cycle timing and same-scan timer contact response;
  - latching emergency stop, thermal relay, fuse, breaker, selector, lamps and three-phase
    motors;
  - 16-pass convergence guard with oscillation fault and state rollback;
  - in-memory snapshots, deterministic logs and live-wire/energized-state output.
- Qt worker-thread simulation controller with queued commands, throttled/batched pacing,
  step/step-N/reset controls, speed multiplier and canvas input operation.
- `.dcj` UTF-8 JSON file format, embedded JSON Schema subset validation, referential integrity
  checks, unknown-field preservation, version refusal/migration framework and deterministic
  round-trips.
- Autosave every 2 minutes plus startup recovery; main project files are never overwritten by
  autosave.
- PNG (1x/2x/4x, transparent option), SVG, PDF and BOM CSV exports; print and print preview
  with a drawn title block.
- Brazilian Portuguese (default) and English Qt Linguist catalogs, compiled into `.qm` assets
  next to the executable; language, theme, font, grid, colors, cycle and speed preferences.
- Five bundled examples (direct-on, star-delta, timer sequence, two-motor interlock,
  emergency stop), a deterministic example generator and integration tests for each scenario.
- 100+ CTest tests including core/sim/io, schema, unknown-field, example, timer, determinism,
  oscillation, UI canvas, worker-thread, translation, export, BOM golden and performance
  tests.
- GitHub Actions jobs for format checking, Linux GCC/Clang, Windows MSVC, ASan/UBSan and
  coverage reporting; issue/PR templates and contributor documentation.
- Measured release performance on the documented reference machine: 0.62 ms per scan for
  500 components / 800 wires and 52.5 ms to open and render a 500-component sheet.

### Fixed

- Preserved `modified_utc` correctness when saving and kept round-trip serialization
  deterministic.
- Prevented timer outputs from enabling both star and delta contactors at the same time in the
  bundled star-delta example.
- Fixed the command-line open path and canvas fit after the window is shown.
- Prevented W from stealing keystrokes in text fields; wire mode is toggled by the view
  shortcut or the toolbar action.

### Known limitations

See `README.md` and `docs/report.md`: no voltage/current/heating model, no file snapshots, no
wire segment dragging, no mirror, no shortcut editor, single recovery slot, and no Phase 4
symbol editor.

### Third-party licenses

| Library | License | Purpose |
|---|---|---|
| Qt 6 | LGPL-3.0 (or commercial) | GUI, SVG rendering, printing, translations |
| nlohmann/json | MIT | JSON parsing and serialization |
| GoogleTest | BSD-3-Clause | Tests only |

**Safety notice:** FioVivo is an educational and prototyping tool. It is not a certified
safety tool and must not be used to design, verify or commission safety-critical electrical
systems.

[0.1.0]: https://github.com/RodrigoKuerten/electrical-diagram/releases/tag/v0.1.0
