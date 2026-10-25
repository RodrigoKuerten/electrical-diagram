"""Replace hand-drawn symbol geometry with exact GDI-traced CADe_SIMU geometry.

For every existing non-legacy type that has a matching GDI trace this script:
  * rebuilds the SVG from the recorded GDI primitives (exact coordinates),
  * resizes the component to the real CADe size (672 px / 297 mm),
  * recalculates pin offsets from the real terminal endpoints,
  * keeps the type id, display names, category, parameters and schema,
  * regenerates active/tripped state variants from the exact drawing.

The 20 legacy types used by the example diagrams are deliberately untouched.
"""
from __future__ import annotations

import json
import html
import os
import re
import shutil
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import make_svg_from_gdi as gdi

ASSETS = os.path.join(ROOT, "assets", "symbols")
BACKUP = os.path.join(ROOT, "docs", "cadesimu", "handdrawn")
MAPPING = os.path.join(ROOT, "docs", "cadesimu", "type_to_cmd.json")
SCALE = gdi.SCALE
INK = "#1f2933"
ACTIVE = "#f2b134"
DANGER = "#c0392b"
LABEL = "#1f5f9e"

LEGACY = {
    "supply_ac3", "supply_ac1", "supply_dc", "fuse", "breaker", "contactor_coil",
    "contactor_no", "contactor_aux_no", "contactor_aux_nc", "pushbutton_no",
    "pushbutton_nc", "emergency_stop", "selector_switch", "thermal_relay",
    "timer_on", "timer_off", "pilot_lamp", "motor_3ph", "terminal", "label_box",
}

TERMINAL_TYPES = ("terminal_", "connector_", "junction", "cable_", "hose")
SUPPLY_TYPES = ("supply_",)
PROTECTION_TYPES = ("fuse_", "breaker_", "disconnector_", "rcd_", "surge_",
                    "motor_breaker", "overvoltage")
CONTACT_TYPES = ("contactor_", "contact_", "relay_contact_")
SWITCH_TYPES = ("pushbutton_", "switch_", "limit_", "sensor_", "pressure_",
                "vacuum_", "thermostat_", "mushroom_", "selector_")
COIL_TYPES = ("coil_", "timer_", "flasher", "solenoid_", "contactor_coil")
MOTOR_TYPES = ("motor_", "stepper_")
LOGIC_TYPES = ("logic_",)
LOAD_TYPES = ("lamp_", "bell", "horn", "siren", "buzzer")

# Terminal blocks are drawn as bare circles/leads without text labels.  The automatic label
# matcher cannot recover their connection points, so use the exact lead ends recorded by GDI.
MANUAL_PINS = {
    "terminal_round": [("1", 450, 300)],
    "terminal_3": [("1", 450, 300), ("2", 462, 300), ("3", 474, 300)],
    "terminal_4": [("1", 450, 300), ("2", 462, 300), ("3", 474, 300), ("4", 486, 300)],
    "hose_entry": [("1", 450, 300), ("2", 474, 300)],
}


def starts_any(type_id: str, prefixes) -> bool:
    return any(type_id.startswith(prefix) or type_id == prefix for prefix in prefixes)


def role_for(type_id: str, name: str, old_roles: dict[str, str]) -> str:
    if starts_any(type_id, COIL_TYPES):
        if name in ("A2", "B2", "X2", "2") or name.endswith("2"):
            return "coil_output"
        return "coil_input"
    if starts_any(type_id, MOTOR_TYPES):
        if name == "PE":
            return "earth"
        if name in ("U1", "V1", "W1"):
            return "supply_in"
        return "terminal"
    if starts_any(type_id, LOGIC_TYPES):
        if name in ("OUT", "Q"):
            return "signal_out"
        return "signal_in"
    if starts_any(type_id, SUPPLY_TYPES):
        if name == "PE":
            return "earth"
        if name == "N":
            return "neutral"
        if name == "-":
            return "supply_out"
        return "supply_in"
    if type_id in ("bell", "horn", "siren", "buzzer") or type_id.startswith("lamp_"):
        return "terminal"
    if starts_any(type_id, PROTECTION_TYPES):
        return "terminal"
    if starts_any(type_id, CONTACT_TYPES):
        if name in ("A1",):
            return "coil_input"
        if name in ("A2",):
            return "coil_output"
        if name in ("13", "21", "11", "1", "3", "5", "7"):
            return "contact_common"
        if name in ("14", "12", "22", "2", "4", "6", "8"):
            return "contact_nc" if type_id.endswith(("_nc", "_nf")) else "contact_no"
        return "terminal"
    if starts_any(type_id, SWITCH_TYPES):
        if name in ("11", "13", "21", "1"):
            return "contact_common"
        if type_id.endswith(("_nc", "_nf")):
            return "contact_nc"
        if type_id.endswith(("_co", "_com", "_con")):
            if name in ("12", "22"):
                return "contact_nc"
            if name in ("14",):
                return "contact_no"
            return "contact_no"
        return "contact_no"
    if starts_any(type_id, TERMINAL_TYPES):
        return "terminal"
    if name in old_roles:
        return old_roles[name]
    return "terminal"


