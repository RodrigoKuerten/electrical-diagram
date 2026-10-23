# FioVivo symbol library — Phase 1

Original, hand-drawn vector symbols for the FioVivo diagram editor. The artwork follows the
visual language of IEC 60617 and common Brazilian NBR-5280 command-diagram practice, but it is
an original interpretation. It is **not** a certified reproduction of either standard.

This directory contains the complete Phase 1 library: one JSON manifest per symbol type and the
SVG artwork referenced by each manifest. There are no scripts, external references, fonts,
stylesheets, raster images or embedded resources. Every file is UTF-8.

## File conventions

- Lowercase filenames. The base SVG is `<type>.svg`; state variants are
  `<type>_<state>.svg` (for example `contactor_no_active.svg`).
- One scene unit equals one millimetre.
- Every SVG root uses the same attributes:

  ```svg
  <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 W H" width="Wmm" height="Hmm"
       stroke="#1f2933" stroke-width="0.8" fill="none"
       stroke-linecap="round" stroke-linejoin="round">
  ```

- The background is transparent. No frame, paper colour or background rectangle is drawn.
- Line art uses `#1f2933` at `0.8` stroke width. Function labels use
  `font-family="sans-serif"`, `fill="#1f2933"` and `stroke="none"`.
- Terminal stubs are 2.5 units long and are drawn inward from each declared pin. Pins are drawn
  by the application, not by the SVG: the artwork does not add terminal dots, pin-name circles
  or pin labels. Labels explicitly required by the symbol definition (L1/L2/L3/N/PE, selector
  1–4, thermal 95–98, TON/TOF, motor M/3~) are drawn as plain text.
- Because several pins lie exactly on the viewBox boundary, the 2.5-unit terminal stubs touch
  that boundary by design. All other geometry is kept at least 1.5 units inside the viewBox.
- Coordinates follow the pin tables exactly. The nominal design grid is 10 mm, but pins and
  contact geometry use the exact coordinates from the manifest, which are not all multiples of
  10.
- Accent fills are limited to state indication: `#f2b134` for an energised coil or timer clock,
  `#c0392b` for the emergency-stop mushroom and the active red pushbutton, `#4caf50` for the
  active green pushbutton, `#f7d154` for a glowing lamp and `#f2d27a` for a running motor.
- Active/variant SVGs keep the base viewBox, physical size and pin coordinates.

## Manifest schema

Each `<type>.json` has these keys:

| Key | Type | Meaning |
|---|---|---|
| `symbol_version` | integer | Manifest schema version. Currently `1`. |
| `type` | string | Stable type identifier; matches the JSON/SVG filename stem. |
| `display_name` | string | English display name. |
| `display_name_pt` | string | Brazilian Portuguese display name. |
| `category` | string | One of `supplies`, `protection`, `contactors`, `control`, `timers`, `indication`, `machines`, `terminals`, `annotations`. |
| `size` | `[width, height]` | Symbol footprint in scene units (mm). |
| `pins` | array | Pin objects: `name`, `role`, `x`, `y`, `required`. |
| `default_parameters` | object | Initial parameter values. Every key also appears in the schema. |
| `parameter_schema` | array | Parameter descriptors (see below). |
| `states` | object | State key to variant SVG filename. Empty when there is no variant. |
| `element_ids` | object (optional) | Logical state name to a list of SVG element IDs. |

### Pin roles

Allowed values only:

`terminal`, `coil_input`, `coil_output`, `contact_common`, `contact_no`, `contact_nc`,
`supply_in`, `supply_out`, `neutral`, `earth`, `signal_in`, `signal_out`.

### Parameter schema

Each entry has `key`, `type`, `label` and `label_pt`. Optional keys are `unit`, `min`, `max` and
(for enums) `options`.

| Type | Additional fields |
|---|---|
| `bool` | – |
| `int` | `min`, `max` where meaningful |
| `number` | `unit`, `min`, `max` where meaningful |
| `string` | – |
| `enum` | `options` (array of strings) |

### States

The allowed state keys are `active`, `blown`, `tripped`, `position_1`, `position_2` and
`position_3`. A state that is not defined is omitted from `states`; the app falls back to the
base SVG. The base SVG always shows the unactuated/default state:

- NO contacts open; NC contacts closed.
- Emergency stop not triggered (NC closed), thermal relay not tripped (95–96 closed, 97–98 open).
- Fuse intact; breaker closed; contactor coil, lamp and motor de-energised.
- Timer output contact open.
- Selector in position 1 (blade on pin 2).
- Thermal relay base state: 95–96 closed, 97–98 open.

