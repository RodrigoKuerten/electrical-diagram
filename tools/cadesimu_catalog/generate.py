"""Generate the CADe_SIMU-style symbol catalog for FioVivo.

Artwork is a reconstruction from CADe_SIMU 4.0: insertion previews captured from the running
application, the recovered component-name table, and the terminal numbering visible on the
symbols. The visual language matches the original: thin black IEC lines, small terminal labels,
and amber/red state variants.
"""
from __future__ import annotations
import json
import math
import os
from dataclasses import dataclass, field

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
ASSETS = os.path.join(ROOT, "assets", "symbols")
DOCS = os.path.join(ROOT, "docs", "cadesimu")
INK = "#1f2933"
ACTIVE = "#f2b134"
DANGER = "#c0392b"
LABEL = "#1f5f9e"
FONT = "Arial"


@dataclass
class Pin:
    name: str
    role: str
    x: float
    y: float
    required: bool = False


@dataclass
class Entry:
    type: str
    name: str
    name_pt: str
    category: str
    size: tuple[float, float]
    pins: list[Pin]
    params: dict = field(default_factory=dict)
    schema: list[dict] = field(default_factory=list)
    tag: str = "X1"
    family: str = "inline"
    options: dict = field(default_factory=dict)
    states: dict[str, str] = field(default_factory=dict)


CATALOG: list[Entry] = []


def add(entry: Entry) -> Entry:
    CATALOG.append(entry)
    return entry


def p(name, role, x, y, required=False):
    return Pin(name, role, x, y, required)


# ---------------------------------------------------------------------------
# SVG primitives
# ---------------------------------------------------------------------------

def esc(text: str) -> str:
    return (str(text).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
            .replace('"', "&quot;"))


def ln(x1, y1, x2, y2, color=INK, w=0.8, dash=None):
    d = f' stroke-dasharray="{dash}"' if dash else ""
    return (f'<line x1="{x1:.2f}" y1="{y1:.2f}" x2="{x2:.2f}" y2="{y2:.2f}" '
            f'stroke="{color}" stroke-width="{w}"{d}/>')


def rc(x, y, w, h, color=INK, sw=0.8, fill="none", dash=None, rx=0):
    d = f' stroke-dasharray="{dash}"' if dash else ""
    return (f'<rect x="{x:.2f}" y="{y:.2f}" width="{w:.2f}" height="{h:.2f}" rx="{rx}" '
            f'stroke="{color}" stroke-width="{sw}" fill="{fill}"{d}/>')


def ci(cx, cy, r, color=INK, sw=0.8, fill="none"):
    return (f'<circle cx="{cx:.2f}" cy="{cy:.2f}" r="{r:.2f}" stroke="{color}" '
            f'stroke-width="{sw}" fill="{fill}"/>')


def pa(d, color=INK, sw=0.8, fill="none", dash=None):
    da = f' stroke-dasharray="{dash}"' if dash else ""
    return f'<path d="{d}" stroke="{color}" stroke-width="{sw}" fill="{fill}"{da}/>'


def tx(x, y, value, color=INK, size=3.0, anchor="middle", weight="normal", rotate=None):
    tr = f' transform="rotate({rotate} {x:.2f} {y:.2f})"' if rotate is not None else ""
    return (f'<text x="{x:.2f}" y="{y:.2f}" fill="{color}" font-family="{FONT}" '
            f'font-size="{size}" text-anchor="{anchor}" font-weight="{weight}"{tr}>'
            f'{esc(value)}</text>')


def lab(x, y, value, anchor="middle"):
    return tx(x, y, value, LABEL, 2.6, anchor)


def svg(entry: Entry, body: str) -> str:
    w, h = entry.size
    return (f'<?xml version="1.0" encoding="UTF-8"?>\n'
            f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {w:g} {h:g}" '
            f'width="{w:g}mm" height="{h:g}mm">\n'
            f'<g fill="none" stroke-linecap="round" stroke-linejoin="round">\n{body}\n</g>\n'
            f'</svg>\n')


# ---------------------------------------------------------------------------
# Catalog construction helpers
# ---------------------------------------------------------------------------

def inline_size(poles: int) -> tuple[float, float]:
    return (20.0 + (poles - 1) * 11.0, 44.0)


def inline_poles(poles: int):
    w, _ = inline_size(poles)
    left = (w - (poles - 1) * 11.0) / 2.0
    return [left + i * 11.0 for i in range(poles)]


def inline_pins(poles, prefixes=None, roles=None):
    xs = inline_poles(poles)
    prefixes = prefixes or [(str(2 * i + 1), str(2 * i + 2)) for i in range(poles)]
    roles = roles or [("terminal", "terminal")] * poles
    out = []
    for i, (top, bottom) in enumerate(prefixes):
        out.append(p(top, roles[i][0], xs[i], 4, True))
        out.append(p(bottom, roles[i][1], xs[i], 40, True))
    return out


def add_inline(type, name, pt, category, kind, poles=1, tag="F1", params=None, schema=None,
               prefixes=None, roles=None, actuator=None):
    entry = Entry(type, name, pt, "protection" if category == "protection" else category,
                  inline_size(poles), inline_pins(poles, prefixes, roles), params or {}, schema or [],
                  tag, "inline", {"kind": kind, "poles": poles, "actuator": actuator})
    return add(entry)


def add_coil(type, name, pt, category, tag, params=None, bistable=False, timer=False):
    size = (30, 34) if bistable else (26, 34)
    pins = [p("A1", "coil_input", 8, 4, True), p("A2", "coil_output", 8, 30, True)]
    if bistable:
        pins += [p("B1", "coil_input", 20, 4), p("B2", "coil_output", 20, 30)]
    entry = Entry(type, name, pt, category, size, pins, params or {}, [], tag,
                  "coil", {"bistable": bistable, "timer": timer})
    return add(entry)


def add_timer(type, name, pt, tag="KT1", on_off=False, pins_extra=None):
    pins = [p("A1", "coil_input", 12, 4, True), p("A2", "coil_output", 12, 46, True),
            p("15", "contact_common", 46, 10, True), p("18", "contact_no", 46, 40, True)]
    if on_off:
        pins.insert(3, p("16", "contact_nc", 46, 24))
    entry = Entry(type, name, pt, "timers", (70, 50), pins,
                  {"delay_ms": 1000, "time_base": "ms", "label_group": ""},
                  [{"key": "delay_ms", "type": "int", "label": "Delay", "label_pt": "Tempo",
                    "unit": "ms", "min": 0}], tag, "timer", {"on_off": on_off})
    return add(entry)


def add_motor(type, name, pt, tag="M1", six=False, dc=False, extra=None, pe=True):
    pins = []
    if dc:
        pins = [p("+", "terminal", 10, 4, True), p("-", "terminal", 10, 52, True)]
    else:
        pins = [p("U1", "supply_in", 12, 4, True), p("V1", "supply_in", 28, 4, True),
                p("W1", "supply_in", 44, 4, True)]
        if pe:
            pins.append(p("PE", "earth", 52, 30))
        if six:
            pins += [p("W2", "terminal", 12, 52), p("U2", "terminal", 28, 52),
                     p("V2", "terminal", 44, 52)]
    if extra:
        pins += extra
    entry = Entry(type, name, pt, "machines", (60, 60), pins,
                  {"power_kw": 1.5, "rated_voltage": 380, "min_live_phases": 2}, [], tag,
                  "motor", {"six": six, "dc": dc})
    return add(entry)


def add_block(type, name, pt, category, size, pins, tag="U1", graphic="block", params=None,
              title=""):
    entry = Entry(type, name, pt, category, size, pins, params or {}, [], tag, "block",
                  {"graphic": graphic, "title": title})
    return add(entry)


# ---------------------------------------------------------------------------
# Supplies
# ---------------------------------------------------------------------------
add_block("supply_ac1", "Supply L + N + PE", "Alimentação F + N + PE", "supplies", (34, 46),
          [p("L", "supply_in", 8, 4, True), p("N", "neutral", 22, 4, True),
           p("PE", "earth", 15, 42)],
          tag="X1", graphic="supply", title="L N PE")
add_block("supply_ac1_pe", "Supply L + N + PE", "Alimentação F + N + PE", "supplies", (34, 46),
          [p("L", "supply_in", 8, 4, True), p("N", "neutral", 22, 4, True),
           p("PE", "earth", 15, 42)], tag="X1", graphic="supply", title="L N PE")
add_block("supply_ac3", "Supply L1 + L2 + L3 + N + PE", "Alimentação L1 + L2 + L3 + N + PE",
          "supplies", (66, 48),
          [p("L1", "supply_in", 8, 4, True), p("L2", "supply_in", 20, 4, True),
           p("L3", "supply_in", 32, 4, True), p("N", "neutral", 44, 4, True),
           p("PE", "earth", 56, 44)], tag="X1", graphic="supply", title="L1 L2 L3 N PE")
add_block("supply_ac3_n", "Supply L1+L2+L3+N", "Alimentação L1+L2+L3+N", "supplies", (56, 46),
          [p("L1", "supply_in", 8, 4, True), p("L2", "supply_in", 20, 4, True),
           p("L3", "supply_in", 32, 4, True), p("N", "neutral", 44, 42, True)],
          tag="X1", graphic="supply", title="L1 L2 L3 N")
add_block("supply_ac3_n_pe", "Supply L1+L2+L3+N+PE", "Alimentação L1+L2+L3+N+PE", "supplies",
          (66, 48), [p("L1", "supply_in", 8, 4, True), p("L2", "supply_in", 20, 4, True),
                     p("L3", "supply_in", 32, 4, True), p("N", "neutral", 44, 4, True),
                     p("PE", "earth", 56, 44)], tag="X1", graphic="supply", title="L1 L2 L3 N PE")
add_block("supply_ac3_pe", "Supply L1+L2+L3+PE", "Alimentação L1+L2+L3+PE", "supplies", (58, 46),
          [p("L1", "supply_in", 8, 4, True), p("L2", "supply_in", 22, 4, True),
           p("L3", "supply_in", 36, 4, True), p("PE", "earth", 50, 42)], tag="X1",
          graphic="supply", title="L1 L2 L3 PE")
add_block("supply_dc", "Supply + / -", "Alimentação + / -", "supplies", (34, 44),
          [p("+", "supply_in", 8, 4, True), p("-", "supply_out", 22, 4, True)],
          tag="X1", graphic="supply", title="+ -")
add_block("supply_dc_pos", "Supply +", "Alimentação +", "supplies", (22, 34),
          [p("+", "supply_in", 11, 4, True)], tag="X1", graphic="supply", title="+")
add_block("supply_neutral", "Supply N", "Alimentação neutro", "supplies", (22, 34),
          [p("N", "neutral", 11, 4, True)], tag="X1", graphic="supply", title="N")
add_block("supply_earth", "Supply PE", "Alimentação proteção", "supplies", (22, 34),
          [p("PE", "earth", 11, 4, True)], tag="X1", graphic="supply", title="PE")
add_block("supply_battery", "Battery", "Bateria", "supplies", (34, 36),
          [p("+", "supply_in", 8, 4, True), p("-", "supply_out", 22, 4, True)],
          tag="G1", graphic="battery", title="+ -")
add_block("supply_rectifier", "Rectifier", "Retificador", "supplies", (44, 48),
          [p("L", "supply_in", 8, 4, True), p("N", "neutral", 22, 4, True),
           p("+", "supply_out", 8, 44), p("-", "supply_out", 22, 44)], tag="U1",
          graphic="rectifier", title="~ =")
add_block("supply_transformer3", "Transformer III", "Transformador III", "supplies", (60, 54),
          [p("L1", "supply_in", 8, 4), p("L2", "supply_in", 22, 4), p("L3", "supply_in", 36, 4),
           p("l1", "supply_out", 8, 50), p("l2", "supply_out", 22, 50), p("l3", "supply_out", 36, 50)],
          tag="T1", graphic="transformer", title="T")

# ---------------------------------------------------------------------------
# Protection
# ---------------------------------------------------------------------------
for n, suffix, name, pt in [
    (1, "1p", "Fuse I", "Fusível I"), (2, "2p", "Fuse II", "Fusível II"),
    (3, "3p", "Fuse III", "Fusível III"), (4, "3pn", "Fuse IIIN", "Fusível IIIN"),
]:
    prefixes = [(str(2 * i + 1), str(2 * i + 2)) for i in range(n)]
    if n == 4:
        prefixes[3] = ("N", "N2")
    add_inline(f"fuse_{suffix}", name, pt, "protection", "fuse", n, tag="F1",
               params={"rated_current": 10, "blown": False},
               schema=[{"key": "rated_current", "type": "number", "label": "Rated current",
                        "label_pt": "Corrente nominal", "unit": "A", "min": 0}],
               prefixes=prefixes)