def logic_pin_names(type_id: str, pins: list[tuple[str, int, int]]) -> list[tuple[str, int, int]]:
    """Give logic gates the semantic pin names the simulator expects."""
    if not pins:
        return pins
    ordered = sorted(pins, key=lambda p: (p[2], p[1]))
    names: list[str]
    if type_id == "logic_sr":
        names = ["S", "R", "OUT", "Q"]
    elif type_id in ("logic_counter", "logic_timer_on"):
        names = ["IN1", "IN2", "OUT", "RST"]
    else:
        if len(ordered) <= 2:
            names = ["IN1", "OUT"]
        else:
            names = ["IN1", "IN2"] + [f"OUT{i}" if i > 2 else "OUT" for i in range(2, len(ordered))]
            names[2] = "OUT"
    return [(names[i] if i < len(names) else f"P{i+1}", p[1], p[2])
            for i, p in enumerate(ordered)]


def build_svg(items, x0, y0, pin_names: set[str]) -> str:
    w = (max(
        [x.get("x", 0) for x in items if x["fn"] in ("MoveToEx", "LineTo")] +
        [x.get("r", 0) for x in items if x["fn"] == "Rectangle"] +
        [x.get("x", 0) for x in items if x["fn"].startswith("TextOut")] +
        [x0]
    ) - x0)
    # Use the bounding box already computed by the caller for size; here we only need the viewBox.
    return ""


