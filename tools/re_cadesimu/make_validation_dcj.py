"""Create a validation sheet containing representative exact CADe symbols."""
import json
import os

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
ASSETS = os.path.join(ROOT, "assets", "symbols")

TYPES = [
    "fuse_1p", "fuse_3p", "breaker_2p", "rcd_4p", "contactor_3p",
    "contact_no", "contact_co", "contactor_aux_no", "contactor_coil_bistable", "timer_on_off",
    "mushroom_no", "switch_no", "sensor_inductive_no", "motor_3ph_6", "logic_and",
    "terminal_round", "supply_earth", "supply_battery", "power_supply", "diode",
    "resistor_1p", "vfd_ac_3p", "pneumatic_cylinder_single", "lamp_flashing", "bell",
    "horn", "siren", "display_7seg", "pressure_switch_no", "selector_2pos",
]

components = []
pin_counter = 1
for index, type_id in enumerate(TYPES):
    manifest = json.load(open(os.path.join(ASSETS, type_id + ".json"), encoding="utf-8"))
    size = manifest.get("size", [20.0, 20.0])
    col = index % 5
    row = index // 5
    position = [20.0 + col * 78.0, 22.0 + row * 62.0]
    pins = []
    for pin in manifest.get("pins", []):
        pins.append({
            "id": f"p{pin_counter}",
            "name": pin["name"],
            "offset": [pin.get("x", 0.0), pin.get("y", 0.0)],
            "required": bool(pin.get("required", False)),
            "role": pin.get("role", "terminal"),
        })
        pin_counter += 1
    components.append({
        "description": "",
        "id": f"c{index + 1}",
        "labels": {
            "description_offset": [0.0, 6.0],
            "show_description": False,
            "show_tag": True,
            "tag_offset": [0.0, -6.0],
        },
        "parameters": manifest.get("default_parameters", {}),
        "pins": pins,
        "position": position,
        "rotation": 0,
        "size": size,
        "tag": f"X{index + 1}",
        "type": type_id,
    })

document = {
    "app_version": "0.1.0",
    "format": "dcj",
    "format_version": 1,
    "library_refs": [{"symbol_version": 1, "type": t} for t in TYPES],
    "metadata": {
        "author": "FioVivo validation",
        "created_utc": "2026-10-25T00:00:00Z",
        "modified_utc": "2026-10-25T00:00:00Z",
        "revision": "A",
        "title": "CADe exact symbol validation",
    },
    "sheets": [{
        "annotations": [],
        "components": components,
        "id": "s1",
        "margin": 10.0,
        "name": "Validation",
        "orientation": "landscape",
        "paper": "A3",
        "show_frame": True,
        "wires": [],
    }],
}
path = os.path.join(ROOT, "docs", "cadesimu", "validation.dcj")
with open(path, "w", encoding="utf-8") as handle:
    json.dump(document, handle, ensure_ascii=False, indent=2)
print(path, len(components))
