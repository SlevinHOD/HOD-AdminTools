// HOD Admin Tools - main admin menu (House of D gaming branding: black / green / gold)

class HOD_ItemIndex
{
	static ref array<string> s_Names;
	static ref array<string> s_Lower;
	static ref array<int>    s_Cat; // 0 gear, 1 weapon, 2 vehicle

	static void Build()
	{
		if (s_Names)
			return;
		s_Names = new array<string>();
		s_Lower = new array<string>();
		s_Cat = new array<int>();
		AddRoot("CfgVehicles");
		AddRoot("CfgWeapons");
		AddRoot("CfgMagazines");
	}

	protected static void AddRoot(string root)
	{
		int n = GetGame().ConfigGetChildrenCount(root);
		for (int i = 0; i < n; i++)
		{
			string name;
			GetGame().ConfigGetChildName(root, i, name);
			if (GetGame().ConfigGetInt(root + " " + name + " scope") != 2)
				continue;
			int cat = 0;
			if (root == "CfgWeapons")
				cat = 1;
			else if (root == "CfgVehicles" && GetGame().IsKindOf(name, "Transport"))
				cat = 2;
			else if (root == "CfgVehicles" && !GetGame().IsKindOf(name, "Inventory_Base") && !GetGame().IsKindOf(name, "DZ_LightAI"))
				continue; // skip buildings / static stuff
			string lower = name;
			lower.ToLower();
			s_Names.Insert(name);
			s_Lower.Insert(lower);
			s_Cat.Insert(cat);
		}
	}
}

class HOD_AdminMenu : UIScriptedMenu
{
	protected HOD_Client m_C;
	protected Widget m_Window;
	protected Widget m_Content;
	protected TextWidget m_Status;
	protected TextWidget m_Subtitle;

	protected ref map<Widget, string> m_Actions = new map<Widget, string>();
	protected ref map<Widget, int>    m_BaseColor = new map<Widget, int>();
	protected ref array<Widget>       m_Tabs = new array<Widget>();
	protected ref array<ButtonWidget> m_TabButtons = new array<ButtonWidget>();
	protected int m_ActiveTab = -1;

	// tab indices
	static const int TAB_PLAYERS = 0;
	static const int TAB_ITEMS = 1;
	static const int TAB_MAP = 2;
	static const int TAB_WORLD = 3;
	static const int TAB_SELF = 4;
	static const int TAB_EXPANSION = 5;
	static const int TAB_BANS = 6;
	static const int TAB_VEHICLES = 7;

	// players
	protected EditBoxWidget m_PlayerFilter;
	protected TextListboxWidget m_PlayerList;
	protected TextWidget m_PlayerCount;
	protected ref array<TextWidget> m_PlayerInfo = new array<TextWidget>();
	protected ref array<int> m_PlayerRows = new array<int>(); // list row -> player index
	protected EditBoxWidget m_MsgEdit;
	protected EditBoxWidget m_ReasonEdit;
	protected EditBoxWidget m_BanHoursEdit;
	protected string m_SelectedId = "";
	protected string m_SelectedName = "";
	protected int m_LastPlayerRow = -2;
	protected string m_LastPlayerFilter = "";

	// items
	protected EditBoxWidget m_ItemSearch;
	protected TextListboxWidget m_ItemList;
	protected TextWidget m_ItemSelected;
	protected TextWidget m_ItemSpawnTarget;
	protected EditBoxWidget m_ItemCount;
	protected ref array<string> m_ItemRows = new array<string>();
	protected ref array<ButtonWidget> m_CatButtons = new array<ButtonWidget>();
	protected int m_ItemCat = -1;
	protected string m_LastItemSearch = "~";
	protected int m_LastItemRow = -2;
	protected string m_SelectedItem = "";

	// map / teleport
	protected TextListboxWidget m_TpList;
	protected EditBoxWidget m_TpName;
	protected EditBoxWidget m_TpCoords;
	protected MapWidget m_Map;
	protected bool m_MapCentered;

	// world
	protected EditBoxWidget m_HourEdit;
	protected EditBoxWidget m_MinuteEdit;
	protected EditBoxWidget m_BroadcastEdit;

	// self / expansion
	protected TextWidget m_SelfLooking;
	protected TextWidget m_SelfPos;
	protected ButtonWidget m_GodButton;
	protected TextWidget m_ExpLooking;
	protected TextWidget m_ExpPlayer;
	protected EditBoxWidget m_AtmAmount;

	// vehicles
	protected TextListboxWidget m_VehList;
	protected EditBoxWidget m_VehFilter;
	protected TextWidget m_VehCount;
	protected ref array<TextWidget> m_VehInfo = new array<TextWidget>();
	protected ref array<int> m_VehRows = new array<int>();
	protected int m_LastVehRow = -2;
	protected string m_LastVehFilter = "";
	protected string m_VehLo = "";
	protected string m_VehHi = "";

	// bans
	protected TextListboxWidget m_BanList;
	protected EditBoxWidget m_BanIdEdit;
	protected EditBoxWidget m_BanIdHours;
	protected EditBoxWidget m_BanIdReason;

	protected float m_Tick;
	protected float m_RefreshTick;
	protected bool m_HomeWasDown;
	protected bool m_EscWasDown;

	void HOD_AdminMenu()
	{
		m_C = HOD_Client.Get();
	}

	void ~HOD_AdminMenu()
	{
		if (m_C && m_C.m_Menu == this)
			m_C.m_Menu = null;
	}

	override int GetID()
	{
		return HOD_MENU_ID;
	}

	// ================================================================ build

