#!/usr/bin/env python3
"""Cheap static sanity checks for Enforce Script (we can't run the DayZ compiler on Linux).

Catches: unbalanced braces/parens/brackets, unterminated strings, ternary
operators, reserved words used as identifiers, duplicate class names, and
classes used across modules in the wrong direction (3_Game can't see 4_World).
"""
import os
import re
import sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else "src/HOD_AdminTools/scripts"
RESERVED = {"set", "map", "array", "vector", "Class", "Managed", "Param", "out", "inout", "ref", "class", "auto", "local", "string", "typename", "func", "null", "this", "super", "new", "delete", "notnull", "autoptr", "proto", "native", "static", "const", "private", "protected", "override", "event", "sealed", "modded"}
MODULES = ["3_Game", "4_World", "5_Mission"]


def strip(src):
    """Remove comments and string contents, keep line structure."""
    out = []
    i, n = 0, len(src)
    while i < n:
        c = src[i]
        if src.startswith("//", i):
            j = src.find("\n", i)
            i = n if j < 0 else j
        elif src.startswith("/*", i):
            j = src.find("*/", i + 2)
            seg = src[i: n if j < 0 else j + 2]
            out.append("\n" * seg.count("\n"))
            i = n if j < 0 else j + 2
        elif c == '"':
            j = i + 1
            while j < n and src[j] != '"':
                if src[j] == "\\":
                    j += 1
                if src[j] == "\n":
                    raise ValueError(f"unterminated string at offset {i}")
                j += 1
            out.append('""')
            i = j + 1
        else:
            out.append(c)
            i += 1
    return "".join(out)


def main():
    errors = []
    classes = {}
    files = []
    for mod in MODULES:
        for root, _, names in os.walk(os.path.join(ROOT, mod)):
            for nm in sorted(names):
                if nm.endswith(".c"):
                    files.append((mod, os.path.join(root, nm)))

    stripped = {}
    for mod, path in files:
        src = open(path, encoding="utf-8").read()
        try:
            s = strip(src)
        except ValueError as e:
            errors.append(f"{path}: {e}")
            continue
        stripped[path] = (mod, s)
        stack = []
        pairs = {")": "(", "]": "[", "}": "{"}
        for ln, line in enumerate(s.split("\n"), 1):
            for ch in line:
                if ch in "([{":
                    stack.append((ch, ln))
                elif ch in ")]}":
                    if not stack or stack[-1][0] != pairs[ch]:
                        errors.append(f"{path}:{ln}: unbalanced '{ch}'")
                        stack = []
                        break
                    stack.pop()
            if re.search(r"\?\s*[^:;]+\s*:", line) and "?" in line:
                errors.append(f"{path}:{ln}: looks like a ternary (not supported)")
            m = re.search(r"\b(?:int|float|bool|string|vector|array<[^>]+>|[A-Z]\w*)\s+(\w+)\s*[=;,)]", line)
            if m and m.group(1) in RESERVED:
                errors.append(f"{path}:{ln}: reserved word used as identifier: {m.group(1)}")
        if stack:
            errors.append(f"{path}: unclosed '{stack[-1][0]}' from line {stack[-1][1]}")
        for m in re.finditer(r"^\s*(modded\s+)?class\s+(\w+)", s, re.M):
            if m.group(1):
                continue
            name = m.group(2)
            if name in classes:
                errors.append(f"duplicate class {name}: {path} and {classes[name][1]}")
            classes[name] = (mod, path)

    # cross-module visibility: a class defined in module N can't be used from module < N
    order = {m: i for i, m in enumerate(MODULES)}
    for path, (mod, s) in stripped.items():
        for name, (cmod, cpath) in classes.items():
            if order[cmod] > order[mod] and re.search(r"\b" + name + r"\b", s):
                errors.append(f"{path}: uses {name} from {cmod} (not visible from {mod})")

    for e in errors:
        print("ERROR", e)
    print(f"checked {len(files)} files, {len(classes)} classes, {len(errors)} problem(s)")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
