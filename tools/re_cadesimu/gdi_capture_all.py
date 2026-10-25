"""Record exact GDI vector calls for every CADe_SIMU component insertion.

Requires a running CADe_SIMU 4.0 process (password dialog dismissed) and Node frida in the
frida-node temp directory. Writes one JSON per command plus a baseline.
"""
import ctypes, os, re, subprocess, sys, time, json
from ctypes import wintypes

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
NODE = r"C:\Program Files\nodejs\node.exe"
FRIDA_DIR = os.path.join(os.environ["LOCALAPPDATA"], "Temp", "opencode", "frida-node")
SERVICE = os.path.join(ROOT, "tools", "re_cadesimu", "gdi_service.js")
OUT = os.path.join(ROOT, "docs", "cadesimu", "gdi")
os.makedirs(OUT, exist_ok=True)

PID = int(sys.argv[1]) if len(sys.argv) > 1 else 15168
LIMIT = int(sys.argv[2]) if len(sys.argv) > 2 else 1000
START = int(sys.argv[3]) if len(sys.argv) > 3 else 0

u = ctypes.windll.user32
EnumProc = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)
handles = {}
def cb(h, l):
    cls = ctypes.create_unicode_buffer(128); u.GetClassNameW(h, cls, 128)
    pid = wintypes.DWORD(); u.GetWindowThreadProcessId(h, ctypes.byref(pid))
    if pid.value != PID:
        return True
    if cls.value.startswith("Afx:400000:8"):
        handles["frame"] = h
    return True
u.EnumWindows(EnumProc(cb), 0)
if "frame" not in handles:
    raise SystemExit("CADe_SIMU frame not found")
def cb2(h, l):
    cls = ctypes.create_unicode_buffer(128); u.GetClassNameW(h, cls, 128)
    if cls.value == "AfxFrameOrView42d":
        handles["view"] = h
    return True
u.EnumChildWindows(handles["frame"], EnumProc(cb2), 0)
frame = handles["frame"]; view = handles["view"]
print("frame", frame, "view", view, flush=True)

# Read the same command list used by the screenshot capture.
EXCLUDE_COMMANDS = {32809, 32812, 32813, 32814, 32815, 32816, 32817, 32818, 32819, 32820, 32821, 32822, 32823, 32824, 32825, 32826, 32828, 32857, 32858, 32869, 32905, 32906, 32907, 32908, 32909, 32964, 32971, 32972, 32973, 32974, 32975, 33002, 33011, 33029, 33030, 33042, 33068, 33069, 33080, 33081, 33088, 33105, 33106, 33107, 33108, 33109, 33111, 33232, 33233, 33234, 33242, 33245, 33266, 33310, 33311, 33349, 33350, 33351, 33352, 33359, 33360, 33361, 33364, 33365}
UTILITY_NAME = re.compile(r'(barra|interruptor de|simul|selecion|seleccion|visualiza|asignar|prueba|exportar|espesor|girar|espejo|s.bolos|configur|redibujar|l.nea|rect.ngulo|elipse|color|rellenar|zoom|rejilla|copiar|borra|busca|deshacer|nuevo|cerrar|imprimir)', re.I)

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
            if not (32800 <= cmd <= 33399) or bars & {128, 159, 171, 301, 307}:
                continue
            if cmd in EXCLUDE_COMMANDS or UTILITY_NAME.search(name):
                continue
            rows.append((cmd, name))
    seen = set(); unique = []
    for cmd, name in rows:
        if cmd not in seen:
            seen.add(cmd); unique.append((cmd, name))
    return unique

commands = read_commands()[START:START + LIMIT]

env = dict(os.environ)
env["NODE_PATH"] = os.path.join(FRIDA_DIR, "node_modules")
proc = subprocess.Popen([NODE, SERVICE, str(PID), OUT], cwd=FRIDA_DIR, env=env,
                        stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                        text=True, bufsize=1)
def read_until(prefix, timeout=30):
    start = time.time()
    while time.time() - start < timeout:
        line = proc.stdout.readline()
        if not line:
            continue
        if line.startswith(prefix):
            return line
        print("node:", line.rstrip(), flush=True)
    raise RuntimeError("timeout waiting " + prefix)
read_until("READY")

def click(x, y):
    l = (y << 16) | x
    u.SendMessageW(view, 0x0200, 0, l); time.sleep(0.03)
    u.SendMessageW(view, 0x0201, 1, l); time.sleep(0.02)
    u.SendMessageW(view, 0x0202, 0, l)

def clear():
    u.SendMessageW(frame, 0x0111, 32973, 0); time.sleep(0.12)
    u.SendMessageW(view, 0x0100, 0x2E, 0); u.SendMessageW(view, 0x0101, 0x2E, 0); time.sleep(0.15)
    u.SendMessageW(frame, 0x0111, 32772, 0); time.sleep(0.15)

def repaint():
    u.InvalidateRect(view, None, True)
    u.UpdateWindow(view)
    time.sleep(0.18)

# Baseline with an empty sheet.
clear()
proc.stdin.write("start\n"); proc.stdin.flush(); read_until("STARTED")
repaint()
proc.stdin.write("stop baseline\n"); proc.stdin.flush(); read_until("STOPPED")
print("baseline recorded", flush=True)

for index, (cmd, name) in enumerate(commands):
    clear()
    u.SendMessageW(frame, 0x0111, cmd, 0); time.sleep(0.15)
    click(450, 300); time.sleep(0.18)
    u.SendMessageW(view, 0x0100, 0x1B, 0); u.SendMessageW(view, 0x0101, 0x1B, 0); time.sleep(0.12)
    proc.stdin.write("start\n"); proc.stdin.flush(); read_until("STARTED")
    repaint()
    safe = re.sub(r"[^A-Za-z0-9]+", "_", name)[:50]
    fname = f"{cmd}_{safe}"
    proc.stdin.write(f"stop {fname}\n"); proc.stdin.flush()
    line = read_until("STOPPED", 30)
    print(f"[{index + START}] {cmd} {name!r} {line.strip()}", flush=True)

proc.stdin.write("quit\n"); proc.stdin.flush()
time.sleep(0.5)
proc.terminate()
print("done", len(commands))

