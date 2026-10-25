import ctypes
from ctypes import wintypes

user32 = ctypes.windll.user32
hwnd = 9112190

GetMenu = user32.GetMenu
GetMenu.restype = wintypes.HMENU
GetMenu.argtypes = [wintypes.HWND]
GetSubMenu = user32.GetSubMenu
GetSubMenu.restype = wintypes.HMENU
GetSubMenu.argtypes = [wintypes.HMENU, ctypes.c_int]
GetMenuItemCount = user32.GetMenuItemCount
GetMenuItemCount.argtypes = [wintypes.HMENU]
GetMenuStringW = user32.GetMenuStringW
GetMenuStringW.argtypes = [wintypes.HMENU, wintypes.UINT, wintypes.LPWSTR, ctypes.c_int, wintypes.UINT]
GetMenuItemID = user32.GetMenuItemID
GetMenuItemID.argtypes = [wintypes.HMENU, ctypes.c_int]
GetMenuState = user32.GetMenuState
GetMenuState.argtypes = [wintypes.HMENU, wintypes.UINT, wintypes.UINT]

MF_BYPOSITION = 0x400

def text(menu, pos):
    buf = ctypes.create_unicode_buffer(512)
    GetMenuStringW(menu, pos, buf, 512, MF_BYPOSITION)
    return buf.value

def walk(menu, depth=0, path=""):
    n = GetMenuItemCount(menu)
    for i in range(n):
        t = text(menu, i)
        mid = GetMenuItemID(menu, i)
        state = GetMenuState(menu, i, MF_BYPOSITION)
        print("  "*depth + f"[{i}] id={mid} state=0x{state:08x} {t!r}")
        sub = GetSubMenu(menu, i)
        if sub:
            walk(sub, depth+1, path + "/" + t)

m = GetMenu(hwnd)
walk(m)