	override Widget Init()
	{
		layoutRoot = GetGame().GetWorkspace().CreateWidgets(HOD_UI.L + "hod_root.layout");
		m_Window = layoutRoot.FindAnyWidget("hod_window");

		// frame: gold outline + dark body (sticker style)
		HOD_UI.Panel(m_Window, 0, 0, 1180, 720, HOD_COL_GOLD);
		HOD_UI.Panel(m_Window, 2, 2, 1176, 716, 0xF60C0F0C);

		// header
		HOD_UI.Panel(m_Window, 2, 2, 1176, 72, 0xFF111A13);
		HOD_UI.Panel(m_Window, 2, 2, 6, 72, HOD_COL_GREEN);
		HOD_UI.Image(m_Window, 16, 5, 64, 64, HOD_LOGO);
		HOD_UI.Label(m_Window, 92, 8, 600, 34, "HOD ADMIN TOOLS", 28, HOD_COL_GOLD);
		m_Subtitle = HOD_UI.Label(m_Window, 92, 42, 900, 22, "", 14, HOD_COL_GREY);
		ButtonWidget close = HOD_UI.Button(m_Window, 1128, 20, 36, 34, "X");
		Register(close, "close", 0xFF5A1E1A);
		HOD_UI.Panel(m_Window, 2, 74, 1176, 2, HOD_COL_GOLD);

		// sidebar
		HOD_UI.Panel(m_Window, 2, 76, 172, 612, 0xFF0F1410);
		HOD_UI.Panel(m_Window, 174, 76, 1, 612, 0x66D4AF37);
		AddTabButton("PLAYERS", 0);
		AddTabButton("ITEMS", 1);
		AddTabButton("MAP / TELEPORT", 2);
		AddTabButton("WORLD", 3);
		AddTabButton("SELF / CURSOR", 4);
		AddTabButton("EXPANSION", 5);
		AddTabButton("BANS", 6);
		AddTabButton("VEHICLES", 7);
		HOD_UI.Label(m_Window, 12, 640, 156, 20, "HOUSE OF D", 14, HOD_COL_GOLD, true);
		HOD_UI.Label(m_Window, 12, 660, 156, 20, HOD_VERSION, 12, HOD_COL_GREY, true);

		// content
		m_Content = HOD_UI.Frame(m_Window, 186, 88, 982, 594);
		BuildPlayers();
		BuildItems();
		BuildMap();
		BuildWorld();
		BuildSelf();
		BuildExpansion();
		BuildBans();
		BuildVehicles();

		// footer
		HOD_UI.Panel(m_Window, 2, 688, 1176, 1, 0x88D4AF37);
		HOD_UI.Panel(m_Window, 2, 689, 1176, 29, 0xFF0F1410);
		m_Status = HOD_UI.Label(m_Window, 14, 692, 1000, 24, "Ready", 14, HOD_COL_GREEN);
		HOD_UI.Label(m_Window, 1020, 692, 146, 24, "HOD", 14, HOD_COL_GOLD, true);

		if (m_C)
		{
			m_C.m_Menu = this;
			string sub = m_C.m_ServerName + "  |  " + m_C.m_World + "  |  Role: " + m_C.m_Group + "  |  Expansion: " + m_C.m_ExpModules;
			m_Subtitle.SetText(sub);
			if (m_C.m_Status != "")
				SetStatus(m_C.m_Status);
			m_C.Send("players");
			m_C.Send("tp.list");
		}

		ShowTab(TAB_PLAYERS);
		return layoutRoot;
	}

	protected void Register(ButtonWidget b, string action, int color = HOD_COL_BUTTON)
	{
		if (!b)
			return;
		m_Actions.Set(b, action);
		m_BaseColor.Set(b, color);
		HOD_UI.SetButtonColor(b, color);
	}

	protected ButtonWidget Btn(Widget parent, float x, float y, float w, float h, string text, string action, int color = HOD_COL_BUTTON)
	{
		ButtonWidget b = HOD_UI.Button(parent, x, y, w, h, text);
		Register(b, action, color);
		return b;
	}

	protected void AddTabButton(string text, int idx)
	{
		ButtonWidget b = Btn(m_Window, 10, 88 + idx * 44, 156, 36, text, "tab:" + idx.ToString());
		m_TabButtons.Insert(b);
	}

	protected Widget NewTab()
	{
		Widget f = HOD_UI.Frame(m_Content, 0, 0, 982, 594);
		f.Show(false);
		m_Tabs.Insert(f);
		return f;
	}

	protected void Section(Widget parent, float x, float y, float w, string title)
	{
		HOD_UI.Label(parent, x, y, w, 22, title, 15, HOD_COL_GOLD);
		HOD_UI.Panel(parent, x, y + 23, w, 1, 0x66D4AF37);
	}

	protected void BuildPlayers()
	{
		Widget t = NewTab();
		HOD_UI.Label(t, 0, 0, 60, 30, "Filter", 15, HOD_COL_GREY);
		m_PlayerFilter = HOD_UI.Edit(t, 56, 0, 240, 30);
		Btn(t, 306, 0, 110, 30, "REFRESH", "players.refresh");
		m_PlayerCount = HOD_UI.Label(t, 0, 566, 420, 26, "0 online", 14, HOD_COL_GREY);
		m_PlayerList = HOD_UI.List(t, 0, 40, 420, 520);

		Section(t, 440, 0, 540, "SELECTED PLAYER");
		for (int i = 0; i < 7; i++)
			m_PlayerInfo.Insert(HOD_UI.Label(t, 440, 30 + i * 25, 540, 24, "", 15));
		m_PlayerInfo[0].SetText("Select a player from the list");

		Section(t, 440, 210, 540, "ACTIONS");
		Btn(t, 440, 242, 128, 32, "HEAL", "p.heal");
		Btn(t, 576, 242, 128, 32, "TP TO", "p.tpto");
		Btn(t, 712, 242, 128, 32, "BRING", "p.bring");
		Btn(t, 848, 242, 128, 32, "GODMODE", "p.god");
		Btn(t, 440, 282, 128, 32, "COPY ID", "p.copy");
		Btn(t, 576, 282, 128, 32, "SHOW ON MAP", "p.locate");
		Btn(t, 712, 282, 128, 32, "SPAWN ITEMS", "p.items");
		Btn(t, 848, 282, 128, 32, "KILL", "p.kill", 0xFF5A1E1A);

		Section(t, 440, 330, 540, "MESSAGE");
		m_MsgEdit = HOD_UI.Edit(t, 440, 362, 400, 30);
		Btn(t, 848, 362, 128, 30, "SEND", "p.msg");

		Section(t, 440, 410, 540, "KICK / BAN");
		HOD_UI.Label(t, 440, 440, 80, 30, "Reason", 15, HOD_COL_GREY);
		m_ReasonEdit = HOD_UI.Edit(t, 520, 440, 456, 30);
		HOD_UI.Label(t, 440, 480, 170, 30, "Ban hours (0 = perm)", 15, HOD_COL_GREY);
		m_BanHoursEdit = HOD_UI.Edit(t, 612, 480, 90, 30, "0");
		Btn(t, 712, 480, 128, 30, "KICK", "p.kick", 0xFF5A3A1A);
		Btn(t, 848, 480, 128, 30, "BAN", "p.ban", 0xFF5A1E1A);
	}

