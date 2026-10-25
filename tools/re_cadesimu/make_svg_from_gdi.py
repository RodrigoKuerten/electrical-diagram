"""Convert a GDI trace (baseline + component) into an exact SVG and pin table.

The CADe_SIMU canvas draws at 672 px for a 297 mm A4 page, so the calibration is 2.262626 px/mm.
The script removes the empty-sheet baseline, keeps the drawing primitives that belong to the
inserted symbol, and emits a compact SVG in device-pixel coordinates plus a pin list in mm.
"""
from __future__ import annotations
import json, os, re, sys, math
from collections import Counter
import html

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
GDI = os.path.join(ROOT, "docs", "cadesimu", "gdi")
SCALE = 672.0 / 297.0
PRIMITIVES = {"MoveToEx", "LineTo", "Ellipse", "Rectangle", "TextOutA", "TextOutW",
              "Polyline", "Polygon"}


def key(x):
    return (x["fn"], x.get("hdc"), x.get("x"), x.get("y"), x.get("l"), x.get("t"),
            x.get("r"), x.get("b"), x.get("text"),
            tuple(map(tuple, x.get("points", []))) if x.get("points") else None)


def extras(base, run):
    counts = Counter(key(x) for x in base if x["fn"] in PRIMITIVES)
    out = []
    seen = set()
    for x in run:
        if x["fn"] not in PRIMITIVES:
            continue
        k = key(x)
        if counts[k] > 0:
            counts[k] -= 1
            continue
        if k in seen:
            continue
        seen.add(k)
        out.append(x)
    return out


def extras_pair(before, after):
    """Difference between two traces recorded on the same canvas.

    Object handles change on every repaint, so only drawing primitives are compared.
    """
    return extras(before, after)


def is_tag(text):
    t = text.strip()
    return t.startswith("-") or (t.startswith("+") and len(t) > 1)


def clean_text(text):
    return text.replace("\x00", "").strip()


def bounds(items):
    xs, ys = [], []
    for x in items:
        if x["fn"] in ("MoveToEx", "LineTo"):
            xs.append(x.get("x", 0)); ys.append(x.get("y", 0))
        elif x["fn"].startswith("TextOut"):
            t = clean_text(x.get("text", ""))
            w = 3.7 * max(1, len(t))
            xs += [x.get("x", 0), x.get("x", 0) + w]
            ys += [x.get("y", 0) - 6, x.get("y", 0) + 1]
        elif x["fn"] in ("Ellipse", "Rectangle"):
            xs += [x["l"], x["r"]]; ys += [x["t"], x["b"]]
        elif x["fn"] in ("Polyline", "Polygon"):
            for px, py in x.get("points", []):
                xs.append(px); ys.append(py)
    return min(xs), min(ys), max(xs), max(ys)


def nearest_endpoint(item, points):
    tx, ty = item["x"], item["y"]
    return min(points, key=lambda p: (p[0] - tx) ** 2 + (p[1] - ty) ** 2)


def outermost_endpoint(item, points, center):
    """Choose the terminal end, not the end nearest the label.

    CADe draws the pin number just inside the symbol while the wire connects at the outer
    end of the lead.  Picking the farthest candidate from the symbol centre recovers the
    real connection point.
    """
    tx, ty = item["x"], item["y"]
    candidates = [p for p in points if abs(p[0] - tx) <= 6 and abs(p[1] - ty) <= 6]
    if not candidates:
        return None
    return max(candidates, key=lambda p: (p[0] - center[0]) ** 2 + (p[1] - center[1]) ** 2)


