# Simulation semantics

This document defines the behavior of the FioVivo scan-cycle simulator. It is the reference
for implementation and tests. It describes the target Phase 2 behavior; features marked
**planned** are not part of the current build.

The simulator is an educational model of command circuits. It reasons about the presence or
absence of energy in a network of wires, contacts and loads. It does not calculate voltage,
current, impedance, heating, mechanical motion or safety. It must not be used to design,
verify or commission a real electrical installation.

All identifiers in code and documentation are English. Portuguese appears only in translated
UI text, in the glossary and in quoted domain terms such as "selo".

## 1. Model primitives

The electrical model is built from pins, wires, pass-through elements and loads.

| Primitive | Role in the simulation |
|---|---|
| Supply terminal | Seeds a source group. Examples are L1, L2, L3 and N. PE never seeds a group and is never a return. |
| Wire | Connects two or more pins. Wires are ideal conductors and have no resistance, capacitance or inductance. |
| Pass-through element | Conducts between its terminals only when closed. Closed contacts, closed selector paths, intact fuses and closed breakers are pass-throughs. |
| Load | A coil, lamp, motor or similar device. A load does not conduct between its terminals; it is energized when its terminals reach distinct source groups. |
| Contact | A pass-through whose open/closed state is controlled by a coil, a pushbutton, a selector, a timer, a thermal relay or an emergency stop. |
| Junction dot | A visual marker for three or more wires on a pin. It has no separate electrical behavior; the wires themselves define connectivity. |

Multiple wires per pin are allowed. This is a deliberate deviation from a strict
one-wire-per-pin rule so that parallel circuits, self-holding and indicator lamps can be
drawn. All wires on a pin are connected.

A load has two terminals in the general case. A three-phase motor has three phase terminals
plus a PE terminal. Loads are never used as conductive links between other devices.

## 2. Source groups

A source group is the simulator's stand-in for the identity of a supply conductor. It is not
a voltage level, a phase angle or a direction of current.

- At session start, the engine enumerates supply terminals in ascending `PinId` order. Each
  supply terminal receives a distinct `SourceGroupId`.
- A terminal for L1, L2, L3 or N may seed a group. A PE terminal never seeds a group and is
  never counted as a return path.
- Source group IDs are stable for the session. They are allocated deterministically and are
  not derived from pointers, hash values or scene order.
- A net with no source group is dead.
- If wires or closed pass-throughs connect two source terminals, their groups end up in the
  same net. The engine does not calculate a short-circuit current and does not trip a device
  because of that connection.
- Distinct groups are distinct by identity. The engine does not check whether two groups have
  the same nominal voltage or phase. Two independent supplies are not reconciled; the
  intended model is one supply with L1/L2/L3/N terminals. Connect common terminals with
  wiring if they must be common.

The engine does not assign a voltage value to a source group. A 24 V coil and a 220 V coil
are indistinguishable when each sees two distinct groups.

## 3. Connectivity and load energization

### 3.1 Nets and union-find

A net is a maximal set of pins connected by ideal conductors. Connectivity is recomputed
inside every contact-evaluation pass:

1. Start with one set per pin.
2. Union the two endpoints of every wire, in ascending `WireId` order.
3. Union the conductive terminals of every closed pass-through element, in ascending
   `ComponentId` order.
4. Attach the source group of each supply terminal to the resulting net.
5. Record the set of source groups reachable by each net.

Union-find uses deterministic union order. A representative may be chosen by rank with the
lower `PinId` as the tie-breaker, and the source-group set of a net is kept sorted. The
implementation may use a connected-component traversal instead, provided the resulting nets
and groups are identical. No iteration order may depend on a hash table or pointer address.

### 3.2 Energization rule

A two-terminal load is energized when its two terminal nets contain at least one pair of
distinct source groups.

```text
energized(load) =
  exists gA in groups(net(terminal A)), gB in groups(net(terminal B)):
    gA != gB
```

Consequences:

- A load with both terminals on the same net is off.
- A load with one terminal on a dead net is off.
- A load terminal on PE never counts as a return. A two-terminal load with a terminal on the
  PE net is off, even if that net is accidentally bonded to a source group.
- A load does not merge its terminal nets. It senses a difference; it does not conduct.
- Two loads in series are not supported. The first load's terminals remain separate nets, so
  neither load sees a complete supply-and-return path. Each load must be on a branch that
  reaches two distinct groups.