	protected void BuildItems()
	{
		Widget t = NewTab();
		HOD_UI.Label(t, 0, 0, 60, 30, "Search", 15, HOD_COL_GREY);
		m_ItemSearch = HOD_UI.Edit(t, 64, 0, 220, 30);
		m_CatButtons.Insert(Btn(t, 292, 0, 56, 30, "ALL", "cat:-1"));
		m_CatButtons.Insert(Btn(t, 352, 0, 56, 30, "GEAR", "cat:0"));
		m_CatButtons.Insert(Btn(t, 412, 0, 52, 30, "GUNS", "cat:1"));
		m_CatButtons.Insert(Btn(t, 468, 0, 52, 30, "CARS", "cat:2"));
		m_ItemList = HOD_UI.List(t, 0, 40, 520, 554);

		Section(t, 540, 0, 440, "SELECTED ITEM");
		m_ItemSelected = HOD_UI.Label(t, 540, 30, 440, 30, "none", 20, HOD_COL_GREEN);
		HOD_UI.Label(t, 540, 72, 80, 30, "Count", 15, HOD_COL_GREY);
		m_ItemCount = HOD_UI.Edit(t, 620, 72, 90, 30, "1");

		Section(t, 540, 120, 440, "SPAWN");
		Btn(t, 540, 152, 440, 36, "MY INVENTORY  (falls back to ground)", "item:inv");
		Btn(t, 540, 196, 440, 36, "GROUND AT MY FEET", "item:ground");
		Btn(t, 540, 240, 440, 36, "WHERE I'M LOOKING (crosshair)", "item:cursor");
		Btn(t, 540, 284, 440, 36, "SELECTED PLAYER'S INVENTORY", "item:player");
		m_ItemSpawnTarget = HOD_UI.Label(t, 540, 328, 440, 24, "Selected player: none", 14, HOD_COL_GREY);
		HOD_UI.Label(t, 540, 370, 440, 24, "Vehicles spawn assembled and fuelled.", 14, HOD_COL_GREY);
		HOD_UI.Label(t, 540, 394, 440, 24, "Search needs at least 2 letters. Max 300 results.", 14, HOD_COL_GREY);
		HOD_UI.Label(t, 540, 418, 440, 24, "Expansion vehicles: use EXPANSION tab to pair a key.", 14, HOD_COL_GREY);
	}

	protected void BuildMap()
	{
		Widget t = NewTab();
		Section(t, 0, 0, 300, "TELEPORT PRESETS");
		m_TpList = HOD_UI.List(t, 0, 30, 300, 340);
		Btn(t, 0, 378, 146, 32, "GO", "tp.go");
		Btn(t, 154, 378, 146, 32, "DELETE", "tp.del", 0xFF5A1E1A);
		HOD_UI.Label(t, 0, 418, 300, 22, "Save current position as:", 14, HOD_COL_GREY);
		m_TpName = HOD_UI.Edit(t, 0, 442, 190, 30);
		Btn(t, 198, 442, 102, 30, "SAVE", "tp.add");
		HOD_UI.Label(t, 0, 480, 300, 22, "Coordinates (x z):", 14, HOD_COL_GREY);
		m_TpCoords = HOD_UI.Edit(t, 0, 504, 190, 30);
		Btn(t, 198, 504, 102, 30, "GO", "tp.coords");
		Btn(t, 0, 548, 300, 32, "REFRESH PLAYER MARKERS", "map.refresh");

		HOD_UI.Panel(t, 314, 0, 668, 594, HOD_COL_GOLD);
		m_Map = HOD_UI.Map(t, 316, 2, 664, 590);
		HOD_UI.Label(t, 324, 6, 640, 22, "Double-click the map to teleport", 14, HOD_COL_GOLD);
	}

	protected void BuildWorld()
	{
		Widget t = NewTab();
		Section(t, 0, 0, 600, "TIME");
		m_HourEdit = HOD_UI.Edit(t, 0, 32, 70, 30, "12");
		HOD_UI.Label(t, 74, 32, 12, 30, ":", 18, HOD_COL_GOLD);
		m_MinuteEdit = HOD_UI.Edit(t, 88, 32, 70, 30, "00");
		Btn(t, 168, 32, 110, 30, "SET TIME", "time.set");
		Btn(t, 0, 72, 110, 30, "06:00", "time:6");
		Btn(t, 118, 72, 110, 30, "12:00", "time:12");
		Btn(t, 236, 72, 110, 30, "18:00", "time:18");
		Btn(t, 354, 72, 110, 30, "00:00", "time:0");

		Section(t, 0, 124, 600, "WEATHER");
		Btn(t, 0, 156, 110, 32, "CLEAR", "weather:clear");
		Btn(t, 118, 156, 110, 32, "CLOUDY", "weather:cloudy");
		Btn(t, 236, 156, 110, 32, "RAIN", "weather:rain");
		Btn(t, 354, 156, 110, 32, "STORM", "weather:storm");
		Btn(t, 472, 156, 110, 32, "FOG", "weather:fog");

		Section(t, 0, 212, 980, "BROADCAST TO ALL PLAYERS");
		m_BroadcastEdit = HOD_UI.Edit(t, 0, 244, 820, 30);
		Btn(t, 830, 244, 150, 30, "SEND", "broadcast");

		Section(t, 0, 300, 980, "SERVER");
		Btn(t, 0, 332, 240, 32, "RELOAD HOD CONFIGS", "reload");
		HOD_UI.Label(t, 250, 332, 720, 32, "Re-reads Admins.json, Settings.json, Bans.json and teleports (needs '*').", 14, HOD_COL_GREY);
	}

	protected void BuildSelf()
	{
		Widget t = NewTab();
		Section(t, 0, 0, 300, "SELF");
		m_GodButton = Btn(t, 0, 32, 300, 36, "GODMODE: OFF", "self.god");
		Btn(t, 0, 76, 300, 36, "HEAL ME", "self.heal");

		Section(t, 0, 132, 300, "CROSSHAIR");
		Btn(t, 0, 164, 300, 36, "TELEPORT TO CROSSHAIR", "cur.tp");
		Btn(t, 0, 208, 300, 36, "REPAIR TARGET", "cur.repair");
		Btn(t, 0, 252, 300, 36, "REFUEL VEHICLE", "cur.refuel");
		Btn(t, 0, 296, 300, 36, "DELETE TARGET", "cur.delete", 0xFF5A1E1A);

		Section(t, 330, 0, 650, "INFO");
		m_SelfLooking = HOD_UI.Label(t, 330, 32, 650, 24, "", 15);
		m_SelfPos = HOD_UI.Label(t, 330, 58, 650, 24, "", 15);
		HOD_UI.Label(t, 330, 100, 650, 22, "Aim at something before opening the menu (Home).", 14, HOD_COL_GREY);
		HOD_UI.Label(t, 330, 122, 650, 22, "LCtrl + End = teleport to crosshair without the menu.", 14, HOD_COL_GREY);
		HOD_UI.Label(t, 330, 144, 650, 22, "Rebind both in Options > Controls > HOD Admin Tools.", 14, HOD_COL_GREY);
		HOD_UI.Label(t, 330, 166, 650, 22, "Godmode is re-applied every few seconds so Expansion safezones can't strip it.", 14, HOD_COL_GREY);
	}

