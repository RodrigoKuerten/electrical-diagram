import ctypes, time, os, sys
from ctypes import wintypes
sys.path.insert(0, os.path.dirname(__file__))
from capture_window import capture

u = ctypes.windll.user32
hwnd = 9177726
outdir = r"C:\Users\rodri\Documents\eletric-diagram\docs\cadesimu\screens\bars"
os.makedirs(outdir, exist_ok=True)

bars = []
EnumProc = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)
def cb(h, l):
    cls = ctypes.create_unicode_buffer(128); u.GetClassNameW(h, cls, 128)
    if cls.value == 'ToolbarWindow32':
        bars.append((h, u.SendMessageW(h, 0x418, 0, 0)))
    return True
u.EnumChildWindows(hwnd, EnumProc(cb), 0)
bars = sorted(bars)
print("bars", bars)
base = {722634, 4261806, 10225752}
for h, count in bars:
    if h in base:
        continue
    for hh, _ in bars:
        if hh not in base:
            u.ShowWindow(hh, 0)
    u.ShowWindow(h, 5)
    u.SendMessageW(hwnd, 0x0005, 0, 0)  # WM_SIZE
    time.sleep(0.5)
    path = os.path.join(outdir, f"bar_{h}_{count}.png")
    capture(hwnd, path)
    print("captured", path)
