import json, os, sys, collections
sys.path.insert(0, os.path.dirname(__file__))
import generate

root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
types = json.load(open(os.path.join(root, "docs", "cadesimu", "type_names.json"), encoding="utf-8"))

behavior = {
    "supplies": "Source groups (L/N/PE, AC/DC, transformer/rectifier)",
    "protection": "Pass-through poles; trip/blown opens the protected path",
    "contactors": "Coil plus group-controlled main/auxiliary contacts",
    "relays": "Coil plus group-controlled changeover contacts",
    "contacts": "NO/NC/CO contacts; timed and thermal variants follow their group",
    "control": "Momentary or maintained digital inputs; selector positions",
    "sensors": "Digital input (NO/NC) driven by the user or a linked actuator",
    "timers": "TON, TOF, ON/OFF, pulse and flasher timing",
    "indication": "Load energized from two distinct source groups",
    "machines": "Load; three-phase machines require the configured live phases",
    "power": "Pass-through, source or visual power stage",
    "logic": "Boolean gate with a switched output source",
    "plc": "Visual I/O block; no user PLC program is executed",
    "pneumatic": "Solenoid coil loads and visual pneumatic symbols; no pressure model",
    "terminals": "Connection points, terminals, cables and junctions",
    "measurement": "Visual instrument; high impedance, no measurement value",
    "grafcet": "Visual GRAFCET step/transition/action elements; no token engine",
    "annotations": "Text label",
}

counts = collections.Counter(e.category for e in generate.CATALOG)
lines = ["# CADe_SIMU component inventory", ""]
lines.append(f"Generated from the reconstructed catalog: **{len(generate.CATALOG)} component definitions**.")
lines.append("")
lines.append("## Counts by category")
lines.append("")
lines.append("| Category | Types | Electrical behaviour |")
lines.append("|---|---:|---|")
for cat in sorted(counts):
    lines.append(f"| {cat} | {counts[cat]} | {behavior.get(cat, 'See catalog')} |")
lines.append("")
lines.append("## Catalog")
lines.append("")
lines.append("| Type | English name | Portuguese name | Category | Pins | States | Behaviour |")
lines.append("|---|---|---|---|---:|---|---|")
for e in generate.CATALOG:
    states = ", ".join(e.states.keys()) if e.states else "—"
    lines.append(f"| `{e.type}` | {e.name} | {e.name_pt} | {e.category} | {len(e.pins)} | {states} | "
                 f"{behavior.get(e.category, '—')} |")
lines.append("")
lines.append("## Recovered CADe_SIMU type IDs")
lines.append("")
lines.append("The property dialog in the shipped executable enumerates the following internal type IDs. "
             "They are retained here as the evidence trail for the catalog; FioVivo uses stable "
             "string type identifiers instead of the original numeric IDs.")
lines.append("")
lines.append("| CADe_SIMU ID | Recovered name | FioVivo coverage |")
lines.append("|---|---|---|")
for item in types:
    name = item.get("resolved") or item["name"]
    if name.startswith("DAT_"):
        name = item["name"]
    if item["id"] < 1010:
        cov = "`motor_*` family"
    elif item["id"] < 1100:
        cov = "`vfd_*` / power family"
    elif item["id"] < 3000:
        cov = "`contactor_*` / `switch_power_*`"
    elif item["id"] < 4000:
        cov = "`supply_*` / `power_supply`"
    elif item["id"] < 5000:
        cov = "`terminal_*` / `cable_*` / `connector_*`"
    elif item["id"] < 6000:
        cov = "`fuse_*` / `fuse_switch_*` / `disconnector_*`"
    elif item["id"] < 7000:
        cov = "`breaker_*` / `rcd_*` / `thermal_relay` / `motor_breaker_3p`"
    elif item["id"] < 8000:
        cov = "`contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo`"
    elif item["id"] < 9000:
        cov = "`pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*`"
    elif item["id"] < 10000:
        cov = "`*_coil` / `timer_*` / `pilot_lamp` / `buzzer` / `siren` / `bell` / `horn`"
    elif item["id"] < 10100:
        cov = "`logic_*` / `contact_*`"
    else:
        cov = "`grafcet_*`"
    lines.append(f"| {item['id']} (0x{item['id']:x}) | {name} | {cov} |")
lines.append("")
lines.append("## Evidence")
lines.append("")
lines.append("- `docs/cadesimu/type_names.json` — 159 type IDs and names recovered from the property-dialog dispatcher.")
lines.append("- `docs/cadesimu/resources/` — 715 PE resources, including 316 bitmap palette assets and 23 MFC toolbars.")
lines.append("- `docs/cadesimu/symbols/` — 162 insertion captures from the running CADe_SIMU 4.0 application.")
lines.append("- `docs/cadesimu/decompiled/` — Ghidra decompilation of the property/behaviour dispatcher and related functions.")
lines.append("- `docs/cadesimu/visual-comparison/` — side-by-side comparisons of representative symbols.")
lines.append("")

open(os.path.join(root, "docs", "cadesimu", "inventory.md"), "w", encoding="utf-8").write("\n".join(lines))
print("wrote inventory.md", len(generate.CATALOG), "types", len(types), "recovered IDs")