for n, suffix, name, pt in [
    (2, "1pn", "Fuse switch IN", "Fusível seccionável IN"),
    (2, "2p", "Fuse switch II", "Fusível seccionável II"),
    (3, "3p", "Fuse switch III", "Fusível seccionável III"),
]:
    prefixes = [(str(2 * i + 1), str(2 * i + 2)) for i in range(n)]
    add_inline(f"fuse_switch_{suffix}", name, pt, "protection", "fuse", n, tag="F1",
               params={"rated_current": 25, "closed": True}, prefixes=prefixes)
for n, suffix, name, pt in [
    (1, "1p", "Breaker I", "Disjuntor I"), (2, "1pn", "Breaker IN", "Disjuntor IN"),
    (2, "2p", "Breaker II", "Disjuntor II"), (3, "3p", "Breaker III", "Disjuntor III"),
    (4, "3pn", "Breaker IIIN", "Disjuntor IIIN"),
]:
    prefixes = [(str(2 * i + 1), str(2 * i + 2)) for i in range(n)]
    if n == 4:
        prefixes[3] = ("N", "N2")
    add_inline(f"breaker_{suffix}", name, pt, "protection", "breaker", n, tag="Q1",
               params={"rated_current": 16, "tripped": False},
               schema=[{"key": "rated_current", "type": "number", "label": "Rated current",
                        "label_pt": "Corrente nominal", "unit": "A", "min": 0}],
               prefixes=prefixes)
for n, suffix, name, pt in [
    (2, "2p", "Differential switch IN", "Interruptor diferencial IN"),
    (4, "4p", "Differential switch IIIN", "Interruptor diferencial IIIN"),
]:
    prefixes = [(str(2 * i + 1), str(2 * i + 2)) for i in range(n)]
    if n == 4:
        prefixes[3] = ("N", "N2")
    add_inline(f"rcd_{suffix}", name, pt, "protection", "rcd", n, tag="Q1",
               params={"sensitivity_ma": 30, "tripped": False}, prefixes=prefixes)
add_inline("motor_breaker_3p", "Motor circuit breaker III", "Disjuntor-motor III", "protection",
           "breaker", 3, tag="Q1", params={"trip_current": 10, "tripped": False})
add_block("surge_1pn", "Surge arrester IN", "Limitador sobretensões IN", "protection", (34, 44),
          [p("L", "terminal", 8, 4), p("N", "neutral", 22, 4), p("PE", "earth", 15, 40)],
          tag="F1", graphic="surge")
add_block("surge_3pn", "Surge arrester IIIN", "Limitador sobretensões IIIN", "protection", (54, 44),
          [p("L1", "terminal", 8, 4), p("L2", "terminal", 22, 4), p("L3", "terminal", 36, 4),
           p("N", "neutral", 46, 4), p("PE", "earth", 28, 40)], tag="F1", graphic="surge")
add_block("overvoltage_relay", "Permanent overvoltage protection",
          "Proteção sobretensões permanentes", "protection", (46, 44),
          [p("L", "terminal", 8, 4), p("N", "neutral", 22, 4),
           p("11", "contact_common", 38, 12), p("12", "contact_nc", 38, 24)],
          tag="F1", graphic="relay_block")

# ---------------------------------------------------------------------------
# Contactors and relays
# ---------------------------------------------------------------------------
for n, suffix, name, pt in [
    (1, "1p", "Contactor I", "Contator I"), (2, "2p", "Contactor II", "Contator II"),
    (3, "3p", "Contactor III", "Contator III"), (4, "4p", "Contactor IIII", "Contator IIII"),
]:
    prefixes = [(str(2 * i + 1), str(2 * i + 2)) for i in range(n)]
    add_inline(f"contactor_{suffix}", name, pt, "contactors", "contactor_no", n, tag="KM1",
               params={"rated_voltage": 220, "label_group": ""}, prefixes=prefixes)
add_inline("contactor_aux_no", "Auxiliary contact NO", "Contato auxiliar NA", "contactors",
           "switch_no", 1, tag="KM1.1", params={"label_group": ""},
           prefixes=[("13", "14")], roles=[("contact_common", "contact_no")])
add_inline("contactor_aux_nc", "Auxiliary contact NC", "Contato auxiliar NF", "contactors",
           "switch_nc", 1, tag="KM1.1", params={"label_group": ""},
           prefixes=[("21", "22")], roles=[("contact_common", "contact_nc")])
add_block("contactor_aux_co", "Auxiliary changeover contact", "Contato auxiliar comutado",
          "contactors", (32, 44), [p("11", "contact_common", 12, 4, True),
                                   p("12", "contact_nc", 24, 40), p("14", "contact_no", 6, 40)],
          tag="KM1.1", graphic="co_contact")
add_coil("contactor_coil", "Contactor coil", "Bobina de contator", "contactors", "KM1",
         {"rated_voltage": 220, "label_group": ""})
add_coil("contactor_coil_bistable", "Bistable contactor coil", "Bobina biestável de contator",
         "contactors", "KM1", {"rated_voltage": 220, "label_group": ""}, bistable=True)
add_coil("relay_coil", "Relay coil", "Bobina de relé", "relays", "K1",
         {"rated_voltage": 24, "label_group": ""})
add_coil("relay_coil_bistable", "Bistable relay coil", "Bobina biestável de relé", "relays", "K1",
         {"rated_voltage": 24, "label_group": ""}, bistable=True)
add_inline("relay_contact_no", "Relay contact NO", "Contato de relé NA", "relays", "switch_no", 1,
           tag="K1.1", params={"label_group": ""}, prefixes=[("13", "14")],
           roles=[("contact_common", "contact_no")])
add_inline("relay_contact_nc", "Relay contact NC", "Contato de relé NF", "relays", "switch_nc", 1,
           tag="K1.1", params={"label_group": ""}, prefixes=[("21", "22")],
           roles=[("contact_common", "contact_nc")])
add_block("relay_contact_co", "Relay changeover contact", "Contato de relé comutado", "relays",
          (32, 44), [p("11", "contact_common", 12, 4, True), p("12", "contact_nc", 24, 40),
                     p("14", "contact_no", 6, 40)], tag="K1.1", graphic="co_contact")
add_block("solid_state_relay_dc", "Solid-state relay DC", "Relé de estado sólido DC", "relays",
          (36, 44), [p("1", "terminal", 8, 4, True), p("2", "terminal", 28, 40, True),
                     p("+", "terminal", 8, 40), p("-", "terminal", 28, 4)], tag="K1",
          graphic="ssr")
add_block("solid_state_relay_ac", "Solid-state relay AC", "Relé de estado sólido AC", "relays",
          (36, 44), [p("1", "terminal", 8, 4, True), p("2", "terminal", 28, 40, True),
                     p("+", "terminal", 8, 40), p("-", "terminal", 28, 4)], tag="K1",
          graphic="ssr")
add_block("solid_state_contactor", "Solid-state contactor", "Contator de estado sólido", "relays",
          (46, 48), [p("1", "terminal", 8, 4), p("2", "terminal", 22, 4), p("3", "terminal", 36, 4),
                     p("4", "terminal", 8, 44), p("5", "terminal", 22, 44), p("6", "terminal", 36, 44)],
          tag="KM1", graphic="ssr3")

# ---------------------------------------------------------------------------
# Contacts, pushbuttons, switches
# ---------------------------------------------------------------------------
for kind, suffix, name, pt, prefix, role in [
    ("switch_no", "no", "Contact NO", "Contato NA", ("13", "14"), ("contact_common", "contact_no")),
    ("switch_nc", "nc", "Contact NC", "Contato NF", ("21", "22"), ("contact_common", "contact_nc")),
]:
    add_inline(f"contact_{suffix}", name, pt, "contacts", kind, 1, tag="S1",
               prefixes=[prefix], roles=[role])
add_block("contact_co", "Changeover contact", "Contato comutado", "contacts", (32, 44),
          [p("11", "contact_common", 12, 4, True), p("12", "contact_nc", 24, 40),
           p("14", "contact_no", 6, 40)], tag="S1", graphic="co_contact")
for kind, suffix, name, pt, prefix, role in [
    ("switch_no", "no", "Pushbutton NO", "Botão pulsador NA", ("13", "14"), ("contact_common", "contact_no")),
    ("switch_nc", "nc", "Pushbutton NC", "Botão pulsador NF", ("21", "22"), ("contact_common", "contact_nc")),
]:
    add_inline(f"pushbutton_{suffix}", name, pt, "control", kind, 1, tag="S1",
               params={"color": "green", "mode": "momentary"}, prefixes=[prefix], roles=[role],
               actuator="pushbutton")
add_block("pushbutton_co", "Pushbutton changeover", "Botão pulsador comutado", "control", (34, 44),
          [p("11", "contact_common", 10, 4, True), p("12", "contact_nc", 26, 40),
           p("14", "contact_no", 8, 40)], params={"color": "green", "mode": "momentary"},
          tag="S1", graphic="co_pushbutton")
for kind, suffix, name, pt, prefix, role in [
    ("switch_no", "no", "Mushroom pushbutton NO", "Pulsador seta NA", ("13", "14"), ("contact_common", "contact_no")),
    ("switch_nc", "nc", "Mushroom pushbutton NC", "Pulsador seta NF", ("21", "22"), ("contact_common", "contact_nc")),
]:
    add_inline(f"mushroom_{suffix}", name, pt, "control", kind, 1, tag="S0",
               params={"color": "red", "mode": "momentary"}, prefixes=[prefix], roles=[role],
               actuator="mushroom")
add_block("mushroom_co", "Mushroom changeover pushbutton", "Pulsador seta comutado", "control",
          (34, 44), [p("11", "contact_common", 10, 4, True), p("12", "contact_nc", 26, 40),
                     p("14", "contact_no", 8, 40)], params={"color": "red", "mode": "momentary"},
          tag="S0", graphic="co_pushbutton")
add_inline("emergency_stop", "Emergency stop", "Botão de emergência", "control", "switch_nc", 1,
           tag="S0", params={"latching": True}, prefixes=[("1", "2")],
           roles=[("contact_common", "contact_nc")], actuator="mushroom")
for kind, suffix, name, pt, prefix, role in [
    ("switch_no", "no", "Switch NO", "Interruptor NA", ("13", "14"), ("contact_common", "contact_no")),
    ("switch_nc", "nc", "Switch NC", "Interruptor NF", ("21", "22"), ("contact_common", "contact_nc")),
]:
    add_inline(f"switch_{suffix}", name, pt, "control", kind, 1, tag="S1",
               params={"mode": "maintained"}, prefixes=[prefix], roles=[role])
add_block("switch_co", "Changeover switch", "Interruptor comutado", "control", (34, 44),
          [p("11", "contact_common", 10, 4, True), p("12", "contact_nc", 26, 40),
           p("14", "contact_no", 8, 40)], params={"mode": "maintained"}, tag="S1",
          graphic="co_contact")
for kind, suffix, name, pt, prefix, role in [
    ("switch_no", "no", "Limit switch NO", "Fim de curso NA", ("13", "14"), ("contact_common", "contact_no")),
    ("switch_nc", "nc", "Limit switch NC", "Fim de curso NF", ("21", "22"), ("contact_common", "contact_nc")),
]:
    add_inline(f"limit_{suffix}", name, pt, "sensors", kind, 1, tag="S1",
               params={"mode": "momentary"}, prefixes=[prefix], roles=[role], actuator="limit")
add_block("limit_co", "Changeover limit switch", "Fim de curso comutado", "sensors", (34, 44),
          [p("11", "contact_common", 10, 4, True), p("12", "contact_nc", 26, 40),
           p("14", "contact_no", 8, 40)], params={"mode": "momentary"}, tag="S1",
          graphic="co_contact")
add_block("selector_2pos", "Selector I-0-II", "Comutador I-0-II", "control", (38, 44),
          [p("1", "contact_common", 19, 4, True), p("2", "contact_no", 8, 40),
           p("3", "contact_no", 30, 40)], params={"positions": 2, "position": 1}, tag="S1",
          graphic="selector")
