# Simulation semantics

This document describes the simulator that is actually implemented in `src/sim/` and the
worker adapter in `src/ui/`. It is an educational model of command circuits: it reasons
about the presence or absence of energy, not about voltage, current, impedance, heating,
mechanical motion or electrical safety. It must not be used to design, verify or commission
a real electrical installation.

## 1. Model primitives

| Primitive | Simulation meaning |
|---|---|
| Pin | A graph node. Each pin belongs to one component. |
| Wire | An ideal conductor between two pins. Multiple wires may meet at a pin. |
| Pass-through element | A device that conducts between a pair of pins when closed: contacts, selector paths, intact fuse, closed breaker, timer contact. |
| Load | A coil, timer coil, lamp or motor. Loads do not connect their terminal nets; they are energized by a difference of source groups. |
| Supply terminal | Seeds a source group (L1, L2, L3, N for AC; + and − for DC). PE never seeds a group and is never a return. |

Scene units are millimetres. The engine does not use scene geometry; it uses only pin IDs
and wire endpoints.

## 2. Source groups and energy

A source group is the simulator's stand-in for the identity of a supply conductor. It is not
a voltage magnitude, phase angle or current direction.

- Every supply pin whose role is `supply_in`, `supply_out` or `neutral` seeds a group named
  `<component type>:<pin name>`, for example `supply_ac3:L1` or `supply_dc:-`.
- A two-terminal load is energized if its two terminals can reach at least one pair of
  distinct source groups through wires and closed pass-through elements.
- A motor is energized when at least `min_live_phases` (default 2) of U1/V1/W1 reach source
  groups and at least two distinct groups are present. PE is not used.
- A load whose terminals are on the same net is off. A load with only one terminal connected
  to a supply is off. A load never serves as a conductive edge for another load.
- Wires and closed pass-throughs are unioned with a deterministic disjoint-set structure.
  Source groups are collected per net. There is no graph traversal that crosses a load.
- A 24 V coil and a 220 V coil behave identically when both see two distinct groups. Voltage
  levels are not simulated.
- Two loads in series do not work: each must have its own path to a supply group and a
  different return group.

`supply_ac1`, `supply_ac3` and `supply_dc` are sources. `PE` is excluded from source seeding.
The neutral is a legitimate return group, not a special ground.

## 3. Scan cycle

`sim::Simulator::step()` executes one logical scan. The cycle period is an integer number of
milliseconds, default 10 ms in the application. Simulated time is `cycle_count * cycle_ms`;
wall-clock time never determines state.

Actual order in `step()`:

1. If a previous oscillation fault is latched, return it without changing state.
2. Save the previous component states, log size, live-wire set and simulated time.
3. Build the base connectivity graph using the current timer output contacts and all fixed
   pass-through devices.
4. Run the coil fixed point (up to 16 passes):
   - **Without contator contacts** the base graph is identical for all coils; it is built
     once per pass and all coils are evaluated against it.
   - **With contactor contacts** coils are visited in ascending `(tag, id)` order. A trial
     graph includes the current contactor-group states. When a coil changes state, its group
     snapshot is updated immediately, so a later coil sees the new contact position in the
     same pass. This produces deterministic "first wins" behavior for simultaneous
     interlocked starts and allows self-holding to emerge.
   - A pass with no coil change is converged. Sixteen passes without convergence latches an
     `Oscillation` error.
5. Build the final connectivity graph and commit coil states.
6. Advance TON/TOF timers once using the final coil energization of this scan.
7. If any timer output changed, rebuild the graph with the new timer contact positions and
   re-solve the coils. This makes timer-driven contactors respond in the same scan in which
   the timer times out.
8. Evaluate lamps and motors against the final graph.
9. Update visual closure states, live wires, simulated time and the cycle counter; then
   append state-change log entries by comparing with the saved states.
10. Notify the observer with the committed frame.

If a fixed-point or re-solve fails, the saved states/time/log are restored, the fault is
latched, and `step()` returns the error. No partial unstable frame is committed. Until
`reset()` is called, further steps return the same fault.

Contactor/relay contacts are derived from the coil group each time a trial graph is built;
they are not stored as independent editable state. Timer contacts use the current
`timer_output`.

## 4. Coil/contact pairing

- A coil and its contacts share the `label_group` parameter.
- If the parameter is empty, the component tag with a trailing `:<suffix>`, `#<suffix>` or
  `.<suffix>` removed is used. `KM1` → `KM1`; `KM1.2` → `KM1`.
- Contacts in a group with no corresponding contactor coil remain at rest for simulation
  purposes; validation warns.
- Multiple coils with the same group energize the group contacts when any of them is on.
- Main contacts (`contactor_no`), auxiliary NO (`contactor_aux_no`) and timer contacts close
  when active. Auxiliary NC (`contactor_aux_nc`) opens when active.

## 5. Device behaviour

### Pushbuttons

- NO: closed while `input_active` is true. Momentary by default; maintained mode is a UI
  toggle that keeps the level on until clicked again.
- NC: closed unless pressed.
- The engine receives only the level; the UI decides whether a click toggles (maintained) or
  is held (momentary).

### Emergency stop

- Latching NC. `setInput(id, true)` latches the stop and opens the NC contact.
- It stays open until `setInput(id, false)` is called explicitly. The UI exposes this through
  the context menu and **Simulate > Reset Emergency Stop(s)**, and a pushbutton with
  `action = reset_emergency` performs the reset when clicked.
