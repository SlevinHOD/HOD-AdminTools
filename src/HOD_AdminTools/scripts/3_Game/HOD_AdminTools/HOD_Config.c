// HOD Admin Tools - JSON config models (server side, $profile:HOD_AdminTools)

class HOD_PermissionGroup
{
	string Name;
	ref array<string> Permissions = new array<string>();

	void HOD_PermissionGroup(string name = "")
	{
		Name = name;
	}
}

class HOD_AdminEntry
{
	string Name;
	string SteamID;
	string Group;

	void HOD_AdminEntry(string name = "", string steamId = "", string group = "")
	{
		Name = name;
		SteamID = steamId;
		Group = group;
	}
}

class HOD_AdminsConfig
{
	string _Readme = "Add admins by Steam64 ID. Group must match a group Name. Permission '*' = everything, 'players.*' = all players.* permissions.";
	ref array<ref HOD_PermissionGroup> Groups = new array<ref HOD_PermissionGroup>();
	ref array<ref HOD_AdminEntry> Admins = new array<ref HOD_AdminEntry>();

	void Defaults()
	{
		HOD_PermissionGroup owner = new HOD_PermissionGroup("Owner");
		owner.Permissions.Insert("*");
		Groups.Insert(owner);

		HOD_PermissionGroup admin = new HOD_PermissionGroup("Admin");
		admin.Permissions.Insert("players.*");
		admin.Permissions.Insert("items.*");
		admin.Permissions.Insert("teleport.*");
		admin.Permissions.Insert("world.*");
		admin.Permissions.Insert("self.*");
		admin.Permissions.Insert("vehicles.*");
		admin.Permissions.Insert("server.broadcast");
		admin.Permissions.Insert("expansion.vehicles");
		admin.Permissions.Insert("expansion.codelocks");
		Groups.Insert(admin);

		HOD_PermissionGroup mod = new HOD_PermissionGroup("Moderator");
		mod.Permissions.Insert(HOD_Perm.PLAYERS_VIEW);
		mod.Permissions.Insert(HOD_Perm.PLAYERS_TELEPORT);
		mod.Permissions.Insert(HOD_Perm.PLAYERS_KICK);
		mod.Permissions.Insert(HOD_Perm.PLAYERS_MESSAGE);
		mod.Permissions.Insert(HOD_Perm.TELEPORT_SELF);
		mod.Permissions.Insert(HOD_Perm.SELF_HEAL);
		Groups.Insert(mod);

		Admins.Insert(new HOD_AdminEntry("Example - replace me", "76561190000000000", "Owner"));
	}

	HOD_AdminEntry FindAdmin(string steamId)
	{
		foreach (HOD_AdminEntry a : Admins)
		{
			if (a && a.SteamID == steamId)
				return a;
		}
		return null;
	}

	HOD_PermissionGroup FindGroup(string name)
	{
		foreach (HOD_PermissionGroup g : Groups)
		{
			if (g && g.Name == name)
				return g;
		}
		return null;
	}

	bool IsAdmin(string steamId)
	{
		return FindAdmin(steamId) != null;
	}

	bool HasPermission(string steamId, string perm)
	{
		HOD_AdminEntry a = FindAdmin(steamId);
		if (!a)
			return false;
		HOD_PermissionGroup g = FindGroup(a.Group);
		if (!g)
			return false;
		foreach (string p : g.Permissions)
		{
			if (p == "*" || p == perm)
				return true;
			int len = p.Length();
			if (len > 2 && p.Substring(len - 2, 2) == ".*")
			{
				string prefix = p.Substring(0, len - 1); // keep the dot
				if (perm.IndexOf(prefix) == 0)
					return true;
			}
		}
		return false;
	}

	array<string> GetPermissions(string steamId)
	{
		array<string> result = new array<string>();
		foreach (string perm : HOD_Perm.All())
		{
			if (HasPermission(steamId, perm))
				result.Insert(perm);
		}
		return result;
	}
}

class HOD_Settings
{
	string ServerName = "HOD DayZ";
	bool LogToFile = true;
	bool LogToDiscord = false;
	string DiscordWebhookURL = "";
	int GodmodeReapplyMs = 2000;
	bool UseExpansionNotifications = true;
	int MaxSpawnCount = 50;
	bool AnnounceKicksAndBans = false;
}

