HOD ADMIN TOOLS - TEST BUILD 0.1.0
House of D

INSTALL (server)
 1. Copy @HOD_AdminTools to the server root.
 2. Add it to -mod= (clients need it too - it carries the UI).
      -mod=@CF;@DayZ-Expansion-Core;...;@HOD_AdminTools
    Load order doesn't matter. Expansion is optional - Expansion features
    switch on automatically when Expansion is loaded.
 3. Unsigned test build: set verifySignatures = 0 in serverDZ.cfg, or sign
    it (see Keys/README.txt).
 4. Start once. Configs are created in:  <profiles>/HOD_AdminTools/
      Admins.json    - who is admin + permission groups
      Settings.json  - Discord webhook, server name, limits
      Bans.json      - managed in-game
      Teleports/<map>.json - teleport presets, editable in-game
      Logs/          - every admin action, one file per day
    Examples are in ServerProfile/ in this folder.
 5. Put your Steam64 ID in Admins.json, restart (or use RELOAD in-game).

IN GAME
  Home            open / close the admin menu
  LCtrl + End     teleport to crosshair
  (rebind under Options > Controls > HOD Admin Tools)

PERMISSIONS
  players.view players.heal players.kill players.teleport players.kick
  players.ban players.godmode players.message
  items.spawn items.spawnothers
  teleport.self teleport.edit
  world.delete world.repair world.weather world.time
  server.broadcast  self.godmode  self.heal
  expansion.vehicles expansion.codelocks expansion.atm
  vehicles.view  (server-wide VEHICLES tab)
  "*" = everything, "players.*" = every players permission.
  RELOAD in-game needs "*".

EXPANSION FEATURES (only when the matching Expansion module is loaded)
  Vehicles      unlock, give paired key, unpair all keys
  BaseBuilding  show / remove code on doors, gates, safes, tents
  Market        add / remove ATM money for a player
  Core          admin messages use Expansion notifications
  Godmode is re-applied every few seconds so safezone exits can't strip it.