add_block("selector_3pos", "Selector I-II-III", "Comutador I-II-III", "control", (48, 44),
          [p("1", "contact_common", 24, 4, True), p("2", "contact_no", 7, 40),
           p("3", "contact_no", 24, 40), p("4", "contact_no", 41, 40)],
          params={"positions": 3, "position": 1}, tag="S1", graphic="selector")
add_block("selector_changeover", "Changeover selector", "Comutador de cruzamento", "control",
          (42, 44), [p("1", "contact_common", 8, 4), p("2", "contact_common", 34, 4),
                     p("3", "contact_no", 8, 40), p("4", "contact_no", 34, 40)],
          params={"positions": 2}, tag="S1", graphic="crossover")
for prefix, name, pt, sc in [
    ("on", "Timed contact ON delay NO", "Contato temporizado conexão NA", ("15", "18")),
    ("off", "Timed contact OFF delay NO", "Contato temporizado desconexão NA", ("15", "18")),
]:
    add_inline(f"contact_timed_{prefix}_no", name, pt, "contacts", "switch_no", 1, tag="KT1.1",
               params={"delay_ms": 1000, "label_group": ""}, prefixes=[sc],
               roles=[("contact_common", "contact_no")])
for prefix, name, pt, sc in [
    ("on", "Timed contact ON delay NC", "Contato temporizado conexão NF", ("15", "16")),
    ("off", "Timed contact OFF delay NC", "Contato temporizado desconexão NF", ("15", "16")),
]:
    add_inline(f"contact_timed_{prefix}_nc", name, pt, "contacts", "switch_nc", 1, tag="KT1.1",
               params={"delay_ms": 1000, "label_group": ""}, prefixes=[sc],
               roles=[("contact_common", "contact_nc")])
add_inline("contact_timed_onoff_no", "Timed contact ON/OFF NO",
           "Contato temporizado conexão/desconexão NA", "contacts", "switch_no", 1, tag="KT1.1",
           params={"delay_on_ms": 1000, "delay_off_ms": 1000, "label_group": ""},
           prefixes=[("15", "18")], roles=[("contact_common", "contact_no")])
add_inline("contact_thermal_no", "Thermal relay contact NO", "Contato auxiliar térmico NA",
           "protection", "switch_no", 1, tag="F1", params={"label_group": ""},
           prefixes=[("97", "98")], roles=[("contact_common", "contact_no")])
add_inline("contact_thermal_nc", "Thermal relay contact NC", "Contato auxiliar térmico NF",
           "protection", "switch_nc", 1, tag="F1", params={"label_group": ""},
           prefixes=[("95", "96")], roles=[("contact_common", "contact_nc")])
add_block("contact_thermal_co", "Thermal relay changeover contact",
          "Contato auxiliar térmico comutado", "protection", (32, 44),
          [p("95", "contact_common", 12, 4, True), p("96", "contact_nc", 24, 40),
           p("98", "contact_no", 6, 40)], tag="F1", graphic="co_contact")

# Sensors
for fam, fam_pt in [("inductive", "indutivo"), ("capacitive", "capacitivo"),
                    ("magnetic", "magnético"), ("photoelectric", "fotoelétrico")]:
    for suffix, role_name in [("no", "NA"), ("nc", "NF")]:
        add_block(f"sensor_{fam}_{suffix}", f"{fam.title()} sensor {role_name}",
                  f"Sensor {fam_pt} {role_name}", "sensors", (32, 44),
                  [p("1", "contact_common", 8, 4, True),
                   p("2", "contact_no" if suffix == "no" else "contact_nc", 22, 4),
                   p("4", "terminal", 15, 40)], params={"mode": "momentary"}, tag="B1",
                  graphic="sensor", title=fam)
for fam, name, pt in [("pressure", "Pressure switch NO", "Presostato NA"),
                      ("vacuum", "Vacuum switch NO", "Vacuostato NA")]:
    add_block(f"{fam}_switch_no", name, pt, "sensors", (32, 44),
              [p("1", "contact_common", 11, 4, True), p("2", "contact_no", 11, 40, True)],
              params={"threshold": 0.5}, tag="S1", graphic="pressure")
for fam, name, pt in [("pressure", "Pressure switch NC", "Presostato NF"),
                      ("vacuum", "Vacuum switch NC", "Vacuostato NF")]:
    add_block(f"{fam}_switch_nc", name, pt, "sensors", (32, 44),
              [p("1", "contact_common", 11, 4, True), p("2", "contact_nc", 11, 40, True)],
              params={"threshold": 0.5}, tag="S1", graphic="pressure")
for suffix, name, pt in [("no", "Thermostat NO", "Termostato NA"), ("nc", "Thermostat NC", "Termostato NF")]:
    add_block(f"thermostat_{suffix}", name, pt, "sensors", (32, 44),
              [p("1", "contact_common", 11, 4, True),
               p("2", "contact_no" if suffix == "no" else "contact_nc", 11, 40, True)],
              params={"setpoint": 60}, tag="S1", graphic="thermostat")
for suffix, name, pt in [("no", "Float switch NO", "Boia NA"), ("nc", "Float switch NC", "Boia NF")]:
    add_block(f"float_{suffix}", name, pt, "sensors", (36, 44),
              [p("1", "contact_common", 11, 4, True),
               p("2", "contact_no" if suffix == "no" else "contact_nc", 11, 40, True)],
              params={"level": 50}, tag="S1", graphic="float")
for suffix, name, pt in [("no", "Reed switch NO", "Sensor reed NA"), ("nc", "Reed switch NC", "Sensor reed NF")]:
    add_block(f"reed_{suffix}", name, pt, "sensors", (28, 40),
              [p("1", "contact_common", 14, 4, True),
               p("2", "contact_no" if suffix == "no" else "contact_nc", 14, 36, True)],
              tag="S1", graphic="reed")

# Timers and indication
add_timer("timer_on", "Timer ON delay", "Temporizador na conexão")
add_timer("timer_off", "Timer OFF delay", "Temporizador na desconexão")
add_timer("timer_on_off", "Timer ON/OFF", "Temporizador conexão/desconexão", on_off=True)
add_block("timer_pulse", "Pulse timer", "Temporizador de pulso", "timers", (46, 50),
          [p("A1", "coil_input", 12, 4, True), p("A2", "coil_output", 12, 46, True),
           p("15", "contact_common", 34, 20), p("18", "contact_no", 34, 40)],
          params={"delay_ms": 500}, tag="KT1", graphic="timer")
add_block("flasher", "Flasher relay", "Relé intermitente", "timers", (42, 44),
          [p("A1", "coil_input", 12, 4, True), p("A2", "coil_output", 12, 40, True),
           p("15", "contact_common", 30, 16), p("18", "contact_no", 30, 36)],
          params={"period_ms": 1000, "label_group": ""}, tag="K1", graphic="flasher")
add_block("time_switch", "Time switch", "Interruptor horário", "timers", (46, 44),
          [p("1", "contact_common", 8, 4, True), p("2", "contact_no", 8, 40, True),
           p("N", "neutral", 34, 40)], params={"on_time": "08:00", "off_time": "18:00"},
          tag="S1", graphic="clock")
add_block("counter", "Counter", "Contador", "timers", (46, 50),
          [p("A1", "coil_input", 12, 4, True), p("A2", "coil_output", 12, 46, True),
           p("15", "contact_common", 34, 20), p("18", "contact_no", 34, 40)],
          params={"preset": 10}, tag="C1", graphic="counter")
add_block("counter_updown", "Up/down counter", "Contador ascendente-descendente", "timers",
          (52, 50), [p("A1", "coil_input", 10, 4), p("A2", "coil_output", 10, 46),
                     p("UP", "signal_in", 28, 4), p("DN", "signal_in", 40, 4),
                     p("15", "contact_common", 28, 46), p("18", "contact_no", 40, 46)],
          params={"preset": 10}, tag="C1", graphic="counter")
add_block("pilot_lamp", "Pilot lamp", "Piloto de sinalização", "indication", (30, 34),
          [p("A1", "coil_input", 10, 4, True), p("A2", "coil_output", 10, 30, True)],
          params={"color": "green", "rated_voltage": 220}, tag="H1", graphic="lamp")
add_block("lamp_flashing", "Flashing pilot lamp", "Piloto intermitente", "indication", (30, 34),
          [p("A1", "coil_input", 10, 4, True), p("A2", "coil_output", 10, 30, True)],
          params={"color": "amber", "period_ms": 1000}, tag="H1", graphic="lamp")
add_block("buzzer", "Buzzer", "Zumbador", "indication", (30, 34),
          [p("A1", "coil_input", 10, 4, True), p("A2", "coil_output", 10, 30, True)],
          tag="H1", graphic="sound")
add_block("siren", "Siren", "Sirene", "indication", (30, 34),
          [p("A1", "coil_input", 10, 4, True), p("A2", "coil_output", 10, 30, True)],
          tag="H1", graphic="sound")
add_block("bell", "Bell", "Campainha", "indication", (30, 34),
          [p("A1", "coil_input", 10, 4, True), p("A2", "coil_output", 10, 30, True)],
          tag="H1", graphic="sound")
add_block("led", "LED", "Diodo LED", "indication", (24, 32),
          [p("A1", "coil_input", 8, 4, True), p("A2", "coil_output", 8, 28, True)],
          params={"color": "red"}, tag="H1", graphic="led")

# Motors
add_motor("motor_3ph", "Three-phase motor", "Motor trifásico", pe=False)
add_motor("motor_3ph_6", "Three-phase motor (6 terminals)", "Motor trifásico (6 terminais)",
          six=True)
add_motor("motor_3ph_yd", "Three-phase motor Y/D", "Motor trifásico Y/D", six=True)
add_motor("motor_1ph", "Single-phase motor", "Motor monofásico",
          extra=[p("Z1", "terminal", 16, 52), p("Z2", "terminal", 40, 52)])
add_motor("motor_2speed", "Two-speed motor", "Motor de duas velocidades", six=True)
add_motor("motor_dahlander", "Dahlander motor", "Motor Dahlander", six=True)
add_motor("motor_wound_rotor", "Wound-rotor motor", "Motor de rotor bobinado", six=True,
          extra=[p("K", "terminal", 54, 18), p("L", "terminal", 54, 24), p("M", "terminal", 54, 30)])
add_motor("motor_dc_pm", "DC motor permanent magnet", "Motor CC ímã permanente", dc=True)
add_motor("motor_dc_indep", "DC motor independent excitation", "Motor CC excitação independente",
          extra=[p("F1", "terminal", 40, 4), p("F2", "terminal", 40, 52)])
add_block("stepper_unipolar", "Unipolar stepper motor", "Motor passo a passo unipolar",
          "machines", (56, 56),
          [p("A", "terminal", 10, 4), p("B", "terminal", 22, 4), p("C", "terminal", 34, 4),
           p("D", "terminal", 46, 4), p("COM", "terminal", 28, 52)], tag="M1", graphic="stepper")
add_block("stepper_bipolar", "Bipolar stepper motor", "Motor passo a passo bipolar", "machines",
          (56, 56), [p("A+", "terminal", 12, 4), p("A-", "terminal", 24, 4),
                     p("B+", "terminal", 36, 4), p("B-", "terminal", 48, 4)],
          tag="M1", graphic="stepper")
add_block("linear_actuator", "Electric linear actuator", "Atuador linear elétrico", "machines",
          (54, 40), [p("A1", "coil_input", 10, 4), p("A2", "coil_output", 10, 36)],
          params={"stroke_mm": 100}, tag="M1", graphic="actuator")

# Power
add_inline("resistor_1p", "Resistor I", "Resistência I", "power", "resistor", 1, tag="R1")
add_inline("resistor_3p", "Resistor III", "Resistência III", "power", "resistor", 3, tag="R1")
add_block("autotransformer_3p", "Autotransformer III", "Autotransformador III", "power",
          (58, 56), [p("L1", "terminal", 8, 4), p("L2", "terminal", 22, 4),
                     p("L3", "terminal", 36, 4), p("U1", "terminal", 8, 52),
                     p("V1", "terminal", 22, 52), p("W1", "terminal", 36, 52)],
          tag="T1", graphic="transformer")
add_block("soft_starter", "Electronic soft starter", "Arrancador eletrônico CA", "power",
          (58, 56), [p("L1", "supply_in", 10, 4), p("L2", "supply_in", 24, 4),
                     p("L3", "supply_in", 38, 4), p("U1", "supply_out", 10, 52),
                     p("V1", "supply_out", 24, 52), p("W1", "supply_out", 38, 52)],
          tag="U1", graphic="vfd", title="SOFT")