	protected void BuildExpansion()
	{
		Widget t = NewTab();
		string mods = "not detected";
		if (m_C)
			mods = m_C.m_ExpModules;
		Section(t, 0, 0, 980, "DAYZ EXPANSION");
		HOD_UI.Label(t, 0, 30, 980, 24, "Loaded modules: " + mods, 15, HOD_COL_GREEN);
		m_ExpLooking = HOD_UI.Label(t, 0, 56, 980, 24, "", 15);

		Section(t, 0, 96, 480, "VEHICLES (aim at the vehicle)");
		Btn(t, 0, 128, 236, 34, "UNLOCK VEHICLE", "exp.unlock");
		Btn(t, 244, 128, 236, 34, "GIVE ME A PAIRED KEY", "exp.key");
		Btn(t, 0, 170, 236, 34, "UNPAIR ALL KEYS", "exp.resetkeys", 0xFF5A3A1A);

		Section(t, 500, 96, 480, "CODE LOCKS (aim at door / gate / safe / tent)");
		Btn(t, 500, 128, 236, 34, "SHOW CODE", "exp.readcode");
		Btn(t, 744, 128, 236, 34, "REMOVE CODE", "exp.clearcode", 0xFF5A3A1A);

		Section(t, 0, 232, 480, "MARKET ATM");
		m_ExpPlayer = HOD_UI.Label(t, 0, 262, 480, 24, "Selected player: none (pick one in PLAYERS)", 15);
		HOD_UI.Label(t, 0, 294, 80, 30, "Amount", 15, HOD_COL_GREY);
		m_AtmAmount = HOD_UI.Edit(t, 80, 294, 150, 30, "1000");
		Btn(t, 240, 294, 116, 30, "ADD", "exp.atm.add");
		Btn(t, 364, 294, 116, 30, "REMOVE", "exp.atm.remove", 0xFF5A3A1A);
		HOD_UI.Label(t, 0, 334, 480, 22, "Player must have opened an ATM at least once.", 14, HOD_COL_GREY);
	}

	protected void BuildBans()
	{
		Widget t = NewTab();
		Section(t, 0, 0, 640, "BAN LIST");
		m_BanList = HOD_UI.List(t, 0, 30, 640, 516);
		Btn(t, 0, 554, 200, 32, "UNBAN SELECTED", "ban.unban");
		Btn(t, 208, 554, 140, 32, "REFRESH", "ban.refresh");

		Section(t, 660, 0, 320, "BAN BY STEAM64 ID");
		HOD_UI.Label(t, 660, 30, 320, 22, "Steam64 ID", 14, HOD_COL_GREY);
		m_BanIdEdit = HOD_UI.Edit(t, 660, 54, 320, 30);
		HOD_UI.Label(t, 660, 92, 320, 22, "Hours (0 = permanent)", 14, HOD_COL_GREY);
		m_BanIdHours = HOD_UI.Edit(t, 660, 116, 100, 30, "0");
		HOD_UI.Label(t, 660, 154, 320, 22, "Reason", 14, HOD_COL_GREY);
		m_BanIdReason = HOD_UI.Edit(t, 660, 178, 320, 30);
		Btn(t, 660, 222, 200, 34, "BAN ID", "ban.id", 0xFF5A1E1A);
	}

	protected void BuildVehicles()
	{
		Widget t = NewTab();
		HOD_UI.Label(t, 0, 0, 60, 30, "Filter", 15, HOD_COL_GREY);
		m_VehFilter = HOD_UI.Edit(t, 56, 0, 300, 30);
		Btn(t, 366, 0, 110, 30, "REFRESH", "veh.refresh");
		m_VehList = HOD_UI.List(t, 0, 40, 560, 520);
		m_VehCount = HOD_UI.Label(t, 0, 566, 560, 26, "", 14, HOD_COL_GREY);

		Section(t, 580, 0, 400, "SELECTED VEHICLE");
		for (int i = 0; i < 6; i++)
			m_VehInfo.Insert(HOD_UI.Label(t, 580, 30 + i * 25, 400, 24, "", 15));
		m_VehInfo[0].SetText("Select a vehicle from the list");

		Section(t, 580, 190, 400, "ACTIONS");
		Btn(t, 580, 222, 196, 32, "TELEPORT TO", "veh.tp");
		Btn(t, 784, 222, 196, 32, "SHOW ON MAP", "veh.locate");
		Btn(t, 580, 262, 196, 32, "REPAIR", "veh.obj.repair");
		Btn(t, 784, 262, 196, 32, "REFUEL", "veh.obj.refuel");
		Btn(t, 580, 302, 400, 32, "DELETE VEHICLE", "veh.obj.delete", 0xFF5A1E1A);

		Section(t, 580, 352, 400, "EXPANSION");
		Btn(t, 580, 384, 196, 32, "UNLOCK", "veh.exp.unlock");
		Btn(t, 784, 384, 196, 32, "GIVE PAIRED KEY", "veh.exp.key");
		Btn(t, 580, 424, 400, 32, "UNPAIR ALL KEYS", "veh.exp.resetkeys", 0xFF5A3A1A);
		HOD_UI.Label(t, 580, 466, 400, 22, "Owner / key / lock columns need Expansion Vehicles.", 14, HOD_COL_GREY);
		HOD_UI.Label(t, 580, 488, 400, 22, "Works on vehicles anywhere on the map.", 14, HOD_COL_GREY);
	}

	protected void RefreshVehicles()
	{
		if (!m_VehList || !m_C)
			return;
		string filter = m_VehFilter.GetText();
		filter.ToLower();
		m_VehList.ClearItems();
		m_VehRows.Clear();
		int vs = HOD_Client.VEH_STRIDE;
		int count = m_C.m_Vehicles.Count() / vs;
		int selRow = -1;
		for (int i = 0; i < count; i++)
		{
			int b = i * vs;
			string line = m_C.m_Vehicles[b + 2] + "  |  " + m_C.m_Vehicles[b + 4] + "%  |  " + m_C.m_Vehicles[b + 5];
			string lower = line;
			lower.ToLower();
			if (filter != "" && lower.IndexOf(filter) < 0)
				continue;
			int row = m_VehList.AddItem(line, null, 0);
			m_VehRows.Insert(i);
			if (m_C.m_Vehicles[b] == m_VehLo && m_C.m_Vehicles[b + 1] == m_VehHi)
				selRow = row;
		}
		if (selRow >= 0)
			m_VehList.SelectRow(selRow);
		m_LastVehRow = m_VehList.GetSelectedRow();
		m_VehCount.SetText(count.ToString() + " vehicles on the server");
		UpdateVehicleInfo();
	}

