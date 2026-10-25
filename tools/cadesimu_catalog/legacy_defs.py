"""Legacy FioVivo component types retained for project/test compatibility.

The catalog replaces the artwork of these types, but their type identifiers, pin names and
roles remain stable so existing projects, examples and tests keep loading.
"""


def add_legacy(add_inline, add_block, p):
    add_inline("fuse", "Fuse", "Fusível", "protection", "fuse", 1, tag="F1",
               params={"rated_current": 10, "blown": False},
               prefixes=[("1", "2")], roles=[("contact_common", "contact_common")])
    add_inline("breaker", "Circuit breaker", "Disjuntor", "protection", "breaker", 1, tag="Q1",
               params={"rated_current": 16, "poles": 1, "tripped": False},
               prefixes=[("1", "2")], roles=[("contact_common", "contact_common")])
    add_inline("contactor_no", "Contactor main contact", "Contato principal de contator",
               "contactors", "contactor_no", 1, tag="KM1.1", params={"label_group": ""},
               prefixes=[("1", "2")], roles=[("contact_common", "contact_no")])
    add_block("selector_switch", "Selector switch", "Chave seletora", "control", (60, 50),
              [p("1", "contact_common", 10, 40, True), p("2", "contact_no", 25, 40),
               p("3", "contact_no", 40, 40), p("4", "contact_no", 55, 40)],
              params={"positions": 2, "mode": "maintained", "position": 1, "label_group": ""},
              tag="SEL1", graphic="selector", title="")
    add_block("thermal_relay", "Thermal overload relay", "Relé térmico", "protection", (60, 60),
              [p("95", "contact_common", 10, 10, True), p("96", "contact_nc", 10, 22, True),
               p("97", "contact_common", 10, 34, True), p("98", "contact_no", 10, 46, True)],
              params={"trip_current": 10, "reset_mode": "manual", "label_group": "",
                      "tripped": False}, tag="F1", graphic="thermal_relay")
    add_block("label_box", "Text label", "Etiqueta de texto", "annotations", (60, 20), [],
              params={"text": "Label", "font_size": 3.5}, tag="TXT1", graphic="label")