### 3.3 Load types

| Load | Terminals | Energization rule |
|---|---|---|
| Contactor coil | A1/A2 or the manifest's coil pins | Two-terminal rule |
| Relay coil | A1/A2 | Two-terminal rule |
| Timer coil or enable input | Timer coil pins | Two-terminal rule; drives the timer state machine |
| Lamp / pilot light | Two terminals | Two-terminal rule; no brightness or color semantics |
| Motor | U1, V1, W1 and PE | Count the distinct source groups reachable at U1, V1 and W1. The motor runs when the count is at least `min_live_phases` (default 2). PE is excluded. |
| Other resistive load | Two terminals | Two-terminal rule; no current, power or heat is calculated |

A motor is not required to have a neutral. Phase sequence, rotation direction, torque,
speed and current are not modelled. Swapping two phase connections has no simulated effect
other than changing which source groups reach the motor terminals.

A lamp or coil that is rated for a different voltage still turns on if it sees two distinct
groups. This is intentional: voltage levels are not simulated.

## 4. Time, scan period and input

### 4.1 Cycle period

The scan period is the configured `cycle_ms`, an integer number of milliseconds greater than
zero. The engine does not assume a particular default; the application and the project
settings provide the value, and tests set it explicitly. Changing `cycle_ms` requires a new
simulation session.

At the start of scan `N`:

```text
scan_index = N
simulated_time_ms = N * cycle_ms
```

Scan `N` covers simulated time `[N * cycle_ms, (N + 1) * cycle_ms)`. Simulated time is
integer milliseconds and does not use the wall clock. Timer delays that are not an exact
multiple of `cycle_ms` are rounded up to the next scan.

Pausing freezes simulated time and does not advance timers. Stepping executes exactly one
scan, even while paused. There is no catch-up: if the host is busy and a timer tick is late,
the simulator still performs one scan and advances time by one cycle. Wall-clock pacing is
best effort; the logical scan sequence is deterministic.

### 4.2 User input snapshot

Inputs are level-based and are captured at the scan boundary:

- Pushbuttons, selector positions and maintained switches use their latest level.
- A press and release that both happen between two scan boundaries may be missed. This is a
  limitation of a level-sampled educational simulator.
- Emergency-stop trigger and thermal-relay trip are latched commands, not momentary levels.
  A trigger before a scan boundary sets the latch even if the actuator is released before the
  next scan.
- Input changes received while a scan is running are queued and apply to the next scan.
- Commands from the UI are processed by the worker in FIFO order. The UI cannot change engine
  state directly.

At reset, the engine clears runtime state, sets `scan_index` to 0 and samples the current
input levels for the first scan. A start button that is held during reset can therefore
start the circuit on the first scan.

### 4.3 Scan order

Each scan performs the following steps in order.

| Step | Action |
|---|---|
| 1 | Drain queued commands in order and capture the input snapshot. Set the scan's simulated time. |
| 2 | Apply latched commands, manual protection states and input levels to the devices. |
| 3 | Seed the pass-through state from the committed contacts. Timer contacts are those committed at the end of the previous scan. |
| 4 | Evaluate connectivity and resolve contactor/relay coils in the fixed point, up to 16 passes. |
| 5 | Commit the converged coil, contact and load states. |
| 6 | Advance timers once, using the final coil/load energization of this scan. Compute the timer outputs and the timer contact positions for the next scan. |
| 7 | Detect oscillation or a fault, append log events, and emit the committed state and events to the observer. |
| 8 | The next scan applies the committed contact positions, including timer contacts, and repeats. |

Timer contacts deliberately become effective in the next scan. Contactor and relay contacts
are applied inside the current fixed point, as described in section 5. This matches the
decision log's allowance that a timer output changes at the end of the scan that reaches the
delay and that a contactor response may take one additional scan in paths driven by timers.

### 4.4 Session commands

| Command | Effect |
|---|---|
| Start | Create a new session, freeze the diagram snapshot, reset runtime state and begin at scan 0 |
| Pause | Stop issuing scans; simulated time and timers freeze |
| Resume | Continue from the next scan index |
| Step | Execute one scan while paused, then remain paused |
| Reset | Clear coils, contacts, timers and latches, set time to zero, and sample current inputs for the first scan; the log is retained |
| Stop | End the session; no further scans |
| Set input | Queue a new level or a latched trigger for the next boundary |
| Restore snapshot | Replace runtime state from an in-memory snapshot (planned) |

