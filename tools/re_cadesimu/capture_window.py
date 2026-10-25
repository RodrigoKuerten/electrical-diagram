import ctypes
from ctypes import wintypes
from PIL import Image
import sys, time

user32 = ctypes.windll.user32
gdi32 = ctypes.windll.gdi32

class RECT(ctypes.Structure):
    _fields_ = [("left", ctypes.c_long), ("top", ctypes.c_long), ("right", ctypes.c_long), ("bottom", ctypes.c_long)]

class BITMAPINFOHEADER(ctypes.Structure):
    _fields_ = [
        ("biSize", wintypes.DWORD), ("biWidth", ctypes.c_long), ("biHeight", ctypes.c_long),
        ("biPlanes", wintypes.WORD), ("biBitCount", wintypes.WORD),
        ("biCompression", wintypes.DWORD), ("biSizeImage", wintypes.DWORD),
        ("biXPelsPerMeter", ctypes.c_long), ("biYPelsPerMeter", ctypes.c_long),
        ("biClrUsed", wintypes.DWORD), ("biClrImportant", wintypes.DWORD),
    ]

class BITMAPINFO(ctypes.Structure):
    _fields_ = [("bmiHeader", BITMAPINFOHEADER), ("bmiColors", wintypes.DWORD * 3)]

def capture(hwnd, path, flags=2):
    r = RECT(); user32.GetWindowRect(hwnd, ctypes.byref(r))
    w = r.right - r.left; h = r.bottom - r.top
    hdc = user32.GetWindowDC(hwnd)
    mdc = gdi32.CreateCompatibleDC(hdc)
    bmp = gdi32.CreateCompatibleBitmap(hdc, w, h)
    gdi32.SelectObject(mdc, bmp)
    user32.PrintWindow(hwnd, mdc, flags)
    bmi = BITMAPINFO()
    bmi.bmiHeader.biSize = ctypes.sizeof(BITMAPINFOHEADER)
    bmi.bmiHeader.biWidth = w
    bmi.bmiHeader.biHeight = -h
    bmi.bmiHeader.biPlanes = 1
    bmi.bmiHeader.biBitCount = 32
    bmi.bmiHeader.biCompression = 0
    buf = ctypes.create_string_buffer(w * h * 4)
    gdi32.GetDIBits(mdc, bmp, 0, h, buf, ctypes.byref(bmi), 0)
    img = Image.frombuffer("RGBA", (w, h), buf, "raw", "BGRA", 0, 1)
    img.convert("RGB").save(path)
    gdi32.DeleteObject(bmp); gdi32.DeleteDC(mdc); user32.ReleaseDC(hwnd, hdc)
    return w, h

def visible_toolbars(hwnd):
    out = []
    EnumProc = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)
    def cb(h, l):
        cls = ctypes.create_unicode_buffer(128)
        user32.GetClassNameW(h, cls, 128)
        if cls.value == "ToolbarWindow32" and user32.IsWindowVisible(h):
            r = RECT(); user32.GetWindowRect(h, ctypes.byref(r))
            out.append((h, (r.left, r.top, r.right, r.bottom)))
        return True
    user32.EnumChildWindows(hwnd, EnumProc(cb), 0)
    return out

if __name__ == "__main__":
    hwnd = int(sys.argv[1]); cmd = int(sys.argv[2]); path = sys.argv[3]
    user32.SendMessageW(hwnd, 0x0111, cmd, 0)
    time.sleep(0.7)
    print("visible toolbars:", visible_toolbars(hwnd))
    print(capture(hwnd, path))
