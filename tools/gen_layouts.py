#!/usr/bin/env python3
"""Generates the primitive .layout files used by the HOD admin menu.

The menu itself is assembled in script (HOD_UI.c) from these primitives so
that positions/colours live in one place and layouts stay tiny.
"""
import os
import sys

OUT = sys.argv[1] if len(sys.argv) > 1 else "src/HOD_AdminTools/GUI/layouts"
FONT = "gui/fonts/sdf_MetronBook24"
FONT_BOLD = "gui/fonts/sdf_MetronBook24"


def exact(x=0, y=0, w=100, h=24, rel_size=False):
    hs = 0 if rel_size else 1
    return [
        f"position {x} {y}",
        f"size {w} {h}",
        "hexactpos 1",
        "vexactpos 1",
        f"hexactsize {hs}",
        f"vexactsize {hs}",
    ]


def block(cls, name, props, children=None, indent=0):
    pad = " " * indent
    out = [f"{pad}{cls} {name} {{"]
    for p in props:
        out.append(f"{pad} {p}")
    if children:
        out.append(f"{pad} {{")
        for c in children:
            out.append(c)
        out.append(f"{pad} }}")
    out.append(f"{pad}}}")
    return "\n".join(out)


def text_props(size=16, halign=None):
    p = [
        f'font "{FONT}"',
        '"exact text" 1',
        f'"exact text size" {size}',
        '"text valign" center',
    ]
    if halign:
        p.append(f'"text halign" {halign}')
    return p


layouts = {}

layouts["hod_root"] = block("FrameWidgetClass", "hod_root", [
    "ignorepointer 1",
    "position 0 0",
    "size 1 1",
    "hexactpos 1",
    "vexactpos 1",
    "hexactsize 0",
    "vexactsize 0",
], [
    block("PanelWidgetClass", "hod_dim", [
        "ignorepointer 1",
        "color 0 0 0 0.55",
        "position 0 0",
        "size 1 1",
        "hexactpos 1",
        "vexactpos 1",
        "hexactsize 0",
        "vexactsize 0",
        "style rover_sim_colorable",
    ], indent=2),
    block("FrameWidgetClass", "hod_window", [
        "ignorepointer 0",
        "position 0 0",
        "size 1180 720",
        "halign center_ref",
        "valign center_ref",
        "hexactpos 1",
        "vexactpos 1",
        "hexactsize 1",
        "vexactsize 1",
    ], indent=2),
])

layouts["hod_panel"] = block("PanelWidgetClass", "hod_panel", [
    "ignorepointer 1",
    "color 1 1 1 1",
] + exact() + ["style rover_sim_colorable"])

layouts["hod_frame"] = block("FrameWidgetClass", "hod_frame", ["ignorepointer 0"] + exact())

layouts["hod_label"] = block("TextWidgetClass", "hod_label", [
    "ignorepointer 1",
    "color 0.91 0.91 0.91 1",
] + exact(w=200, h=24) + ['text ""'] + text_props(16))

layouts["hod_label_center"] = block("TextWidgetClass", "hod_label_center", [
    "ignorepointer 1",
    "color 0.91 0.91 0.91 1",
] + exact(w=200, h=24) + ['text ""'] + text_props(16, "center"))

layouts["hod_button"] = block("ButtonWidgetClass", "hod_button", [
    "ignorepointer 0",
    "color 1 1 1 1",
] + exact(w=140, h=30) + ["style Empty"], [
    block("PanelWidgetClass", "hod_btn_bg", [
        "ignorepointer 1",
        "color 0.133 0.188 0.149 1",
        "position 0 0",
        "size 1 1",
        "hexactpos 1",
        "vexactpos 1",
        "hexactsize 0",
        "vexactsize 0",
        "style rover_sim_colorable",
    ], indent=2),
    block("PanelWidgetClass", "hod_btn_edge", [
        "ignorepointer 1",
        "color 0.831 0.686 0.216 1",
        "position 0 0",
        "size 3 1",
        "hexactpos 1",
        "vexactpos 1",
        "hexactsize 1",
        "vexactsize 0",
        "style rover_sim_colorable",
    ], indent=2),
    block("TextWidgetClass", "hod_btn_lbl", [
        "ignorepointer 1",
        "color 0.91 0.91 0.91 1",
        "position 0 0",
        "size 1 1",
        "hexactpos 1",
        "vexactpos 1",
        "hexactsize 0",
        "vexactsize 0",
        'text ""',
    ] + text_props(15, "center"), indent=2),
])

layouts["hod_edit"] = block("PanelWidgetClass", "hod_edit_bg", [
    "ignorepointer 1",
    "color 0.05 0.06 0.05 1",
] + exact(w=200, h=28) + ["style rover_sim_colorable"], [
    block("PanelWidgetClass", "hod_edit_line", [
        "ignorepointer 1",
        "color 0.831 0.686 0.216 0.8",
        "position 0 0",
        "size 1 1",
        "valign bottom_ref",
        "hexactpos 1",
        "vexactpos 1",
        "hexactsize 0",
        "vexactsize 1",
        "style rover_sim_colorable",
    ], indent=2),
    block("EditBoxWidgetClass", "hod_edit_box", [
        "ignorepointer 0",
        "color 1 1 1 1",
        "position 6 0",
        "size 0.97 1",
        "hexactpos 1",
        "vexactpos 1",
        "hexactsize 0",
        "vexactsize 0",
        "style ServerBrowser",
        '"Use default text" 0',
        '"exact text" 1',
        '"exact text size" 16',
        f'font "{FONT}"',
    ], indent=2),
])

layouts["hod_list"] = block("PanelWidgetClass", "hod_list_bg", [
    "ignorepointer 1",
    "color 0.05 0.06 0.05 1",
] + exact(w=300, h=300) + ["style rover_sim_colorable"], [
    block("TextListboxWidgetClass", "hod_list_box", [
        "ignorepointer 0",
        "position 4 4",
        "size 0.98 0.98",
        "hexactpos 1",
        "vexactpos 1",
        "hexactsize 0",
        "vexactsize 0",
        "style Editor",
        "lines 30",
        f'font "{FONT}"',
        '"exact text" 1',
        '"exact text size" 15',
    ], indent=2),
])

layouts["hod_image"] = block("ImageWidgetClass", "hod_image", [
    "ignorepointer 1",
    "color 1 1 1 1",
] + exact(w=64, h=64) + [
    "mode blend",
    '"src alpha" 1',
    '"no wrap" 1',
    "stretch 1",
])

layouts["hod_map"] = block("MapWidgetClass", "hod_map", [
    "ignorepointer 0",
] + exact(w=600, h=500))

os.makedirs(OUT, exist_ok=True)
for name, body in layouts.items():
    with open(os.path.join(OUT, name + ".layout"), "w", newline="\r\n") as f:
        f.write(body + "\n")
print(f"wrote {len(layouts)} layouts to {OUT}")
