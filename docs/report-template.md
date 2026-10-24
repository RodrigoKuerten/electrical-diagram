# FioVivo final report

> **Template.** Replace every `[TODO: ...]` placeholder before delivery. This report is
> about an educational prototyping tool; see the safety statement in section 8.

## Document control

| Field | Value |
|---|---|
| Version | `[TODO: 0.1.0]` |
| Commit | `[TODO: full commit SHA]` |
| Date | `[TODO: YYYY-MM-DD]` |
| Author | `[TODO: name]` |
| Reviewer | `[TODO: name]` |
| Target phase | `[TODO: Phase 1 / 2 / 3 / 4]` |
| Release tag | `[TODO: v0.1.0-phaseN, or "none"]` |

## 1. Implemented

`[TODO: 1-2 paragraph summary of what was actually delivered, with links to merge requests
or commits.]`

Only list behavior that can be demonstrated and is covered by a test or a manual check.

| Phase | Feature / requirement | Status | Evidence (tests, files, screenshots) |
|---|---|---|---|
| 1 | `[TODO: feature]` | `[TODO: complete / partial]` | `[TODO: link or test name]` |
| 2 | `[TODO: feature]` | `[TODO: complete / partial]` | `[TODO: link or test name]` |
| 3 | `[TODO: feature]` | `[TODO: complete / partial]` | `[TODO: link or test name]` |
| 4 | `[TODO: feature]` | `[TODO: complete / partial]` | `[TODO: link or test name]` |

## 2. Not implemented

`[TODO: explain whether the target phase was met and what was deferred.]`

| Item | Plan reference | Reason for deferral | Impact | Planned phase |
|---|---|---|---|---|
| `[TODO: item]` | `[TODO: docs/plan.md section]` | `[TODO: reason]` | `[TODO: impact]` | `[TODO: phase]` |
| Voltage/current, impedance, heating, mechanical motion and protection coordination | `docs/simulation-semantics.md`, README "Known limitations" | Out of scope: only presence/absence of energy is simulated | Results must not be treated as electrical verification | `[TODO: future / not planned]` |
| `[TODO: item]` | `[TODO: section]` | `[TODO: reason]` | `[TODO: impact]` | `[TODO: phase]` |

## 3. Known bugs

| ID | Severity | Summary | Reproduction | Platform | Workaround | Status |
|---|---|---|---|---|---|---|
| BUG-001 | `[TODO: critical / major / minor / cosmetic]` | `[TODO: summary]` | `[TODO: steps]` | `[TODO: OS, compiler, Qt]` | `[TODO: workaround]` | `[TODO: open / fixed / linked issue]` |
| BUG-002 | `[TODO]` | `[TODO]` | `[TODO]` | `[TODO]` | `[TODO]` | `[TODO]` |

If there are no known bugs, state that explicitly rather than deleting the table header.

## 4. Deviations from specification

`[TODO: list every deliberate deviation and include the owner's decision or mark it as
pending. Deviations already documented in docs/plan.md §3 must appear here.]`

| ID | Specification reference | Requirement | Implemented behavior | Rationale | Owner decision | Status |
|---|---|---|---|---|---|---|
| DEV-001 | Original spec §3.3 (one wire per pin) | A pin may connect to at most one wire | Multiple wires per pin are allowed; a junction dot is drawn when three or more conductors meet; there is no hard cap in this build | The required parallel circuits, self-holding, indicator lamps and bundled examples cannot be built with one wire per pin | Pending: docs/plan.md §5, question 2 | `[TODO: accepted / rejected / pending]` |
| DEV-002 | `[TODO: specification section]` | `[TODO: original requirement]` | `[TODO: actual behavior]` | `[TODO: reason]` | `[TODO: decision or pending]` | `[TODO: status]` |
| DEV-003 | `[TODO: specification section]` | `[TODO: original requirement]` | `[TODO: actual behavior]` | `[TODO: reason]` | `[TODO: decision or pending]` | `[TODO: status]` |

## 5. Open questions

