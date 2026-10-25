import re, json, os

root = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
path = os.path.join(root, "docs", "cadesimu", "decompiled", "FUN_0087aca0_0087aca0.c")
text = open(path, encoding="utf-8", errors="replace").read()

# Split on each comparison with the global current type id.
pat = re.compile(r"if \(DAT_00bd95cc == (0x[0-9a-fA-F]+|\d+)\) \{")
matches = list(pat.finditer(text))
rows = []
for i, m in enumerate(matches):
    start = m.end()
    end = matches[i + 1].start() if i + 1 < len(matches) else min(len(text), start + 12000)
    body = text[start:end]
    name = ""
    for sm in re.finditer(r'Ordinal_736\("([^"]*)"\)|Ordinal_736\(&([A-Za-z0-9_]+)\)', body):
        name = sm.group(1) or sm.group(2)
        break
    nums = [int(x) for x in re.findall(r"Ordinal_4951\((\d+)\)", body)]
    if name:
        rows.append({"id": int(m.group(1), 0), "name": name, "nums": nums[:40], "index": i})

# Deduplicate ids keeping the first non-empty name.
seen = {}
for r in rows:
    if r["id"] not in seen or (not seen[r["id"]]["name"] and r["name"]):
        seen[r["id"]] = r
out = [seen[k] for k in sorted(seen)]
json.dump(out, open(os.path.join(root, "docs", "cadesimu", "type_names.json"), "w", encoding="utf-8"),
          indent=1, ensure_ascii=False)
print("types", len(out))
for r in out:
    print(f"{r['id']:5d} 0x{r['id']:04x} {r['name']!r} nums={r['nums'][:12]}")
