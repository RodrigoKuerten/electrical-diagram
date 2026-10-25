import os, glob, subprocess, sys
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
GDI = os.path.join(ROOT, "docs", "cadesimu", "gdi")
SCRIPT = os.path.join(ROOT, "tools", "re_cadesimu", "make_svg_from_gdi.py")
PY = sys.executable
for f in sorted(glob.glob(os.path.join(GDI, "*.json"))):
    name = os.path.basename(f)[:-5]
    if name == "baseline":
        continue
    r = subprocess.run([PY, SCRIPT, name], capture_output=True, text=True)
    if r.returncode != 0 or "no geometry" in r.stdout:
        print("SKIP", name, r.stdout.strip(), r.stderr.strip())
    else:
        print(r.stdout.strip())