	protected int SelectedVehicleIndex()
	{
		if (!m_C || m_VehLo == "")
			return -1;
		int vs = HOD_Client.VEH_STRIDE;
		int count = m_C.m_Vehicles.Count() / vs;
		for (int i = 0; i < count; i++)
		{
			if (m_C.m_Vehicles[i * vs] == m_VehLo && m_C.m_Vehicles[i * vs + 1] == m_VehHi)
				return i;
		}
		return -1;
	}

	protected void UpdateVehicleInfo()
	{
		int idx = SelectedVehicleIndex();
		if (idx < 0)
		{
			m_VehInfo[0].SetText("Select a vehicle from the list");
			for (int k = 1; k < m_VehInfo.Count(); k++)
				m_VehInfo[k].SetText("");
			return;
		}
		int b = idx * HOD_Client.VEH_STRIDE;
		array<string> V = m_C.m_Vehicles;
		vector pos = V[b + 3].ToVector();
		m_VehInfo[0].SetText("Type:   " + V[b + 2]);
		m_VehInfo[1].SetText(string.Format("Position:   %1 / %2", Math.Round(pos[0]), Math.Round(pos[2])));
		m_VehInfo[2].SetText("Health:   " + V[b + 4] + "%");
		m_VehInfo[3].SetText("Owner:   " + V[b + 5]);
		m_VehInfo[4].SetText("Keys:   " + V[b + 6]);
		m_VehInfo[5].SetText("Lock:   " + V[b + 7]);
	}

	protected bool NeedVehicle()
	{
		if (m_VehLo == "")
		{
			SetStatus("Select a vehicle first");
			return false;
		}
		return true;
	}

	// ================================================================ state

	void SetStatus(string text)
	{
		if (m_Status)
			m_Status.SetText(text);
	}

	protected void ShowTab(int idx)
	{
		for (int i = 0; i < m_Tabs.Count(); i++)
		{
			m_Tabs[i].Show(i == idx);
			int col = HOD_COL_BUTTON;
			if (i == idx)
				col = HOD_COL_TAB_ACTIVE;
			m_BaseColor.Set(m_TabButtons[i], col);
			HOD_UI.SetButtonColor(m_TabButtons[i], col);
		}
		m_ActiveTab = idx;
		if (!m_C)
			return;
		if (idx == TAB_PLAYERS || idx == TAB_MAP)
			m_C.Send("players");
		if (idx == TAB_MAP)
		{
			m_C.Send("tp.list");
			CenterMap();
		}
		if (idx == TAB_BANS)
			m_C.Send("bans");
		if (idx == TAB_VEHICLES)
			m_C.Send("vehicles");
		if (idx == TAB_ITEMS)
		{
			HOD_ItemIndex.Build();
			m_LastItemSearch = "~";
		}
	}

	protected void CenterMap()
	{
		if (!m_Map || m_MapCentered)
			return;
		Man p = GetGame().GetPlayer();
		if (p)
		{
			m_Map.SetMapPos(p.GetPosition());
			m_Map.SetScale(0.25);
			m_MapCentered = true;
		}
	}

	// called by HOD_Client after every server message
	void OnServerData(string cmd)
	{
		switch (cmd)
		{
			case "players":
				RefreshPlayerList();
				RefreshMapMarkers();
				break;
			case "tplist":
				RefreshTeleports();
				break;
			case "bans":
				RefreshBans();
				break;
			case "vehicles":
				RefreshVehicles();
				break;
			case "god":
				UpdateGodButton();
				break;
			case "hello":
				UpdateGodButton();
				break;
		}
	}

	protected void UpdateGodButton()
	{
		if (!m_C)
			return;
		if (m_C.m_God)
			HOD_UI.SetButtonText(m_GodButton, "GODMODE: ON");
		else
			HOD_UI.SetButtonText(m_GodButton, "GODMODE: OFF");
	}

	protected void RefreshPlayerList()
	{
		if (!m_C || !m_PlayerList)
			return;
		string filter = m_PlayerFilter.GetText();
		filter.ToLower();
		m_PlayerList.ClearItems();
		m_PlayerRows.Clear();
		int stride = HOD_Client.PLAYER_STRIDE;
		int count = m_C.m_Players.Count() / stride;
		int selRow = -1;
		for (int i = 0; i < count; i++)
		{
			string name = m_C.m_Players[i * stride + 1];
			string lower = name;
			lower.ToLower();
			if (filter != "" && lower.IndexOf(filter) < 0 && m_C.m_Players[i * stride].IndexOf(filter) < 0)
				continue;
			string line = name;
			if (m_C.m_Players[i * stride + 5] == "1")
				line += "  [GOD]";
			if (m_C.m_Players[i * stride + 7] == "0")
				line += "  [DEAD]";
			int row = m_PlayerList.AddItem(line, null, 0);
			m_PlayerRows.Insert(i);
			if (m_C.m_Players[i * stride] == m_SelectedId)
				selRow = row;
		}
		if (selRow >= 0)
			m_PlayerList.SelectRow(selRow);
		m_LastPlayerRow = m_PlayerList.GetSelectedRow();
		m_PlayerCount.SetText(count.ToString() + " online");
		UpdatePlayerInfo();
	}

	protected int SelectedPlayerIndex()
	{
		if (!m_C || m_SelectedId == "")
			return -1;
		int stride = HOD_Client.PLAYER_STRIDE;
		int count = m_C.m_Players.Count() / stride;
		for (int i = 0; i < count; i++)
		{
			if (m_C.m_Players[i * stride] == m_SelectedId)
				return i;
		}
		return -1;
	}