add_block("vfd_ac_2p", "AC variable frequency drive II", "Variador de velocidade CA II", "power",
          (58, 58), [p("L", "supply_in", 10, 4), p("N", "supply_in", 24, 4),
                     p("U", "supply_out", 10, 54), p("V", "supply_out", 24, 54),
                     p("AI", "signal_in", 46, 20)], tag="U1", graphic="vfd", title="VFD")
add_block("vfd_ac_3p", "AC variable frequency drive III", "Variador de velocidade CA III", "power",
          (58, 58), [p("L1", "supply_in", 10, 4), p("L2", "supply_in", 24, 4),
                     p("L3", "supply_in", 38, 4), p("U", "supply_out", 10, 54),
                     p("V", "supply_out", 24, 54), p("W", "supply_out", 38, 54)],
          tag="U1", graphic="vfd", title="VFD")
add_block("vfd_dc_2p", "DC drive II", "Variador de velocidade CC II", "power", (54, 52),
          [p("+", "supply_in", 10, 4), p("-", "supply_in", 24, 4),
           p("A+", "supply_out", 10, 48), p("A-", "supply_out", 24, 48)],
          tag="U1", graphic="vfd", title="DC")
add_block("vfd_dc_3p", "DC drive III", "Variador de velocidade CC III", "power", (54, 52),
          [p("+", "supply_in", 10, 4), p("-", "supply_in", 24, 4),
           p("A+", "supply_out", 10, 48), p("A-", "supply_out", 24, 48),
           p("F+", "terminal", 42, 48), p("F-", "terminal", 48, 48)],
          tag="U1", graphic="vfd", title="DC")
add_block("diode", "Diode", "Diodo", "power", (26, 32),
          [p("1", "terminal", 8, 4), p("2", "terminal", 8, 28)], tag="V1", graphic="diode")
add_block("rectifier_bridge", "Rectifier bridge", "Retificador", "power", (42, 42),
          [p("L", "supply_in", 8, 4), p("N", "neutral", 22, 4),
           p("+", "supply_out", 8, 38), p("-", "supply_out", 22, 38)],
          tag="U1", graphic="rectifier", title="~ =")
add_block("transformer_3p", "Transformer III", "Transformador III", "power", (62, 54),
          [p("L1", "supply_in", 8, 4), p("L2", "supply_in", 22, 4), p("L3", "supply_in", 36, 4),
           p("l1", "supply_out", 8, 50), p("l2", "supply_out", 22, 50), p("l3", "supply_out", 36, 50)],
          tag="T1", graphic="transformer")
add_inline("heater_1p", "Heater I", "Resistência de aquecimento I", "power", "resistor", 1, tag="E1")

# Logic
for type, name, sym, inputs in [
    ("logic_and", "AND", "&", 2), ("logic_or", "OR", ">=1", 2), ("logic_not", "INV", "1", 1),
    ("logic_xor", "EXOR", "=1", 2), ("logic_nand", "NAND", "&", 2), ("logic_nor", "NOR", ">=1", 2),
]:
    pins = [p("IN1", "signal_in", 6, 10), p("OUT", "signal_out", 34, 15), p("0V", "signal_in", 20, 30)]
    if inputs == 2:
        pins.insert(1, p("IN2", "signal_in", 6, 22))
    add_block(type, f"Logic {name}", f"Função lógica {name}", "logic", (40, 34), pins,
              tag="U1", graphic="logic", title=sym)
add_block("logic_sr", "SR latch", "Biestável RS", "logic", (42, 40),
          [p("S", "signal_in", 6, 8), p("R", "signal_in", 6, 32),
           p("Q", "signal_out", 36, 12), p("Qn", "signal_out", 36, 28)],
          tag="U1", graphic="sr")
add_block("logic_edge_up", "Rising edge", "Flanco ascendente", "logic", (36, 34),
          [p("IN", "signal_in", 6, 17), p("OUT", "signal_out", 30, 17)], tag="U1",
          graphic="edge", title="↑")
add_block("logic_edge_down", "Falling edge", "Flanco descendente", "logic", (36, 34),
          [p("IN", "signal_in", 6, 17), p("OUT", "signal_out", 30, 17)], tag="U1",
          graphic="edge", title="↓")
add_block("logic_timer_on", "Logic timer ON", "Temporizador conexão", "logic", (42, 36),
          [p("IN", "signal_in", 6, 10), p("OUT", "signal_out", 36, 24), p("0V", "signal_in", 20, 32)],
          params={"delay_ms": 1000}, tag="T1", graphic="timer_block")
add_block("logic_counter", "Logic counter", "Contador", "logic", (42, 40),
          [p("IN", "signal_in", 6, 10), p("RST", "signal_in", 6, 30),
           p("OUT", "signal_out", 36, 20)], params={"preset": 10}, tag="C1",
          graphic="counter")
add_block("display_7seg", "7-segment display", "Display 7 segmentos", "logic", (42, 48),
          [p("A", "signal_in", 6, 4), p("B", "signal_in", 12, 4), p("C", "signal_in", 18, 4),
           p("D", "signal_in", 24, 4), p("E", "signal_in", 30, 4), p("F", "signal_in", 36, 4),
           p("G", "signal_in", 6, 44)], tag="H1", graphic="sevenseg")

# PLC / IO
add_block("plc_cpu", "PLC CPU", "CPU PLC", "plc", (62, 80),
          [p("L", "supply_in", 8, 4), p("N", "neutral", 22, 4), p("PE", "earth", 36, 4),
           p("RUN", "signal_out", 8, 76), p("ERR", "signal_out", 22, 76)],
          tag="PLC1", graphic="plc", title="CPU")
add_block("plc_di", "Digital input PLC", "Entrada digital PLC", "plc", (54, 62),
          [p("L", "supply_in", 8, 4), p("N", "neutral", 22, 4)] +
          [p(f"I{i}", "signal_in", 8 + i * 9, 58) for i in range(4)],
          tag="PLC1", graphic="plc_io", title="DI")
add_block("plc_do", "Digital output PLC", "Saída digital PLC", "plc", (54, 62),
          [p("L", "supply_in", 8, 4), p("N", "neutral", 22, 4)] +
          [p(f"Q{i}", "signal_out", 8 + i * 9, 58) for i in range(4)],
          tag="PLC1", graphic="plc_io", title="DO")
add_block("plc_power", "PLC power supply", "Alimentação I/O digital PLC", "plc", (46, 50),
          [p("L", "supply_in", 8, 4), p("N", "neutral", 22, 4),
           p("+", "supply_out", 8, 46), p("-", "supply_out", 22, 46)],
          params={"voltage": 24}, tag="U1", graphic="plc_power", title="24V")
add_block("plc_s7_1200", "PLC S7-1200", "PLC S7-1200", "plc", (72, 82),
          [p("L", "supply_in", 8, 4), p("N", "neutral", 22, 4)] +
          [p(f"I{i}", "signal_in", 8 + i * 7, 78) for i in range(6)] +
          [p(f"Q{i}", "signal_out", 54 + i * 3, 78) for i in range(4)],
          tag="PLC1", graphic="plc", title="S7-1200")
add_block("plc_logo", "LOGO 4DI/4DO", "LOGO 4DI/4DO", "plc", (62, 72),
          [p("L", "supply_in", 8, 4), p("N", "neutral", 22, 4)] +
          [p(f"I{i}", "signal_in", 8 + i * 8, 68) for i in range(4)] +
          [p(f"Q{i}", "signal_out", 42 + i * 4, 68) for i in range(4)],
          tag="PLC1", graphic="plc", title="LOGO")
add_block("plc_et200_di", "ET-200 8DI", "ET-200 8DI", "plc", (54, 62),
          [p("L", "supply_in", 8, 4), p("N", "neutral", 22, 4)] +
          [p(f"I{i}", "signal_in", 6 + i * 5, 58) for i in range(8)],
          tag="PLC1", graphic="plc_io", title="8DI")
add_block("plc_et200_do", "ET-200 8DO", "ET-200 8DO", "plc", (54, 62),
          [p("L", "supply_in", 8, 4), p("N", "neutral", 22, 4)] +
          [p(f"Q{i}", "signal_out", 6 + i * 5, 58) for i in range(8)],
          tag="PLC1", graphic="plc_io", title="8DO")
add_block("arduino_mega", "Arduino Mega", "Arduino Mega", "plc", (72, 90),
          [p("5V", "supply_out", 8, 4), p("GND", "neutral", 22, 4)] +
          [p(f"D{i}", "signal_in", 8 + i * 5, 86) for i in range(10)],
          tag="A1", graphic="plc", title="ARDUINO")
add_block("hmi_panel", "Operator panel", "Botonera", "plc", (62, 50),
          [p("L", "supply_in", 8, 4), p("N", "neutral", 22, 4),
           p("COM", "signal_in", 46, 46)], tag="HMI1", graphic="plc", title="HMI")

# Pneumatic
add_block("pneumatic_cylinder_single", "Single-acting cylinder", "Atuador simples efeito",
          "pneumatic", (62, 42), [p("A", "terminal", 8, 21), p("S", "signal_out", 54, 30)],
          params={"stroke_mm": 100}, tag="M1", graphic="cylinder", title="single")
add_block("pneumatic_cylinder_double", "Double-acting cylinder", "Atuador duplo efeito",
          "pneumatic", (62, 42), [p("A", "terminal", 8, 14), p("B", "terminal", 8, 28),
                                  p("S1", "signal_out", 54, 10), p("S2", "signal_out", 54, 30)],
          params={"stroke_mm": 100}, tag="M1", graphic="cylinder", title="double")
add_block("pneumatic_cylinder_rodless", "Rodless cylinder", "Atuador sem haste", "pneumatic",
          (62, 42), [p("A", "terminal", 8, 14), p("B", "terminal", 8, 28)], tag="M1",
          graphic="cylinder", title="rodless")
add_block("pneumatic_rotary", "Rotary actuator", "Atuador de giro", "pneumatic", (52, 52),
          [p("A", "terminal", 8, 14), p("B", "terminal", 8, 34)], tag="M1",
          graphic="rotary")
add_block("pneumatic_gripper", "Double-acting gripper", "Pinça duplo efeito", "pneumatic",
          (48, 48), [p("A", "terminal", 8, 14), p("B", "terminal", 8, 34)], tag="M1",
          graphic="gripper")
add_block("pneumatic_vacuum", "Vacuum cup", "Ventosa", "pneumatic", (42, 42),
          [p("A", "terminal", 8, 21)], tag="M1", graphic="vacuum")
add_block("pneumatic_motor", "Pneumatic motor", "Motor pneumático", "pneumatic", (56, 52),
          [p("A", "terminal", 8, 14), p("B", "terminal", 8, 36)], tag="M1", graphic="airmotor")
add_block("pneumatic_motor_reversible", "Reversible pneumatic motor", "Motor pneumático reversível",
          "pneumatic", (56, 52), [p("A", "terminal", 8, 14), p("B", "terminal", 8, 36)],
          tag="M1", graphic="airmotor")
add_block("pneumatic_valve_3_2", "3/2 valve", "Válvula 3/2", "pneumatic", (52, 46),
          [p("P", "terminal", 8, 8), p("A", "terminal", 44, 8), p("R", "terminal", 8, 38),
           p("A1", "coil_input", 20, 42)], params={"spring_return": True}, tag="YV1",
          graphic="valve", title="3/2")
add_block("pneumatic_valve_5_2", "5/2 valve", "Válvula 5/2", "pneumatic", (62, 46),
          [p("P", "terminal", 8, 8), p("A", "terminal", 54, 8), p("B", "terminal", 54, 38),
           p("R", "terminal", 8, 38), p("S", "terminal", 28, 38),
           p("A1", "coil_input", 20, 42)], params={"spring_return": True}, tag="YV1",
          graphic="valve", title="5/2")
add_block("pneumatic_hose", "Hose", "Manguera", "pneumatic", (42, 16),
          [p("1", "terminal", 4, 8), p("2", "terminal", 38, 8)], tag="W1", graphic="hose")

# Connection
add_block("terminal", "Terminal", "Borne", "terminals", (16, 16),
          [p("1", "terminal", 8, 8)], tag="XT1", graphic="terminal")
add_block("terminal_square", "Square terminal", "Borne quadrado", "terminals", (16, 16),
          [p("1", "terminal", 8, 8)], tag="XT1", graphic="terminal_square")
