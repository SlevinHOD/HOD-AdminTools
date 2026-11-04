# Upstream: Community Online Tools

- Source: https://github.com/Jacob-Mango/DayZ-CommunityOnlineTools
- Imported commit: 364216ad (dev/1.30 - the branch Workshop COT ships, 2026-11-01)
- License: CC BY-SA 4.0 (see LICENSE.COT). Authors: Jacob_Mango, DannyDog, Arkensor, LiquidRock, LieutenantMaster and contributors.
- Excluded on import: Workbench, AutoTest, ExampleScriptOverride, docs, AI notes, build.ps1.

Internal class names, the `JM_COT` define and CfgPatches names are kept unchanged on purpose:
DayZ Expansion's COT integration (55 files under `#ifdef JM_COT`) depends on them.
HOD changes to files in this tree are marked `// HOD:` so upstream merges stay easy.
