#!/usr/bin/env python3
"""Minimal PBO packer/lister for DayZ (uncompressed, SHA1 trailer).

  pbo.py pack <source dir> <out.pbo> --prefix HOD_AdminTools
  pbo.py list <file.pbo>
"""
import hashlib
import os
import struct
import sys
import time

SKIP_EXT = {".pyc", ".bak", ".tmp", ".meta"}
SKIP_NAMES = {".git", ".DS_Store", "Thumbs.db", "$PREFIX$"}


def collect(src):
    files = []
    for root, dirs, names in os.walk(src):
        dirs[:] = sorted(d for d in dirs if d not in SKIP_NAMES)
        for n in sorted(names):
            if n in SKIP_NAMES or os.path.splitext(n)[1].lower() in SKIP_EXT:
                continue
            full = os.path.join(root, n)
            rel = os.path.relpath(full, src).replace("/", "\\")
            files.append((rel, full))
    return files


def pack(src, out, prefix):
    files = collect(src)
    ts = int(time.time())
    hdr = bytearray()
    # header extension entry
    hdr += b"\0" + struct.pack("<5I", 0x56657273, 0, 0, 0, 0)
    for k, v in (("prefix", prefix), ("product", "dayz"), ("version", "HOD")):
        hdr += k.encode() + b"\0" + v.encode() + b"\0"
    hdr += b"\0"
    blobs = []
    for rel, full in files:
        with open(full, "rb") as f:
            data = f.read()
        blobs.append(data)
        hdr += rel.encode("ascii") + b"\0" + struct.pack("<5I", 0, len(data), 0, ts, len(data))
    hdr += b"\0" + struct.pack("<5I", 0, 0, 0, 0, 0)
    body = bytes(hdr) + b"".join(blobs)
    digest = hashlib.sha1(body).digest()
    with open(out, "wb") as f:
        f.write(body + b"\0" + digest)
    return files


def read_cstr(b, i):
    j = b.index(b"\0", i)
    return b[i:j].decode("latin1"), j + 1


def listing(path):
    b = open(path, "rb").read()
    i = 0
    entries = []
    ext = {}
    first = True
    while True:
        name, i = read_cstr(b, i)
        mime, orig, _, ts, size = struct.unpack_from("<5I", b, i)
        i += 20
        if first and name == "" and mime == 0x56657273:
            while True:
                k, i = read_cstr(b, i)
                if not k:
                    break
                v, i = read_cstr(b, i)
                ext[k] = v
            first = False
            continue
        first = False
        if name == "":
            break
        entries.append((name, size))
    data_start = i
    total = sum(s for _, s in entries)
    ok = hashlib.sha1(b[: data_start + total]).digest() == b[data_start + total + 1:]
    return ext, entries, ok


if __name__ == "__main__":
    cmd = sys.argv[1]
    if cmd == "pack":
        prefix = sys.argv[sys.argv.index("--prefix") + 1]
        files = pack(sys.argv[2], sys.argv[3], prefix)
        print(f"packed {len(files)} files -> {sys.argv[3]} (prefix {prefix})")
    elif cmd == "list":
        ext, entries, ok = listing(sys.argv[2])
        print("extensions:", ext)
        for n, s in entries:
            print(f"{s:>9}  {n}")
        print("sha1 trailer:", "OK" if ok else "MISMATCH")