Stopping and starting again creates a new session and a fresh immutable diagram snapshot.
Reset does not reload the file and does not undo document edits.

## 5. Contacts, coils and the fixed point

### 5.1 Rest states

- A normally open (NO) contact is open at rest and closed when actuated.
- A normally closed (NC) contact is closed at rest and open when actuated.
- A contactor or relay is at rest when its coil is de-energized.
- A timer's timed NO contact is open at rest; its timed NC contact is closed at rest.
- A pushbutton NO contact is closed only while pressed; a pushbutton NC contact is open only
  while pressed.
- An emergency-stop NC contact is closed at rest and open while latched.
- A thermal relay's 95/96 NC contact is closed until the relay trips; 97/98 NO contact is open
  until the relay trips.
- An intact fuse and a closed breaker are pass-throughs; a blown fuse or open breaker is an
  open pass-through.

### 5.2 Coil and contact pairing

Contactor and relay contacts are paired with their coil by the `label_group` parameter. If a
contact or coil has no `label_group`, its component tag is used instead. All contacts in the
same resolved group follow that group's coil.

- A contactor coil changing state changes every NO and NC contact in its group.
- Contacts are pass-through elements and therefore participate in connectivity.
- If a group has no coil, its contacts remain at rest. Validation reports the unmatched
  contact.
- A group is expected to resolve to one coil. If duplicate coils claim the same group,
  validation reports the ambiguity; the engine uses the lowest `ComponentId` as the driver
  but this fallback is not a recommended model. Do not rely on it.
- Auxiliary numbering such as NO 13/14 and NC 21/22 is symbol metadata. It does not change
  the simulation rule.
- Timer contacts are paired with their timer by the same group/tag mechanism, but their
  output changes at the end of the scan and their contacts apply on the next scan.

`label_group` is an English identifier and is not translated. The UI may display it as a
user-editable field with a localized label.

### 5.3 Deterministic coil order and first wins

Coils are resolved in ascending `ComponentId` order. Sheet order, scene order, pointer order
and UI selection order are not used. IDs are allocated per file and never reused, so the
order is stable for a given document.

The resolution algorithm is:

1. Build the nets for the current contact state.
2. Visit contactor/relay coils in ascending `ComponentId` order.
3. For each coil, evaluate its two-terminal energization against the current nets.
4. If the energization state would change, mark the coil and apply its contacts (NO closes
   and NC opens when energized; NO opens and NC closes when de-energized). If any contact
   changed, update connectivity before evaluating the next coil, so the next coil sees the
   new path. The implementation may rebuild the union-find or apply an equivalent
   deterministic incremental update.
5. After all coils have been visited, compare the complete contact signature with the
   signature at the start of the pass. If it is unchanged, the scan has converged.
6. Otherwise start another pass. The engine performs at most 16 passes.

Step 4 is the "first wins" rule. If two coils are wired with mutual NC interlocks and both
would pick up in the same pass, the coil with the lower `ComponentId` energizes first and its
NC contact opens before the later coil is evaluated. The later coil then sees an open path
and remains off. The engine logs the arbitration as a diagnostic event.

This rule is a deterministic modelling convention, not a physical priority and not a safety
mechanism. Drawing order can affect ID allocation and therefore can affect a pathological
simultaneous case. Control circuits should use the intended electrical interlock so that
normal operation does not depend on which coil wins the tie. Mechanical interlocks, contactor
timing differences and welded contacts are not modelled.

### 5.4 Self-holding ("selo")

Self-holding is emergent. It is not a special latch variable.

A typical direct-on-line control branch has a start pushbutton in parallel with an auxiliary
NO contact of the contactor. The coil is connected between a source group and the return
path.

| Scan | Input | Path to coil | Coil | Auxiliary NO |
|---|---|---|---|---|
| N | Start pressed, stop NC closed | Through the start contact | On | Closes during the fixed point |
| N+1 | Start released, stop NC closed | Through the auxiliary NO contact | Stays on | Remains closed |
| M | Stop pressed | Stop NC opens | Off | Opens; the branch stays off |

