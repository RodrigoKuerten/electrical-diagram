"""Parse MFC RT_TOOLBAR (type 241) and RT_STRING resources dumped from CADe_SIMU."""
from __future__ import annotations
import glob, json, os, struct, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
RES = os.path.join(ROOT, "docs", "cadesimu", "resources")


def parse_strings():
    strings = {}
    for path in glob.glob(os.path.join(RES, "6_STRING", "*.str")):
        block = int(os.path.basename(path).split("_")[0])
        data = open(path, "rb").read()
        off = 0
        for i in range(16):
            if off + 2 > len(data):
                break
            n = struct.unpack_from("<H", data, off)[0]
            off += 2
            if n:
                s = data[off:off + n * 2].decode("utf-16le", errors="replace")
                sid = (block - 1) * 16 + i + 1
                strings[sid] = s
            off += n * 2
    return strings


def parse_toolbars():
    bars = {}
    for path in glob.glob(os.path.join(RES, "241_241", "*.bin")):
        name = os.path.basename(path).split("_")[0]
        if not name.isdigit():
            continue
        # Prefer English (1033), else Spanish (3082)
        if not path.endswith("_1033.bin") and os.path.exists(os.path.join(RES, "241_241", f"{name}_1033.bin")):
            continue
        data = open(path, "rb").read()
        if len(data) < 8:
            continue
        version, width, height, count = struct.unpack_from("<HHHH", data, 0)
        items = list(struct.unpack_from("<" + "H" * count, data, 8)) if count and len(data) >= 8 + count * 2 else []
        bars[int(name)] = {"file": path, "version": version, "width": width, "height": height,
                           "count": count, "items": items}
    return bars


def main():
    strings = parse_strings()
    bars = parse_toolbars()
    print(f"strings={len(strings)} bars={len(bars)}")
    out = {}
    for bid, bar in sorted(bars.items()):
        rows = []
        for cmd in bar["items"]:
            if cmd == 0:
                rows.append({"command": 0, "text": ""})
                continue
            rows.append({"command": cmd, "id": cmd, "text": strings.get(cmd, ""), "hex": hex(cmd)})
        out[bid] = {**bar, "buttons": rows}
        nonempty = [r for r in rows if r["command"] and r["text"]]
        print(f"\n=== TOOLBAR {bid} ({bar['count']} items, {len(nonempty)} named) ===")
        for r in rows:
            print(f"  {r['command']:5d} 0x{r['command']:04X} {r['text']!r}")
    json.dump(out, open(os.path.join(ROOT, "docs", "cadesimu", "toolbars.json"), "w", encoding="utf-8"),
              indent=1, ensure_ascii=False)
    json.dump({str(k): v for k, v in strings.items()},
              open(os.path.join(ROOT, "docs", "cadesimu", "strings.json"), "w", encoding="utf-8"),
              indent=0, ensure_ascii=False)


if __name__ == "__main__":
    main()