def convert(type_id: str, command: int, old: dict, name: str) -> dict | None:
    clean = os.path.join(ROOT, "docs", "cadesimu", "gdi_clean")
    before_path = next((os.path.join(clean, f) for f in os.listdir(clean)
                        if f.startswith(f"{command}_") and f.endswith("_before.json")), None) if os.path.isdir(clean) else None
    after_path = next((os.path.join(clean, f) for f in os.listdir(clean)
                       if f.startswith(f"{command}_") and f.endswith("_after.json")), None) if os.path.isdir(clean) else None
    if before_path and after_path:
        before = json.load(open(before_path, encoding="utf-8"))
        after = json.load(open(after_path, encoding="utf-8"))
        items = gdi.extras_pair(before, after)
        clean_source = True
        trace_path = after_path
    else:
        base = json.load(open(os.path.join(gdi.GDI, "baseline.json"), encoding="utf-8"))
        trace_path = None
        for candidate in os.listdir(gdi.GDI):
            if candidate.startswith(f"{command}_") and candidate.endswith(".json"):
                trace_path = os.path.join(gdi.GDI, candidate)
                break
        if trace_path is None:
            return None
        run = json.load(open(trace_path, encoding="utf-8"))
        items = gdi.extras(base, run)
        clean_source = False
    items = [x for x in items if not (x["fn"].startswith("TextOut") and
                                      (gdi.is_tag(x.get("text", "")) or not gdi.clean_text(x.get("text", ""))))]

    def near(x):
        if x["fn"] in ("Ellipse", "Rectangle"):
            cx = (x["l"] + x["r"]) / 2
            cy = (x["t"] + x["b"]) / 2
        elif x["fn"] in ("Polyline", "Polygon"):
            pts = x.get("points", [])
            if not pts:
                return False
            cx = sum(p[0] for p in pts) / len(pts)
            cy = sum(p[1] for p in pts) / len(pts)
        else:
            cx = x.get("x", 0)
            cy = x.get("y", 0)
        return 300 <= cx <= 650 and 230 <= cy <= 425

    items = [x for x in items if near(x)]
    if not items:
        return None
    x0, y0, x1, y1 = gdi.bounds(items)
    w = x1 - x0
    h = y1 - y0
    if w < 2 or h < 2:
        return None
    # Pin extraction (same improved algorithm as make_svg_from_gdi).
    endpoints = []
    for x in items:
        if x["fn"] == "MoveToEx":
            endpoints.append((x["x"], x["y"]))
        elif x["fn"] == "LineTo":
            endpoints.append((x["x"], x["y"]))
    pin_re = re.compile(r"^(?:[A-Z]{1,3}\d*|\d{1,2}[A-Z]?|[A-Z]\d?|PE|N|L\d?|[+-])$")
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
    raw_pins = []
    used = set()
    texts = [x for x in items if x["fn"].startswith("TextOut")]
    texts.sort(key=lambda x: (-len(gdi.clean_text(x.get("text", ""))), x.get("y", 0), x.get("x", 0)))
    for x in texts:
        t = gdi.clean_text(x.get("text", ""))
        if not pin_re.match(t) or t in ("M", "F", "P", "S", "C", "SR", "TON", "R", "0"):
            continue
        endpoint = gdi.outermost_endpoint(x, endpoints, (cx, cy))
        if endpoint is None or endpoint in used:
            continue
        used.add(endpoint)
        raw_pins.append((t, endpoint[0], endpoint[1]))
    if starts_any(type_id, LOGIC_TYPES) and raw_pins:
        raw_pins = logic_pin_names(type_id, raw_pins)
    if type_id in MANUAL_PINS:
        raw_pins = MANUAL_PINS[type_id]
    # If the traced symbol has no textual pin labels (terminals, some supplies), fall back
    # to the old pin names and place them at the nearest outer endpoints.
    old_pins = old.get("pins", [])
    old_roles = {p["name"]: p.get("role", "terminal") for p in old_pins}
    if not raw_pins and old_pins:
        # Keep the old names and map their positions proportionally into the exact box.
        old_w = float(old.get("size", [max(1.0, w / SCALE), max(1.0, h / SCALE)])[0])
        old_h = float(old.get("size", [max(1.0, w / SCALE), max(1.0, h / SCALE)])[1])
        raw_pins = [(p["name"], x0 + float(p.get("x", 0)) / old_w * w if old_w else x0,
                     y0 + float(p.get("y", 0)) / old_h * h if old_h else y0) for p in old_pins]
    if not raw_pins:
        return None
    if starts_any(type_id, LOGIC_TYPES) and len(raw_pins) < 2:
        return None
    if starts_any(type_id, SUPPLY_TYPES) and len(raw_pins) < 2:
        # Composite supplies were not captured as one insertion; keep the old drawing.
        return None
    if not starts_any(type_id, TERMINAL_TYPES) and len(raw_pins) < 2:
        return None

    # Build the SVG with the same primitive mapping as the previewer.
    body = []
    pending = None

    def flush():
        nonlocal pending
        if pending is not None:
            body.append(pending + f'" stroke="{INK}" stroke-width="1.1" fill="none"/>')
            pending = None

    for x in items:
        fn = x["fn"]
        if fn == "MoveToEx":
            flush()
            pending = f'<path d="M {x["x"]-x0} {x["y"]-y0}'
        elif fn == "LineTo":
            if pending is None:
                pending = f'<path d="M {x["x"]-x0} {x["y"]-y0}'
            pending += f' L {x["x"]-x0} {x["y"]-y0}'
        elif fn in ("Ellipse", "Rectangle"):
            flush()
            if fn == "Ellipse":
                ccx = (x["l"] + x["r"]) / 2 - x0
                ccy = (x["t"] + x["b"]) / 2 - y0
                rx = (x["r"] - x["l"]) / 2
                ry = (x["b"] - x["t"]) / 2
                body.append(f'<ellipse cx="{ccx:.2f}" cy="{ccy:.2f}" rx="{rx:.2f}" ry="{ry:.2f}" '
                            f'stroke="{INK}" stroke-width="1.1" fill="none"/>')
            else:
                body.append(f'<rect x="{x["l"]-x0}" y="{x["t"]-y0}" width="{x["r"]-x["l"]}" '
                            f'height="{x["b"]-x["t"]}" stroke="{INK}" stroke-width="1.1" fill="none"/>')
        elif fn in ("Polyline", "Polygon"):
            flush()
            pts = " ".join(f'{px-x0},{py-y0}' for px, py in x.get("points", []))
            tag = "polygon" if fn == "Polygon" else "polyline"
            body.append(f'<{tag} points="{pts}" stroke="{INK}" stroke-width="1.1" fill="none"/>')
        elif fn.startswith("TextOut"):
            flush()
            t = gdi.clean_text(x.get("text", ""))
            if not t:
                continue
            is_pin = any(p[0] == t for p in raw_pins)
            color = LABEL if is_pin else INK
            body.append(f'<text x="{x["x"]-x0}" y="{x["y"]-y0}" fill="{color}" '
                        f'font-family="Arial" font-size="6">{html.escape(t)}</text>')
    flush()

    svg = (f'<?xml version="1.0" encoding="UTF-8"?>\n'
           f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {w:.2f} {h:.2f}" '
           f'width="{w/SCALE:g}mm" height="{h/SCALE:g}mm">\n' + "\n".join(body) + "\n</svg>\n")

    size_w = w / SCALE
    size_h = h / SCALE
    pins = []
    for name, px, py in raw_pins:
        pins.append({
            "name": str(name),
            "role": role_for(type_id, str(name), old_roles),
            "x": round((px - x0) / SCALE, 3),
            "y": round((py - y0) / SCALE, 3),
            "required": False,
        })
    # Drop exact duplicate names, keeping the first.
    unique = []
    seen_names = set()
    for pin in pins:
        if pin["name"] in seen_names:
            continue
        seen_names.add(pin["name"])
        pin["x"] = round(min(max(0.0, pin["x"]), size_w), 3)
        pin["y"] = round(min(max(0.0, pin["y"]), size_h), 3)
        unique.append(pin)
    pins = unique
    if not pins:
        return None
    return {
        "svg": svg,
        "size": [round(size_w, 3), round(size_h, 3)],
        "pins": pins,
        "source": trace_path,
        "clean": clean_source,
    }