- No automatic reset occurs.

### Selector switch

- Common pin 1 connects to pin `position + 1` (position 1 → pin 2, 2 → pin 3, 3 → pin 4).
- Position changes through `setSelectorPosition`; the UI cycles positions on click.
- The number of positions is a parameter, clamped to the pins available.
- Momentary/maintained is a UI parameter; the engine uses the selected position.

### Fuse, breaker and thermal relay

- Fuse and breaker are pass-through when intact/closed; `setTripped(id, true)` opens them.
  The fuse UI labels the state "blown"; a breaker "tripped".
- Thermal relay 95/96 is NC and 97/98 is NO. Not tripped: 95/96 closes. Tripped: 97/98
  closes. There is no heating model.
- These are user-controlled states; the engine does not calculate current or trip time.

### Timers (TON and TOF)

`delay_ms` is converted from `time_base` (`ms` or `s`). The timer coil energization is
solved in the normal connectivity pass.

- **TON:** while the coil is energized, `elapsed` increases by `cycle_ms` and is clamped to
  the delay. Output becomes active when `elapsed >= delay`. When the coil is de-energized,
  `elapsed` resets to zero and the output returns to rest immediately.
- **TOF:** while the coil is energized, `elapsed` is zero and the output is active. When the
  coil de-energizes, the output stays active and `elapsed` increases until it reaches the
  delay, then the output returns to rest. Re-energizing during the off-delay cancels the
  countdown and keeps the output active.
- The timer output drives the timer's 15/18 contact. If it changes, the connectivity graph is
  rebuilt and coils are re-solved in the same scan (step 7 above).
- A timer is available from the scan in which its coil first energizes. With a 30 ms delay
  and a 10 ms cycle, output becomes active at the end of the third scan after the coil
  energizes. Tests assert this exact count.
- Timers show elapsed/delay in the canvas overlay and properties panel. The simulation log
  records output transitions, not every elapsed tick.

### Lamps and motors

- A lamp is lit when energized. No color, brightness or current is simulated.
- A motor is running when its phase terminals see the required number of distinct source
  groups. Star-delta switching is achieved by contactor wiring and timers, not by a
  hard-coded motor sequence.
- The motor symbol exposes U1, V1 and W1 only. Six-terminal winding modelling is not part of
  this version.

### Self-holding ("selo")

An auxiliary NO contact in parallel with the start button holds the coil after the button is
released. This falls directly out of the connectivity model; there is no special-case code.
The direct-on example and unit/integration tests exercise it.

### Interlocks

An NC contact of coil A in the circuit of coil B prevents B from energizing while A is on.
If both start buttons are active in the same scan, coils are evaluated in tag order. The
first coil energizes and opens its NC interlock; the second sees the updated contact and
stays off. The behavior is deterministic and independent of hash/pointer order.

## 6. Inputs and interaction

- Input levels are stored in the engine and applied to the next step. The worker serializes
  commands from the UI in FIFO order.
- A press and release that both occur between two scans may be missed; the model is
  level-sampled.
- Input changes during Running or Paused are accepted. The scene sends them through queued
  controller calls.
- While the simulation is stopped, canvas clicks select/edit instead of operating devices.
- Structural editing stops and discards the session; the next Start copies the edited diagram.
- The worker runs at the configured speed (0.1x–10x). At high speed it may execute several
  fixed cycles per wall-clock timer tick. Simulated timing remains exact; wall-clock pacing is
  best effort.
- Start loads a fresh session, pause freezes time, step executes one cycle, step-N executes N
  cycles and reset returns to time zero with initial device states.

## 7. Determinism and oscillation

- All decisions use ordered containers/vectors. Coil order is `(tag, id)`; source groups are
  sorted strings.
- Running the same diagram twice with the same input sequence and cycle period produces
  identical snapshots and logs; tests compare both.
- A contact feeding its own coil through an NC contact can oscillate. The fixed point is
  bounded at 16 passes. If states do not settle, the simulator restores the previous stable
  state, latches the fault and stops. The UI shows a warning and the log remains inspectable.
- A scan is not allowed to depend on wall-clock timing; only `cycle_ms` matters.

## 8. Snapshots and logging

- `SimulationSnapshot` contains simulated time, cycle count, input states and component
  states. `Simulator::snapshot()`/`restore()` are deterministic and used by tests.
- A snapshot file format is not exposed in this build.
- The log records simulated time, tag, state kind and new value for coil/contact/timer/lamp/
  motor transitions and user input changes. It is exported as CSV by the UI.
- On reset the simulator clears its log; the UI clears the log panel. The log is capped at
  50,000 entries in the engine and 5,000 rows in the UI.

## 9. Explicit non-goals

The following are **not** simulated:

- voltage magnitude, phase angle, current, power, power factor or impedance;
- conductor resistance, voltage drop, short-circuit current, protection coordination;
- heating, fuse/breaker trip curves, motor thermal protection or relay curves;
- relay bounce, contact wear, arc behaviour, mechanical motion, spring dynamics;
- safety integrity, SIL/PL, emergency-stop certification, lockout/tagout;
- PLC ladder programming, IEC 61131-3 execution, real hardware I/O;
- network latency or real-time guarantees.

The simulator is for teaching, demonstrations and early control-logic prototyping only. It
is not a substitute for a real electrical design, simulation package or safety review.
