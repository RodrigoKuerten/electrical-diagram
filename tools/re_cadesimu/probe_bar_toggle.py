import ctypes, time, sys, os
from ctypes import wintypes
sys.path.insert(0, os.path.dirname(__file__))
from capture_window import visible_toolbars

u = ctypes.windll.user32
hwnd = 9177726
GetMenu = u.GetMenu; GetMenu.restype = wintypes.HMENU; GetMenu.argtypes=[wintypes.HWND]
GetSubMenu = u.GetSubMenu; GetSubMenu.restype=wintypes.HMENU; GetSubMenu.argtypes=[wintypes.HMENU, ctypes.c_int]
GetMenuState = u.GetMenuState; GetMenuState.argtypes=[wintypes.HMENU, wintypes.UINT, wintypes.UINT]
MF_BYPOSITION=0x400
menu = GetSubMenu(GetMenu(hwnd), 5)
ids=[]
for i in range(u.GetMenuItemCount(menu)):
    mid=u.GetMenuItemID(menu,i)
    if mid>0: ids.append((i,mid))
def states():
    return {mid: GetMenuState(menu,i,MF_BYPOSITION) for i,mid in ids}
def vis():
    return sorted(h for h,_ in visible_toolbars(hwnd))
print("toolbar heights", {h:r for h,r in visible_toolbars(hwnd)})
print("states", {k:hex(v) for k,v in states().items()})
cmd=32977
for step in range(4):
    u.SendMessageW(hwnd,0x0111,cmd,0); time.sleep(0.6)
    print("step",step,"vis",vis(),"state",hex(states()[cmd]))
