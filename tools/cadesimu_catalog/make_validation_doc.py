import json, os, sys
sys.path.insert(0, os.path.dirname(__file__))
import generate

root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
types = {e.type: e for e in generate.CATALOG}
picks = [
    ("supply_ac3", (15, 20)), ("contactor_3p", (70, 20)), ("motor_3ph_6", (130, 20)),
    ("timer_on", (200, 20)), ("thermal_relay", (15, 110)), ("fuse_3p", (85, 110)),
    ("logic_and", (140, 110)), ("sensor_inductive_no", (195, 110)),
    ("pilot_lamp", (250, 110)), ("plc_s7_1200", (15, 180)),
]
components = []
cid = 0
pid = 0
for type_name, (x, y) in picks:
    e = types[type_name]
    cid += 1
    pins = []
    for pin in e.pins:
        pid += 1
        pins.append({"id": f"p{pid}", "name": pin.name, "role": pin.role,
                     "offset": [pin.x, pin.y], "required": pin.required})
    components.append({
        "id": f"c{cid}", "type": e.type, "tag": f"CAD{cid}", "description": e.name,
        "position": [x, y], "size": list(e.size), "rotation": 0, "pins": pins,
        "parameters": e.params,
        "labels": {"show_tag": True, "show_description": False,
                   "tag_offset": [0, -6], "description_offset": [0, 6]},
    })

doc = {
    "format": "dcj", "format_version": 1, "app_version": "0.1.0",
    "metadata": {"title": "CADe_SIMU validation", "author": "REA",
                 "created_utc": "2026-10-24T00:00:00Z",
                 "modified_utc": "2026-10-24T00:00:00Z", "revision": "A"},
    "sheets": [{
        "id": "s1", "name": "validation", "paper": "A3", "orientation": "landscape",
        "margin": 10, "show_frame": True, "workspace": [520, 360],
        "components": components, "wires": [], "annotations": [],
    }],
    "library_refs": [],
}
out = os.path.join(root, "docs", "cadesimu", "validation.dcj")
json.dump(doc, open(out, "w", encoding="utf-8"), indent=1, ensure_ascii=False)
print(out, len(components))
