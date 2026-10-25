import json, os, re, pefile

root = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
exe = r"C:\Users\rodri\Downloads\CADe_SIMU_4_0\CADe_SIMU_4_0\CADe_SIMU.exe"
pe = pefile.PE(exe)
base = pe.OPTIONAL_HEADER.ImageBase
rows = json.load(open(os.path.join(root, "docs", "cadesimu", "type_names.json"), encoding="utf-8"))

def read_cstr(addr):
    rva = addr - base
    try:
        data = pe.get_data(rva, 256)
    except Exception:
        return None
    end = data.find(b"\0")
    if end >= 0:
        data = data[:end]
    for enc in ("utf-8", "cp1252", "latin1"):
        try:
            s = data.decode(enc)
            if s and all(32 <= ord(c) < 127 or ord(c) > 127 for c in s):
                return s
        except Exception:
            pass
    return None

for r in rows:
    if r["name"].startswith("DAT_"):
        addr = int(r["name"][4:], 16)
        s = read_cstr(addr)
        if s:
            r["resolved"] = s
    print(f"{r['id']:5d} {r['name']!r} -> {r.get('resolved','')!r}")
json.dump(rows, open(os.path.join(root, "docs", "cadesimu", "type_names.json"), "w", encoding="utf-8"),
          indent=1, ensure_ascii=False)
