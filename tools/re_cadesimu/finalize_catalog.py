"""Finalise which symbols are shown in the library.

Only symbols rebuilt from a clean, isolated GDI trace are exposed.  Types used by the
existing example diagrams stay loadable but move to the hidden ``legacy`` category so the
library never shows a hand-drawn approximation next to the exact CADe artwork.
"""
from __future__ import annotations

import glob
import json
import os

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
ASSETS = os.path.join(ROOT, "assets", "symbols")

LEGACY = {
    "supply_ac3", "supply_ac1", "supply_dc", "fuse", "breaker", "contactor_coil",
    "contactor_no", "contactor_aux_no", "contactor_aux_nc", "pushbutton_no",
    "pushbutton_nc", "emergency_stop", "selector_switch", "thermal_relay",
    "timer_on", "timer_off", "pilot_lamp", "motor_3ph", "terminal", "label_box",
}

# Traces that were rebuilt from a source command that does not correspond to this symbol,
# or where the source command inserted an incomplete/misleading drawing.  They remain
# loadable for old documents but are not offered in the library.
HIDE_TYPES = {
    "buzzer",
    "counter", "logic_counter",
    "grafcet_step", "grafcet_step_initial", "grafcet_transition", "grafcet_action",
    "grafcet_link", "grafcet_convergence", "grafcet_divergence",
    "junction",
    "motor_3ph_1ph", "motor_wound_rotor", "motor_3ph_yd", "motor_2speed",
    "terminal_2", "terminal_square",
    "switch_power_1p",
    "transformer_3p", "supply_transformer3",
    "supply_ac1_pe", "supply_ac3_n", "supply_ac3_n_pe", "supply_ac3_pe",
}


def main() -> None:
    visible = []
    hidden = []
    for path in sorted(glob.glob(os.path.join(ASSETS, "*.json"))):
        data = json.load(open(path, encoding="utf-8"))
        type_id = data.get("type", "")
        if not type_id:
            continue
        hide = (type_id in LEGACY or type_id in HIDE_TYPES or not data.get("geometry_clean") or
                float(data.get("size", [0, 0])[0]) <= 0.05 or float(data.get("size", [0, 0])[1]) <= 0.05)
        if hide:
            data["category"] = "legacy"
            data["library_hidden"] = True
            hidden.append(type_id)
        else:
            data.pop("library_hidden", None)
            visible.append(type_id)
        with open(path, "w", encoding="utf-8") as handle:
            json.dump(data, handle, ensure_ascii=False, indent=2)
            handle.write("\n")
    print(f"visible {len(visible)} hidden {len(hidden)}")
    print("visible types:")
    print(" ".join(sorted(visible)))


if __name__ == "__main__":
    main()
