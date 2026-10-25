import ctypes, time, sys, os, json
sys.path.insert(0, os.path.dirname(__file__))
from capture_window import visible_toolbars, capture

u = ctypes.windll.user32
hwnd = 9177726
outdir = r"C:\Users\rodri\Documents\eletric-diagram\docs\cadesimu\screens\bars"
os.makedirs(outdir, exist_ok=True)
base = {722634, 10225752, 4261806}
bars = {
 32977:'Power in',32978:'Fuse',32979:'Automatic',32980:'Contacts',32981:'Motors',
 33010:'Power',32985:'Switch',32983:'Action',33029:'Detektor',32984:'Relay',
 33265:'Electronic relay',33071:'Logic',33280:'Ladder',33087:'Grafcet',
 33110:'In/Out',33244:'Pneumatic',32986:'Connection',
}
mapping = {}
for cmd, name in bars.items():
    before = {h for h,_ in visible_toolbars(hwnd)}
    u.SendMessageW(hwnd, 0x0111, cmd, 0); time.sleep(0.6)
    after = {h for h,_ in visible_toolbars(hwnd)}
    added = after - before
    removed = before - after
    print(f"{cmd} {name}: added={added} removed={removed}")
    if added:
        h = next(iter(added))
        mapping[name] = h
        capture(hwnd, os.path.join(outdir, f"bar_{name.replace(' ','_')}_{h}.png"))
        # hide it again
        u.SendMessageW(hwnd, 0x0111, cmd, 0); time.sleep(0.6)
        now = {x for x,_ in visible_toolbars(hwnd)}
        if now != base:
            print("   WARN state not base:", now)
    elif removed:
        mapping[name] = next(iter(removed))
        # show then capture then hide
        u.SendMessageW(hwnd, 0x0111, cmd, 0); time.sleep(0.6)
        capture(hwnd, os.path.join(outdir, f"bar_{name.replace(' ','_')}_{mapping[name]}.png"))
        u.SendMessageW(hwnd, 0x0111, cmd, 0); time.sleep(0.6)
    else:
        print("   no change; maybe already visible/hidden state desync")
json.dump(mapping, open(os.path.join(outdir, "mapping.json"), "w"), indent=1)
print(json.dumps(mapping, indent=1))
