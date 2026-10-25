from PIL import Image, ImageDraw
import glob, os, json, math

root = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
sym = os.path.join(root, "docs", "cadesimu", "symbols")
idx = json.load(open(os.path.join(sym, "capture_index.json"), encoding="utf-8"))
print("index", len(idx))
cols, cw, ch = 8, 210, 200
rows = math.ceil(len(idx) / cols)
sheet = Image.new("RGB", (cols * cw, rows * ch), "white")
d = ImageDraw.Draw(sheet)
for i, r in enumerate(idx):
    files = glob.glob(os.path.join(sym, f"{r['command']}_*_x4.png"))
    if not files:
        continue
    im = Image.open(files[0]).convert("RGB")
    im.thumbnail((cw - 10, ch - 30))
    x, y = (i % cols) * cw, (i // cols) * ch
    sheet.paste(im, (x + 5, y + 5))
    d.text((x + 4, y + ch - 24), f"{r['command']} {r['name'][:30]}", fill="black")
sheet.save(os.path.join(root, "docs", "cadesimu", "captured_contact_sheet.png"))
print(sheet.size)