	protected void UpdatePlayerInfo()
	{
		int idx = SelectedPlayerIndex();
		string selText = "Selected player: none";
		if (idx < 0)
		{
			if (m_SelectedId == "")
				m_PlayerInfo[0].SetText("Select a player from the list");
			else
				m_PlayerInfo[0].SetText(m_SelectedName + " (offline)");
			for (int k = 1; k < m_PlayerInfo.Count(); k++)
				m_PlayerInfo[k].SetText("");
		}
		else
		{
			int s = HOD_Client.PLAYER_STRIDE;
			int b = idx * s;
			array<string> P = m_C.m_Players;
			vector pos = P[b + 2].ToVector();
			m_SelectedName = P[b + 1];
			m_PlayerInfo[0].SetText("Name:   " + P[b + 1]);
			m_PlayerInfo[1].SetText("Steam64:   " + P[b]);
			m_PlayerInfo[2].SetText(string.Format("Position:   %1 / %2  (alt %3)", Math.Round(pos[0]), Math.Round(pos[2]), Math.Round(pos[1])));
			m_PlayerInfo[3].SetText("Health:   " + P[b + 3] + "%     Blood:   " + P[b + 4]);
			string god = "off";
			if (P[b + 5] == "1")
				god = "ON";
			m_PlayerInfo[4].SetText("Godmode:   " + god);
			m_PlayerInfo[5].SetText("Ping:   " + P[b + 6] + " ms");
			string alive = "alive";
			if (P[b + 7] == "0")
				alive = "dead";
			m_PlayerInfo[6].SetText("State:   " + alive);
			selText = "Selected player: " + m_SelectedName;
		}
		if (m_SelectedId != "" && idx < 0)
			selText = "Selected player: " + m_SelectedName + " (offline)";
		m_ItemSpawnTarget.SetText(selText);
		m_ExpPlayer.SetText(selText);
	}

	protected void RefreshMapMarkers()
	{
		if (!m_Map || !m_C)
			return;
		m_Map.ClearUserMarks();
		int stride = HOD_Client.PLAYER_STRIDE;
		int count = m_C.m_Players.Count() / stride;
		for (int i = 0; i < count; i++)
		{
			vector pos = m_C.m_Players[i * stride + 2].ToVector();
			int col = HOD_COL_GREEN;
			if (m_C.m_Players[i * stride] == m_SelectedId)
				col = HOD_COL_GOLD;
			m_Map.AddUserMark(pos, m_C.m_Players[i * stride + 1], col, HOD_DOT);
		}
		int ts = HOD_Client.TP_STRIDE;
		int tcount = m_C.m_Teleports.Count() / ts;
		for (int j = 0; j < tcount; j++)
		{
			vector tpos = m_C.m_Teleports[j * ts + 1].ToVector();
			m_Map.AddUserMark(tpos, m_C.m_Teleports[j * ts], 0xFFAAAAAA, HOD_DOT);
		}
	}

	protected void RefreshTeleports()
	{
		if (!m_TpList || !m_C)
			return;
		m_TpList.ClearItems();
		int ts = HOD_Client.TP_STRIDE;
		int count = m_C.m_Teleports.Count() / ts;
		for (int i = 0; i < count; i++)
			m_TpList.AddItem(m_C.m_Teleports[i * ts], null, 0);
		RefreshMapMarkers();
	}

	protected void RefreshBans()
	{
		if (!m_BanList || !m_C)
			return;
		m_BanList.ClearItems();
		int bs = HOD_Client.BAN_STRIDE;
		int count = m_C.m_Bans.Count() / bs;
		for (int i = 0; i < count; i++)
		{
			int b = i * bs;
			string line = m_C.m_Bans[b + 1] + "  |  " + m_C.m_Bans[b] + "  |  " + m_C.m_Bans[b + 4] + "  |  " + m_C.m_Bans[b + 2];
			m_BanList.AddItem(line, null, 0);
		}
	}

	protected void RefreshItems()
	{
		if (!m_ItemList || !HOD_ItemIndex.s_Names)
			return;
		string q = m_ItemSearch.GetText();
		q.ToLower();
		q.TrimInPlace();
		m_ItemList.ClearItems();
		m_ItemRows.Clear();
		bool needQuery = m_ItemCat != 2; // vehicles list is short enough to show unfiltered
		if (needQuery && q.Length() < 2)
			return;
		int n = HOD_ItemIndex.s_Names.Count();
		for (int i = 0; i < n; i++)
		{
			if (m_ItemCat >= 0 && HOD_ItemIndex.s_Cat[i] != m_ItemCat)
				continue;
			if (q != "" && HOD_ItemIndex.s_Lower[i].IndexOf(q) < 0)
				continue;
			m_ItemList.AddItem(HOD_ItemIndex.s_Names[i], null, 0);
			m_ItemRows.Insert(HOD_ItemIndex.s_Names[i]);
			if (m_ItemRows.Count() >= 300)
				break;
		}
		m_LastItemRow = -2;
	}

	// ================================================================ actions

	protected string EditText(EditBoxWidget e)
	{
		if (!e)
			return "";
		string s = e.GetText();
		s.TrimInPlace();
		return s;
	}

	protected bool NeedPlayer()
	{
		if (m_SelectedId == "")
		{
			SetStatus("Select a player first");
			return false;
		}
		return true;
	}

	protected bool CursorTarget(out string low, out string high)
	{
		vector pos;
		Object obj;
		HOD_Target.Get(pos, obj, 60);
		if (!obj)
		{
			SetStatus("Nothing under the crosshair (within 60m)");
			return false;
		}
		if (!HOD_Target.NetId(obj, low, high))
		{
			SetStatus(obj.GetType() + " is a static map object - can't be targeted");
			return false;
		}
		return true;
	}

	protected void CursorCmd(string cmd)
	{
		string lo, hi;
		if (CursorTarget(lo, hi))
			m_C.Send(cmd, {lo, hi});
	}

