"""Capture isolated GDI traces with a before/after baseline per command.

The previous run used a single global baseline and Ctrl+A/Delete, which occasionally left
the previous component on the sheet.  Here each command gets its own "before" trace, so the
difference contains only the newly inserted component, and every insertion is also saved as
a PNG for visual verification.
"""
from __future__ import annotations

import ctypes
import os
import re
import subprocess
import sys
import time
from ctypes import wintypes

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
NODE = r"C:\Program Files\nodejs\node.exe"
FRIDA_DIR = os.path.join(os.environ["LOCALAPPDATA"], "Temp", "opencode", "frida-node")
SERVICE = os.path.join(ROOT, "tools", "re_cadesimu", "gdi_service.js")
OUT = os.path.join(ROOT, "docs", "cadesimu", "gdi_clean")
SCREENS = os.path.join(ROOT, "docs", "cadesimu", "gdi_screens")
os.makedirs(OUT, exist_ok=True)
os.makedirs(SCREENS, exist_ok=True)

PID = int(sys.argv[1]) if len(sys.argv) > 1 else 15168
LIMIT = int(sys.argv[2]) if len(sys.argv) > 2 else 1000
START = int(sys.argv[3]) if len(sys.argv) > 3 else 0

u = ctypes.windll.user32
EnumProc = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)
handles = {}


def cb(h, l):
    cls = ctypes.create_unicode_buffer(128)
    u.GetClassNameW(h, cls, 128)
    pid = wintypes.DWORD()
    u.GetWindowThreadProcessId(h, ctypes.byref(pid))
    if pid.value != PID:
        return True
    if cls.value.startswith("Afx:400000:8"):
        handles["frame"] = h
    return True


u.EnumWindows(EnumProc(cb), 0)
if "frame" not in handles:
    raise SystemExit("CADe_SIMU frame not found")


def cb2(h, l):
    cls = ctypes.create_unicode_buffer(128)
    u.GetClassNameW(h, cls, 128)
    if cls.value == "AfxFrameOrView42d":
        handles["view"] = h
    return True


u.EnumChildWindows(handles["frame"], EnumProc(cb2), 0)
frame = handles["frame"]
view = handles["view"]
print("frame", frame, "view", view, flush=True)


def read_commands():
    path = os.path.join(ROOT, "docs", "cadesimu", "named_commands.tsv")
    excluded = {
        32809, 32812, 32813, 32814, 32815, 32816, 32817, 32818, 32819, 32820,
        32821, 32822, 32823, 32824, 32825, 32826, 32828, 32857, 32858, 32869,
        32905, 32906, 32907, 32908, 32909, 32964, 32971, 32972, 32973, 32974,
        32975, 33002, 33011, 33029, 33030, 33042, 33068, 33069, 33080, 33081,
        33088, 33105, 33106, 33107, 33108, 33109, 33111, 33232, 33233, 33234,
        33242, 33245, 33266, 33310, 33311, 33349, 33350, 33351, 33352, 33359,
        33360, 33361, 33364, 33365,
    }
    rows = []
    with open(path, encoding="utf-8") as f:
        next(f)
        for line in f:
            parts = line.rstrip("\n").split("\t")
            if len(parts) < 3:
                continue
            cmd = int(parts[0])
            bars = {int(x) for x in parts[1].split(",") if x}
            name = parts[2]
            if not (32800 <= cmd <= 33399) or bars & {128, 159, 171, 301, 307}:
                continue
            if cmd in excluded:
                continue
            if re.search(
                r"(barra|interruptor de|simul|selecion|seleccion|visualiza|asignar|prueba|exportar|"
                r"espesor|girar|espejo|s\W*bolos|configur|redibujar|l\W*nea|rect\W*ngulo|elipse|"
                r"color|rellenar|zoom|rejilla|copiar|borra|busca|deshacer|nuevo|cerrar|imprimir)",
                name,
                re.I,
            ):
                continue
            rows.append((cmd, name))
    seen = set()
    unique = []
    for cmd, name in rows:
        if cmd not in seen:
            seen.add(cmd)
            unique.append((cmd, name))
    return unique


commands = read_commands()[START:START + LIMIT]

env = dict(os.environ)
env["NODE_PATH"] = os.path.join(FRIDA_DIR, "node_modules")
proc = subprocess.Popen(
    [NODE, SERVICE, str(PID), OUT],
    cwd=FRIDA_DIR,
    env=env,
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT,
    text=True,
    bufsize=1,
)


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
    u.SendMessageW(view, 0x0200, 0, l)
    time.sleep(0.03)
    u.SendMessageW(view, 0x0201, 1, l)
    time.sleep(0.02)
    u.SendMessageW(view, 0x0202, 0, l)


def send_key(vk):
    u.SendMessageW(view, 0x0100, vk, 0)
    u.SendMessageW(view, 0x0101, vk, 0)


def clear_all():
    # Edit > Borra todo. Needs focus on the canvas first.
    click(20, 20)
    time.sleep(0.05)
    u.SendMessageW(frame, 0x0111, 57634, 0)
    time.sleep(0.2)
    # If a confirmation dialog appeared, dismiss it with Enter.
    # (CADe asks before deleting when the sheet is non-empty.)
    u.SendMessageW(frame, 0x0111, 32973, 0)
    send_key(0x2E)
    time.sleep(0.2)
    u.SendMessageW(frame, 0x0111, 32772, 0)
    time.sleep(0.15)


def repaint():
    u.InvalidateRect(view, None, True)
    u.UpdateWindow(view)
    time.sleep(0.18)


def screenshot(path):
    try:
        sys.path.insert(0, os.path.join(ROOT, "tools", "re_cadesimu"))
        from capture_window import capture
        capture(frame, path)
    except Exception as exc:  # pragma: no cover - diagnostics only
        print("screenshot failed", exc, flush=True)


for index, (cmd, name) in enumerate(commands):
    clear_all()
    proc.stdin.write("start\n")
    proc.stdin.flush()
    read_until("STARTED")
    repaint()
    safe = re.sub(r"[^A-Za-z0-9]+", "_", name)[:50]
    base_name = f"{cmd}_{safe}_before"
    proc.stdin.write(f"stop {base_name}\n")
    proc.stdin.flush()
    read_until("STOPPED", 30)

    u.SendMessageW(frame, 0x0111, cmd, 0)
    time.sleep(0.15)
    click(450, 300)
    time.sleep(0.2)
    send_key(0x1B)
    time.sleep(0.1)

    proc.stdin.write("start\n")
    proc.stdin.flush()
    read_until("STARTED")
    repaint()
    after_name = f"{cmd}_{safe}_after"
    proc.stdin.write(f"stop {after_name}\n")
    proc.stdin.flush()
    line = read_until("STOPPED", 30)
    screenshot(os.path.join(SCREENS, f"{cmd}_{safe}.png"))
    print(f"[{index + START}] {cmd} {name!r} {line.strip()}", flush=True)

proc.stdin.write("quit\n")
proc.stdin.flush()
time.sleep(0.5)
proc.terminate()
print("done", len(commands))
