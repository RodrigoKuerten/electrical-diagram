"""Inspect one GDI trace: primitives, endpoints, labels and bbox.

Usage: python inspect_gdi.py <trace-name-without-.json>
"""
import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import make_svg_from_gdi as m

GDI = os.path.join(ROOT, "docs", "cadesimu", "gdi")


def load(name):
    base = json.load(open(os.path.join(GDI, "baseline.json"), encoding="utf-8"))
    run = json.load(open(os.path.join(GDI, name + ".json"), encoding="utf-8"))
    items = m.extras(base, run)
    items = [x for x in items if not (x["fn"].startswith("TextOut") and
                                      (m.is_tag(x.get("text", "")) or not m.clean_text(x.get("text", ""))))]

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

    return [x for x in items if near(x)]


def main():
    name = sys.argv[1]
    items = load(name)
    if not items:
        print("no geometry")
        return
    x0, y0, x1, y1 = m.bounds(items)
    print(f"bbox {x0},{y0} -> {x1},{y1}  ({(x1-x0)/m.SCALE:.2f} x {(y1-y0)/m.SCALE:.2f} mm)")
    print("\n-- primitives --")
    last = None
    for x in items:
        fn = x["fn"]
        if fn == "MoveToEx":
            last = (x["x"], x["y"])
            print(f"  M {x['x']:4d},{x['y']:4d}")
        elif fn == "LineTo":
            print(f"  L {x['x']:4d},{x['y']:4d}")
            last = (x["x"], x["y"])
        elif fn in ("Ellipse", "Rectangle"):
            print(f"  {fn[0]} {x['l']},{x['t']} .. {x['r']},{x['b']}")
        elif fn.startswith("TextOut"):
            print(f"  T {x['x']:4d},{x['y']:4d} {x.get('text','')!r}")
        else:
            print(" ", x)
    print("\n-- line endpoints --")
    eps = []
    for x in items:
        if x["fn"] == "MoveToEx":
            eps.append((x["x"], x["y"]))
        elif x["fn"] == "LineTo":
            eps.append((x["x"], x["y"]))
    # Leaf endpoints: count occurrences; points that are only an endpoint or far from others.
    counts = {}
    for p in eps:
        counts[p] = counts.get(p, 0) + 1
    for p, c in sorted(counts.items(), key=lambda kv: (kv[0][1], kv[0][0])):
        on_border = p[0] <= x0 + 2 or p[0] >= x1 - 2 or p[1] <= y0 + 2 or p[1] >= y1 - 2
        if c == 1 or on_border:
            print(f"  {p} count={c} border={on_border}")
    print("\n-- text labels --")
    for x in items:
        if x["fn"].startswith("TextOut"):
            t = m.clean_text(x.get("text", ""))
            if t:
                print(f"  {t!r} at {x['x']},{x['y']}")


if __name__ == "__main__":
    main()
