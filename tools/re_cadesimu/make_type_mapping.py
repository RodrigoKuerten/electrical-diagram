"""Build a mapping from existing FioVivo type ids to exact GDI-traced CADe commands.

The existing catalog was derived from the same CADe toolbar, so most display names match.
A curated override table handles the names where the automatic translation is ambiguous.
"""
from __future__ import annotations
import difflib
import glob
import json
import os
import re
import sys
import unicodedata

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import make_svg_from_gdi as gdi
ASSETS = os.path.join(ROOT, "assets", "symbols")
GDI = os.path.join(ROOT, "docs", "cadesimu", "gdi")
OUT = os.path.join(ROOT, "docs", "cadesimu", "type_to_cmd.json")

# Types that must stay compatible with existing examples/tests. They are hidden from the library.
LEGACY = {
    "supply_ac3", "supply_ac1", "supply_dc", "fuse", "breaker", "contactor_coil",
    "contactor_no", "contactor_aux_no", "contactor_aux_nc", "pushbutton_no",
    "pushbutton_nc", "emergency_stop", "selector_switch", "thermal_relay",
    "timer_on", "timer_off", "pilot_lamp", "motor_3ph", "terminal", "label_box",
}

# Curated command choices. Only commands whose GDI trace produced real geometry are useful.
OVERRIDES = {
    # supplies / sources
    "supply_ac3_n": 32870, "supply_ac3_n_pe": 32883, "supply_ac3_pe": 32883,
    "supply_ac1_pe": 32873, "supply_neutral": 32875, "supply_earth": 33043,
    "supply_dc_pos": 32877, "supply_battery": 33047, "supply_transformer3": 33046,
    "supply_rectifier": 32998, "power_supply": 32999,
    # fuses / disconnectors
    "fuse_1p": 32854, "fuse_2p": 32840, "fuse_3p": 32841, "fuse_3pn": 32842,
    "fuse_switch_3pn": 32843, "fuse_switch_1pn": 32844, "fuse_switch_2p": 32845,
    "fuse_switch_3p": 32845,
    "disconnector_1p": 32899, "disconnector_1pn": 32860, "disconnector_2p": 32867,
    "disconnector_3p": 33392, "disconnector_3pn": 32863,
    # breakers / RCD / protection
    "breaker_1p": 32899, "breaker_1pn": 32860, "breaker_2p": 32867,
    "breaker_3p": 32868, "breaker_3pn": 32863,
    "rcd_2p": 32864, "rcd_4p": 32865,
    "surge_1pn": 33055, "surge_3pn": 33056,
    "overvoltage_relay": 33346,
    # contactors / contacts
    "contactor_1p": 32900, "contactor_2p": 32835, "contactor_3p": 32834,
    "contactor_4p": 32836,
    "contactor_aux_no": 32938, "contactor_aux_nc": 32939, "contactor_aux_co": 32941,
    "contactor_coil_bistable": 32957,
    "contact_no": 32938, "contact_nc": 32939, "contact_co": 32941,
    "contact_thermal_no": 32928, "contact_thermal_nc": 32929, "contact_thermal_co": 32931,
    "contact_timed_on_no": 32934, "contact_timed_on_nc": 32935,
    "contact_timed_off_no": 32942, "contact_timed_off_nc": 32943,
    "contact_timed_onoff_no": 32946,
    "relay_contact_no": 32938, "relay_contact_nc": 32939, "relay_contact_co": 32941,
    # control
    "mushroom_no": 32915, "mushroom_nc": 32913, "mushroom_co": 32919,
    "switch_no": 32920, "switch_nc": 32921, "switch_co": 32923,
    "limit_no": 32924, "limit_nc": 32925, "limit_co": 32927,
    "selector_2pos": 33048, "selector_3pos": 33049, "selector_changeover": 33050,
    # timers / relays / indication
    "timer_on_off": 32955, "timer_pulse": 33301, "flasher": 32956,
    "lamp_flashing": 32959, "bell": 32962, "horn": 32960, "siren": 32961,
    "buzzer": 32960,
    "solid_state_relay_dc": 33259, "solid_state_relay_ac": 33260,
    "solid_state_contactor": 33261,
    # motors
    "motor_3ph_6": 32829, "motor_3ph_yd": 32830, "motor_1ph": 33104,
    "motor_dahlander": 32832, "motor_wound_rotor": 32833,
    "motor_2speed": 32995, "motor_dc_pm": 33000, "motor_dc_indep": 33001,
    "stepper_unipolar": 33296, "stepper_bipolar": 33297,
    # logic
    "logic_and": 33057, "logic_or": 33058, "logic_not": 33059, "logic_xor": 33060,
    "logic_nand": 33061, "logic_nor": 33062, "logic_sr": 33101,
    "logic_edge_up": 33066, "logic_edge_down": 33067,
    "logic_divider": 33112, "logic_counter": 33098, "logic_timer_on": 33100,
    "counter": 33098, "counter_updown": 33278, "display_7seg": 33117,
    "time_switch": 33099,
    # terminals / wiring
    "terminal_round": 32970, "terminal_square": 32969,
    "terminal_2": 32992, "terminal_3": 32992, "terminal_4": 32993,
    "cable_1": 32885, "cable_neutral": 32896, "cable_pe": 32897,
    "connector_in": 32971, "connector_out": 32898, "junction": 32890,
    "hose_entry": 32891, "pneumatic_hose": 32886,
    # sensors
    "sensor_inductive_no": 33033, "sensor_inductive_nc": 33021,
    "sensor_capacitive_no": 33022, "sensor_capacitive_nc": 33023,
    "sensor_magnetic_no": 33024, "sensor_magnetic_nc": 33025,
    "sensor_photoelectric_no": 33035, "sensor_photoelectric_nc": 33034,
    "pressure_switch_no": 33254, "pressure_switch_nc": 33255,
    "vacuum_switch_no": 33256, "vacuum_switch_nc": 33257,
    "thermostat_no": 33347, "thermostat_nc": 33348,
    # power electronics
    "resistor_1p": 33003, "resistor_3p": 33004,
    "diode": 33044, "led": 33372, "rectifier_bridge": 32998,
    "autotransformer_3p": 33005, "transformer_3p": 33046,
    "soft_starter": 33006, "vfd_ac_2p": 33007, "vfd_ac_3p": 33007,
    "vfd_dc_2p": 33017, "vfd_dc_3p": 33018,
    "linear_actuator": 33258,
    # pneumatic
    "pneumatic_cylinder_single": 33243, "pneumatic_cylinder_double": 33243,
    "pneumatic_cylinder_rodless": 33246, "pneumatic_rotary": 33248,
    "pneumatic_gripper": 33249, "pneumatic_vacuum": 33250,
    "pneumatic_motor": 33251, "pneumatic_motor_reversible": 33252,
    # GRAFCET
    "grafcet_step_initial": 33083, "grafcet_step": 33084,
    "grafcet_transition": 33085, "grafcet_action": 33086,
    "grafcet_link": 33093, "grafcet_convergence": 33094,
    "grafcet_divergence": 33095,
    # PLC / controllers
    "plc_di": 33273, "plc_do": 33274, "plc_cpu": 33275, "plc_power": 33276,
    "plc_et200_di": 33335, "plc_et200_do": 33336, "plc_logo": 33337,
    "arduino_mega": 33326, "button_station": 33324, "hmi_panel": 33324,
    "plc_s7_1200": 33338,
    # remaining electronic / sensor blocks captured in the clean pass
    "solid_state_relay_dc": 33259, "solid_state_relay_ac": 33260,
    "solid_state_contactor": 33261,
    "linear_actuator": 33258,
    "overvoltage_relay": 33346, "thermostat_no": 33347, "thermostat_nc": 33348,
    "sensor_photoelectric_no": 33035, "sensor_photoelectric_nc": 33034,
    "pressure_switch_no": 33254, "pressure_switch_nc": 33255,
    "vacuum_switch_no": 33256, "vacuum_switch_nc": 33257,
    "opto_sensor_no": 33372, "opto_sensor_nc": 33373,
}


