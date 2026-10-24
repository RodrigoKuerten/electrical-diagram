# Symbol library

The library is stored under [`assets/symbols/`](../assets/symbols). Every type has one JSON
manifest and one base SVG; types whose drawing changes during simulation also have preloaded
state SVGs. The artwork is original, drawn with simple vector primitives, and follows the
conventions commonly used in IEC 60617 and Brazilian NBR 5280-style command diagrams. It is
not a certified reproduction of either standard.

The manifests are the source of truth for pin names, roles, default parameters and sizes.
`tests/unit/library/test_symbols.cpp` verifies that all 20 required types load, that every
state file exists, that the pin roles match the product specification and that a component
can be instantiated with fresh IDs.

## Required components

All coordinates are millimetres. The manifest contains the exact pin offsets.

| Type | Size | Pins (name : role) | Main parameters | State variants |
|---|---:|---|---|---|
| `supply_ac3` | 40×60 | L1, L2, L3 : supply_in; N : neutral; PE : earth | rated_voltage, frequency, neutral, earth | — |
| `supply_ac1` | 34×34 | L : supply_in; N : neutral; PE : earth | rated_voltage, frequency, neutral, earth | — |
| `supply_dc` | 30×26 | + : supply_in; − : supply_out | voltage | — |
| `fuse` | 20×40 | 1, 2 : contact_common | rated_current, blown | `blown` |
| `breaker` | 24×44 | 1, 2 : contact_common | rated_current, poles, tripped | `tripped` |
| `contactor_coil` | 40×30 | A1 : coil_input; A2 : coil_output | rated_voltage, label_group | `active` |
| `contactor_no` | 30×40 | 1 : contact_common; 2 : contact_no | label_group | `active` |
| `contactor_aux_no` | 30×40 | 13 : contact_common; 14 : contact_no | label_group | `active` |
| `contactor_aux_nc` | 30×40 | 21 : contact_common; 22 : contact_nc | label_group | `active` |
| `pushbutton_no` | 30×40 | 13 : contact_common; 14 : contact_no | color, mode, label_group | `active` |
| `pushbutton_nc` | 30×40 | 21 : contact_common; 22 : contact_nc | color, mode, label_group | `active` |
| `emergency_stop` | 34×44 | 1 : contact_common; 2 : contact_nc | latching, label_group | `active` |
| `selector_switch` | 60×50 | 1 : contact_common; 2, 3, 4 : contact_no | positions, mode, position, label_group | `position_1`, `position_2`, `position_3` |
| `thermal_relay` | 60×60 | 95, 97 : contact_common; 96 : contact_nc; 98 : contact_no | trip_current, reset_mode, label_group | `tripped` |
| `timer_on` | 70×50 | A1 : coil_input; A2 : coil_output; 15 : contact_common; 18 : contact_no | delay_ms, time_base, label_group | `active` |
| `timer_off` | 70×50 | A1 : coil_input; A2 : coil_output; 15 : contact_common; 18 : contact_no | delay_ms, time_base, label_group | `active` |
| `pilot_lamp` | 30×34 | A1 : coil_input; A2 : coil_output | color, rated_voltage | `active` |
| `motor_3ph` | 56×56 | U1, V1, W1 : supply_in | power_kw, rated_voltage, connection, min_live_phases | `active` |
| `terminal` | 20×20 | 1 : terminal | terminal_number | — |
| `label_box` | 60×20 | none | text, font_size | — |

### Convention notes

- NO contacts are drawn open at rest; NC contacts are drawn closed with a short crossing
  blade. Auxiliary NO uses 13/14 and auxiliary NC uses 21/22; main contacts use 1/2.
- A contactor coil is drawn as an IEC rectangle. The active SVG fills the coil; the
  simulation also adds a colored outline and an `ON` text marker.
- Thermal relay contacts are drawn as two independent sets: 95/96 NC and 97/98 NO. The
  bimetal heater is a visual symbol; heating is not simulated.
- The emergency stop is a latching mushroom actuator with an NC contact. Triggering opens
  the contact; only an explicit reset closes it again.
- The selector maps position 1 to pin 2, position 2 to pin 3 and position 3 to pin 4. The
  common is pin 1. The default is two positions plus the base symbol.
- The motor has three supply terminals only. Winding ends for a real star-delta connection
  are not modelled; star-delta behavior is expressed by contactor control logic.
- PE never seeds a simulation source group and is never used as a return path.
- Fuses and breakers are single-line symbols; the `poles` parameter on a breaker is metadata,
  not a multi-pole drawing.

## Manifest schema

```json
{
  "symbol_version": 1,
  "type": "contactor_coil",
  "display_name": "Contactor coil",
  "display_name_pt": "Bobina de contator",
  "category": "contactors",
  "size": [40, 30],
  "pins": [
    { "name": "A1", "role": "coil_input", "x": 10, "y": 0, "required": true },
    { "name": "A2", "role": "coil_output", "x": 10, "y": 30, "required": true }
  ],
  "default_parameters": { "rated_voltage": 220, "label_group": "" },
  "parameter_schema": [
    {
      "key": "rated_voltage",
      "type": "number",
      "label": "Rated voltage",
      "label_pt": "Tensão nominal",
      "unit": "V",
      "min": 0
    }
  ],
  "states": { "active": "contactor_coil_active.svg" },
  "element_ids": { "coil_energized": ["coil_body"] }
}
```

- `symbol_version` is `1`. A future incompatible manifest bump must be handled by the loader.
- `size` is the unrotated symbol box in millimetres.
- Pin offsets use the top-left corner as origin. `required` controls the validation warning for
  unconnected pins.
- `parameter_schema` drives the properties form. Supported kinds are `bool`, `int`, `number`,
  `string` and `enum`; enum options are listed in `options`. Both English and Portuguese
  labels are required for a complete UI.
- `states` maps a logical state key to a preloaded SVG. The simulation selects a state; it
  never parses SVG during a scan.
- `element_ids` documents logical SVG element names for future symbol editing; the current
  renderer swaps whole preloaded documents.

## Rendering

`SymbolSvgRenderer` parses the base SVG and every state SVG once when the library is loaded.
The canvas paints the selected state with `QSvgRenderer`. Active-state selection:

| Device | State key |
|---|---|
| Contactor contacts | `active` when closed |
| Pushbuttons / emergency stop | `active` when actuated |
| Fuse | `blown` when blown |
| Breaker / thermal relay | `tripped` when tripped |
| Selector | `position_<n>` |
| Timer | `active` when its output is timed out |
| Coil / lamp / motor | `active` when energized/running |

The base SVG is used for every other state. The canvas also paints a state badge and outline
so state is never communicated by color alone.

## Adding a symbol

1. Add `assets/symbols/<type>.svg` (and active variants if needed) using the 0.8 stroke and
   transparent-background conventions.
2. Add `<type>.json` with pins, roles, defaults, parameter schema and state mapping.
3. Add the type to the required-component test if it is a Phase 1 deliverable.
4. Use it in an example only after the manifest tests pass.
5. Do not edit an existing symbol without bumping `symbol_version` if pin positions or state
   keys change in an incompatible way.

Phase 4 symbol editing is not implemented. The manifests and `element_ids` are structured so
that an editor can be added without changing the simulation model.