These refer to **`docs/plan.md` §5** ("Questions that need an answer from the project
owner"). Do not close them in this report without an explicit owner answer.

| # | Question | Current default / assumption | Owner answer | Date |
|---|---|---|---|---|
| 1 | Confirm the final product name and application ID | Working title **FioVivo**, namespace `fiovivo`, extension `.dcj` | `[TODO: pending]` | `[TODO]` |
| 2 | Are multiple wires per pin and explicit junctions acceptable? | Multiple wires allowed, with junction dots; see DEV-001 | `[TODO: pending]` | `[TODO]` |
| 3 | Which phase is the delivery target for the first public release? | Phase 1, Phase 2 and most of Phase 3 are integrated; snapshot files and shortcut remapping remain | `[TODO: pending]` | `[TODO]` |
| 4 | Should the About dialog link to the repository and issue tracker at a final URL? | `[TODO: current behavior]` | `[TODO: pending]` | `[TODO]` |
| 5 | Should the default paper be A4 landscape or A3? | A4 landscape | `[TODO: pending]` | `[TODO]` |
| 6 | Is MIT the intended project license? | MIT (see `LICENSE`) | `[TODO: pending]` | `[TODO]` |

Add any new questions to `docs/plan.md` §5 and reference them here by number.

## 6. Test and coverage results

Report results for the commit in Document control. Do not copy numbers from an earlier
run.

### Test runs

| Job / suite | Platform and compiler | Command | Total | Passed | Failed | Skipped | Duration | Link |
|---|---|---|---|---|---|---|---|---|
| Unit and integration tests | Ubuntu 24.04, GCC | `ctest --output-on-failure` | `[TODO]` | `[TODO]` | `[TODO]` | `[TODO]` | `[TODO]` | `[TODO: CI run URL]` |
| Unit and integration tests | Ubuntu 24.04, Clang | `ctest --output-on-failure` | `[TODO]` | `[TODO]` | `[TODO]` | `[TODO]` | `[TODO]` | `[TODO]` |
| Unit and integration tests | Windows Server 2022, MSVC | `ctest -C Release --output-on-failure` | `[TODO]` | `[TODO]` | `[TODO]` | `[TODO]` | `[TODO]` | `[TODO]` |
| ASan + UBSan | Ubuntu 24.04, GCC | `ctest --output-on-failure` | `[TODO]` | `[TODO]` | `[TODO]` | `[TODO]` | `[TODO]` | `[TODO]` |

Sanitizer configuration:

- `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1`
- `UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`
- Result: `[TODO: clean / findings listed below]`
- Findings: `[TODO: list, or "none"]`

### Coverage

Coverage is reported for information only; CI does not enforce a minimum threshold.

| Metric | Covered | Total | Percent |
|---|---|---|---|
| Lines | `[TODO]` | `[TODO]` | `[TODO]` |
| Functions | `[TODO]` | `[TODO]` | `[TODO]` |
| Branches | `[TODO]` | `[TODO]` | `[TODO]` |

- Scope: headless targets (`core`, `sim`, `io`, `library`) built with
  `FIOVIVO_BUILD_UI=OFF` and `FIOVIVO_COVERAGE=ON`.
- Exclusions: system headers, test sources and fetched dependencies.
- Artifact: `[TODO: CI artifact link or local path]`
- Notes: `[TODO: untested areas and why]`

## 7. Performance reference machine

Record the machine used for reference measurements. Do not combine results from different
machines, and state clearly that the numbers are indicative rather than guarantees.

### Hardware and software

| Field | Value |
|---|---|
| CPU | `[TODO: model, cores/threads, base/boost clock]` |
| RAM | `[TODO: size, speed]` |
| Storage | `[TODO: model/type]` |
| GPU | `[TODO: model, or "not used"]` |
| OS | `[TODO: version and build]` |
| Display | `[TODO: resolution, scaling]` |
| Compiler | `[TODO: name and version]` |
| CMake | `[TODO: version]` |
| Qt | `[TODO: version, kit]` |
| Build type | `[TODO: Release / Debug; sanitizers off unless stated]` |
| Power profile | `[TODO: balanced / performance; relevant governor]` |

### Measurements

Method: `[TODO: benchmark name, warm-up, iteration count, input files, whether the UI was
idle or simulating]`.

| Scenario | Input / size | Iterations | Mean | Median | p95 | Max | Notes |
|---|---|---|---|---|---|---|---|
| Load `.dcj` | `[TODO: file, sheet count]` | `[TODO]` | `[TODO]` ms | `[TODO]` ms | `[TODO]` ms | `[TODO]` ms | `[TODO]` |
| Save `.dcj` | `[TODO: file, sheet count]` | `[TODO]` | `[TODO]` ms | `[TODO]` ms | `[TODO]` ms | `[TODO]` ms | `[TODO]` |
| Simulation step | `[TODO: components, wires]` | `[TODO]` | `[TODO]` ms | `[TODO]` ms | `[TODO]` ms | `[TODO]` ms | `[TODO]` |
| Export SVG/PDF | `[TODO: size, page count]` | `[TODO]` | `[TODO]` ms | `[TODO]` ms | `[TODO]` ms | `[TODO]` ms | `[TODO]` |

## 8. License and third-party notices

- Project license: MIT. See [`LICENSE`](../LICENSE). `[TODO: confirm with owner; see
  docs/plan.md §5, question 6.]`
- Distribution model: `[TODO: source only / binaries; dynamic or static Qt linking]`
- LGPL compliance steps (if binaries are distributed): `[TODO: dynamic linking, license
  text, relinking/source offer as applicable]`
- Asset provenance: `[TODO: confirm that all SVG symbols and other assets were drawn for
  this project and do not reproduce a certified standard]`

| Library | Version | License | Purpose | Notice location |
|---|---|---|---|---|
| Qt 6 | `[TODO]` | LGPL-3.0 (or commercial) | GUI, SVG rendering, printing, translations | `[TODO: LICENSE/NOTICE/About dialog]` |
| nlohmann/json | `[TODO]` | MIT | JSON parsing and serialization | `[TODO]` |
| GoogleTest | `[TODO]` | BSD-3-Clause | Unit and integration tests | `[TODO]` |
| `[TODO: any new dependency]` | `[TODO]` | `[TODO]` | `[TODO]` | `[TODO]` |

No other runtime dependencies are expected. Any dependency added after this report must be
recorded in the README license table and the About dialog notices.

### Safety statement

The README safety disclaimer must remain intact. Quote its final wording here exactly:

> `[TODO: paste the final disclaimer from README.md]`

Confirm that the application and documentation do not claim certification or suitability
for real electrical installations: `[TODO: yes / issues found]`.