## Type / pin / state table

`R` = required, `O` = optional. Positions are `(x, y)` in mm.

| Type | Size | Category | Pins | States |
|---|---|---|---|---|
| `supply_ac3` | 40×60 | supplies | L1 `supply_in` (40,10) R<br>L2 `supply_in` (40,22) R<br>L3 `supply_in` (40,34) R<br>N `neutral` (40,46) O<br>PE `earth` (40,56) O | – |
| `supply_ac1` | 34×34 | supplies | L `supply_in` (34,10) R<br>N `neutral` (34,22) R<br>PE `earth` (34,30) O | – |
| `supply_dc` | 30×26 | supplies | + `supply_in` (30,8) R<br>- `supply_out` (30,18) R | – |
| `fuse` | 20×40 | protection | 1 `contact_common` (10,0) R<br>2 `contact_common` (10,40) R | `blown` → `fuse_blown.svg` |
| `breaker` | 24×44 | protection | 1 `contact_common` (12,0) R<br>2 `contact_common` (12,44) R | `tripped` → `breaker_tripped.svg` |
| `contactor_coil` | 40×30 | contactors | A1 `coil_input` (10,0) R<br>A2 `coil_output` (10,30) R | `active` → `contactor_coil_active.svg` |
| `contactor_no` | 30×40 | contactors | 1 `contact_common` (15,0) R<br>2 `contact_no` (15,40) R | `active` → `contactor_no_active.svg` |
| `contactor_aux_no` | 30×40 | contactors | 13 `contact_common` (15,0) R<br>14 `contact_no` (15,40) R | `active` → `contactor_aux_no_active.svg` |
| `contactor_aux_nc` | 30×40 | contactors | 21 `contact_common` (15,0) R<br>22 `contact_nc` (15,40) R | `active` → `contactor_aux_nc_active.svg` |
| `pushbutton_no` | 30×40 | control | 13 `contact_common` (15,0) R<br>14 `contact_no` (15,40) R | `active` → `pushbutton_no_active.svg` |
| `pushbutton_nc` | 30×40 | control | 21 `contact_common` (15,0) R<br>22 `contact_nc` (15,40) R | `active` → `pushbutton_nc_active.svg` |
| `emergency_stop` | 34×44 | control | 1 `contact_common` (17,0) R<br>2 `contact_nc` (17,44) R | `active` → `emergency_stop_active.svg` |
| `selector_switch` | 60×50 | control | 1 `contact_common` (0,25) R<br>2 `contact_no` (60,10) R<br>3 `contact_no` (60,25) R<br>4 `contact_no` (60,40) R | `position_1` → `selector_switch_position_1.svg`<br>`position_2` → `selector_switch_position_2.svg`<br>`position_3` → `selector_switch_position_3.svg` |
| `thermal_relay` | 60×60 | protection | 95 `contact_common` (0,15) R<br>96 `contact_nc` (60,15) R<br>97 `contact_common` (0,45) R<br>98 `contact_no` (60,45) R | `tripped` → `thermal_relay_tripped.svg` |
| `timer_on` | 70×50 | timers | A1 `coil_input` (0,15) R<br>A2 `coil_output` (0,35) R<br>15 `contact_common` (70,15) R<br>18 `contact_no` (70,35) R | `active` → `timer_on_active.svg` |
| `timer_off` | 70×50 | timers | A1 `coil_input` (0,15) R<br>A2 `coil_output` (0,35) R<br>15 `contact_common` (70,15) R<br>18 `contact_no` (70,35) R | `active` → `timer_off_active.svg` |
| `pilot_lamp` | 30×34 | indication | A1 `coil_input` (15,0) R<br>A2 `coil_output` (15,34) R | `active` → `pilot_lamp_active.svg` |
| `motor_3ph` | 56×56 | machines | U1 `supply_in` (0,14) R<br>V1 `supply_in` (0,28) R<br>W1 `supply_in` (0,42) R | `active` → `motor_3ph_active.svg` |
| `terminal` | 20×20 | terminals | 1 `terminal` (10,20) O | – |
| `label_box` | 60×20 | annotations | – | – |

## Parameter summary