def main():
    name = sys.argv[1]
    base = json.load(open(os.path.join(GDI, "baseline.json")))
    run = json.load(open(os.path.join(GDI, name + ".json")))
    items = extras(base, run)
    # Remove the component tag and empty text records.
    items = [x for x in items if not (x["fn"].startswith("TextOut") and
                                      (is_tag(x.get("text", "")) or not clean_text(x.get("text", ""))))]
    # Keep only the insertion neighbourhood. The baseline subtraction can leave page/title
    # primitives whose text (date/page count) changed between the two repaints.
    def near(x):
        if x["fn"] in ("Ellipse", "Rectangle"):
            cx = (x["l"] + x["r"]) / 2; cy = (x["t"] + x["b"]) / 2
        elif x["fn"] in ("Polyline", "Polygon"):
            pts = x.get("points", [])
            if not pts:
                return False
            cx = sum(p[0] for p in pts) / len(pts); cy = sum(p[1] for p in pts) / len(pts)
        else:
            cx = x.get("x", 0); cy = x.get("y", 0)
        return 300 <= cx <= 650 and 230 <= cy <= 425
    items = [x for x in items if near(x)]
    if not items:
        print("no geometry")
        return
    x0, y0, x1, y1 = bounds(items)
    pad = 1
    x0 -= pad; y0 -= pad; x1 += pad; y1 += pad
    w = x1 - x0; h = y1 - y0
    # Collect line endpoints for pin detection.
    endpoints = []
    last = None
    for x in items:
        if x["fn"] == "MoveToEx":
            last = (x["x"], x["y"])
            endpoints.append(last)
        elif x["fn"] == "LineTo" and last is not None:
            endpoints.append((x["x"], x["y"]))
            last = (x["x"], x["y"])
    # Text labels that look like terminals and sit near a lead.  Longer labels first so that
    # "U1"/"PE" win over an internal "1"/"3" that happens to be close to the same lead.
    pin_re = re.compile(r"^(?:[A-Z]{1,3}\d*|\d{1,2}[A-Z]?|[A-Z]\d?|PE|N|L\d?|[+-])$")
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
    pins = []
    used = set()
    texts = [x for x in items if x["fn"].startswith("TextOut")]
    texts.sort(key=lambda x: (-len(clean_text(x.get("text", ""))), x.get("y", 0), x.get("x", 0)))
    for x in texts:
        t = clean_text(x.get("text", ""))
        if not pin_re.match(t) or t in ("M", "F", "P", "S", "C", "SR", "TON"):
            continue
        endpoint = outermost_endpoint(x, endpoints, (cx, cy))
        if endpoint is None:
            continue
        if endpoint in used:
            continue
        used.add(endpoint)
        pins.append((t, endpoint[0], endpoint[1]))
    # Deduplicate pins by name near identical coordinates.
    uniq = []
    for p in pins:
        if not any(p[0] == q[0] and abs(p[1]-q[1]) < 2 and abs(p[2]-q[2]) < 2 for q in uniq):
            uniq.append(p)
    # Draw body. Moves and lines are accumulated into a single path; any other primitive
    # flushes the pending path first.
    body = []
    pending = None

    def flush():
        nonlocal pending
        if pending is not None:
            body.append(pending + '" stroke="#1f2933" stroke-width="1.1" fill="none"/>')
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
                cx = (x["l"] + x["r"]) / 2 - x0; cy = (x["t"] + x["b"]) / 2 - y0
                rx = (x["r"] - x["l"]) / 2; ry = (x["b"] - x["t"]) / 2
                body.append(f'<ellipse cx="{cx:.2f}" cy="{cy:.2f}" rx="{rx:.2f}" ry="{ry:.2f}" '
                            f'stroke="#1f2933" stroke-width="1.1" fill="none"/>')
            else:
                body.append(f'<rect x="{x["l"]-x0}" y="{x["t"]-y0}" width="{x["r"]-x["l"]}" '
                            f'height="{x["b"]-x["t"]}" stroke="#1f2933" stroke-width="1.1" fill="none"/>')
        elif fn in ("Polyline", "Polygon"):
            flush()
            pts = " ".join(f'{px-x0},{py-y0}' for px, py in x.get("points", []))
            tag = "polygon" if fn == "Polygon" else "polyline"
            body.append(f'<{tag} points="{pts}" stroke="#1f2933" stroke-width="1.1" fill="none"/>')
        elif fn.startswith("TextOut"):
            flush()
            t = clean_text(x.get("text", ""))
            if not t:
                continue
            is_pin = pin_re.match(t) and any(p[0] == t for p in uniq)
            color = "#1f5f9e" if is_pin else "#1f2933"
            body.append(f'<text x="{x["x"]-x0}" y="{x["y"]-y0}" fill="{color}" '
                        f'font-family="Arial" font-size="6">{html.escape(t)}</text>')
    flush()
    svg = (f'<?xml version="1.0" encoding="UTF-8"?>\n'
           f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {w} {h}" '
           f'width="{w/SCALE:g}mm" height="{h/SCALE:g}mm">\n' + "\n".join(body) + "\n</svg>\n")
    out = os.path.join(GDI, name + ".svg")
    open(out, "w", encoding="utf-8").write(svg)
    print("svg", out, "size", w/SCALE, h/SCALE, "pins", uniq)


if __name__ == "__main__":
    main()