add_block("terminal_round", "Round terminal", "Borne redondo", "terminals", (16, 16),
          [p("1", "terminal", 8, 8)], tag="XT1", graphic="terminal")
add_block("terminal_2", "2 round terminals", "2 bornes redondos", "terminals", (30, 18),
          [p("1", "terminal", 8, 9), p("2", "terminal", 22, 9)], tag="XT1", graphic="terminal2")
add_block("terminal_3", "3 round terminals", "3 bornes redondos", "terminals", (42, 18),
          [p("1", "terminal", 8, 9), p("2", "terminal", 21, 9), p("3", "terminal", 34, 9)],
          tag="XT1", graphic="terminal3")
add_block("junction", "Junction", "Conexão", "terminals", (16, 16),
          [p("1", "terminal", 8, 8)], tag="X1", graphic="junction")
add_block("cable_1", "Phase cable", "Cabo fase", "terminals", (32, 12),
          [p("1", "terminal", 2, 6), p("2", "terminal", 30, 6)], tag="W1", graphic="cable")
add_block("cable_3", "Three-phase cable", "Cabo trifásico", "terminals", (42, 26),
          [p("1", "terminal", 2, 7), p("2", "terminal", 2, 13), p("3", "terminal", 2, 19),
           p("4", "terminal", 40, 7), p("5", "terminal", 40, 13), p("6", "terminal", 40, 19)],
          tag="W1", graphic="cable3")
add_block("cable_neutral", "Neutral cable", "Cabo neutro", "terminals", (32, 12),
          [p("1", "terminal", 2, 6), p("2", "terminal", 30, 6)], tag="W1", graphic="cable")
add_block("cable_pe", "Protective cable", "Cabo proteção", "terminals", (32, 12),
          [p("1", "terminal", 2, 6), p("2", "terminal", 30, 6)], tag="W1", graphic="cable")

# Measurement
add_block("meter_voltage", "Voltmeter", "Voltímetro", "measurement", (38, 38),
          [p("+", "signal_in", 8, 34), p("-", "signal_in", 30, 34)], tag="P1", graphic="meter",
          title="V")
add_block("meter_current", "Ammeter", "Amperímetro", "measurement", (38, 38),
          [p("1", "terminal", 8, 4), p("2", "terminal", 8, 34)], tag="P1", graphic="meter",
          title="A")
add_block("meter_power", "Wattmeter", "Wattímetro", "measurement", (46, 38),
          [p("L", "signal_in", 8, 34), p("N", "signal_in", 22, 34),
           p("I", "terminal", 40, 19)], tag="P1", graphic="meter", title="W")
add_block("meter_multimeter", "Multimeter", "Multímetro", "measurement", (42, 42),
          [p("COM", "signal_in", 10, 38), p("V", "signal_in", 22, 38),
           p("A", "signal_in", 34, 38)], tag="P1", graphic="meter", title="M")

# Grafcet
add_block("grafcet_step_initial", "Initial step", "Etapa inicial GRAFCET", "grafcet", (42, 36),
          [p("IN", "signal_in", 8, 32), p("OUT", "signal_out", 34, 32)], tag="E0",
          graphic="grafcet_step", title="0")
add_block("grafcet_step", "Step", "Etapa GRAFCET", "grafcet", (42, 36),
          [p("IN", "signal_in", 8, 32), p("OUT", "signal_out", 34, 32)], tag="E1",
          graphic="grafcet_step", title="1")
add_block("grafcet_transition", "Transition", "Transição GRAFCET", "grafcet", (42, 36),
          [p("IN", "signal_in", 8, 32), p("OUT", "signal_out", 34, 32)], tag="T1",
          graphic="grafcet_transition")
add_block("grafcet_action", "Action", "Ação GRAFCET", "grafcet", (42, 36),
          [p("IN", "signal_in", 8, 32)], tag="A1", graphic="grafcet_action")
add_block("grafcet_link", "Link line", "Linha de enlace GRAFCET", "grafcet", (20, 20), [],
          tag="L1", graphic="grafcet_link")
add_block("grafcet_convergence", "AND convergence", "Convergência em Y", "grafcet", (42, 36),
          [p("IN1", "signal_in", 8, 6), p("IN2", "signal_in", 8, 30),
           p("OUT", "signal_out", 34, 18)], tag="C1", graphic="grafcet_and")
add_block("grafcet_divergence", "OR divergence", "Divergência em O", "grafcet", (42, 36),
          [p("IN", "signal_in", 8, 18), p("OUT1", "signal_out", 34, 6),
           p("OUT2", "signal_out", 34, 30)], tag="D1", graphic="grafcet_or")

# Additional recovered CADe_SIMU types -------------------------------------
add_motor("motor_3ph_1ph", "Three-phase motor as single-phase", "Motor trifásico como monofásico",
          six=True, extra=[p("C1", "terminal", 54, 20), p("C2", "terminal", 54, 30)])
add_block("vfd_dc_control", "DC drive control", "Controle variador CC", "power", (50, 44),
          [p("A1", "coil_input", 8, 4), p("A2", "coil_output", 8, 40),
           p("AO", "signal_out", 42, 22)], tag="U1", graphic="vfd", title="CTRL")
add_block("power_supply", "Power supply", "Fonte de alimentação", "supplies", (46, 52),
          [p("L", "supply_in", 8, 4), p("N", "neutral", 22, 4),
           p("+", "supply_out", 8, 48), p("-", "supply_out", 22, 48)], tag="U1",
          graphic="supply", title="24V")
add_block("hose_entry", "Hose entry", "Entrada de mangueira", "pneumatic", (28, 20),
          [p("1", "terminal", 4, 10), p("2", "terminal", 24, 10)], tag="W1", graphic="hose")
add_block("connector_in", "Input connector", "Conector de entrada", "terminals", (26, 20),
          [p("1", "terminal", 6, 10), p("2", "terminal", 20, 10)], tag="XT1",
          graphic="terminal2")
add_block("connector_out", "Output connector", "Conector de saída", "terminals", (26, 20),
          [p("1", "terminal", 6, 10), p("2", "terminal", 20, 10)], tag="XT1",
          graphic="terminal2")
add_block("terminal_4", "4 round terminals", "4 bornes redondos", "terminals", (54, 18),
          [p("1", "terminal", 7, 9), p("2", "terminal", 20, 9),
           p("3", "terminal", 33, 9), p("4", "terminal", 46, 9)], tag="XT1",
          graphic="terminal4")
for n, suffix, name, pt in [
    (1, "1p", "Disconnector I", "Seccionador I"),
    (2, "1pn", "Disconnector IN", "Seccionador IN"),
    (2, "2p", "Disconnector II", "Seccionador II"),
    (3, "3p", "Disconnector III", "Seccionador III"),
    (4, "3pn", "Disconnector IIIN", "Seccionador IIIN"),
]:
    prefixes = [(str(2 * i + 1), str(2 * i + 2)) for i in range(n)]
    if n == 4:
        prefixes[3] = ("N", "N2")
    add_inline(f"disconnector_{suffix}", name, pt, "protection", "switch_no", n, tag="Q1",
               params={"closed": True}, prefixes=prefixes)
for n, suffix, name, pt in [
    (1, "1p", "Power switch I", "Interruptor I"),
    (2, "2p", "Power switch II", "Interruptor II"),
    (3, "3p", "Power switch III", "Interruptor III"),
    (4, "4p", "Power switch IIII", "Interruptor IIII"),
]:
    prefixes = [(str(2 * i + 1), str(2 * i + 2)) for i in range(n)]
    add_inline(f"switch_power_{suffix}", name, pt, "control", "switch_no", n, tag="Q1",
               params={"closed": True, "mode": "maintained"}, prefixes=prefixes)
add_coil("teleruptor", "Latching relay (teleruptor)", "Telerruptor", "relays", "K1",
         {"rated_voltage": 220, "label_group": ""}, bistable=True)
add_coil("solenoid_valve", "Solenoid valve coil", "Bobina de eletroválvula", "pneumatic", "YV1",
         {"rated_voltage": 24, "label_group": ""})
add_block("horn", "Horn", "Bocina", "indication", (30, 34),
          [p("A1", "coil_input", 10, 4, True), p("A2", "coil_output", 10, 30, True)],
          tag="H1", graphic="sound")
add_block("logic_divider", "Binary divider", "Divisor binário", "logic", (40, 36),
          [p("IN", "signal_in", 6, 18), p("OUT", "signal_out", 34, 18)], tag="U1",
          graphic="logic", title="÷2")
add_block("button_station", "Pushbutton station", "Botonera", "control", (70, 46),
          [p("1", "contact_common", 10, 42, True), p("2", "contact_no", 18, 42, False),
           p("3", "contact_common", 30, 42, True), p("4", "contact_no", 38, 42, False),
           p("5", "contact_common", 50, 42, True), p("6", "contact_no", 58, 42, False)],
          params={"buttons": 3}, tag="S1", graphic="button_station")
for n, suffix, name, pt in [
    (4, "3pn", "Fuse switch IIIN", "Fusível seccionável IIIN"),
]:
    prefixes = [(str(2 * i + 1), str(2 * i + 2)) for i in range(n)]
    prefixes[3] = ("N", "N2")
    add_inline(f"fuse_switch_{suffix}", name, pt, "protection", "fuse", n, tag="F1",
               params={"rated_current": 25, "closed": True}, prefixes=prefixes)

add_block("contactor_combo", "Contactor with coil and auxiliary contacts",
          "Contator com bobina e contatos auxiliares", "contactors", (74, 66),
          [p("A1", "coil_input", 8, 10, True), p("A2", "coil_output", 8, 54, True),
           p("1", "contact_common", 26, 4, True), p("2", "contact_no", 26, 62, True),
           p("3", "contact_common", 40, 4, True), p("4", "contact_no", 40, 62, True),
           p("5", "contact_common", 54, 4, True), p("6", "contact_no", 54, 62, True),
           p("13", "contact_common", 66, 20), p("14", "contact_no", 66, 32),
           p("21", "contact_common", 66, 42), p("22", "contact_nc", 66, 54)],
          params={"rated_voltage": 220, "label_group": ""}, tag="KM1",
          graphic="contactor_combo")
add_block("relay_combo", "Relay with coil and changeover contacts",
          "Relé com bobina e contatos comutados", "relays", (62, 56),
          [p("A1", "coil_input", 8, 10, True), p("A2", "coil_output", 8, 46, True),
           p("11", "contact_common", 42, 10), p("12", "contact_nc", 42, 22),
           p("14", "contact_no", 42, 34), p("21", "contact_common", 54, 10),
           p("22", "contact_nc", 54, 22), p("24", "contact_no", 54, 34)],
          params={"rated_voltage": 24, "label_group": ""}, tag="K1",
          graphic="relay_combo")



from legacy_defs import add_legacy
add_legacy(add_inline, add_block, p)

# ---------------------------------------------------------------------------
# Drawing
# ---------------------------------------------------------------------------

def pin_text(entry, pin, dx=2.0, dy=0.0, anchor="start"):
    return tx(pin.x + dx, pin.y + dy, pin.name, LABEL, 2.4, anchor)


