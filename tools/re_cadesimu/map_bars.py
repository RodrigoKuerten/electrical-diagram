import ctypes, time, sys
from ctypes import wintypes
from capture_window import visible_toolbars

user32 = ctypes.windll.user32
hwnd = 9112190
bars = {
 32977:'Power in',32978:'Fuse',32979:'Automatic',32980:'Contacts',32981:'Motors',
 33010:'Power',32985:'Switch',32983:'Action',33029:'Detektor',32984:'Relay',
 33265:'Electronic relay',33071:'Logic',33280:'Ladder',33087:'Grafcet',
 33110:'In/Out',33244:'Pneumatic',32986:'Connection',
}

def vis():
    return {h: r for h, r in visible_toolbars(hwnd)}

base = vis()
print("baseline:", base)
for cmd, name in bars.items():
    user32.SendMessageW(hwnd, 0x0111, cmd, 0)
    time.sleep(0.4)
    now = vis()
    added = set(now) - set(base)
    removed = set(base) - set(now)
    print(f"{cmd} {name}: added={[(h,now[h]) for h in added]} removed={list(removed)}")
    # restore
    user32.SendMessageW(hwnd, 0x0111, cmd, 0)
    time.sleep(0.4)
    back = vis()
    if set(back) != set(base):
        print("   restore mismatch:", back)
