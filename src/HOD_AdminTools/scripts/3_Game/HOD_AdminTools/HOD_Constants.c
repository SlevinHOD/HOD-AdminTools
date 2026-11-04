// HOD Admin Tools - House of D
// Shared constants

const string HOD_VERSION      = "0.1.0-test";
const int    HOD_RPC_ID       = 1213154305; // "HOD\x01"
const int    HOD_MENU_ID      = 484401;

const string HOD_PROFILE_DIR  = "$profile:HOD_AdminTools";
const string HOD_ADMINS_FILE  = "$profile:HOD_AdminTools/Admins.json";
const string HOD_SETTINGS_FILE = "$profile:HOD_AdminTools/Settings.json";
const string HOD_BANS_FILE    = "$profile:HOD_AdminTools/Bans.json";
const string HOD_TP_DIR       = "$profile:HOD_AdminTools/Teleports";
const string HOD_LOG_DIR      = "$profile:HOD_AdminTools/Logs";

const string HOD_LOGO         = "HOD_AdminTools/GUI/textures/hod_logo.paa";
const string HOD_DOT          = "HOD_AdminTools/GUI/textures/hod_dot.paa";

// ARGB colours (HOD gaming palette)
const int HOD_COL_GOLD        = 0xFFD4AF37;
const int HOD_COL_GREEN       = 0xFF2ECC55;
const int HOD_COL_GREEN_DARK  = 0xFF3F5F45;
const int HOD_COL_PANEL       = 0xF0141814;
const int HOD_COL_BUTTON      = 0xFF223026;
const int HOD_COL_BUTTON_HOT  = 0xFF3F5F45;
const int HOD_COL_TAB_ACTIVE  = 0xFF2E6B3C;
const int HOD_COL_RED         = 0xFFC0392B;
const int HOD_COL_WHITE       = 0xFFE8E8E8;
const int HOD_COL_GREY        = 0xFF8A8F8A;

// Discord embed colour (gold) as decimal
const int HOD_DISCORD_GOLD    = 13938487;

class HOD_Perm
{
	static const string PLAYERS_VIEW     = "players.view";
	static const string PLAYERS_HEAL     = "players.heal";
	static const string PLAYERS_KILL     = "players.kill";
	static const string PLAYERS_TELEPORT = "players.teleport";
	static const string PLAYERS_KICK     = "players.kick";
	static const string PLAYERS_BAN      = "players.ban";
	static const string PLAYERS_GODMODE  = "players.godmode";
	static const string PLAYERS_MESSAGE  = "players.message";
	static const string ITEMS_SPAWN      = "items.spawn";
	static const string ITEMS_SPAWN_OTHERS = "items.spawnothers";
	static const string TELEPORT_SELF    = "teleport.self";
	static const string TELEPORT_EDIT    = "teleport.edit";
	static const string WORLD_DELETE     = "world.delete";
	static const string WORLD_REPAIR     = "world.repair";
	static const string WORLD_WEATHER    = "world.weather";
	static const string WORLD_TIME       = "world.time";
	static const string SERVER_BROADCAST = "server.broadcast";
	static const string SELF_GODMODE     = "self.godmode";
	static const string SELF_HEAL        = "self.heal";
	static const string EXP_VEHICLES     = "expansion.vehicles";
	static const string EXP_CODELOCKS    = "expansion.codelocks";
	static const string EXP_ATM          = "expansion.atm";
	static const string VEHICLES_VIEW    = "vehicles.view";

	static array<string> All()
	{
		return {PLAYERS_VIEW, PLAYERS_HEAL, PLAYERS_KILL, PLAYERS_TELEPORT, PLAYERS_KICK, PLAYERS_BAN,
			PLAYERS_GODMODE, PLAYERS_MESSAGE, ITEMS_SPAWN, ITEMS_SPAWN_OTHERS, TELEPORT_SELF, TELEPORT_EDIT,
			WORLD_DELETE, WORLD_REPAIR, WORLD_WEATHER, WORLD_TIME, SERVER_BROADCAST, SELF_GODMODE, SELF_HEAL,
			EXP_VEHICLES, EXP_CODELOCKS, EXP_ATM, VEHICLES_VIEW};
	}
}
