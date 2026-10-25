"""Dump PE resources from CADe_SIMU.exe using pefile.

Usage:
  python dump_resources.py <exe> <outdir>
"""
from __future__ import annotations
import io
import os
import struct
import sys
import json
import pefile
from PIL import Image

RT_NAMES = {
    1: "CURSOR", 2: "BITMAP", 3: "ICON", 4: "MENU", 5: "DIALOG", 6: "STRING",
    7: "FONTDIR", 8: "FONT", 9: "ACCELERATOR", 10: "RCDATA", 11: "MESSAGETABLE",
    12: "GROUP_CURSOR", 14: "GROUP_ICON", 16: "VERSION", 24: "MANIFEST",
}


def res_name(entry) -> str:
    if entry.name is not None:
        return str(entry.name)
    return str(entry.id)


def dib_to_png(data: bytes, path: str):
    """Convert a BITMAP resource (BITMAPINFOHEADER + palette + pixels)."""
    if len(data) < 40:
        return False
    header_size = struct.unpack_from("<I", data, 0)[0]
    width, height = struct.unpack_from("<ii", data, 4)
    planes, bpp = struct.unpack_from("<HH", data, 12)
    compression = struct.unpack_from("<I", data, 16)[0]
    clr_used = struct.unpack_from("<I", data, 32)[0]
    if width <= 0 or height == 0:
        return False
    height = abs(height)
    # Palette size
    if bpp <= 8:
        colors = clr_used or (1 << bpp)
        palette_bytes = colors * 4
    else:
        palette_bytes = 0
    pixel_offset = 14 + header_size + palette_bytes
    row_stride = ((width * bpp + 31) // 32) * 4
    pixel_bytes = row_stride * height
    if len(data) + 14 < pixel_offset + pixel_bytes:
        # Some resources have unusual stride/padding; let Pillow try anyway.
        pass
    # Build a BMP file
    file_header = struct.pack("<2sIHHI", b"BM", 14 + len(data), 0, 0, 14 + header_size + palette_bytes)
    bmp = file_header + data
    try:
        img = Image.open(io.BytesIO(bmp))
        img.load()
        img.save(path)
        return True
    except Exception as exc:
        print(f"  ! bitmap conversion failed: {exc}")
        return False


def main():
    exe = sys.argv[1]
    outdir = sys.argv[2]
    pe = pefile.PE(exe, fast_load=False)
    # pefile parses the resource directory during PE construction.
    os.makedirs(outdir, exist_ok=True)
    manifest = []
    for entry in pe.DIRECTORY_ENTRY_RESOURCE.entries:
        type_id = entry.id if entry.name is None else None
        type_name = RT_NAMES.get(type_id, res_name(entry))
        tdir = os.path.join(outdir, f"{type_id if type_id is not None else 'named'}_{type_name}")
        os.makedirs(tdir, exist_ok=True)
        for nentry in entry.directory.entries:
            name = res_name(nentry)
            for lentry in nentry.directory.entries:
                lang = lentry.id
                d = lentry.data.struct
                data = pe.get_data(d.OffsetToData, d.Size)
                ext = "bin"
                if type_id == 2:
                    ext = "bmp"
                elif type_id == 3:
                    ext = "ico"
                elif type_id == 5:
                    ext = "dlg"
                elif type_id == 4:
                    ext = "menu"
                elif type_id == 6:
                    ext = "str"
                elif type_id == 16:
                    ext = "ver"
                elif type_id == 24:
                    ext = "manifest"
                base = f"{name}_{lang}.{ext}"
                path = os.path.join(tdir, base)
                if type_id == 2:
                    if not dib_to_png(data, os.path.join(tdir, f"{name}_{lang}.png")):
                        with open(path, "wb") as f:
                            f.write(data)
                else:
                    with open(path, "wb") as f:
                        f.write(data)
                manifest.append({
                    "type_id": type_id, "type_name": type_name, "name": name,
                    "language": lang, "size": len(data), "file": path,
                    "rva": hex(d.OffsetToData),
                })
    with open(os.path.join(outdir, "manifest.json"), "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=1)
    print(f"dumped {len(manifest)} resources to {outdir}")


if __name__ == "__main__":
    main()