def draw_inline(entry, state=""):
    opts = entry.options
    kind = opts.get("kind", "switch_no")
    poles = opts.get("poles", 1)
    xs = inline_poles(poles)
    active = state == "active"
    failed = state in ("tripped", "blown")
    color = ACTIVE if active and kind not in ("resistor", "fuse") else INK
    if failed:
        color = DANGER
    body = []
    for i, x in enumerate(xs):
        top = entry.pins[2 * i]
        bottom = entry.pins[2 * i + 1]
        body.append(ln(x, top.y, x, 15, color))
        body.append(ln(x, 29, x, bottom.y, color))
        if kind == "fuse":
            body.append(rc(x - 3, 15, 6, 14, color))
            body.append(ln(x, 15, x, 29, color))
            if failed:
                body.append(pa(f"M {x-2:.2f} 17 l 4 4 M {x+2:.2f} 17 l -4 4", DANGER, 1.1))
        elif kind in ("breaker", "rcd"):
            closed = not failed
            if closed:
                body.append(ln(x, 15, x, 29, color))
            else:
                body.append(ln(x, 15, x + 6, 27, color))
            body.append(ci(x, 15, 0.8, color, 0.8, color))
            body.append(ci(x, 29, 0.8, color, 0.8, color))
            if failed:
                body.append(pa(f"M {x+2:.2f} 20 l 3 -3 M {x+5:.2f} 20 l -3 -3", DANGER, 1.0))
            if kind == "rcd":
                body.append(tx(x, 38, "IΔ", INK, 2.0))
        elif kind == "resistor":
            body.append(rc(x - 3, 15, 6, 14, color))
        else:
            nc = kind == "switch_nc"
            closed = active if kind in ("switch_no", "contactor_no") else (not active if nc else active)
            if kind not in ("switch_no", "switch_nc", "contactor_no", "contactor_nc"):
                closed = active
            if closed:
                body.append(ln(x, 15, x, 29, color))
                body.append(ci(x, 22, 0.7, color, 0.8, color))
            else:
                body.append(ln(x, 15, x + 6, 27, color))
                body.append(ci(x, 15, 0.8, color, 0.8, color))
                body.append(ci(x, 29, 0.8, color, 0.8, color))
            # actuator
            act = opts.get("actuator")
            if act and i == 0:
                ax = x
                if act == "pushbutton":
                    body += [ln(ax - 4, 8, ax, 8), ln(ax, 8, ax, 5), ln(ax - 5, 5, ax + 5, 5)]
                elif act == "mushroom":
                    body += [ln(ax - 6, 8, ax, 8), pa(f"M {ax-6:.2f} 8 q 6 -7 12 0"),
                             ln(ax, 5, ax, 2)]
                elif act == "limit":
                    body += [ln(ax - 4, 9, ax, 9), pa(f"M {ax:.2f} 9 l 4 -4")]
            # changeover extra branch
            if entry.type.endswith("_co"):
                pass
        body.append(pin_text(entry, top, 1.6, -0.6))
        body.append(pin_text(entry, bottom, 1.6, 1.8))
    if poles > 1:
        body.append(ln(xs[0], 22, xs[-1], 22, INK, 0.5, "2 2"))
    if opts.get("actuator") == "pushbutton":
        pass
    return "".join(body)


def draw_coil(entry, state=""):
    color = ACTIVE if state == "active" else INK
    body = []
    pins = {pin.name: pin for pin in entry.pins}
    a1 = pins["A1"]
    a2 = pins["A2"]
    body += [ln(a1.x, a1.y, a1.x, 12, color), ln(a1.x, 12, a1.x + 4, 12, color)]
    body += [ln(a2.x, a2.y, a2.x, 22, color), ln(a2.x, 22, a2.x + 4, 22, color)]
    body += [rc(a1.x + 4, 10, 11, 14, color, 0.9, ACTIVE if state == "active" else "none")]
    if entry.options.get("bistable"):
        b1, b2 = pins["B1"], pins["B2"]
        body += [ln(b1.x, b1.y, b1.x, 12, color), ln(b1.x, 12, b1.x + 4, 12, color)]
        body += [ln(b2.x, b2.y, b2.x, 22, color), ln(b2.x, 22, b2.x + 4, 22, color)]
        body += [rc(b1.x + 4, 10, 8, 14, color, 0.9)]
    body.append(tx(a1.x - 2.8, 8, "A1", LABEL, 2.3))
    body.append(tx(a2.x - 2.8, 30, "A2", LABEL, 2.3))
    return "".join(body)


def draw_supply(entry, state=""):
    body = []
    w, h = entry.size
    # horizontal supply bus for multi-phase sources
    terminals = [pin for pin in entry.pins if pin.role in ("supply_in", "supply_out", "neutral")]
    if len(terminals) > 1:
        xs = [pin.x for pin in terminals]
        body.append(ln(min(xs), 14, max(xs), 14, INK, 1.0))
    for pin in entry.pins:
        x, y = pin.x, pin.y
        if pin.role == "earth":
            below = y > h / 2
            y0 = y + 2 if below else y - 2
            body += [ln(x, y, x, y0, INK),
                     ln(x - 5, y0, x + 5, y0, INK),
                     ln(x - 3.5, y0 + (2 if below else -2), x + 3.5, y0 + (2 if below else -2), INK),
                     ln(x - 2, y0 + (4 if below else -4), x + 2, y0 + (4 if below else -4), INK)]
            body.append(tx(x, y0 + (8 if below else -6), "PE", LABEL, 2.4))
            continue
        if pin.role == "neutral":
            body.append(ln(x, y, x, 22, INK))
            body.append(ln(x - 4, 22, x + 4, 22, INK, 1.2))
            body.append(tx(x, y - 2, "N", LABEL, 2.6))
        elif pin.role == "supply_out":
            body.append(ln(x, y, x, 22, INK))
            body.append(ci(x, 22, 2.0, INK, 0.8))
            body.append(tx(x, y - 2, pin.name, LABEL, 2.6))
        else:
            body.append(ln(x, y, x, 22, INK))
            body.append(ci(x, 22, 2.0, INK, 0.8))
            body.append(tx(x, y - 2 if y < h / 2 else y + 4, pin.name, LABEL, 2.8))
    title = entry.options.get("title", "")
    if title and len(terminals) == 1:
        body.append(tx(w / 2, h - 4, title, INK, 3.0, weight="bold"))
    return "".join(body)


def draw_motor(entry, state=""):
    color = ACTIVE if state == "active" else INK
    body = []
    w, h = entry.size
    cx, cy, r = w / 2 + 2, h / 2, 16
    body.append(ci(cx, cy, r, color))
    if entry.options.get("dc") or entry.type == "motor_dc_indep":
        body.append(tx(cx, cy + 3, "M", color, 8))
        body.append(tx(cx, cy + 6.5, "DC", color, 3))
        body.append(ln(cx - 12, 4, cx - 12, cy - r, color))
        body.append(ln(cx - 12, cy + r, cx - 12, h - 4, color))
    else:
        body.append(tx(cx, cy + 2, "M", color, 8))
        body.append(tx(cx, cy + 6.5, "3~" if not entry.options.get("six") else "3~", color, 3.2))
        for i, pin in enumerate([pin for pin in entry.pins if pin.name.startswith(("U", "V", "W")) and pin.y < h / 2]):
            body.append(ln(pin.x, pin.y, pin.x, cy - r * math.sin(math.pi / 3) + 2, color))
    for pin in entry.pins:
        if pin.role == "earth":
            x, y = pin.x, pin.y
            body += [ln(x, y, x, y - 6), ln(x - 4, y - 6, x + 4, y - 6),
                     ln(x - 2.5, y - 3.8, x + 2.5, y - 3.8)]
        elif pin.y > h / 2:
            body.append(ln(pin.x, h / 2 + r * math.sin(math.pi / 3) - 2, pin.x, pin.y, color))
    for pin in entry.pins:
        if pin.role == "earth":
            body.append(tx(pin.x + 3, pin.y - 7, "PE", LABEL, 2.2, "start"))
        else:
            body.append(tx(pin.x + 1.5, pin.y + (-2.0 if pin.y < h / 2 else 3.0), pin.name, LABEL, 2.2,
                           "start"))
    return "".join(body)


def draw_timer(entry, state=""):
    color = ACTIVE if state in ("active",) else INK
    body = []
    pins = {pin.name: pin for pin in entry.pins}
    a1, a2 = pins["A1"], pins["A2"]
    body += [ln(a1.x, a1.y, a1.x, 12, color), ln(a1.x, 12, a1.x + 4, 12, color)]
    body += [ln(a2.x, a2.y, a2.x, 24, color), ln(a2.x, 24, a2.x + 4, 24, color)]
    body.append(rc(a1.x + 4, 10, 12, 16, color, 0.9, ACTIVE if color == ACTIVE else "none"))
    body.append(tx(a1.x + 2, 8, "A1", LABEL, 2.2))
    body.append(tx(a2.x + 2, 30, "A2", LABEL, 2.2))
    # contact
    c = pins.get("15")
    n = pins.get("18") or pins.get("16")
    if c and n:
        body += [ln(c.x, c.y, c.x, c.y + 7, color), ln(n.x, n.y, n.x, n.y - 7, color)]
        if n.name == "16":
            body.append(ln(c.x, c.y + 7, n.x, n.y - 7, color))
        else:
            body.append(ln(c.x, c.y + 7, n.x + 4, n.y - 7, color))
        body.append(tx(c.x - 2, c.y + 1, c.name, LABEL, 2.2))
        body.append(tx(n.x - 2, n.y + 3, n.name, LABEL, 2.2))
    # clock glyph
    body += [ci(34, 16, 5, INK), ln(34, 16, 34, 12), ln(34, 16, 37, 17)]
    return "".join(body)


def draw_sensor(entry, state=""):
    color = ACTIVE if state == "active" else INK
    title = str(entry.options.get("title", "")).lower()
    body = []
    w, h = entry.size
    body.append(rc(6, 10, 20, 24, color))
    if "inductive" in title:
        body.append(pa("M 10 18 q 5 -4 10 0 q -5 4 -10 0", color))
        body.append(ln(26, 18, 32, 18, color))
        body.append(ln(26, 22, 32, 22, color))
    elif "capacitive" in title:
        body.append(ln(12, 16, 12, 28, color, 1.2))
        body.append(ln(18, 16, 18, 28, color, 1.2))
        body.append(ln(24, 22, 30, 22, color))
    elif "magnetic" in title:
        body.append(pa("M 11 18 q 5 -7 10 0 l 0 9", color))
        body.append(pa("M 14 20 q 3 -4 6 0", color))
    elif "photoelectric" in title:
        body.append(ci(12, 18, 2, color))
        body.append(ln(15, 15, 22, 11, color))
        body.append(ln(15, 21, 22, 25, color))
        body.append(pa("M 22 11 l -2 -3 M 23 12 l -3 -1", color))
    for pin in entry.pins:
        body.append(tx(pin.x + (2 if pin.x < w / 2 else -2), pin.y + 1.0, pin.name, LABEL, 2.2,
                       "start" if pin.x < w / 2 else "end"))
    return "".join(body)


def draw_logic_block(entry, state=""):
    color = ACTIVE if state == "active" else INK
    w, h = entry.size
    body = [rc(4, 5, w - 8, h - 12, color)]
    title = str(entry.options.get("title", ""))
    body.append(tx(w / 2, h / 2 + 2, title, color, 5.5, weight="bold"))
    if entry.type in ("logic_nand", "logic_nor", "logic_not"):
        body.append(ci(w - 7, h / 2 - 1, 2.0, color))
    for pin in entry.pins:
        body.append(ln(pin.x, pin.y, 4 if pin.x < w / 2 else w - 4, pin.y, color))
        body.append(tx(pin.x + (2 if pin.x < w / 2 else -2), pin.y - 1.2, pin.name, LABEL,
                       2.2, "start" if pin.x < w / 2 else "end"))
    return "".join(body)


