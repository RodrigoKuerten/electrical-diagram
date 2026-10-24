import json
from pathlib import Path


def value(text):
    if isinstance(text, bool):
        return "true" if text else "false"
    if isinstance(text, int):
        return str(text)
    if isinstance(text, float):
        return format(text, ".12g")
    return str(text)


def quote(text):
    return '"' + str(text).replace('"', '""') + '"'


document = json.loads(Path("examples/partida-direta.dcj").read_text(encoding="utf-8"))
rows = []
for sheet in document["sheets"]:
    for component in sheet["components"]:
        parameters = "; ".join(
            f"{key}={value(item)}"
            for key, item in sorted(component.get("parameters", {}).items())
        )
        rows.append(
            (
                sheet["name"],
                component["tag"],
                component["type"],
                component.get("description", ""),
                parameters,
            )
        )
rows.sort(key=lambda row: (row[2], row[1], row[0]))
lines = ['"Sheet","Tag","Type","Description","Parameters"']
lines += [",".join(quote(field) for field in row) for row in rows]
Path("tests/data/golden/partida-direta-bom.csv").write_bytes(
    ("\n".join(lines) + "\n").encode("utf-8")
)
print(f"wrote {len(rows)} rows")
