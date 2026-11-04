# HOD Admin Tools

House of D admin tools for DayZ: a fork of **Community Online Tools** with Object Builder,
XML Search and ZenCOT features added. Drop-in replacement for COT; Expansion's COT
integration keeps working.

Code: CC BY-SA 4.0. HOD branding: all rights reserved. See [NOTICE.md](NOTICE.md).

## Build (Linux, no DayZ Tools needed)
    ./build.sh
Output: `dist/@HOD_AdminTools/` and `dist/HOD_AdminTools_test.zip` (unsigned test build).

## Layout
| Path | What |
|---|---|
| `cot/JM/COT/` | COT fork (upstream 85d7f4d). HOD edits marked `// HOD:`; see `cot/UPSTREAM.md` |
| `hod/HOD_AdminTools/` | HOD addon PBO: Object Builder, XML Search, ZenCOT features, HOD tweaks |
| `tools/` | `pbo.py` packer, `img2paa.py` (DXT5 PAA), `check_scripts.py` |
| `branding/` | Source logo art |
| `docs/` | Server readme |

## Pulling COT updates
Diff upstream against `cot/UPSTREAM.md`'s commit, copy changed files into `cot/JM/COT`,
then re-apply anything marked `// HOD:` (branding only).

## Status
0.2.0-test - not yet compiled by the DayZ engine. Check `script.log` on first boot.
