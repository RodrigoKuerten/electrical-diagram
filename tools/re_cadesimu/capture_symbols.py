"""Insert each CADe_SIMU component command at a fixed canvas point, capture it, then clear the sheet.

Requires the running CADe_SIMU window handles (frame, view) and that the app is in the state after
the password dialog has been dismissed. The script uses WM_COMMAND for menu/toolbar commands and
WM_LBUTTON* messages for canvas clicks.
"""
from __future__ import annotations
import ctypes, json, os, re, sys, time
from ctypes import wintypes
from PIL import Image, ImageChops
sys.path.insert(0, os.path.dirname(__file__))
from capture_window import capture

u = ctypes.windll.user32
FRAME = int(os.environ.get("CAD_FRAME", "9177726"))
VIEW = int(os.environ.get("CAD_VIEW", "3476434"))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
OUT = os.path.join(ROOT, "docs", "cadesimu", "symbols")
os.makedirs(OUT, exist_ok=True)

CMD_ZOOM_NORMAL = 32772
CMD_SELECT_ALL = 32973
CLICK_X, CLICK_Y = 300, 250
# Interior region of the drawing page, far from the green sheet frame.
CROP = (210, 300, 580, 540)

EXCLUDE_COMMANDS = {
    32809, 32812, 32813, 32814, 32815, 32816, 32817, 32818, 32819, 32820, 32821, 32822, 32823,
    32824, 32825, 32826, 32828, 32857, 32858, 32869, 32905, 32906, 32907, 32908, 32909, 32964,
    32971, 32972, 32973, 32974, 32975, 33002, 33011, 33029, 33030, 33042, 33068, 33069, 33080,
    33081, 33088, 33105, 33106, 33107, 33108, 33109, 33111, 33232, 33233, 33234, 33242, 33245,
    33266, 33310, 33311, 33349, 33350, 33351, 33352, 33359, 33360, 33361, 33364, 33365,
}

UTILITY_NAME = re.compile(
    r"(barra|interruptor de|simul|selecion|seleccion|visualiza|asignar|prueba|exportar|"
    r"espesor|girar|espejo|s.imbolos|configur|redibujar|l.nea|rect.ngulo|elipse|color|"
    r"rellenar|zoom|rejilla|copiar|borra|busca|deshacer|nuevo|cerrar|imprimir|"
    r"potencia$|aparatos de medida$|detectores$|l.gica$|contactores, interruptores$|"
    r"contactos auxiliares, contactos temporizados$|fusibles, seccionadores$|alimentaciones$|"
    r"autom.ticos, diferenciales|actuadores neum.ticos$|entrada/salida$)",
    re.I,
)


def read_commands():
    path = os.path.join(ROOT, "docs", "cadesimu", "named_commands.tsv")
    rows = []
    with open(path, encoding="utf-8") as f:
        next(f)
        for line in f:
            parts = line.rstrip("\n").split("\t")
            if len(parts) < 3:
                continue
            cmd = int(parts[0]); bars = {int(x) for x in parts[1].split(",") if x}
            name = parts[2]
            if not (32800 <= cmd <= 33399):
                continue
            if bars & {128, 159, 171, 301, 307}:
                continue
            if cmd in EXCLUDE_COMMANDS or UTILITY_NAME.search(name):
                continue
            rows.append((cmd, name, bars))
    seen = set(); unique = []
    for cmd, name, bars in rows:
        if cmd in seen: continue
        seen.add(cmd); unique.append((cmd, name, bars))
    return unique


def click(x, y):
    l = (y << 16) | x
    u.SendMessageW(VIEW, 0x0200, 0, l); time.sleep(0.08)
    u.SendMessageW(VIEW, 0x0201, 1, l); time.sleep(0.05)
    u.SendMessageW(VIEW, 0x0202, 0, l)


def clear():
    u.SendMessageW(FRAME, 0x0111, CMD_SELECT_ALL, 0); time.sleep(0.25)
    u.SendMessageW(VIEW, 0x0100, 0x2E, 0); u.SendMessageW(VIEW, 0x0101, 0x2E, 0)
    time.sleep(0.30)


def snap():
    tmp = os.path.join(OUT, "_tmp.png")
    capture(FRAME, tmp)
    return Image.open(tmp).convert("RGB")


def ink_bbox(img):
    gray = img.convert("L")
    # Grid dots and page background are > 190; symbol lines/text are much darker.
    mask = gray.point(lambda p: 255 if p < 160 else 0)
    return mask.getbbox()


def main():
    limit = int(sys.argv[1]) if len(sys.argv) > 1 else 100000
    start = int(sys.argv[2]) if len(sys.argv) > 2 else 0
    commands = read_commands()
    print("commands", len(commands))
    u.SendMessageW(FRAME, 0x0111, CMD_ZOOM_NORMAL, 0)
    time.sleep(0.5)
    clear()
    results = []
    for index, (cmd, name, bars) in enumerate(commands[start:start + limit], start=start):
        clear()
        u.SendMessageW(FRAME, 0x0111, cmd, 0)
        time.sleep(0.25)
        click(CLICK_X, CLICK_Y)
        time.sleep(0.35)
        u.SendMessageW(VIEW, 0x0100, 0x1B, 0); u.SendMessageW(VIEW, 0x0101, 0x1B, 0)
        time.sleep(0.20)
        img = snap().crop(CROP)
        bbox = ink_bbox(img)
        if bbox and (bbox[2] - bbox[0]) * (bbox[3] - bbox[1]) > 12:
            margin = 12
            box = (max(0, bbox[0]-margin), max(0, bbox[1]-margin),
                   min(img.width, bbox[2]+margin), min(img.height, bbox[3]+margin))
            crop = img.crop(box)
            safe = re.sub(r'[^A-Za-z0-9]+', '_', name)[:60]
            crop.save(os.path.join(OUT, f"{cmd}_{safe}.png"))
            crop.resize((crop.width*4, crop.height*4), Image.LANCZOS).save(
                os.path.join(OUT, f"{cmd}_{safe}_x4.png"))
            results.append({"command": cmd, "name": name, "bars": sorted(bars),
                            "bbox": box, "w": crop.width, "h": crop.height})
            print(f"[{index}] {cmd} {name!r} INSERTED {crop.size}")
        else:
            print(f"[{index}] {cmd} {name!r} no insertion")
        if (index + 1) % 10 == 0:
            json.dump(results, open(os.path.join(OUT, "capture_index.json"), "w", encoding="utf-8"),
                      indent=1, ensure_ascii=False)
    json.dump(results, open(os.path.join(OUT, "capture_index.json"), "w", encoding="utf-8"),
              indent=1, ensure_ascii=False)
    print("done", len(results), "inserted")


if __name__ == "__main__":
    main()
