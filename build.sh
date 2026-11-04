#!/usr/bin/env bash
# Builds dist/@HOD_AdminTools (unsigned test build).
set -euo pipefail
cd "$(dirname "$0")"

python3 tools/gen_layouts.py
python3 tools/check_scripts.py

OUT=dist/@HOD_AdminTools
rm -rf "$OUT"
mkdir -p "$OUT/Addons" "$OUT/Keys" "$OUT/ServerProfile/HOD_AdminTools"

python3 tools/pbo.py pack src/HOD_AdminTools "$OUT/Addons/HOD_AdminTools.pbo" --prefix HOD_AdminTools
python3 tools/pbo.py list "$OUT/Addons/HOD_AdminTools.pbo" | tail -1

cp mod/mod.cpp "$OUT/mod.cpp"
cp docs/ServerProfile/*.json "$OUT/ServerProfile/HOD_AdminTools/"
cp docs/README_SERVER.txt "$OUT/README.txt"
cp mod/Keys/README.txt "$OUT/Keys/README.txt"

( cd dist && rm -f HOD_AdminTools_test.zip && python3 -m zipfile -c HOD_AdminTools_test.zip @HOD_AdminTools )
echo "built $OUT and dist/HOD_AdminTools_test.zip"