The auxiliary contact is applied in the same fixed point as the coil pickup. If the start
button is released before the first scan that sees it pressed, the pulse may be missed. The
start command must be held long enough for at least one scan.

A self-holding branch does not require a "seal" flag in the engine. It is simply a parallel
path that becomes conductive after the coil picks up. If the return path is lost, the coil
drops and the seal opens.

### 5.5 NC interlock

An NC interlock places an NC contact of contactor A in series with the coil of contactor B
(and usually the reverse as well). When A is energized, its NC contact opens and B cannot
energize. When A drops, the contact closes and B can be started, subject to its own start
conditions.

The interlock is electrical, not mechanical. If both start buttons are held, first-wins
decides the simultaneous case deterministically. Releasing the winner's start button and
pressing the other does not transfer the running state unless the circuit is wired to do so.

## 6. Device semantics

### 6.1 Contactors and relays

- A contactor/relay coil is a two-terminal load.
- NO contacts close and NC contacts open when the coil is energized.
- Contacts return to rest when the coil is de-energized.
- There is no pickup delay, dropout delay, bounce, arc, welded-contact state or coil
  burnout.
- Contact state changes are applied inside the fixed point. A contactor controlled by a
  timer contact sees the timer's new position on the next scan.
- The coil itself does not conduct between its terminals; it is a load.
- Auxiliary contacts follow the same group as the coil, regardless of which sheet they are
  on.

### 6.2 Pushbuttons

- A pushbutton is momentary. Its NO contact closes while the actuator is pressed and opens
  on release; its NC contact does the opposite.
- The input is level-sampled at the scan boundary.
- A press shorter than one cycle may be missed.
- A maintained switch is represented by a selector or a maintained input, not by a
  pushbutton.

### 6.3 Selector switches

A selector switch has a common pin (pin 1) and position pins 2 through n. Exactly one
position path is closed at a time while a position is selected.

- Maintained mode: the selected position remains connected until the user selects another
  position.
- Momentary mode: the connection exists only while the position input is held. Releasing it
  opens all position paths.
- The number of positions and the maintained/momentary mode are component parameters.
- Changing position is break-before-make at the scan boundary. The old path opens and the new
  path closes in the same input update; there is no make-before-break overlap.
- Pin 1 is not a source group by itself. It is a common conductor that reaches whichever
  source group the selected position is wired to.

### 6.4 Emergency stop

An emergency stop is a latching NC command device.

- At rest, its NC contact is closed.
- A trigger command sets a latched open state. The contact opens at the next scan boundary.
- Releasing the actuator does not close the contact. An explicit reset command is required.
- Reset closes the contact only if the actuator has been released. A reset while the button
  is still pressed is ignored and logged.
- The latch survives pause and resume. It is cleared by an explicit emergency-stop reset or
  by a full simulation reset that starts a new session. If the actuator is still pressed when
  the new session starts, the input level re-latches it before the first scan; a reset must
  not leave a pressed emergency stop electrically closed.
- Reset does not start the motor or re-energize a contactor; it only restores the contact.
  A separate start command is still required.
- Multiple emergency-stop devices latch independently. A global reset applies to all devices
  whose actuators are released; any device still pressed remains latched.
- The emergency stop opens its control contact. It does not directly break the motor phases
  in the model. The motor stops only if the contactor circuit wired through the emergency
  stop drops the contactor.

### 6.5 Thermal overload relay

The thermal relay is a user-driven protection device in this educational model.

- The relay has a 95/96 NC contact and a 97/98 NO contact.
- A trip command latches the relay. 95/96 opens and 97/98 closes at the next scan boundary.
- A reset command clears the latch and returns the contacts to rest.
- The relay does not trip automatically from current or heating. There is no thermal model,
  no inverse-time curve and no motor thermal capacity.
- `trip_current` and `reset_mode` are stored parameters. Because heating is not modelled, the
  engine does not act on `reset_mode`; all trips are user commands and all resets are explicit.
- The relay does not directly open the motor circuit. The motor stops only if the 95/96
  contact is wired into the contactor coil circuit (or another explicit command path).
- Reset does not restart the motor.

### 6.6 Fuses and circuit breakers

- An intact fuse and a closed breaker are pass-through elements.
- A blown fuse and an open breaker are open pass-through elements.
- The state is controlled by the user or by a file-provided initial state. The engine does
  not blow a fuse, trip a breaker or coordinate protection.
