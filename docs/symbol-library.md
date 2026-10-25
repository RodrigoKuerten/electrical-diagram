# Symbol library

The library under [`assets/symbols/`](../assets/symbols) now contains **215 CADe_SIMU-style
component definitions** plus their state variants. Each type has one JSON manifest and one base
SVG; dynamic types also have preloaded state SVGs (`active`, `tripped`, `blown` or
`position_1..3`). The artwork is a vector reconstruction of the shipped CADe_SIMU 4.0 symbols,
using the same visual language: thin black IEC-style lines, small blue terminal labels, amber
active parts and red fault marks.

The complete type list, pin counts, states and behaviour is generated in
[`cadesimu/inventory.md`](cadesimu/inventory.md). The reverse-engineering evidence and the
remaining visual differences are documented in [`cadesimu/report.md`](cadesimu/report.md).

## Categories

| Category | Examples |
|---|---|
| `supplies` | L+N+PE, L1+L2+L3+N+PE, DC, battery, rectifier, transformer, 24 V supply |
| `protection` | fuse I/II/III/IIIN, fuse switch, breaker I/II/III/IIIN, RCD, motor breaker, thermal relay, surge arrester |
| `contactors` | contactor I/II/III/IIII, coil, bistable coil, auxiliary NO/NC/CO, integrated contactor |
| `relays` | relay coil, bistable coil, changeover contacts, integrated relay, solid-state relay/contactor |
| `contacts` | NO, NC, CO, timed ON/OFF contacts, thermal auxiliary contacts |
| `control` | pushbuttons, mushroom buttons, emergency stop, switches, selectors |
| `sensors` | inductive, capacitive, magnetic, photoelectric, pressure, vacuum, thermostat, float, reed, limit |
| `timers` | TON, TOF, ON/OFF, pulse, flasher, time switch, counters |
| `indication` | pilot lamp, flashing lamp, LED, buzzer, siren, bell, horn |
| `machines` | three-phase (3/6 terminals), Y/D, single-phase, Dahlander, wound rotor, two-speed, DC, stepper, linear actuator |
| `power` | resistors, autotransformer, soft starter, AC/DC drives, diode, rectifier, transformer, heater |
| `logic` | AND, OR, NOT, XOR, NAND, NOR, RS, edges, timer, counter, 7-segment, divider |
| `plc` | CPU, DI, DO, power supply, S7-1200, LOGO, ET-200, Arduino, HMI |
| `pneumatic` | cylinders, rotary actuator, gripper, vacuum, air motor, 3/2 and 5/2 valves, hose |
| `terminals` | round/square terminals, 2/3/4-way blocks, junction, cables, connectors |
| `measurement` | voltmeter, ammeter, wattmeter, multimeter (visual) |
| `grafcet` | initial step, step, transition, action, link, convergence, divergence (visual) |
| `annotations` | text label |

## Manifest schema

```json
{
  "symbol_version": 1,
  "type": "contactor_3p",
  "display_name": "Contactor III",
  "display_name_pt": "Contator III",
  "category": "contactors",
  "size": [42, 44],
  "pins": [
    { "name": "1", "role": "contact_common", "x": 10, "y": 4, "required": true },
    { "name": "2", "role": "contact_no", "x": 10, "y": 40, "required": true }
  ],
  "default_parameters": { "rated_voltage": 220, "label_group": "" },
  "parameter_schema": [
    { "key": "rated_voltage", "type": "number", "label": "Rated voltage",
      "label_pt": "Tensão nominal", "unit": "V", "min": 0 }
  ],
  "states": { "active": "contactor_3p_active.svg" },
  "element_ids": {}
}
```

- `size` and pin offsets are millimetres; the top-left corner is the origin.
- Pin names follow the CADe_SIMU terminal convention (`1/2`, `3/4`, `13/14`, `21/22`,
  `95/96`, `97/98`, `A1/A2`, `U1/V1/W1`, PLC `I0/Q0`, etc.).
- `parameter_schema` drives the properties panel. The generator infers labels, units and types
  for the defaults when a type does not define a hand-written schema.
- `states` maps a simulation state to a preloaded SVG. The simulator never parses SVG during a
  scan.

## State selection

| Device family | State key |
|---|---|
| Contactors, relays and auxiliary contacts | `active` when closed |
| Pushbuttons, switches, sensors, emergency stop | `active` when actuated/pressed |
| Fuses | `blown` |
| Breakers, RCDs, motor breakers, thermal relay | `tripped` |
| Selectors | `position_1`, `position_2`, `position_3` |
| Timers and flashers | `active` when the output is timed out |
| Coils, lamps, LEDs, buzzers, motors | `active` when energized/running |
| Logic gates | `active` when the output is true |

The canvas also draws the tag/status text and a highlight rectangle, so state is not conveyed
by colour alone.

## Legacy FioVivo types

The 20 original identifiers (`supply_ac3`, `supply_ac1`, `supply_dc`, `fuse`, `breaker`,
`contactor_coil`, `contactor_no`, `contactor_aux_no`, `contactor_aux_nc`, `pushbutton_no`,
`pushbutton_nc`, `emergency_stop`, `selector_switch`, `thermal_relay`, `timer_on`, `timer_off`,
`pilot_lamp`, `motor_3ph`, `terminal`, `label_box`) remain loadable. Their manifests preserve
the pin names and roles used by existing projects and by the five example files; their artwork
was replaced with the CADe_SIMU-style drawing. This is deliberate: it lets old `.dcj` files open
without migration while giving them the new visual language.

## Behaviours

- **Sources** create distinct source groups per conductor. PE never seeds a source.
- **Pass-through protection** connects its pole pairs while intact; a trip/blown state opens
  every pole.
- **Contactor/relay groups** share `label_group`; a NO contact closes and an NC contact opens
  when any coil in the group is energized. Multi-pole devices add one edge per pole.
- **Timed contacts** follow a timer with the same `label_group`; **thermal contacts** follow a
  thermal relay with the same group.
- **Digital inputs** (pushbuttons, switches, limit switches, sensors, float/pressure/thermostat)
  are actuated on the canvas. Momentary inputs release on mouse-up; maintained switches toggle.
- **Timers** support TON, TOF, ON/OFF, pulse and flasher behaviour.
- **Logic gates** evaluate their input pins against source groups and drive their output as a
  switched source, so they can energize relay/contactor coils.
- **Motors** are loads; a three-phase motor is considered running when the configured minimum
  number of phases is fed.
- **PLC, pneumatic, GRAFCET and measurement** symbols are present and interactive where
  applicable, but their advanced runtime models are not implemented; see the report.

## Adding a symbol

1. Add the entry to `tools/cadesimu_catalog/generate.py` (or a new template family).
2. Run `python tools/cadesimu_catalog/generate.py`; it writes the manifest, base SVG and state
   SVGs into `assets/symbols/`.
3. Add the type to the relevant unit test if it introduces a new behaviour family.
4. Rebuild so the asset copy step refreshes `build/<preset>/bin/assets/symbols/`.
