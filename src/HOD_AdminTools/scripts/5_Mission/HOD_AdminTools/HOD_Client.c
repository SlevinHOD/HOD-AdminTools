// HOD Admin Tools - client state + RPC receiver

class HOD_Target
{
	// Raycast from the camera. Returns false if nothing was hit.
	static bool Get(out vector pos, out Object obj, float dist = 1500)
	{
		vector from = GetGame().GetCurrentCameraPosition();
		vector to = from + GetGame().GetCurrentCameraDirection() * dist;
		vector normal;
		int component;
		set<Object> hits = new set<Object>();
		Object ignore = GetGame().GetPlayer();
		bool hit = DayZPhysics.RaycastRV(from, to, pos, normal, component, hits, null, ignore, false, false, ObjIntersectView, 0.0);
		obj = null;
		if (hits.Count() > 0)
			obj = hits.Get(0);
		return hit;
	}

	static bool NetId(Object obj, out string low, out string high)
	{
		if (!obj)
			return false;
		int lo, hi;
		obj.GetNetworkID(lo, hi);
		if (lo == 0 && hi == 0)
			return false;
		low = lo.ToString();
		high = hi.ToString();
		return true;
	}

	static string Describe(Object obj)
	{
		if (!obj)
			return "nothing";
		string s = obj.GetType();
		string lo, hi;
		if (!NetId(obj, lo, hi))
			s += " (static map object)";
		return s;
	}
}

class HOD_Client : HOD_RPCHandler
{
	protected static ref HOD_Client s_Instance;

	bool   m_HelloReceived;
	bool   m_IsAdmin;
	string m_Version;
	string m_World;
	string m_ExpModules;
	string m_Group;
	string m_ServerName;
	bool   m_God;
	string m_Status = "";

	ref set<string>   m_Perms     = new set<string>();
	ref array<string> m_Players   = new array<string>(); // stride 8
	ref array<string> m_Teleports = new array<string>(); // stride 2
	ref array<string> m_Bans      = new array<string>(); // stride 5
	ref array<string> m_Vehicles  = new array<string>(); // stride 8

	HOD_AdminMenu m_Menu; // weak

	static const int PLAYER_STRIDE = 8;
	static const int TP_STRIDE = 2;
	static const int BAN_STRIDE = 5;
	static const int VEH_STRIDE = 8;

	static HOD_Client Get()
	{
		return s_Instance;
	}

	static HOD_Client Create()
	{
		if (!s_Instance)
			s_Instance = new HOD_Client();
		return s_Instance;
	}

	void HOD_Client()
	{
		HOD_RPC.s_ClientHandler = this;
	}

	bool Has(string perm)
	{
		return m_Perms.Find(perm) >= 0;
	}

	void Send(string cmd, array<string> args = null)
	{
		HOD_RPC.ToServer(cmd, args);
	}

	void SetStatus(string text)
	{
		m_Status = text;
		if (m_Menu)
			m_Menu.SetStatus(text);
	}

	override void OnHODRPC(PlayerIdentity sender, string cmd, array<string> args)
	{
		switch (cmd)
		{
			case "hello":
				OnHello(args);
				break;
			case "players":
				m_Players = args;
				break;
			case "tplist":
				m_Teleports = args;
				break;
			case "bans":
				m_Bans = args;
				break;
			case "vehicles":
				m_Vehicles = args;
				break;
			case "msg":
				if (args.Count() > 0)
					SetStatus(args[0]);
				break;
			case "notify":
				if (args.Count() > 1)
					NotificationSystem.AddNotificationExtended(6, args[0], args[1], HOD_LOGO);
				break;
			case "chat":
				if (args.Count() > 0)
					ChatLine(args[0]);
				break;
			case "god":
				m_God = args.Count() > 0 && args[0] == "ON";
				break;
		}
		if (m_Menu)
			m_Menu.OnServerData(cmd);
	}

	protected void OnHello(array<string> args)
	{
		m_HelloReceived = true;
		m_Perms.Clear();
		m_IsAdmin = args.Count() > 0 && args[0] == "1";
		if (!m_IsAdmin || args.Count() < 6)
			return;
		m_Version = args[1];
		m_World = args[2];
		m_ExpModules = args[3];
		m_Group = args[4];
		m_ServerName = args[5];
		for (int i = 6; i < args.Count(); i++)
			m_Perms.Insert(args[i]);
		Print("[HOD] Admin access granted (" + m_Group + "), " + m_Perms.Count() + " permissions");
	}

	static void ChatLine(string text)
	{
		Mission mission = GetGame().GetMission();
		if (mission)
			mission.OnEvent(ChatMessageEventTypeID, new ChatMessageEventParams(CCSystem, "", text, ""));
	}
}