- A fuse can be marked blown and later replaced/reset by a user action. A breaker can be
  opened and closed manually.
- There is no current, trip curve, breaking capacity, discrimination or selectivity.
- A fuse or breaker does not create a source group; it only conducts or isolates.

### 6.7 Timers (TON and TOF)

A timer has a coil/enable input, a preset delay in milliseconds, a logical output and timed
contacts. The timer coil is evaluated with the two-terminal load rule. The timer output is
updated once per scan, after the contact fixed point has converged. Its contacts take effect
on the next scan.

Let `cycle_ms` be the scan period, `elapsed_ms` the accumulated time and `preset_ms` the
configured delay. Delays are rounded up to a whole number of scans.

#### TON (on-delay)

| Input condition | Elapsed at end of scan | Output at end of scan | Contacts |
|---|---|---|---|
| Coil de-energized | Reset to 0 | False | Timed NO open, timed NC closed on the next scan |
| Coil energized, elapsed < preset | `elapsed + cycle_ms` | False | Rest |
| Coil energized, elapsed reaches or exceeds preset | Keeps accumulating or clamps at preset | True | Timed NO closes, timed NC opens on the next scan |
| Coil de-energized after being energized | Reset to 0 | False immediately | Reset on the next scan |

A TON output becomes true at the end of the scan in which the accumulated time reaches the
preset. If `preset_ms` is zero, the output becomes true at the end of the first scan in which
the coil is energized.

#### TOF (off-delay)

| Input condition | Elapsed at end of scan | Output at end of scan | Contacts |
|---|---|---|---|
| Coil energized | Reset to 0 | True | Timed NO closed, timed NC open on the next scan |
| Coil de-energized, output was true, elapsed < preset | `elapsed + cycle_ms` | True | Remain in the timed state |
| Coil de-energized, elapsed reaches or exceeds preset | Keeps or clamps at preset | False | Timed NO opens, timed NC closes on the next scan |
| Coil re-energized during timing | Reset to 0 | True | Remain in the timed state |

A TOF output becomes true as soon as the coil is energized. After the coil drops, the output
stays true for the delay and then becomes false at the end of the scan that reaches the
preset. If `preset_ms` is zero, the output becomes false at the end of the first scan after
the coil drops.

Timer rules:

- Timers are updated in ascending `ComponentId` order after the contact fixed point.
- Elapsed time advances once per scan, not once per fixed-point pass.
- The timer output does not change during the contact fixed point.
- Timer contacts are applied at the start of the next scan, so a timer-driven contactor may
  respond one scan later than a contactor driven directly by a pushbutton.
- A TON resets immediately when its coil drops. A TOF resets its delay when its coil
  re-energizes.
- Pausing freezes elapsed time. Resetting the simulation resets all timers.
- A timer output that feeds its own contact can produce a slow oscillation across scans. That
  is timed behavior, not the fixed-point oscillation fault described in section 7.
- Timer elapsed time and output are part of the runtime state and of a snapshot.

### 6.8 Lamps and indicators

- A lamp is a two-terminal load.
- It is on when its terminals reach distinct source groups.
- There is no brightness, color, aging or current-dependent behavior. The visual overlay is
  a UI concern.
- A lamp does not conduct; it cannot be used as a pass-through.

### 6.9 Motors

- A three-phase motor has U1, V1 and W1 phase terminals and a PE terminal.
- The motor runs when the number of distinct source groups reachable at U1, V1 and W1 is at
  least `min_live_phases`. The default is 2.
- PE is excluded from the count. A motor with only PE and one phase live is not running.
- If all three phase terminals reach the same source group, the distinct count is 1 and the
  motor does not run under the default.
- There is no current, torque, speed, acceleration, direction, braking or mechanical load.
- Starting and stopping are instantaneous at scan boundaries. There is no inrush or
  star-delta transition physics; star-delta is represented by contactors, a timer and
  interlocked contacts, like any other command circuit.
- `voltage`, `power` and `connection` are stored metadata. They do not affect energization
  until a future voltage model is explicitly added.

## 7. Oscillation detection and faults

The contact fixed point is bounded at 16 passes per scan. A pass is one full sweep of all
contactor/relay coils. If the contact signature is unchanged at the end of a pass, the scan
converges. If the signature still changes after the 16th pass, the scan is non-convergent.

