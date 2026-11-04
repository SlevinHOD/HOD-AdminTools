#!/usr/bin/env python3
"""HOD image -> PAA converter (DXT5, uncompressed mip chain).

Usage: img2paa.py <input image> <output.paa> [size]
  size  - square power-of-two output size (default 512). Image is fitted
          (aspect preserved) and centered on a transparent canvas.

Also writes <output>.dds next to it when --dds is passed, so the DXT5
encoding can be checked with any DDS viewer.
"""
import struct
import sys

import numpy as np
from PIL import Image


def fit_square(img, size):
    img = img.convert("RGBA")
    img.thumbnail((size, size), Image.LANCZOS)
    canvas = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    canvas.paste(img, ((size - img.width) // 2, (size - img.height) // 2), img)
    return canvas


def fit_exact(img, w, h):
    img = img.convert("RGBA")
    img.thumbnail((w, h), Image.LANCZOS)
    canvas = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    canvas.paste(img, ((w - img.width) // 2, (h - img.height) // 2), img)
    return canvas


def to565(c):
    c = c.astype(np.int32)
    return ((c[..., 0] >> 3) << 11) | ((c[..., 1] >> 2) << 5) | (c[..., 2] >> 3)


def from565(v):
    r = (v >> 11) & 31
    g = (v >> 5) & 63
    b = v & 31
    return np.stack([(r << 3) | (r >> 2), (g << 2) | (g >> 4), (b << 3) | (b >> 2)], -1).astype(np.int32)


def encode_dxt5(rgba):
    """rgba: HxWx4 uint8 (H, W multiples of 4). Returns bytes."""
    h, w, _ = rgba.shape
    blocks = rgba.reshape(h // 4, 4, w // 4, 4, 4).transpose(0, 2, 1, 3, 4).reshape(-1, 16, 4).astype(np.int32)
    n = blocks.shape[0]

    # ---- alpha (8-value mode, a0 > a1) ----
    a = blocks[..., 3]
    a0 = a.max(1)
    a1 = a.min(1)
    same = a0 == a1
    a0 = np.where(same & (a0 < 255), a0 + 1, a0)
    a1 = np.where(same & (a0 == 255) & (a1 == 255), 254, a1)
    pal_a = np.zeros((n, 8), np.int32)
    pal_a[:, 0] = a0
    pal_a[:, 1] = a1
    for i in range(1, 7):
        pal_a[:, i + 1] = ((7 - i) * a0 + i * a1) // 7
    ai = np.abs(a[:, :, None] - pal_a[:, None, :]).argmin(2).astype(np.uint64)
    abits = np.zeros(n, np.uint64)
    for p in range(16):
        abits |= ai[:, p] << np.uint64(3 * p)

    # ---- color (4-colour mode, c0 > c1) ----
    rgb = blocks[..., :3]
    lum = rgb[..., 0] * 3 + rgb[..., 1] * 6 + rgb[..., 2]
    # weight by alpha so invisible pixels don't pull endpoints
    vis = a > 8
    lum_hi = np.where(vis, lum, -1)
    lum_lo = np.where(vis, lum, 1 << 20)
    hi = rgb[np.arange(n), lum_hi.argmax(1)]
    lo = rgb[np.arange(n), lum_lo.argmin(1)]
    c0 = to565(hi)
    c1 = to565(lo)
    swap = c0 < c1
    c0, c1 = np.where(swap, c1, c0), np.where(swap, c0, c1)
    eq = c0 == c1
    p0 = from565(c0)
    p1 = from565(c1)
    pal = np.stack([p0, p1, (2 * p0 + p1) // 3, (p0 + 2 * p1) // 3], 1)
    d = ((rgb[:, :, None, :] - pal[:, None, :, :]) ** 2).sum(3)
    ci = d.argmin(2).astype(np.uint32)
    ci[eq] = 0
    cbits = np.zeros(n, np.uint32)
    for p in range(16):
        cbits |= ci[:, p] << np.uint32(2 * p)

    out = bytearray()
    for i in range(n):
        out += struct.pack("<BB", int(a0[i]), int(a1[i]))
        out += int(abits[i]).to_bytes(6, "little")
        out += struct.pack("<HHI", int(c0[i]), int(c1[i]), int(cbits[i]))
    return bytes(out)


def mip_chain(img):
    mips = [img]
    w, h = img.size
    while w > 4 and h > 4:
        w //= 2
        h //= 2
        mips.append(img.resize((w, h), Image.LANCZOS))
    return mips


def write_paa(img, path):
    mips = mip_chain(img)
    data = [encode_dxt5(np.asarray(m, dtype=np.uint8)) for m in mips]
    arr = np.asarray(img, dtype=np.float64)
    vis = arr[..., 3] > 0
    avg = arr[vis].mean(0) if vis.any() else np.zeros(4)
    avgc = bytes([int(avg[2]), int(avg[1]), int(avg[0]), int(avg[3])])  # BGRA
    maxc = bytes([255, 255, 255, 255])

    def tagg(name, payload):
        return b"GGAT" + name[::-1].encode() + struct.pack("<I", len(payload)) + payload

    head = struct.pack("<H", 0xFF05)
    head += tagg("AVGC", avgc)
    head += tagg("MAXC", maxc)
    head += tagg("FLAG", struct.pack("<I", 1))  # interpolated alpha
    offs_pos = len(head)
    head += tagg("OFFS", b"\0" * 64)
    head += struct.pack("<H", 0)  # palette: none

    body = bytearray()
    offsets = []
    for m, d in zip(mips, data):
        offsets.append(len(head) + len(body))
        body += struct.pack("<HH", m.width, m.height)
        body += len(d).to_bytes(3, "little")
        body += d
    body += b"\0" * 6
    offs = b"".join(struct.pack("<I", o) for o in offsets).ljust(64, b"\0")
    head = head[: offs_pos + 12] + offs + head[offs_pos + 12 + 64:]
    with open(path, "wb") as f:
        f.write(head + bytes(body))


def write_dds(img, path):
    """Single-level DXT5 DDS, for verifying the encoder."""
    w, h = img.size
    d = encode_dxt5(np.asarray(img, dtype=np.uint8))
    hdr = struct.pack("<4sIIIIIII44x", b"DDS ", 124, 0x1 | 0x2 | 0x4 | 0x1000 | 0x80000, h, w, len(d), 0, 1)
    pf = struct.pack("<II4sIIIII", 32, 0x4, b"DXT5", 0, 0, 0, 0, 0)
    caps = struct.pack("<IIII4x", 0x1000, 0, 0, 0)
    with open(path, "wb") as f:
        f.write(hdr + pf + caps + d)


if __name__ == "__main__":
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    src, dst = args[0], args[1]
    if len(args) > 2 and "x" in args[2]:
        w, h = (int(v) for v in args[2].split("x"))
        im = fit_exact(Image.open(src), w, h)
    else:
        im = fit_square(Image.open(src), int(args[2]) if len(args) > 2 else 512)
    write_paa(im, dst)
    if "--dds" in sys.argv:
        write_dds(im, dst.rsplit(".", 1)[0] + ".dds")
    print(f"{src} -> {dst} ({im.width}x{im.height})")