	protected void HandleAction(string a)
	{
		if (!m_C)
			return;

		if (a.IndexOf("tab:") == 0)
		{
			ShowTab(a.Substring(4, a.Length() - 4).ToInt());
			return;
		}
		if (a.IndexOf("cat:") == 0)
		{
			m_ItemCat = a.Substring(4, a.Length() - 4).ToInt();
			for (int c = 0; c < m_CatButtons.Count(); c++)
			{
				int col = HOD_COL_BUTTON;
				if (c - 1 == m_ItemCat)
					col = HOD_COL_TAB_ACTIVE;
				m_BaseColor.Set(m_CatButtons[c], col);
				HOD_UI.SetButtonColor(m_CatButtons[c], col);
			}
			RefreshItems();
			return;
		}
		if (a.IndexOf("time:") == 0)
		{
			m_C.Send("world.time", {a.Substring(5, a.Length() - 5), "0"});
			return;
		}
		if (a.IndexOf("weather:") == 0)
		{
			m_C.Send("world.weather", {a.Substring(8, a.Length() - 8)});
			return;
		}
		if (a.IndexOf("veh.obj.") == 0 || a.IndexOf("veh.exp.") == 0)
		{
			if (NeedVehicle())
			{
				m_C.Send(a.Substring(4, a.Length() - 4), {m_VehLo, m_VehHi});
				GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(RequestVehicles, 600, false);
			}
			return;
		}
		if (a.IndexOf("item:") == 0)
		{
			SpawnItem(a.Substring(5, a.Length() - 5));
			return;
		}

		switch (a)
		{
			case "close":
				Close();
				break;
			case "players.refresh":
			case "map.refresh":
				m_C.Send("players");
				break;
			case "p.heal":
				if (NeedPlayer()) m_C.Send("player.heal", {m_SelectedId});
				break;
			case "p.kill":
				if (NeedPlayer()) m_C.Send("player.kill", {m_SelectedId});
				break;
			case "p.tpto":
				if (NeedPlayer()) m_C.Send("player.tpto", {m_SelectedId});
				break;
			case "p.bring":
				if (NeedPlayer()) m_C.Send("player.bring", {m_SelectedId});
				break;
			case "p.god":
				if (NeedPlayer()) m_C.Send("player.god", {m_SelectedId});
				GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(RequestPlayers, 300, false);
				break;
			case "p.copy":
				if (NeedPlayer())
				{
					GetGame().CopyToClipboard(m_SelectedId);
					SetStatus("Copied " + m_SelectedId);
				}
				break;
			case "p.locate":
				if (NeedPlayer())
				{
					int idx = SelectedPlayerIndex();
					if (idx >= 0)
					{
						ShowTab(TAB_MAP);
						m_Map.SetMapPos(m_C.m_Players[idx * HOD_Client.PLAYER_STRIDE + 2].ToVector());
						m_Map.SetScale(0.1);
					}
				}
				break;
			case "p.items":
				if (NeedPlayer())
					ShowTab(TAB_ITEMS);
				break;
			case "p.msg":
				if (NeedPlayer() && EditText(m_MsgEdit) != "")
				{
					m_C.Send("player.msg", {m_SelectedId, EditText(m_MsgEdit)});
					m_MsgEdit.SetText("");
				}
				break;
			case "p.kick":
				if (NeedPlayer()) m_C.Send("player.kick", {m_SelectedId, EditText(m_ReasonEdit)});
				break;
			case "p.ban":
				if (NeedPlayer()) m_C.Send("player.ban", {m_SelectedId, EditText(m_BanHoursEdit), EditText(m_ReasonEdit)});
				break;
			case "tp.go":
				TeleportPreset();
				break;
			case "tp.del":
				int tr = m_TpList.GetSelectedRow();
				if (tr >= 0 && tr * HOD_Client.TP_STRIDE < m_C.m_Teleports.Count())
					m_C.Send("tp.del", {m_C.m_Teleports[tr * HOD_Client.TP_STRIDE]});
				break;
			case "tp.add":
				if (EditText(m_TpName) != "")
				{
					m_C.Send("tp.add", {EditText(m_TpName)});
					m_TpName.SetText("");
				}
				break;
			case "tp.coords":
				TeleportCoords();
				break;
			case "time.set":
				m_C.Send("world.time", {EditText(m_HourEdit), EditText(m_MinuteEdit)});
				break;
			case "broadcast":
				if (EditText(m_BroadcastEdit) != "")
				{
					m_C.Send("broadcast", {EditText(m_BroadcastEdit)});
					m_BroadcastEdit.SetText("");
				}
				break;
			case "reload":
				m_C.Send("reload");
				break;
			case "self.god":
				m_C.Send("self.god");
				break;
			case "self.heal":
				m_C.Send("self.heal");
				break;
			case "cur.tp":
				HOD_AdminMenu.TeleportToCursor();
				break;
			case "cur.repair":
				CursorCmd("obj.repair");
				break;
			case "cur.refuel":
				CursorCmd("obj.refuel");
				break;
			case "cur.delete":
				CursorCmd("obj.delete");
				break;
			case "exp.unlock":
			case "exp.key":
			case "exp.resetkeys":
			case "exp.readcode":
			case "exp.clearcode":
				CursorCmd(a);
				break;
			case "exp.atm.add":
				if (NeedPlayer()) m_C.Send("exp.atm", {m_SelectedId, EditText(m_AtmAmount)});
				break;
			case "exp.atm.remove":
				if (NeedPlayer()) m_C.Send("exp.atm", {m_SelectedId, "-" + EditText(m_AtmAmount)});
				break;
			case "veh.refresh":
				m_C.Send("vehicles");
				break;
			case "veh.tp":
				if (NeedVehicle()) m_C.Send("veh.tp", {m_VehLo, m_VehHi});
				break;
			case "veh.locate":
				int vi = SelectedVehicleIndex();
				if (vi >= 0)
				{
					ShowTab(TAB_MAP);
					m_Map.SetMapPos(m_C.m_Vehicles[vi * HOD_Client.VEH_STRIDE + 3].ToVector());
					m_Map.SetScale(0.1);
				}
				break;
			case "ban.refresh":
				m_C.Send("bans");
				break;
			case "ban.unban":
				int br = m_BanList.GetSelectedRow();
				if (br >= 0 && br * HOD_Client.BAN_STRIDE < m_C.m_Bans.Count())
					m_C.Send("unban", {m_C.m_Bans[br * HOD_Client.BAN_STRIDE]});
				break;
			case "ban.id":
				if (EditText(m_BanIdEdit) != "")
				{
					m_C.Send("player.ban", {EditText(m_BanIdEdit), EditText(m_BanIdHours), EditText(m_BanIdReason)});
					GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(RequestBans, 500, false);
				}
				break;
		}
	}

	void RequestPlayers()
	{
		if (m_C)
			m_C.Send("players");
	}

	void RequestVehicles()
	{
		if (m_C)
			m_C.Send("vehicles");
	}

	void RequestBans()
	{
		if (m_C)
			m_C.Send("bans");
	}

	protected void SpawnItem(string mode)
	{
		if (m_SelectedItem == "")
		{
			SetStatus("Pick an item from the list first");
			return;
		}
		string target = "";
		string cmdMode = mode;
		if (mode == "player")
		{
			if (!NeedPlayer())
				return;
			target = m_SelectedId;
			cmdMode = "inv";
		}
		string cursor = "0 0 0";
		if (mode == "cursor")
		{
			vector pos;
			Object obj;
			if (!HOD_Target.Get(pos, obj, 1500))
			{
				SetStatus("Crosshair isn't on anything");
				return;
			}
			cursor = string.Format("%1 %2 %3", pos[0], pos[1], pos[2]);
		}
		string count = EditText(m_ItemCount);
		if (count == "")
			count = "1";
		m_C.Send("item.spawn", {m_SelectedItem, count, cmdMode, target, cursor});
	}

	protected void TeleportPreset()
	{
		int row = m_TpList.GetSelectedRow();
		int ts = HOD_Client.TP_STRIDE;
		if (row < 0 || row * ts >= m_C.m_Teleports.Count())
		{
			SetStatus("Select a preset first");
			return;
		}
		m_C.Send("tp.pos", {m_C.m_Teleports[row * ts + 1]});
	}