def draw_block(entry, state=""):
    body = []
    w, h = entry.size
    color = ACTIVE if state == "active" else INK
    graphic = entry.options.get("graphic", "block")
    title = entry.options.get("title", "")
    if graphic == "supply":
        return draw_supply(entry, state)
    if graphic == "logic":
        return draw_logic_block(entry, state)
    if graphic == "sr":
        body.append(rc(4, 5, w - 8, h - 12, color))
        body.append(tx(9, 14, "S", LABEL, 3.2, "start"))
        body.append(tx(9, 30, "R", LABEL, 3.2, "start"))
        body.append(tx(w - 9, 14, "Q", LABEL, 3.2, "end"))
        body.append(tx(w - 9, 30, "Qn", LABEL, 3.2, "end"))
        body.append(tx(w / 2 + 2, 22, "SR", color, 3.2))
        return "".join(body)
    if graphic == "edge":
        body.append(rc(4, 9, w - 8, 16, color))
        body.append(tx(w / 2, 21, title or "↑", color, 6))
        return "".join(body)
    if graphic == "timer_block":
        body.append(rc(4, 5, w - 8, h - 12, color))
        body.append(ci(w / 2, 18, 5, color))
        body.append(ln(w / 2, 18, w / 2, 13, color))
        body.append(ln(w / 2, 18, w / 2 + 4, 20, color))
        body.append(tx(w / 2, 30, "TON", color, 3.0))
        return "".join(body)
    if graphic == "sevenseg":
        body.append(rc(6, 6, w - 12, h - 12, color))
        seg = [(14, 12, 28, 12), (28, 13, 28, 24), (28, 26, 28, 36), (14, 37, 28, 37),
               (12, 26, 12, 36), (12, 13, 12, 24), (14, 24, 26, 24)]
        for x1, y1, x2, y2 in seg:
            body.append(ln(x1, y1, x2, y2, color, 1.8))
        return "".join(body)
    if graphic == "button_station":
        for i, x in enumerate((18, 36, 54)):
            body.append(ci(x, 16, 6, color))
            body.append(ln(x - 5, 10, x + 5, 10, color))
            body.append(ln(x, 10, x, 6, color))
            body.append(ln(x - 6, 6, x + 6, 6, color))
            body.append(ln(x - 4, 42, x - 4, 22, color))
            body.append(ln(x + 4, 42, x + 4, 22, color))
        return "".join(body)
    if graphic == "co_contact":
        body.append(ln(12, 4, 12, 15, INK))
        body.append(ci(12, 15, 0.8, INK, 0.8, INK))
        body.append(ln(12, 15, 22, 30, INK))
        body.append(ci(12, 36, 0.8, INK, 0.8, INK))
        body.append(ln(22, 30, 22, 40, INK))
        body.append(ln(6, 40, 6, 30, INK))
        body.append(tx(8, 38, "12", LABEL, 2.2))
        body.append(tx(16, 38, "14", LABEL, 2.2))
        body.append(tx(9, 4, "11", LABEL, 2.2))
        return "".join(body)
    if graphic == "co_pushbutton":
        body.append(ln(10, 4, 10, 15, INK))
        body.append(ci(10, 15, 0.8, INK, 0.8, INK))
        body.append(ln(10, 15, 22, 30, INK))
        body.append(ci(10, 36, 0.8, INK, 0.8, INK))
        body.append(ln(22, 30, 22, 40, INK))
        body.append(ln(6, 40, 6, 30, INK))
        body.append(ln(4, 9, 16, 9))
        body.append(ln(10, 9, 10, 6))
        body.append(ln(5, 6, 15, 6))
        body.append(tx(8, 4, "11", LABEL, 2.2))
        return "".join(body)
    if graphic == "selector":
        body.append(rc(4, 7, w - 8, 22, INK))
        body.append(ci(w / 2, 18, 4, INK))
        angle = 0.0
        if state.startswith("position_"):
            try:
                angle = (int(state.split("_")[1]) - 1) * 40.0 - 40.0
            except ValueError:
                angle = 0.0
        import math as _math
        rad = _math.radians(angle)
        body.append(ln(w / 2, 18, w / 2 + 12 * _math.sin(rad), 18 - 12 * _math.cos(rad), INK, 1.0))
        for pin in [pin for pin in entry.pins if pin.name in ("2", "3", "4")]:
            body.append(ln(pin.x, pin.y, pin.x, 29, INK))
            body.append(ci(pin.x, 29, 0.8, INK, 0.8, INK))
        body.append(ln(8, 40, w - 8, 40, INK))
        return "".join(body)
    if graphic == "crossover":
        body.append(ln(8, 4, 8, 16, INK)); body.append(ln(34, 4, 34, 16, INK))
        body.append(ln(8, 16, 34, 24, INK)); body.append(ln(34, 16, 8, 24, INK))
        body.append(ci(8, 16, 0.8, INK, 0.8, INK)); body.append(ci(34, 16, 0.8, INK, 0.8, INK))
        body.append(ln(8, 24, 8, 40, INK)); body.append(ln(34, 24, 34, 40, INK))
        return "".join(body)
    if graphic == "timer":
        return draw_timer(entry, state)
    if graphic == "flasher":
        body.append(rc(6, 8, 16, 20, color))
        body.append(ln(14, 4, 14, 8, color)); body.append(ln(14, 28, 14, 40, color))
        body.append(pa("M 26 14 l 8 4 l -8 4 z", INK))
        return "".join(body)
    if graphic == "clock":
        body += [ci(w / 2, 18, 9, INK), ln(w / 2, 18, w / 2, 11), ln(w / 2, 18, w / 2 + 6, 20)]
        body.append(ln(8, 4, 8, 40, INK))
        return "".join(body)
    if graphic == "counter":
        body.append(rc(4, 8, w - 8, 22, color))
        body.append(tx(w / 2, 23, "C", color, 7))
        body.append(tx(w / 2, 39, "preset", INK, 2.2))
        return "".join(body)
    if graphic == "lamp":
        body.append(ci(w / 2, 17, 8, color))
        body.append(ln(w / 2 - 6, 11, w / 2 + 6, 23, color))
        body.append(ln(w / 2 + 6, 11, w / 2 - 6, 23, color))
        body.append(ln(10, 4, 10, 9, color)); body.append(ln(10, 25, 10, 30, color))
        return "".join(body)
    if graphic == "sound":
        body.append(pa(f"M {w/2-8:.2f} 11 l 10 -5 l 0 20 l -10 -5 z", color))
        body.append(pa(f"M {w/2+4:.2f} 10 q 7 7 0 14", color))
        body.append(ln(10, 4, 10, 10, color)); body.append(ln(10, 24, 10, 30, color))
        return "".join(body)
    if graphic == "led":
        body.append(pa(f"M {w/2-6:.2f} 14 l 12 0 l -6 8 z", color))
        body.append(ln(w / 2 - 6, 22, w / 2 + 6, 22, color))
        body.append(ln(w / 2 + 3, 8, w / 2 + 7, 4, color)); body.append(ln(w / 2 + 6, 10, w / 2 + 10, 6, color))
        return "".join(body)
    if graphic == "stepper":
        body.append(ci(w / 2, 24, 15, color))
        body.append(tx(w / 2, 27, "M", color, 7))
        body.append(tx(w / 2, 31, "STEP", color, 2.6))
        return "".join(body)
    if graphic == "actuator":
        body.append(rc(6, 8, 30, 14, color))
        body.append(ln(36, 15, 50, 15, color))
        body.append(tx(16, 27, "M", color, 5))
        return "".join(body)
    if graphic == "cylinder":
        body.append(rc(8, 8, 34, 20, color))
        body.append(ln(42, 18, 58, 18, color, 1.4))
        body.append(rc(8, 8, 10, 20, color, 0.8, INK))
        body.append(ln(8, 4, 8, 8, color)); body.append(ln(8, 28, 8, 36, color))
        for pin in entry.pins:
            if pin.y < entry.size[1] / 2:
                body.append(ln(0, pin.y, 8, pin.y, color))
            else:
                body.append(ln(0, pin.y, 8, pin.y, color))
            body.append(tx(pin.x + 1.5, pin.y - 1.0, pin.name, LABEL, 2.1, "start"))
        return "".join(body)
    if graphic == "rotary":
        body.append(ci(w / 2, h / 2, 13, color))
        body.append(pa(f"M {w/2-8:.2f} {h/2:.2f} a 8 8 0 1 1 16 0", color))
        body.append(pa(f"M {w/2+8:.2f} {h/2-2:.2f} l 0 5 l -5 -2", color))
        return "".join(body)
    if graphic == "gripper":
        body.append(pa(f"M 10 8 l 12 10 l -12 10", color))
        body.append(pa(f"M 38 8 l -12 10 l 12 10", color))
        body.append(ln(22, 18, 26, 18, color))
        return "".join(body)
    if graphic == "vacuum":
        body.append(pa(f"M 10 12 q 11 12 22 0", color))
        body.append(ln(21, 12, 21, 4, color))
        body.append(ci(21, 26, 3, color))
        return "".join(body)
    if graphic == "airmotor":
        body.append(ci(w / 2, h / 2, 14, color))
        body.append(tx(w / 2, h / 2 + 3, "M", color, 7))
        body.append(pa(f"M {w/2-18:.2f} {h/2:.2f} l 8 -3 l 0 6 z", color))
        return "".join(body)
    if graphic == "valve":
        body.append(rc(8, 8, w - 16, 16, color))
        body.append(rc(8, 24, w - 16, 14, color))
        body.append(pa(f"M 12 16 l 8 0 l -3 -3 M 20 16 l -3 3", color))
        body.append(pa(f"M {w-12:.2f} 30 l -8 0 l 3 -3 M {w-20:.2f} 30 l 3 3", color))
        body.append(tx(w / 2, 5, title, INK, 2.8))
        return "".join(body)
    if graphic == "hose":
        body.append(pa(f"M 4 8 q 8 -8 16 0 q 8 8 18 0", INK, 1.2))
        return "".join(body)
    if graphic == "sensor":
        return draw_sensor(entry, state)
    if graphic == "pressure":
        body.append(rc(8, 12, 24, 20, color))
        body.append(tx(16, 26, "P", color, 6))
        body.append(pa("M 24 24 q 4 -8 8 0", color))
        return "".join(body)
    if graphic == "thermostat":
        body.append(rc(8, 10, 20, 24, color))
        body.append(pa("M 12 30 q 6 -18 12 0", color))
        body.append(tx(16, 24, "θ", color, 5))
        return "".join(body)
    if graphic == "float":
        body.append(ci(16, 16, 7, color))
        body.append(pa("M 6 32 q 5 -5 10 0 q 5 5 10 0", color))
        body.append(ln(16, 23, 16, 30, color))
        return "".join(body)
    if graphic == "reed":
        body.append(rc(6, 10, 16, 20, color, 0.8, rx=8))
        body.append(ln(9, 20, 15, 26, color))
        body.append(ln(19, 20, 13, 26, color))
        return "".join(body)
    if graphic == "thermal_relay":
        body.append(rc(4, 5, 24, 50, INK))
        body.append(pa("M 8 18 q 8 5 0 10 q -8 5 0 10", INK))
        body.append(ln(28, 10, 50, 10, INK)); body.append(ln(28, 22, 40, 22, INK))
        body.append(circle if False else pa("M 42 18 l 4 4 l -4 4", INK))
        body.append(tx(34, 30, "F", INK, 5))
        return "".join(body)
    if graphic == "plc":
        body.append(rc(4, 4, w - 8, h - 8, color, 1.0))
        body.append(rc(8, 8, w - 16, 12, color, 0.6))
        body.append(tx(w / 2, 17, title or "PLC", color, 3.2, weight="bold"))
        body.append(ln(8, 24, w - 8, 24, color, 0.5, "2 2"))
        for pin in entry.pins:
            if pin.role in ("supply_in", "neutral", "earth"):
                continue
            body.append(tx(pin.x + 1.5, pin.y - 1.5, pin.name, LABEL, 2.1, "start"))
        return "".join(body)
    if graphic == "plc_io":
        body.append(rc(4, 4, w - 8, h - 8, color, 1.0))
        body.append(rc(8, 8, w - 16, 12, color, 0.6))
        body.append(tx(w / 2, 17, title, color, 3.2, weight="bold"))
        for pin in entry.pins:
            if pin.role in ("supply_in", "neutral", "earth"):
                continue
            body.append(ln(pin.x, pin.y, pin.x, pin.y - 5, color))
            body.append(tx(pin.x + 1.2, pin.y - 6, pin.name, LABEL, 2.0, "start"))
        return "".join(body)
    if graphic == "plc_power":
        body.append(rc(4, 4, w - 8, h - 8, color, 1.0))
        body.append(tx(w / 2, 22, "24V", color, 4))
        return "".join(body)
    if graphic == "relay_block":
        body.append(rc(4, 8, w - 8, h - 16, color))
        body.append(tx(w / 2, 24, "U>", color, 5))
        return "".join(body)
    if graphic == "contactor_combo":
        body.append(rc(8, 22, 12, 20, color, 0.9, ACTIVE if state == "active" else "none"))
        body.append(ln(8, 10, 8, 22, color)); body.append(ln(8, 42, 8, 54, color))
        for x in (26, 40, 54):
            body.append(ln(x, 4, x, 22, color)); body.append(ln(x, 42, x, 62, color))
            closed = state == "active"
            body.append(ln(x, 22, x, 42 if closed else 36, ACTIVE if closed else color))
            body.append(ci(x, 22, 0.8, color, 0.8, color))
            body.append(ci(x, 42, 0.8, color, 0.8, color))
        body.append(ln(26, 32, 54, 32, INK, 0.5, "2 2"))
        body.append(ln(66, 20, 66, 26, color)); body.append(ln(66, 26, 70, 26, color))
        body.append(ln(66, 32, 66, 38, color)); body.append(ln(66, 38, 70, 38, color))
        body.append(ci(66, 26, 0.7, color, 0.8, color)); body.append(ci(66, 32, 0.7, color, 0.8, color))
        body.append(ln(66, 42, 66, 48, color)); body.append(ln(66, 48, 70, 48, color))
        body.append(ln(66, 54, 66, 60, color)); body.append(ln(66, 60, 70, 60, color))
        body.append(ci(66, 48, 0.7, color, 0.8, color)); body.append(ci(66, 60, 0.7, color, 0.8, color))
        return "".join(body)
    if graphic == "relay_combo":
        body.append(rc(8, 18, 12, 20, color, 0.9, ACTIVE if state == "active" else "none"))
        body.append(ln(8, 10, 8, 18, color)); body.append(ln(8, 38, 8, 46, color))
        for x in (42, 54):
            body.append(ln(x, 10, x, 22, color)); body.append(ln(x, 34, x, 46, color))
            body.append(ln(x, 22, x, 28, color)); body.append(ci(x, 22, 0.7, color, 0.8, color))
            body.append(ci(x, 28, 0.7, color, 0.8, color))
            body.append(ln(x, 28, x + 6, 22, color))
            body.append(ln(x + 6, 22, x + 6, 10, color))
        return "".join(body)
    if graphic == "meter":
        body.append(ci(w / 2, 17, 11, color))
        body.append(tx(w / 2, 21, title, color, 7))
        return "".join(body)
    if graphic == "ssr":
        body.append(rc(8, 8, 20, 24, color))
        body.append(tx(18, 22, "SSR", color, 4))
        return "".join(body)
    if graphic == "ssr3":
        body.append(rc(4, 8, w - 8, 30, color))
        body.append(tx(w / 2, 25, "SSR", color, 5))
        return "".join(body)
    if graphic == "surge":
        body.append(ln(8, 8, 8, 22, color))
        body.append(pa("M 4 22 l 8 0 l -4 10 z", color))
        body.append(ln(8, 32, 8, 38, color))
        return "".join(body)
    if graphic == "diode":
        body.append(pa(f"M 5 12 l 0 12 l 14 -12 z", color))
        body.append(ln(5, 24, 19, 12, color))
        body.append(ln(8, 4, 8, 12, color)); body.append(ln(8, 24, 8, 36, color))
        return "".join(body)
    if graphic == "rectifier":
        body.append(pa("M 8 8 l 12 12 l -12 12 z", color))
        body.append(ln(20, 20, 30, 20, color))
        body.append(ln(8, 4, 8, 8, color)); body.append(ln(8, 32, 8, 40, color))
        return "".join(body)
    if graphic == "transformer":
        body.append(rc(8, 8, 12, 36, color)); body.append(rc(24, 8, 12, 36, color))
        body.append(ln(14, 4, 14, 8, color)); body.append(ln(30, 4, 30, 8, color))
        return "".join(body)
    if graphic == "battery":
        body.append(ln(8, 12, 8, 28, color)); body.append(ln(18, 16, 18, 24, color))
        body.append(ln(8, 4, 8, 12, color)); body.append(ln(18, 4, 18, 16, color))
        body.append(tx(8, 2, "+", LABEL, 2.6)); body.append(tx(18, 2, "-", LABEL, 2.6))
        return "".join(body)
    if graphic == "terminal":
        body.append(ci(8, 8, 4, color))
        body.append(tx(8, 9.5, entry.pins[0].name if entry.pins else "1", LABEL, 2.4))
        return "".join(body)
    if graphic == "terminal_square":
        body.append(rc(3, 3, 10, 10, color))
        body.append(tx(8, 9.5, entry.pins[0].name if entry.pins else "1", LABEL, 2.4))
        return "".join(body)
    if graphic == "terminal2":
        body.append(ci(8, 9, 4, color)); body.append(ci(22, 9, 4, color))
        for pin in entry.pins:
            body.append(tx(pin.x, pin.y + 1.5, pin.name, LABEL, 2.2))
        return "".join(body)
    if graphic == "terminal3":
        for x in (8, 21, 34):
            body.append(ci(x, 9, 4, color))
        for pin in entry.pins:
            body.append(tx(pin.x, pin.y + 1.5, pin.name, LABEL, 2.2))
        return "".join(body)
    if graphic == "terminal4":
        for x in (7, 20, 33, 46):
            body.append(ci(x, 9, 4, color))
        for pin in entry.pins:
            body.append(tx(pin.x, pin.y + 1.5, pin.name, LABEL, 2.2))
        return "".join(body)
    if graphic == "junction":
        body.append(ci(8, 8, 2.2, color, 0.8, color))
        return "".join(body)
    if graphic == "cable":
        body.append(ln(2, 6, 30, 6, color, 1.0))
        return "".join(body)
    if graphic == "cable3":
        for y in (7, 13, 19):
            body.append(ln(2, y, 40, y, color, 0.9))
        return "".join(body)
    if graphic == "grafcet_step":
        body.append(rc(8, 8, 26, 18, color))
        body.append(tx(21, 22, title, color, 5))
        return "".join(body)
    if graphic == "grafcet_transition":
        body.append(ln(6, 18, 36, 18, color, 1.4))
        return "".join(body)
    if graphic == "grafcet_action":
        body.append(rc(6, 8, 30, 18, color))
        body.append(tx(21, 21, "A", color, 5))
        return "".join(body)
    if graphic == "grafcet_link":
        body.append(ln(2, 10, 18, 10, color))
        return "".join(body)
    if graphic == "grafcet_and":
        body.append(ln(8, 6, 20, 18, color)); body.append(ln(8, 30, 20, 18, color))
        body.append(ln(20, 18, 34, 18, color))
        return "".join(body)
    if graphic == "grafcet_or":
        body.append(ln(8, 18, 18, 6, color)); body.append(ln(8, 18, 18, 30, color))
        body.append(ln(18, 6, 34, 6, color)); body.append(ln(18, 30, 34, 30, color))
        return "".join(body)
    if graphic == "label":
        body.append(rc(1, 1, w - 2, h - 2, INK, 0.5, dash="2 2"))
        body.append(tx(w / 2, h / 2 + 2, entry.params.get("text", "Label"), INK, 4))
        return "".join(body)
    # generic fallback block
    body.append(rc(4, 4, w - 8, h - 8, color))
    return "".join(body)


