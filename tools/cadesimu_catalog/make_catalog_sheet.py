import json, os, sys
sys.path.insert(0, os.path.dirname(__file__))
import generate

root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
out = os.path.join(root, "docs", "cadesimu", "catalog.dcj")

sheets = []
cid = 0
pid = 0
order = ["supplies", "protection", "contactors", "relays", "contacts", "control", "sensors",
         "timers", "indication", "machines", "power", "logic", "plc", "pneumatic", "terminals",
         "measurement", "grafcet", "annotations"]
by_cat = {}
for entry in generate.CATALOG:
    by_cat.setdefault(entry.category, []).append(entry)

sheet_index = 0
for cat in order:
    entries = by_cat.get(cat, [])
    if not entries:
        continue
    sheet_index += 1
    components = []
    for i, entry in enumerate(entries):
        cid += 1
        col = i % 6
        row = i // 6
        pins = []
        for pin in entry.pins:
            pid += 1
            pins.append({"id": f"p{pid}", "name": pin.name, "role": pin.role,
                         "offset": [pin.x, pin.y], "required": pin.required})
        components.append({
            "id": f"c{cid}", "type": entry.type, "tag": f"{entry.tag}{cid}",
            "description": "", "position": [40 + col * 95, 45 + row * 85],
            "size": list(entry.size), "rotation": 0, "pins": pins,
            "parameters": entry.params,
            "labels": {"show_tag": True, "show_description": False,
                       "tag_offset": [0, -6], "description_offset": [0, 6]},
        })
    sheets.append({
        "id": f"s{sheet_index}", "name": cat,
        "paper": "A1", "orientation": "landscape", "margin": 10, "show_frame": True,
        "workspace": [1300, 1000],
        "components": components, "wires": [], "annotations": [],
    })

doc = {
    "format": "dcj", "format_version": 1, "app_version": "0.1.0",
    "metadata": {"title": "CADe_SIMU catalog", "author": "REA",
                 "created_utc": "2026-10-24T00:00:00Z", "modified_utc": "2026-10-24T00:00:00Z",
                 "revision": "A"},
    "sheets": sheets, "library_refs": [],
}
with open(out, "w", encoding="utf-8") as f:
    json.dump(doc, f, indent=1, ensure_ascii=False)
print(out, len(generate.CATALOG), "components", len(sheets), "sheets")
