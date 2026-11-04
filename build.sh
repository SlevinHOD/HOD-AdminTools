#!/usr/bin/env bash
# Builds dist/@HOD_AdminTools: the HOD fork of COT + the HOD addon PBO (unsigned test build).
set -euo pipefail
cd "$(dirname "$0")"

OUT=dist/@HOD_AdminTools
rm -rf "$OUT"
mkdir -p "$OUT/Addons" "$OUT/Keys"

pack() { # <source dir> <pbo name> <prefix>
  python3 tools/pbo.py pack "$1" "$OUT/Addons/$2.pbo" --prefix "$3"
}

# COT fork. File names are HOD; internal prefixes + CfgPatches stay COT's so Expansion's COT integration keeps working
pack cot/JM/COT/Scripts       HOD_AdminTools_Core     'JM\COT\Scripts'
pack cot/JM/COT/GUI           HOD_AdminTools_GUI      'JM\COT\GUI'
pack cot/JM/COT/languagecore  HOD_AdminTools_Language 'JM\COT\languagecore'
pack cot/JM/COT/Objects/Debug HOD_AdminTools_Objects  'JM\COT\Objects\Debug'

# HOD addon
if [ -d hod/HOD_AdminTools ]; then
  python3 tools/check_scripts.py hod/HOD_AdminTools/scripts
  pack hod/HOD_AdminTools HOD_AdminTools 'HOD_AdminTools'
fi

cp mod/mod.cpp "$OUT/mod.cpp"
cp docs/README_SERVER.txt "$OUT/README.txt"
cp mod/Keys/README.txt "$OUT/Keys/README.txt"
cp NOTICE.md "$OUT/NOTICE.md"

for f in "$OUT"/Addons/*.pbo; do python3 tools/pbo.py list "$f" | tail -1 | sed "s|^|$(basename "$f"): |"; done
( cd dist && rm -f HOD_AdminTools_test.zip && python3 -m zipfile -c HOD_AdminTools_test.zip @HOD_AdminTools )
echo "built $OUT and dist/HOD_AdminTools_test.zip"
