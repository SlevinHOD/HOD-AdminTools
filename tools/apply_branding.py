#!/usr/bin/env python3
"""Re-applies HOD branding to a freshly imported COT tree.

  python3 tools/apply_branding.py cot/JM/COT

Safe to re-run: colours already converted are left alone, missing strings are reported.
Palette: COT's daisyUI navy/indigo -> HOD green-black with gold accents.
"""
import glob
import os
import re
import sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else "cot/JM/COT"
GOLD = "D4AF37"

# RGB (no alpha) -> RGB. Alpha is always preserved.
PALETTE = {
    # accent / primary indigo -> HOD green
    "605DFF": "2E9E4A",
    "7979FF": "3CBF5C",
    "4D4CC7": "25803C",
    "8384FF": GOLD,
    "9194FF": GOLD,
    "2C2D62": "1C3A24",
    "3A3B8E": "245030",
    "ADB3FF": "A8E0B6",
    "EDF1FE": "EEFBF1",
    # navy-grey ink -> green-black ink
    "09090B": "080A08",
    "12171D": "101410",
    "15191E": "121612",
    "191E24": "161B17",
    "1D232A": "1A201B",
    "222932": "1F2620",
    "32383F": "30362F",
    "3A424C": "38423A",
    "464E55": "454D46",
    "656E75": "646D65",
    "8F999F": "8F998F",
    "B8C4CA": "B8C4B9",
    "ECF9FF": "EEF6EC",
}
PAL = {int(k, 16): int(v, 16) for k, v in PALETTE.items()}

# theme slots that should be gold rather than the mapped green
GOLD_SLOTS = ["SIDEBAR_ACCENT", "BORDER_RING_HOVER", "SELECTED_OUTLINE"]

STRINGS = [
    ("Scripts/4_World/CommunityOnlineTools/Classes/GUI/JMCOTSideBar.c", 'm_TitleBarText.SetText("Community Online Tools");', 'm_TitleBarText.SetText("HOD Admin Tools"); // HOD: branding'),
    ("Scripts/4_World/CommunityOnlineTools/Classes/GUI/JMCOTSideBar.c", 'm_TitleBarText.SetText("COT");', 'm_TitleBarText.SetText("HOD"); // HOD: branding'),
    ("GUI/layouts/sidebar_menu.layout", 'text "Community Online Tools"', 'text "HOD Admin Tools"'),
    ("Scripts/4_World/CommunityOnlineTools/Classes/Webhook/JMWebhookModule.c", 'embed.SetAuthor( "Community Online Tools",', 'embed.SetAuthor( "HOD Admin Tools", /* HOD: branding */'),
    ("Scripts/config.cpp", 'name = "Community Online Tools";', 'name = "HOD Admin Tools"; // HOD: fork of Community Online Tools'),
    ("Scripts/config.cpp", 'credits = "Jacob_Mango, DannyDog, Arkensor, LiquidRock, LieutenantMaster";', 'credits = "House of D | based on Community Online Tools by Jacob_Mango, DannyDog, Arkensor, LiquidRock, LieutenantMaster"; // HOD:'),
]


def rd(p):
    return open(p, encoding="utf-8", errors="surrogateescape").read()


def wr(p, s):
    open(p, "w", encoding="utf-8", errors="surrogateescape", newline="").write(s)


def hex_sub(s):
    def f(m):
        a, rgb = m.group(1), int(m.group(2), 16)
        if rgb in PAL:
            return "0x%s%06X" % (a.upper(), PAL[rgb])
        return m.group(0)
    return re.sub(r"0x([0-9A-Fa-f]{2})([0-9A-Fa-f]{6})\b", f, s)


def styles_sub(s):
    def f(m):
        v = int(m.group(1))
        a, rgb = v >> 24, v & 0xFFFFFF
        if rgb in PAL:
            return 'Color="%d"' % ((a << 24) | PAL[rgb])
        return m.group(0)
    return re.sub(r'Color="(\d+)"', f, s)


def layout_sub(s):
    def f(m):
        vals = [float(x) for x in m.group(2).split()]
        rgb = [round(x * 255) for x in vals[:3]]
        for src, dst in PAL.items():
            sr = [(src >> 16) & 255, (src >> 8) & 255, src & 255]
            if all(abs(a - b) <= 1 for a, b in zip(rgb, sr)):
                d = [(dst >> 16) & 255, (dst >> 8) & 255, dst & 255]
                out = " ".join("%.3f" % (c / 255) for c in d)
                if len(vals) > 3:
                    out += " " + m.group(2).split()[3]
                return m.group(1) + out
        return m.group(0)
    return re.sub(r"(\bcolor )([0-9.]+ [0-9.]+ [0-9.]+(?: [0-9.]+)?)", f, s)


def main():
    changed = 0
    for p in glob.glob(os.path.join(ROOT, "Scripts/**/*.c"), recursive=True):
        s = rd(p); n = hex_sub(s)
        if n != s:
            wr(p, n); changed += 1
    for p in glob.glob(os.path.join(ROOT, "GUI/styles/*.styles")):
        s = rd(p); n = styles_sub(s)
        if n != s:
            wr(p, n); changed += 1
    for p in glob.glob(os.path.join(ROOT, "GUI/layouts/**/*.*ayout"), recursive=True):
        s = rd(p); n = layout_sub(s)
        if n != s:
            wr(p, n); changed += 1
    print("recoloured files:", changed)

    theme = os.path.join(ROOT, "Scripts/3_Game/CommunityOnlineTools/JMTheme.c")
    if os.path.exists(theme):
        s = rd(theme)
        for slot in GOLD_SLOTS:
            s = re.sub(r"(static const int %s\s*=\s*0x)([0-9A-Fa-f]{2})[0-9A-Fa-f]{6}" % slot, lambda m: m.group(1) + m.group(2) + GOLD, s)
        wr(theme, s)

    for rel, a, b in STRINGS:
        p = os.path.join(ROOT, rel)
        if not os.path.exists(p):
            print("MISSING FILE", rel); continue
        s = rd(p)
        if b in s:
            continue
        if a not in s:
            print("NOT FOUND", rel, a[:50]); continue
        wr(p, s.replace(a, b))

    st = os.path.join(ROOT, "languagecore/stringtable.csv")
    s = rd(st)
    print("stringtable renames:", s.count("Community Online Tools"))
    wr(st, s.replace("Community Online Tools", "HOD Admin Tools"))


if __name__ == "__main__":
    main()