class HOD_BanEntry
{
	string SteamID;
	string Name;
	string Reason;
	string BannedBy;
	string Date;
	int ExpiresAt; // HOD_Time seconds, 0 = permanent
}

class HOD_BanList
{
	ref array<ref HOD_BanEntry> Bans = new array<ref HOD_BanEntry>();

	HOD_BanEntry Find(string steamId)
	{
		foreach (HOD_BanEntry b : Bans)
		{
			if (b && b.SteamID == steamId)
				return b;
		}
		return null;
	}

	// Removes expired bans, returns true if anything changed
	bool Prune()
	{
		bool changed = false;
		int now = HOD_Time.Now();
		for (int i = Bans.Count() - 1; i >= 0; i--)
		{
			HOD_BanEntry b = Bans[i];
			if (!b || (b.ExpiresAt > 0 && b.ExpiresAt <= now))
			{
				Bans.Remove(i);
				changed = true;
			}
		}
		return changed;
	}

	bool Remove(string steamId)
	{
		for (int i = Bans.Count() - 1; i >= 0; i--)
		{
			if (Bans[i] && Bans[i].SteamID == steamId)
			{
				Bans.Remove(i);
				return true;
			}
		}
		return false;
	}
}

class HOD_TeleportPoint
{
	string Name;
	vector Position;

	void HOD_TeleportPoint(string name = "", vector pos = "0 0 0")
	{
		Name = name;
		Position = pos;
	}
}

class HOD_TeleportList
{
	ref array<ref HOD_TeleportPoint> Points = new array<ref HOD_TeleportPoint>();

	void Add(string name, float x, float z)
	{
		Points.Insert(new HOD_TeleportPoint(name, Vector(x, 0, z)));
	}

	// y = 0 means "snap to ground" on teleport
	void Defaults(string world)
	{
		world.ToLower();
		if (world == "chernarusplus")
		{
			Add("Chernogorsk", 6600, 2500);
			Add("Elektrozavodsk", 10450, 2300);
			Add("Kamyshovo", 12000, 3500);
			Add("Solnichniy", 13400, 6200);
			Add("Berezino", 12000, 9100);
			Add("Novodmitrovsk", 11500, 14500);
			Add("Severograd", 8400, 12700);
			Add("NWAF", 4600, 10200);
			Add("Tisy", 1700, 14000);
			Add("Vybor", 3800, 8900);
			Add("Stary Sobor", 6100, 7700);
			Add("Gorka", 9500, 8800);
			Add("Zelenogorsk", 2700, 5200);
			Add("Green Mountain", 3700, 6000);
		}
	}
}

class HOD_Time
{
	// Seconds since 1970-01-01 (local server clock). Good until 2038.
	static int Now()
	{
		int y, mo, d, h, mi, s;
		GetYearMonthDay(y, mo, d);
		GetHourMinuteSecond(h, mi, s);
		return DaysFromCivil(y, mo, d) * 86400 + h * 3600 + mi * 60 + s;
	}

	static int DaysFromCivil(int y, int m, int d)
	{
		if (m <= 2)
			y -= 1;
		int era;
		if (y >= 0)
			era = y / 400;
		else
			era = (y - 399) / 400;
		int yoe = y - era * 400;
		int mp;
		if (m > 2)
			mp = m - 3;
		else
			mp = m + 9;
		int doy = (153 * mp + 2) / 5 + d - 1;
		int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
		return era * 146097 + doe - 719468;
	}

	static string Stamp()
	{
		int y, mo, d, h, mi, s;
		GetYearMonthDay(y, mo, d);
		GetHourMinuteSecond(h, mi, s);
		return string.Format("%1-%2-%3 %4:%5:%6", y, Pad(mo), Pad(d), Pad(h), Pad(mi), Pad(s));
	}

	static string DateTag()
	{
		int y, mo, d;
		GetYearMonthDay(y, mo, d);
		return string.Format("%1-%2-%3", y, Pad(mo), Pad(d));
	}

	static string Pad(int v)
	{
		if (v < 10)
			return "0" + v.ToString();
		return v.ToString();
	}
}
