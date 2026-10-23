## Summary

<!-- Describe the change and why it is needed. -->

See `docs/plan.md` for phase scope and decisions, and `docs/contributing.md` for the
contribution workflow.

## Scope and phase

- Phase: <!-- 1 / 2 / 3 / 4 / tooling / docs / N/A -->
- Related issue: <!-- Closes #... -->

- [ ] The change is limited to the stated phase and scope from `docs/plan.md`; any scope
      expansion is explained below.
- [ ] Architectural boundaries are preserved: `fiovivo_core` and `fiovivo_sim` contain no
      Qt, and Qt remains confined to rendering/UI targets.
- [ ] Error handling follows the project convention (`fiovivo::Result<T>` in core/sim/io;
      no exceptions across the core/UI boundary).
- [ ] New third-party dependencies are avoided, or their license and rationale are recorded
      in the README third-party table and the About dialog notices.
- [ ] Commits follow Conventional Commits.

## Tests

- [ ] Tests were added or updated for the new behavior, or the reason they are not needed
      is explained below.
- [ ] `ctest --output-on-failure` passes locally (use `-C Release` for multi-config builds).
- [ ] New tests are deterministic and headless unless they explicitly exercise the UI.
- [ ] `FIOVIVO_WARNINGS_AS_ERRORS=ON` builds without warnings.

## Golden and fixture files

- [ ] Golden files, `.dcj` examples and schema/migration fixtures were changed only where
      intended, and the new output was reviewed.
- [ ] Round-trip and version/migration tests still pass.

## Formatting and static analysis

- [ ] `clang-format --dry-run --Werror --style=file` reports no violations.
- [ ] `.clang-tidy` findings were reviewed (see the rules in `.clang-tidy`).

## Internationalization

- [ ] User-visible strings use Qt translation macros rather than hard-coded text.
- [ ] Brazilian Portuguese and English translations were updated under `src/i18n` when UI
      text changed.
- [ ] The UI was checked at a different text length/locale where practical.

## Documentation

- [ ] README or files under `docs/` were updated when behavior, commands or limitations
      changed.
- [ ] Deliberate specification deviations are recorded in `docs/plan.md` §3 and the final
      report.
- [ ] `CHANGELOG.md` has an entry under `Unreleased` for user-visible changes.

## Safety

- [ ] The safety disclaimer is intact and no text, screenshot or feature description
      implies certification or suitability for real electrical installations.
- [ ] The change does not present simulated presence-of-energy results as electrical
      verification.

## How was this tested?

<!-- OS, compiler, Qt version, CMake preset/command, and manual steps. -->

| Environment | Version |
|---|---|
| OS | |
| Compiler | |
| Qt | |
| Command | |

## Screenshots or recordings

<!-- Required for visible UI changes; otherwise write "N/A". -->

## Notes for reviewers

<!-- Known limitations, follow-ups, deviations, or questions for the owner
     (see docs/plan.md §5). -->
