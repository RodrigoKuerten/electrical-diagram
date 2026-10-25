from PIL import Image, ImageDraw
import os, sys, math
sys.path.insert(0, os.path.dirname(__file__))
import generate

root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
png = os.path.join(root, "build", "symbol_pngs")
outdir = os.path.join(root, "docs", "cadesimu", "symbol_sheets")
os.makedirs(outdir, exist_ok=True)
by = {}
for e in generate.CATALOG:
    by.setdefault(e.category, []).append(e)
for cat, entries in by.items():
    cols, cw, ch = 5, 240, 240
    rows = math.ceil(len(entries)/cols)
    sheet = Image.new("RGB", (cols*cw, rows*ch), "white")
    d = ImageDraw.Draw(sheet)
    for i, e in enumerate(entries):
        path = os.path.join(png, e.type + ".svg.png")
        if not os.path.exists(path):
            continue
        im = Image.open(path).convert("RGB")
        im.thumbnail((cw-10, ch-30))
        x, y = (i % cols)*cw, (i//cols)*ch
        sheet.paste(im, (x+5, y+5))
        d.text((x+5, y+ch-24), e.type, fill="black")
        d.text((x+5, y+ch-12), e.name[:34], fill="gray")
    sheet.save(os.path.join(outdir, cat + ".png"))
    print(cat, len(entries), sheet.size)
