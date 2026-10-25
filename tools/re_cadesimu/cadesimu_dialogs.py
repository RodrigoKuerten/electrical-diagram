"""Dismiss the CADe_SIMU 4.0 limited-mode password/error dialogs and return the main window."""
from __future__ import annotations

import ctypes
import sys
import time
from ctypes import wintypes

u = ctypes.windll.user32
EnumProc = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)
BM_CLICK = 0x00F5


def text(h):
    buf = ctypes.create_unicode_buffer(256)
    u.GetWindowTextW(h, buf, 256)
    return buf.value


def cls(h):
    buf = ctypes.create_unicode_buffer(128)
    u.GetClassNameW(h, buf, 128)
    return buf.value


def windows(pid):
    result = []
    def cb(h, l):
        p = wintypes.DWORD()
        u.GetWindowThreadProcessId(h, ctypes.byref(p))
        if p.value == pid and u.IsWindowVisible(h):
            result.append(h)
        return True
    u.EnumWindows(EnumProc(cb), 0)
    return result


def children(h):
    result = []
    def cb(ch, l):
        result.append(ch)
        return True
    u.EnumChildWindows(h, EnumProc(cb), 0)
    return result


def click_button(dialog, label):
    for child in children(dialog):
        if text(child).strip().lower() == label.lower() or text(child).strip().lower().startswith(label.lower()):
            u.SendMessageW(child, BM_CLICK, 0, 0)
            return True
    return False


def main():
    pid = int(sys.argv[1]) if len(sys.argv) > 1 else None
    if pid is None:
        # Find the CADe_SIMU top-level window automatically.
        found = []
        def cb(h, l):
            if "CADe_SIMU" in text(h):
                p = wintypes.DWORD()
                u.GetWindowThreadProcessId(h, ctypes.byref(p))
                found.append(p.value)
            return True
        u.EnumWindows(EnumProc(cb), 0)
        if not found:
            raise SystemExit("CADe_SIMU is not running")
        pid = found[0]
    # Dismiss password dialog.
    for h in windows(pid):
        if cls(h) == "#32770" and "password" in text(h).lower():
            click_button(h, "Cnl")
            time.sleep(0.4)
    # Dismiss the "you did not enter the password" error.
    for h in windows(pid):
        if cls(h) == "#32770" and ("senha" in text(h).lower() or "password" in text(h).lower()):
            click_button(h, "OK")
            time.sleep(0.4)
    # Report the frame/view handles.
    frame = None
    for h in windows(pid):
        if cls(h).startswith("Afx:400000:8"):
            frame = h
    view = None
    if frame:
        for h in children(frame):
            if cls(h) == "AfxFrameOrView42d":
                view = h
    print(f"pid={pid} frame={frame} view={view}")


if __name__ == "__main__":
    main()