def norm(s: str) -> str:
    s = unicodedata.normalize("NFKD", s).encode("ascii", "ignore").decode().lower()
    s = re.sub(r"[^a-z0-9]+", " ", s)
    rep = {
        "fusivel": "fusible", "contator": "contactor", "disjuntor": "disyuntor",
        "chave": "interruptor", "botao": "pulsador", "contato": "contacto",
        "conexao": "conexion", "desconexao": "desconexion", "trifasico": "trifasico",
        "protecao": "proteccion", "seccionavel": "seccionable", "comutado": "con",
        "comutador": "conmutador", "boia": "flotador", "campainha": "timbre",
        "sirene": "sirena", "acionamento": "accionamiento",
    }
    return " ".join(rep.get(w, w) for w in s.split())


def main() -> None:
    commands = {}
    for line in open(os.path.join(ROOT, "docs", "cadesimu", "named_commands.tsv"), encoding="utf-8"):
        parts = line.rstrip("\n").split("\t")
        if len(parts) >= 3 and parts[0].isdigit():
            commands[int(parts[0])] = parts[2]
    captured = {}
    for path in glob.glob(os.path.join(GDI, "*.svg")):
        base = os.path.basename(path)[:-4]
        head = base.split("_")[0]
        if not head.isdigit():
            continue
        cmd = int(head)
        captured[cmd] = commands.get(cmd, base)
    clean_dir = os.path.join(ROOT, "docs", "cadesimu", "gdi_clean")
    if os.path.isdir(clean_dir):
        for path in glob.glob(os.path.join(clean_dir, "*_after.json")):
            base = os.path.basename(path)[:-11]
            head = base.split("_")[0]
            if not head.isdigit():
                continue
            cmd = int(head)
            befores = glob.glob(os.path.join(clean_dir, f"{cmd}_*_before.json"))
            if not befores:
                continue
            try:
                before = json.load(open(befores[0], encoding="utf-8"))
                after = json.load(open(path, encoding="utf-8"))
                diff = gdi.extras_pair(before, after)
            except Exception:
                continue
            if len(diff) >= 2 or cmd in (32885, 32890):
                captured[cmd] = commands.get(cmd, base)
    mapping = {}
    for path in glob.glob(os.path.join(ASSETS, "*.json")):
        data = json.load(open(path, encoding="utf-8"))
        type_id = data["type"]
        if type_id in LEGACY:
            continue
        if type_id in mapping:
            continue
        if type_id in OVERRIDES:
            cmd = OVERRIDES[type_id]
            if cmd in captured:
                mapping[type_id] = {"command": cmd, "name": captured[cmd], "confidence": 1.0}
            continue
        source = norm(data.get("display_name_pt") or data.get("display_name") or type_id)
        best = max(
            ((difflib.SequenceMatcher(None, source, norm(name)).ratio(), cmd) for cmd, name in captured.items()),
            default=(0.0, 0),
        )
        score, cmd = best
        if score >= 0.82:
            mapping[type_id] = {"command": cmd, "name": captured[cmd], "confidence": round(score, 3)}
    json.dump(mapping, open(OUT, "w", encoding="utf-8"), ensure_ascii=False, indent=2)
    print(f"mapped {len(mapping)} types -> {OUT}")
    for type_id in sorted(mapping):
        entry = mapping[type_id]
        print(f"{type_id:30s} <- {entry['command']:5d} {entry['confidence']:.2f} {entry['name']}")


if __name__ == "__main__":
    main()