| Type(s) | Parameters |
|---|---|
| `supply_ac3`, `supply_ac1` | `rated_voltage` (number, V), `frequency` (number, Hz), `neutral` (bool), `earth` (bool) |
| `supply_dc` | `voltage` (number, V) |
| `fuse` | `rated_current` (number, A), `blown` (bool, runtime state; default `false`) |
| `breaker` | `rated_current` (number, A), `poles` (int, 1–4), `tripped` (bool) |
| `contactor_coil` | `rated_voltage` (number, V), `label_group` (string) |
| `contactor_no`, `contactor_aux_no`, `contactor_aux_nc` | `label_group` (string) |
| `pushbutton_no`, `pushbutton_nc` | `color` (enum: green/red/black), `mode` (enum: momentary/maintained), `label_group` (string) |
| `emergency_stop` | `latching` (bool), `label_group` (string) |
| `selector_switch` | `positions` (int, 2–3), `mode` (enum: maintained/momentary), `position` (int, 1–3), `label_group` (string) |
| `thermal_relay` | `trip_current` (number, A), `reset_mode` (enum: manual/automatic), `label_group` (string) |
| `timer_on`, `timer_off` | `delay_ms` (int, ms, min 0), `time_base` (enum: ms/s), `label_group` (string) |
| `pilot_lamp` | `color` (enum: green/red/yellow/blue/white), `rated_voltage` (number, V) |
| `motor_3ph` | `power_kw` (number, kW), `rated_voltage` (number, V), `connection` (enum: star/delta), `min_live_phases` (int, 1–3) |
| `terminal` | `terminal_number` (string) |
| `label_box` | `text` (string), `font_size` (number, mm) |

## State artwork

- `fuse_blown.svg`: the fuse element is visibly interrupted inside the body.
- `breaker_tripped.svg`: the breaker blade is open at 30° and the trip square is retained.
- `contactor_*_active.svg`: the blade closes (NO) or opens (NC); the actuator link follows.
- `contactor_coil_active.svg`: the IEC coil rectangle is filled with `#f2b134`.
- `pushbutton_*_active.svg`: the round actuator is pressed toward the contact and filled with the
  default colour; NO closes, NC opens.
- `emergency_stop_active.svg`: the red mushroom is depressed 1.5 mm toward the contact and the
  NC contact is open.
- `selector_switch_position_1.svg`: blade on pin 2. `position_2`: blade on pin 3.
  `position_3`: blade on pin 4.
- `thermal_relay_tripped.svg`: 95–96 open, 97–98 closed.
- `timer_*_active.svg`: the 15–18 NO contact closes and the clock face is filled `#f2b134`.
- `pilot_lamp_active.svg`: lamp body filled `#f7d154`, with eight short glow rays.
- `motor_3ph_active.svg`: motor circle filled `#f2d27a`.

## Assumptions and notes

1. `supply_ac1` declares `neutral` and `earth` parameters in addition to the two defaults listed
   in the brief, so that every parameter in its schema has a default. Both default to `true`,
   matching the optional N/PE pins.
2. The selector's default `positions` is `2` and its default `position` is `1`, as specified.
   The `position_3` state and SVG exist for configurations where `positions` is `3`. The base
   `selector_switch.svg` intentionally shows position 1 and is identical to the
   `position_1` variant.
3. `contactor_coil` uses the required logical `element_ids` key `coil_energized` even though its
   `states` key is `active`. Applications should treat `coil_energized` as the logical alias of
   the active state. The element ID `coil_body` is present in both the base and active SVGs.
4. The `breaker` symbol is a single-line representation; its `poles` parameter is metadata for
   the application and does not change the artwork.
5. `label_box.svg` is intentionally empty and valid. The editor renders the `text` parameter in
   the UI; no border is drawn so exported diagrams do not get unwanted boxes.
6. The pushbutton base actuator is unfilled line art. The `color` parameter recolours the
   pressed actuator in the active variant (green NO, red NC by default). The emergency-stop
   mushroom is permanently red `#c0392b`, as specified.
7. The timer coil is always drawn as an IEC rectangle. TON/TOF semantics belong to the simulator;
   the symbols differ by their function label, and both use a separate 15–18 output contact.
8. All parameters have both English and Brazilian Portuguese labels. The application UI is
   expected to be Brazilian Portuguese by default.

## Inventory

- 20 JSON manifests.
- 37 SVG files (20 base symbols plus 17 state variants).
- 1 README (this file).
- Total: 58 files.

All JSON files parse with `ConvertFrom-Json`; all SVGs are well-formed XML; every state filename
resolves to an existing file; every `element_ids` entry resolves inside the appropriate SVG;
and every SVG viewBox equals its manifest `size`.