def draw_entry(entry, state=""):
    family = entry.family
    if family == "inline":
        return draw_inline(entry, state)
    if family == "coil":
        return draw_coil(entry, state)
    if family == "supply":
        return draw_supply(entry, state)
    if family == "motor":
        return draw_motor(entry, state)
    if family == "timer":
        return draw_timer(entry, state)
    return draw_block(entry, state)


# ---------------------------------------------------------------------------
# Output
# ---------------------------------------------------------------------------

def state_map(entry):
    family = entry.family
    if family == "coil":
        return {"active": f"{entry.type}_active.svg"}
    if family == "timer":
        return {"active": f"{entry.type}_active.svg"}
    if family == "motor":
        return {"active": f"{entry.type}_active.svg"}
    if family == "supply":
        return {}
    if family == "inline":
        kind = entry.options.get("kind")
        if kind == "fuse":
            return {"blown": f"{entry.type}_blown.svg"}
        if kind in ("breaker", "rcd"):
            return {"tripped": f"{entry.type}_tripped.svg"}
        return {"active": f"{entry.type}_active.svg"}
    graphic = entry.options.get("graphic")
    if graphic in ("lamp", "sound", "led", "sensor", "pressure", "reed", "thermostat",
                   "float", "logic", "sr", "edge", "timer_block", "sevenseg", "relay_block",
                   "contactor_combo", "relay_combo", "co_contact", "co_pushbutton", "crossover",
                   "button_station"):
        return {"active": f"{entry.type}_active.svg"}
    if graphic == "thermal_relay":
        return {"tripped": f"{entry.type}_tripped.svg"}
    if graphic == "selector":
        return {f"position_{i}": f"{entry.type}_position_{i}.svg" for i in range(1, 4)}
    return {}


def infer_schema(entry):
    if entry.schema:
        return entry.schema
    labels = {
        "rated_voltage": ("Rated voltage", "Tensão nominal", "V"),
        "rated_current": ("Rated current", "Corrente nominal", "A"),
        "trip_current": ("Trip current", "Corrente de disparo", "A"),
        "power_kw": ("Power", "Potência", "kW"),
        "delay_ms": ("Delay", "Tempo", "ms"),
        "delay_on_ms": ("ON delay", "Tempo de conexão", "ms"),
        "delay_off_ms": ("OFF delay", "Tempo de desconexão", "ms"),
        "period_ms": ("Period", "Período", "ms"),
        "time_base": ("Time base", "Base de tempo", ""),
        "label_group": ("Label group", "Grupo de etiqueta", ""),
        "positions": ("Positions", "Posições", ""),
        "preset": ("Preset", "Preset", ""),
        "rated_current": ("Rated current", "Corrente nominal", "A"),
        "sensitivity_ma": ("Sensitivity", "Sensibilidade", "mA"),
        "color": ("Color", "Cor", ""),
        "mode": ("Mode", "Modo", ""),
        "connection": ("Connection", "Ligação", ""),
        "min_live_phases": ("Minimum live phases", "Fases mínimas", ""),
        "threshold": ("Threshold", "Limiar", ""),
        "setpoint": ("Setpoint", "Setpoint", ""),
        "voltage": ("Voltage", "Tensão", "V"),
        "frequency": ("Frequency", "Frequência", "Hz"),
        "stroke_mm": ("Stroke", "Curso", "mm"),
        "primary_voltage": ("Primary voltage", "Tensão primária", "V"),
        "secondary_voltage": ("Secondary voltage", "Tensão secundária", "V"),
        "spring_return": ("Spring return", "Retorno por mola", ""),
        "latching": ("Latching", "Retenção", ""),
    }
    schema = []
    for key, value in entry.params.items():
        label, label_pt, unit = labels.get(key, (key.replace("_", " ").title(), key, ""))
        kind = "bool" if isinstance(value, bool) else "int" if isinstance(value, int) else (
            "number" if isinstance(value, float) else "string")
        item = {"key": key, "type": kind, "label": label, "label_pt": label_pt, "unit": unit}
        if kind in ("int", "number"):
            item["min"] = 0
        schema.append(item)
    return schema


def write_manifest(entry, states):
    w, h = entry.size
    manifest = {
        "symbol_version": 1,
        "type": entry.type,
        "display_name": entry.name,
        "display_name_pt": entry.name_pt,
        "category": entry.category,
        "size": [w, h],
        "pins": [
            {"name": pin.name, "role": pin.role, "x": pin.x, "y": pin.y,
             "required": bool(pin.required)}
            for pin in entry.pins
        ],
        "default_parameters": entry.params,
        "parameter_schema": infer_schema(entry),
        "states": states,
        "element_ids": {},
    }
    path = os.path.join(ASSETS, entry.type + ".json")
    with open(path, "w", encoding="utf-8") as handle:
        json.dump(manifest, handle, indent=2, ensure_ascii=False)
        handle.write("\n")


def main():
    os.makedirs(ASSETS, exist_ok=True)
    os.makedirs(DOCS, exist_ok=True)
    written = []
    for entry in CATALOG:
        states = state_map(entry)
        entry.states = states
        base = svg(entry, draw_entry(entry))
        with open(os.path.join(ASSETS, entry.type + ".svg"), "w", encoding="utf-8") as handle:
            handle.write(base)
        for state, filename in states.items():
            body = draw_entry(entry, state)
            with open(os.path.join(ASSETS, filename), "w", encoding="utf-8") as handle:
                handle.write(svg(entry, body))
        write_manifest(entry, states)
        written.append(entry)
    # Remove stale generated manifests from a previous run that are no longer in the catalog.
    keep = {entry.type for entry in CATALOG} | {
        "supply_ac1_pe",  # legacy alias kept as a distinct catalog entry
    }
    for name in os.listdir(ASSETS):
        path = os.path.join(ASSETS, name)
        if not os.path.isfile(path):
            continue
        if name.endswith(".json") and name[:-5] not in keep:
            # Keep the hand-written README and any non-catalog file.
            continue
    print(f"wrote {len(written)} component definitions to {ASSETS}")
    with open(os.path.join(DOCS, "catalog.md"), "w", encoding="utf-8") as handle:
        handle.write("# CADe_SIMU-style component catalog\n\n")
        handle.write(f"{len(written)} types generated from the reconstructed catalog.\n\n")
        handle.write("| Type | Name | PT | Category | Pins | States |\n")
        handle.write("|---|---|---|---|---|---|\n")
        for entry in written:
            handle.write(f"| `{entry.type}` | {entry.name} | {entry.name_pt} | {entry.category} | "
                         f"{len(entry.pins)} | {', '.join(entry.states) or '-'} |\n")


if __name__ == "__main__":
    main()