On non-convergence:

1. The scan is aborted and is not committed.
2. The last committed state from the previous scan remains visible.
3. An `OscillationDetected` event is logged with the scan index and the IDs of the components
   whose contacts changed in the final pass.
4. The session enters a fault state and pauses. Reset or a new session is required before
   another start.
5. Timers do not advance for the aborted scan.

The fault is a modelling diagnostic. It usually indicates mutually contradictory contacts or
a circuit that has no stable state in the boolean model. It is not a safety assessment and it
does not predict how real hardware would behave.

A direct short between source groups is not an oscillation. It collapses groups into one net;
the engine does not compute short-circuit current and does not automatically trip a breaker.

## 8. Snapshot and restore (planned)

Snapshot/restore is planned for the simulation UI phase. It is a runtime debugging and
reproducibility feature, not document undo.

A simulation snapshot is an immutable value captured at a committed scan boundary. It
contains:

- the session/model revision identifier and `cycle_ms`;
- `scan_index` and simulated time;
- current input levels and latched input commands;
- contactor/relay coil states and contact positions;
- timer elapsed values, outputs and pending contact positions;
- emergency-stop and thermal-relay latches;
- fuse/breaker manual states;
- source-group assignment;
- fault/oscillation state.

It does not contain the diagram document, the undo stack, UI selection, camera state or the
simulation log. Restoring a snapshot:

- validates that it belongs to the same session and model revision;
- replaces runtime state exactly, including timer phase and pending contacts;
- does not re-run past scans and does not replay inputs;
- emits a state update and a `SnapshotRestored` event;
- leaves the log in place and appends the restore event.

Restoring is not a substitute for document undo. If the diagram has changed, the restore is
rejected with an error rather than applied to a different model.

## 9. Visual state updates

The engine emits one plain `SimulationState` value after each committed scan. It never sends
a partially evaluated fixed point. The UI maps component IDs to canvas items and updates
overlays; it does not infer electrical state from geometry.

| Component kind | State exposed to the UI |
|---|---|
| Contactor/relay | Coil energized, contact positions |
| Pushbutton | Pressed, contact position |
| Selector | Selected position, closed path |
| Emergency stop | Latched, contact position |
| Thermal relay | Tripped, 95/96 and 97/98 positions |
| Fuse/breaker | Open/closed or intact/blown |
| Timer | Elapsed milliseconds, output, timed contact positions |
| Lamp | On/off |
| Motor | Running/stopped, live phase count |

A timer's output may lead its timed contact by one scan. The state exposes both so the UI can
show the latency; the UI must not force the contact to follow the output early.

Rules:

- State is keyed by strong ID. The UI ignores IDs that no longer exist in its document.
- The UI never reads the worker's model snapshot or computes connectivity.
- Repaints may be coalesced. The latest committed state wins for display; events remain in
  order.
- On pause, the last state remains visible. On reset, the UI returns to rest states. On stop,
  the last state may remain visible with a stopped indicator; it is not a live reading.
- Visual colors and animations are presentation choices and are not part of the electrical
  semantics.
- No widget, pixmap or SVG renderer is accessed by the engine or worker.

State entries are emitted in ascending component ID order so that tests and logs are stable.
The state value is passed across the thread boundary by value or by an immutable copy. It
contains no pointers into the engine.

## 10. Simulation log

The simulation log is an ordered event stream. It is intended for teaching and debugging, not
as a safety or compliance record.

Each event carries:

- `scan_index`;
- simulated time in milliseconds;
- severity (information, warning, fault);
- a stable English event code;
- the component, pin or wire IDs involved;
- optional arguments such as a timer preset or a contact group;
- a diagnostic message key, translated by the UI.

There are no wall-clock timestamps. Ordering is deterministic. The log is bounded by a ring
buffer; the exact capacity is a setting.

