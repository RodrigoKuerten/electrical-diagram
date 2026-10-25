import ctypes, subprocess, sys, os, time, json
from ctypes import wintypes

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
NODE = r"C:\Program Files\nodejs\node.exe"
FRIDA_DIR = os.path.join(os.environ["LOCALAPPDATA"], "Temp", "opencode", "frida-node")
SERVICE = os.path.join(ROOT, "tools", "re_cadesimu", "gdi_service.js")
OUT = os.path.join(ROOT, "docs", "cadesimu", "gdi")
os.makedirs(OUT, exist_ok=True)

u = ctypes.windll.user32
PID = int(sys.argv[1]) if len(sys.argv) > 1 else 15168

# Locate frame and canvas view.
handles = {}
EnumProc = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)
def cb(h, l):
    cls = ctypes.create_unicode_buffer(128); u.GetClassNameW(h, cls, 128)
    if cls.value.startswith("Afx:400000:8"):
        handles["frame"] = h
    if cls.value == "AfxFrameOrView42d":
        handles["view"] = h
    return True
u.EnumChildWindows(0, EnumProc(cb), 0)
# EnumChildWindows with NULL parent only enumerates top-level; use EnumWindows.
handles.clear()
def cb2(h, l):
    cls = ctypes.create_unicode_buffer(128); u.GetClassNameW(h, cls, 128)
    pid = wintypes.DWORD(); u.GetWindowThreadProcessId(h, ctypes.byref(pid))
    if pid.value != PID:
        return True
    if cls.value.startswith("Afx:400000:8"):
        handles["frame"] = h
    if cls.value == "AfxFrameOrView42d":
        handles["view"] = h
    return True
u.EnumWindows(EnumProc(cb2), 0)
if "frame" in handles:
    def cb3(h, l):
        cls = ctypes.create_unicode_buffer(128); u.GetClassNameW(h, cls, 128)
        if cls.value == "AfxFrameOrView42d":
            handles["view"] = h
        return True
    u.EnumChildWindows(handles["frame"], EnumProc(cb3), 0)
print("handles", handles)
frame = handles["frame"]; view = handles["view"]

env = dict(os.environ)
env["NODE_PATH"] = os.path.join(FRIDA_DIR, "node_modules")
proc = subprocess.Popen([NODE, SERVICE, str(PID), OUT], cwd=FRIDA_DIR,
                        env=env,
                        stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                        text=True, bufsize=1)
def read_until(prefix, timeout=15):
    start = time.time()
    while time.time() - start < timeout:
        line = proc.stdout.readline()
        if not line:
            continue
        print("node:", line.rstrip())
        if line.startswith(prefix):
            return line
    raise RuntimeError("timeout waiting " + prefix)
read_until("READY")

def click(x, y):
    l = (y << 16) | x
    u.SendMessageW(view, 0x0200, 0, l); time.sleep(0.05)
    u.SendMessageW(view, 0x0201, 1, l); time.sleep(0.03)
    u.SendMessageW(view, 0x0202, 0, l)

# Clear, normal zoom.
u.SendMessageW(frame, 0x0111, 32973, 0); time.sleep(0.2)
u.SendMessageW(view, 0x0100, 0x2E, 0); u.SendMessageW(view, 0x0101, 0x2E, 0); time.sleep(0.2)
u.SendMessageW(frame, 0x0111, 32772, 0); time.sleep(0.3)

proc.stdin.write("start\n"); proc.stdin.flush(); read_until("STARTED")
u.SendMessageW(frame, 0x0111, 32843, 0); time.sleep(0.2)
click(450, 300); time.sleep(0.3)
u.SendMessageW(view, 0x0100, 0x1B, 0); u.SendMessageW(view, 0x0101, 0x1B, 0); time.sleep(0.2)
# Force a repaint of the canvas.
u.InvalidateRect(view, None, True)
u.UpdateWindow(view)
time.sleep(0.5)
proc.stdin.write("stop fuse_test\n"); proc.stdin.flush(); read_until("STOPPED", 20)
proc.stdin.write("quit\n"); proc.stdin.flush()
time.sleep(0.5)
proc.terminate()
print("done")