	protected void TeleportCoords()
	{
		string txt = EditText(m_TpCoords);
		txt.Replace(",", " ");
		array<string> parts = new array<string>();
		txt.Split(" ", parts);
		array<float> nums = new array<float>();
		foreach (string p : parts)
		{
			if (p != "")
				nums.Insert(p.ToFloat());
		}
		if (nums.Count() < 2)
		{
			SetStatus("Enter coordinates as: x z");
			return;
		}
		float x = nums[0];
		float z = nums[1];
		if (nums.Count() >= 3)
			z = nums[2]; // x y z given
		m_C.Send("tp.pos", {string.Format("%1 0 %2", x, z)});
	}

	static void TeleportToCursor()
	{
		HOD_Client c = HOD_Client.Get();
		if (!c)
			return;
		vector pos;
		Object obj;
		if (!HOD_Target.Get(pos, obj, 1500))
		{
			c.SetStatus("Crosshair isn't on anything (max 1500m)");
			return;
		}
		c.Send("tp.pos", {string.Format("%1 %2 %3", pos[0], pos[1] + 0.2, pos[2])});
	}

	// ================================================================ events

	override bool OnClick(Widget w, int x, int y, int button)
	{
		string action;
		if (m_Actions.Find(w, action))
		{
			HandleAction(action);
			return true;
		}
		return super.OnClick(w, x, y, button);
	}

	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		if (w == m_Map && m_C)
		{
			vector pos = m_Map.ScreenToMap(Vector(x, y, 0));
			m_C.Send("tp.pos", {string.Format("%1 0 %2", pos[0], pos[2])});
			return true;
		}
		if (w == m_TpList)
		{
			TeleportPreset();
			return true;
		}
		return super.OnDoubleClick(w, x, y, button);
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		ButtonWidget b = ButtonWidget.Cast(w);
		if (b && m_BaseColor.Contains(w))
			HOD_UI.SetButtonColor(b, HOD_COL_BUTTON_HOT);
		return super.OnMouseEnter(w, x, y);
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		ButtonWidget b = ButtonWidget.Cast(w);
		if (b && m_BaseColor.Contains(w))
			HOD_UI.SetButtonColor(b, m_BaseColor.Get(w));
		return super.OnMouseLeave(w, enterW, x, y);
	}

	override void OnShow()
	{
		super.OnShow();
		Mission mission = GetGame().GetMission();
		if (mission)
			mission.AddActiveInputExcludes({"menu"});
		m_HomeWasDown = true; // ignore the key press that opened us
		m_EscWasDown = true;
	}

	override void OnHide()
	{
		super.OnHide();
		Mission mission = GetGame().GetMission();
		if (mission)
			mission.RemoveActiveInputExcludes({"menu"}, true);
		if (m_C && m_C.m_Menu == this)
			m_C.m_Menu = null;
	}

	override void Update(float timeslice)
	{
		super.Update(timeslice);

		// close keys (raw keys so they work while UI has focus)
		bool typing = EditBoxWidget.Cast(GetFocus()) != null;
		bool home = KeyState(KeyCode.KC_HOME) != 0;
		bool esc = KeyState(KeyCode.KC_ESCAPE) != 0;
		if (home && !m_HomeWasDown && !typing)
		{
			Close();
			return;
		}
		if (esc && !m_EscWasDown)
		{
			Close();
			return;
		}
		m_HomeWasDown = home;
		m_EscWasDown = esc;

		m_Tick += timeslice;
		m_RefreshTick += timeslice;
		if (m_Tick < 0.2)
			return;
		m_Tick = 0;

		// player list selection / filter
		if (m_ActiveTab == TAB_PLAYERS)
		{
			string f = m_PlayerFilter.GetText();
			if (f != m_LastPlayerFilter)
			{
				m_LastPlayerFilter = f;
				RefreshPlayerList();
			}
			int row = m_PlayerList.GetSelectedRow();
			if (row != m_LastPlayerRow)
			{
				m_LastPlayerRow = row;
				if (row >= 0 && row < m_PlayerRows.Count())
				{
					int pi = m_PlayerRows[row];
					m_SelectedId = m_C.m_Players[pi * HOD_Client.PLAYER_STRIDE];
					m_SelectedName = m_C.m_Players[pi * HOD_Client.PLAYER_STRIDE + 1];
					UpdatePlayerInfo();
				}
			}
		}

		if (m_ActiveTab == TAB_ITEMS)
		{
			string q = m_ItemSearch.GetText();
			if (q != m_LastItemSearch)
			{
				m_LastItemSearch = q;
				RefreshItems();
			}
			int irow = m_ItemList.GetSelectedRow();
			if (irow != m_LastItemRow)
			{
				m_LastItemRow = irow;
				if (irow >= 0 && irow < m_ItemRows.Count())
				{
					m_SelectedItem = m_ItemRows[irow];
					m_ItemSelected.SetText(m_SelectedItem);
				}
			}
		}

		if (m_ActiveTab == TAB_VEHICLES)
		{
			string vf = m_VehFilter.GetText();
			if (vf != m_LastVehFilter)
			{
				m_LastVehFilter = vf;
				RefreshVehicles();
			}
			int vrow = m_VehList.GetSelectedRow();
			if (vrow != m_LastVehRow)
			{
				m_LastVehRow = vrow;
				if (vrow >= 0 && vrow < m_VehRows.Count())
				{
					int vb = m_VehRows[vrow] * HOD_Client.VEH_STRIDE;
					m_VehLo = m_C.m_Vehicles[vb];
					m_VehHi = m_C.m_Vehicles[vb + 1];
					UpdateVehicleInfo();
				}
			}
		}

		if (m_ActiveTab == TAB_SELF || m_ActiveTab == TAB_EXPANSION)
		{
			vector pos;
			Object obj;
			HOD_Target.Get(pos, obj, 60);
			string looking = "Looking at: " + HOD_Target.Describe(obj);
			m_SelfLooking.SetText(looking);
			m_ExpLooking.SetText(looking);
			Man me = GetGame().GetPlayer();
			if (me)
			{
				vector mp = me.GetPosition();
				m_SelfPos.SetText(string.Format("My position: %1 / %2  (alt %3)", Math.Round(mp[0]), Math.Round(mp[2]), Math.Round(mp[1])));
			}
		}

		// auto refresh player data
		if (m_RefreshTick > 5.0)
		{
			m_RefreshTick = 0;
			if (m_C && (m_ActiveTab == TAB_PLAYERS || m_ActiveTab == TAB_MAP))
				m_C.Send("players");
		}
	}
}