| Event | When |
|---|---|
| `SessionStarted` | A new session begins |
| `SessionPaused` / `SessionResumed` | Pause state changes |
| `ScanStepped` | A single step completes |
| `SessionReset` | Runtime state is reset |
| `SessionStopped` | The session ends |
| `InputChanged` | A level or latched input changes |
| `CoilPickedUp` / `CoilDroppedOut` | A contactor/relay coil changes state |
| `CoilPriorityArbitration` | First-wins resolves simultaneous interlocked coils |
| `TimerStarted` / `TimerDone` / `TimerReset` | Timer state transition |
| `EmergencyStopLatched` / `EmergencyStopReset` | Emergency-stop latch changes |
| `ThermalTripped` / `ThermalReset` | Thermal relay latch changes |
| `FuseBlown` / `BreakerOpened` / `BreakerClosed` | Manual protection state changes |
| `MotorStarted` / `MotorStopped` | Motor energization changes |
| `OscillationDetected` | Fixed point fails to converge within 16 passes |
| `SnapshotRestored` | A runtime snapshot is restored (planned) |
| `FaultRaised` / `FaultCleared` | A session fault state changes |

Only state transitions are logged; the engine does not log every net or every wire on every
scan. The log is cleared when a new session starts. A reset appends an event and keeps the
existing log.

## 11. Determinism guarantees

Given the same diagram, the same `cycle_ms`, the same sequence of per-scan input snapshots and
the same starting snapshot, the engine produces the same sequence of states and events.

The guarantees depend on:

- ascending `ComponentId` for coil evaluation and first-wins;
- ascending `PinId` for source-group allocation;
- ascending `WireId` and `ComponentId` for union operations;
- `std::map` or sorted `std::vector` for any iteration that affects behavior, logs or files;
- no pointer-order, hash-order or memory-address tie-breakers; the documented
  `ComponentId` creation order is the only creation-order effect;
- no wall-clock time in the engine;
- no randomness;
- no locale-dependent parsing or formatting in the engine or file format;
- integer simulated time;
- no floating-point comparison in connectivity or energization.

Real-time pacing is not deterministic. A busy host can delay scans, but it does not change the
logical scan sequence. Tests run a fixed number of scans and compare states and events.

First-wins is deterministic for a given document, but it is not a physical priority. ID
allocation follows creation order, so two otherwise identical diagrams can resolve a
pathological simultaneous start differently if their components were created in a different
order. Circuits should be designed so that this tie-break is not observable in normal
operation.

## 12. What is not simulated

This list is part of the contract, not a temporary disclaimer.

### Electrical

- Voltage magnitude, voltage levels, voltage drop and nominal voltage compatibility.
- Current, impedance, resistance, inductance, capacitance, power, power factor, phase angle
  and frequency response.
- Short-circuit current, arc flash, insulation failure, earth fault and protective
  coordination.
- Phase sequence, phase rotation, unbalanced supplies and multiple independent supply
  reconciliation.
- Transients, inrush current, back-EMF, harmonics, noise and electromagnetic compatibility.
- Wire sizing, cable ampacity, terminal resistance and source capacity.
- Loads in series. Each load must have a branch that reaches two distinct source groups.
- Current-based operation of fuses, breakers, thermal relays or any other protection.
- Heating, cooling, thermal capacity, motor thermal models and contactor coil burnout.

### Mechanical

- Torque, speed, acceleration, inertia, friction, mechanical load and braking.
- Motor direction and phase rotation.
- Contact bounce, arc duration, welded contacts and mechanical wear.
- Mechanical interlocks, springs, armatures and physical travel.
- Star-delta transition physics. Star-delta is only the contactor/timer/interlock logic that
  the user draws.

### Safety and certification

- Functional safety, SIL/PL, risk assessment, lockout/tagout, arc-flash boundaries and
  personnel protection.
- Compliance with NBR 5280, IEC 60617 or any other standard. The symbols and conventions are
  inspired by common practice; they are not a certified reproduction.
- Protection coordination, selectivity, breaking capacity or discrimination.
- Commissioning, verification or validation of a real installation.

### Other

- PLC program execution, analog signals, communication networks and SCADA.
- Panel layout, heat dissipation, enclosure sizing and mechanical clearances.
- Real-time guarantees, hardware-in-the-loop behavior and cycle jitter.
- Any physical process driven by the motor.

## 13. Educational status

FioVivo is a prototyping and teaching tool. It models the logical presence or absence of
energy in a command diagram. It is not a certified electrical design, protection
coordination or safety tool.

Do not use its results to design, verify, commission or maintain a real electrical
installation. Always follow local standards, manufacturer data and applicable law when
working on real equipment.