def write_states(type_id: str, manifest: dict, svg_text: str) -> None:
    for state, filename in list(manifest.get("states", {}).items()):
        if state == "active":
            color = ACTIVE
        elif state in ("tripped", "blown", "fault"):
            color = DANGER
        else:
            color = ACTIVE
        state_svg = svg_text.replace(f'stroke="{INK}"', f'stroke="{color}"')
        with open(os.path.join(ASSETS, filename), "w", encoding="utf-8") as handle:
            handle.write(state_svg)


def main() -> None:
    mapping = json.load(open(MAPPING, encoding="utf-8"))
    os.makedirs(BACKUP, exist_ok=True)
    converted = 0
    skipped = []
    for type_id, entry in sorted(mapping.items()):
        manifest_path = os.path.join(ASSETS, type_id + ".json")
        if not os.path.isfile(manifest_path):
            continue
        old = json.load(open(manifest_path, encoding="utf-8"))
        if old.get("legacy") or type_id in LEGACY:
            continue
        result = convert(type_id, int(entry["command"]), old, entry["name"])
        if result is None:
            # A trace that cannot produce a complete symbol must not remain exposed from an
            # earlier pass.  Keep the file loadable for old documents but hide it.
            old["geometry_clean"] = False
            old["category"] = "legacy"
            old["library_hidden"] = True
            with open(manifest_path, "w", encoding="utf-8") as handle:
                json.dump(old, handle, ensure_ascii=False, indent=2)
                handle.write("\n")
            skipped.append((type_id, "sem geometria/pinos suficientes"))
            continue
        # Keep a copy of the old hand-drawn assets before replacing them.
        backup_dir = os.path.join(BACKUP, type_id)
        os.makedirs(backup_dir, exist_ok=True)
        if not os.path.isfile(os.path.join(backup_dir, type_id + ".json")):
            shutil.copy2(manifest_path, os.path.join(backup_dir, type_id + ".json"))
            old_svg = os.path.join(ASSETS, type_id + ".svg")
            if os.path.isfile(old_svg):
                shutil.copy2(old_svg, os.path.join(backup_dir, type_id + ".svg"))
        old["size"] = result["size"]
        old["pins"] = result["pins"]
        old["geometry_source"] = "cadesimu-gdi"
        old["geometry_clean"] = bool(result.get("clean"))
        old["geometry_command"] = entry["command"]
        with open(manifest_path, "w", encoding="utf-8") as handle:
            json.dump(old, handle, ensure_ascii=False, indent=2)
            handle.write("\n")
        with open(os.path.join(ASSETS, type_id + ".svg"), "w", encoding="utf-8") as handle:
            handle.write(result["svg"])
        write_states(type_id, old, result["svg"])
        converted += 1
        print(f"OK   {type_id:30s} <- {entry['command']} {result['size']} pins={len(result['pins'])}")
    for type_id, reason in skipped:
        print(f"SKIP {type_id:30s} {reason}")
    print(f"\nconverted {converted}, skipped {len(skipped)}")


if __name__ == "__main__":
    main()
