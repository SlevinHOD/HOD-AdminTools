HOD ADMIN TOOLS 0.2.1 - TEST BUILD (COT dev/1.30 base)
House of D | based on Community Online Tools (Jacob_Mango & team, CC BY-SA 4.0)

WHAT THIS IS
  A drop-in replacement for COT. Everything COT does, plus:
   - Object Builder   building sets: place, move, rotate, save, spawn on boot, export
   - XML Search       search the server's types.xml files (incl. Expansion types)
   - ZenCOT features  show all of a type on the map, clear markers, lifetime display,
                      copy cursor position (P), admin base-build helpers (Zenarchist)
   - HOD tweaks       confirm dialogs before deleting territories / objects / ESP objects
  Expansion's own COT integration (territories, groups, vehicle keys, ESP extras)
  keeps working because COT's internal names are unchanged.

INSTALL
 1. REMOVE the official @Community-Online-Tools from -mod= (they can't both load).
 2. Keep @CF. Add @HOD_AdminTools to -mod= on server and clients.
    Addons/ holds 5 PBOs, all named HOD_AdminTools*.pbo. When updating from 0.2.6 or
    older, DELETE any JM_COT_*.pbo files first (same addons under old names).
      -mod=@CF;@DayZ-Expansion-...;@HOD_AdminTools
 3. Your existing COT config carries over: same profile folder (CommunityOnlineTools),
    same permissions/roles files.
 4. Unsigned test build: set verifySignatures = 0, or sign it (see Keys/README.txt).

NEW PERMISSIONS (add to your COT roles)
  HOD.ObjectBuilder.View   open the Object Builder
  HOD.ObjectBuilder.Edit   create/delete sets, place/move/delete objects
  HOD.XMLSearch.View       open XML Search

OBJECT BUILDER
  Sets are saved to <profiles>/HOD_AdminTools/ObjectBuilder/<name>.json and
  "live" sets are spawned on every server start. Use Copy init.c / Copy XML to bake a
  finished set into your mission instead.
  Build-mode hotkeys (tick the checkbox): left click select, Left Ctrl deselect,
  mouse wheel rotate, Delete removes the selected object. Rebind under
  Options > Controls > HOD Admin Tools. Holster your weapon in build mode.

XML SEARCH
  Reads $mission:db/types.xml plus every type="types" file listed in
  cfgeconomycore.xml. Read-only. "Re-read files" after you edit them.
