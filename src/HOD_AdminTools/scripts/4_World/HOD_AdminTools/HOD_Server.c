// HOD Admin Tools - server side. Every request is permission-checked here;
// the client UI is only a convenience and is never trusted.

class HOD_Server : HOD_RPCHandler
{
	protected static ref HOD_Server s_Instance;

	ref HOD_AdminsConfig m_Admins;
	ref HOD_Settings     m_Settings;
	ref HOD_BanList      m_Bans;
	ref HOD_TeleportList m_Teleports;
	string               m_World;
	ref set<string>      m_GodPlayers;
	int                  m_StartTime;

	static HOD_Server Get()
	{
		return s_Instance;
	}

	static HOD_Server Create()
	{
		if (!s_Instance)
			s_Instance = new HOD_Server();
		return s_Instance;
	}

	void HOD_Server()
	{
		m_GodPlayers = new set<string>();
		m_StartTime = HOD_Time.Now();
		GetGame().GetWorldName(m_World);
		m_World.ToLower();
		LoadAll();
		HOD_RPC.s_ServerHandler = this;
		int reapply = m_Settings.GodmodeReapplyMs;
		if (reapply < 500)
			reapply = 500;
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(GodTick, reapply, true);
		HOD_Logger.Log("HOD Admin Tools " + HOD_VERSION + " started on " + m_World + " | Expansion: " + HOD_Expansion.Modules());
	}

	// ------------------------------------------------------------------ config

	void LoadAll()
	{
		if (!FileExist(HOD_PROFILE_DIR))
			MakeDirectory(HOD_PROFILE_DIR);
		if (!FileExist(HOD_TP_DIR))
			MakeDirectory(HOD_TP_DIR);

		m_Settings = new HOD_Settings();
		if (FileExist(HOD_SETTINGS_FILE))
			JsonFileLoader<HOD_Settings>.JsonLoadFile(HOD_SETTINGS_FILE, m_Settings);
		JsonFileLoader<HOD_Settings>.JsonSaveFile(HOD_SETTINGS_FILE, m_Settings); // writes new fields
		HOD_Logger.Configure(m_Settings);

		m_Admins = new HOD_AdminsConfig();
		if (FileExist(HOD_ADMINS_FILE))
		{
			JsonFileLoader<HOD_AdminsConfig>.JsonLoadFile(HOD_ADMINS_FILE, m_Admins);
		}
		else
		{
			m_Admins.Defaults();
			JsonFileLoader<HOD_AdminsConfig>.JsonSaveFile(HOD_ADMINS_FILE, m_Admins);
		}

		m_Bans = new HOD_BanList();
		if (FileExist(HOD_BANS_FILE))
			JsonFileLoader<HOD_BanList>.JsonLoadFile(HOD_BANS_FILE, m_Bans);
		if (m_Bans.Prune() || !FileExist(HOD_BANS_FILE))
			SaveBans();

		m_Teleports = new HOD_TeleportList();
		string tpFile = TeleportFile();
		if (FileExist(tpFile))
		{
			JsonFileLoader<HOD_TeleportList>.JsonLoadFile(tpFile, m_Teleports);
		}
		else
		{
			m_Teleports.Defaults(m_World);
			SaveTeleports();
		}
	}

	string TeleportFile()
	{
		return HOD_TP_DIR + "/" + m_World + ".json";
	}

	void SaveBans()
	{
		JsonFileLoader<HOD_BanList>.JsonSaveFile(HOD_BANS_FILE, m_Bans);
	}

	void SaveTeleports()
	{
		JsonFileLoader<HOD_TeleportList>.JsonSaveFile(TeleportFile(), m_Teleports);
	}

	// ------------------------------------------------------------------ helpers

	static PlayerBase FindPlayer(string steamId)
	{
		array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);
		foreach (Man m : players)
		{
			PlayerBase p = PlayerBase.Cast(m);
			if (p && p.GetIdentity() && p.GetIdentity().GetPlainId() == steamId)
				return p;
		}
		return null;
	}

	static Object FindObject(string low, string high)
	{
		return GetGame().GetObjectByNetworkId(low.ToInt(), high.ToInt());
	}

	static string PlayerLabel(PlayerBase p)
	{
		if (!p || !p.GetIdentity())
			return "?";
		return p.GetIdentity().GetName() + " (" + p.GetIdentity().GetPlainId() + ")";
	}

	static int ClampInt(int v, int lo, int hi)
	{
		if (v < lo)
			return lo;
		if (v > hi)
			return hi;
		return v;
	}

	static string VecStr(vector v)
	{
		return string.Format("%1 %2 %3", v[0], v[1], v[2]);
	}

	static vector SafeGround(vector pos)
	{
		float y = GetGame().SurfaceY(pos[0], pos[2]);
		float road = GetGame().SurfaceRoadY(pos[0], pos[2]);
		if (road > y)
			y = road;
		if (pos[1] < y)
			pos[1] = y;
		return pos;
	}

	void Reply(PlayerIdentity id, string text)
	{
		HOD_RPC.ToClient(id, "msg", {text});
	}

	bool Check(PlayerIdentity id, string perm)
	{
		if (!id)
			return false;
		if (m_Admins.HasPermission(id.GetPlainId(), perm))
			return true;
		Reply(id, "Missing permission: " + perm);
		HOD_Logger.Log("DENIED " + id.GetName() + " (" + id.GetPlainId() + ") -> " + perm);
		return false;
	}

	void Act(PlayerIdentity id, string action, string detail)
	{
		HOD_Logger.Action(id.GetName(), id.GetPlainId(), action, detail);
	}

	PlayerBase SenderPlayer(PlayerIdentity id)
	{
		return FindPlayer(id.GetPlainId());
	}

	void Notify(PlayerIdentity id, string title, string text)
	{
		if (m_Settings.UseExpansionNotifications && HOD_Expansion.Notify(id, title, text))
			return;
		HOD_RPC.ToClient(id, "notify", {title, text});
	}

	// ------------------------------------------------------------------ godmode

	void SetGod(PlayerBase p, bool on)
	{
		if (!p || !p.GetIdentity())
			return;
		string id = p.GetIdentity().GetPlainId();
		if (on)
			m_GodPlayers.Insert(id);
		else
		{
			int idx = m_GodPlayers.Find(id);
			if (idx >= 0)
				m_GodPlayers.Remove(idx);
		}
		p.SetAllowDamage(!on);
	}

	bool IsGod(PlayerBase p)
	{
		if (!p || !p.GetIdentity())
			return false;
		return m_GodPlayers.Find(p.GetIdentity().GetPlainId()) >= 0;
	}

	// Re-apply so other mods (e.g. Expansion safezone exit) can't silently strip godmode
	void GodTick()
	{
		if (m_GodPlayers.Count() == 0)
			return;
		array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);
		foreach (Man m : players)
		{
			PlayerBase p = PlayerBase.Cast(m);
			if (p && IsGod(p) && p.GetAllowDamage())
				p.SetAllowDamage(false);
		}
	}

	static void Heal(PlayerBase p)
	{
		p.SetHealth("", "", p.GetMaxHealth("", ""));
		p.SetHealth("GlobalHealth", "Blood", p.GetMaxHealth("GlobalHealth", "Blood"));
		p.SetHealth("GlobalHealth", "Shock", p.GetMaxHealth("GlobalHealth", "Shock"));
		p.SetHealth("RightLeg", "", p.GetMaxHealth("RightLeg", ""));
		p.SetHealth("LeftLeg", "", p.GetMaxHealth("LeftLeg", ""));
		p.SetHealth("RightFoot", "", p.GetMaxHealth("RightFoot", ""));
		p.SetHealth("LeftFoot", "", p.GetMaxHealth("LeftFoot", ""));
		if (p.GetBrokenLegs() != eBrokenLegs.NO_BROKEN_LEGS)
			p.SetBrokenLegs(eBrokenLegs.NO_BROKEN_LEGS);
		if (p.GetStatWater())
			p.GetStatWater().Set(p.GetStatWater().GetMax());
		if (p.GetStatEnergy())
			p.GetStatEnergy().Set(p.GetStatEnergy().GetMax());
		if (p.GetBleedingManagerServer())
			p.GetBleedingManagerServer().RemoveAllSources();
		p.RemoveAllAgents();
	}

	static void RepairEntity(EntityAI e)
	{
		if (!e)
			return;
		e.SetHealthMax("", "");
		array<EntityAI> children = new array<EntityAI>();
		e.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, children);
		foreach (EntityAI c : children)
		{
			if (c && c != e)
				c.SetHealthMax("", "");
		}
	}

	// ------------------------------------------------------------------ connect / bans

	// returns true if the player was rejected
	bool CheckBan(PlayerIdentity identity)
	{
		if (!identity)
			return false;
		HOD_BanEntry ban = m_Bans.Find(identity.GetPlainId());
		if (!ban)
			return false;
		if (ban.ExpiresAt > 0 && ban.ExpiresAt <= HOD_Time.Now())
		{
			m_Bans.Remove(ban.SteamID);
			SaveBans();
			return false;
		}
		HOD_Logger.Log("Rejected banned player " + identity.GetName() + " (" + identity.GetPlainId() + "): " + ban.Reason);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(KickIdentity, 1500, false, identity);
		return true;
	}

	void KickIdentity(PlayerIdentity identity)
	{
		if (identity)
			GetGame().DisconnectPlayer(identity, identity.GetId());
	}

	// ------------------------------------------------------------------ RPC

	override void OnHODRPC(PlayerIdentity sender, string cmd, array<string> args)
	{
		if (!sender)
			return;

		string steamId = sender.GetPlainId();

		if (cmd == "hello")
		{
			SendHello(sender);
			return;
		}

		// everything below needs admin
		if (!m_Admins.IsAdmin(steamId))
			return;

		switch (cmd)
		{
			case "players":      CmdPlayers(sender); break;
			case "player.heal":  CmdPlayerHeal(sender, args); break;
			case "player.kill":  CmdPlayerKill(sender, args); break;
			case "player.tpto":  CmdPlayerTpTo(sender, args); break;
			case "player.bring": CmdPlayerBring(sender, args); break;
			case "player.kick":  CmdPlayerKick(sender, args); break;
			case "player.ban":   CmdPlayerBan(sender, args); break;
			case "player.god":   CmdPlayerGod(sender, args); break;
			case "player.msg":   CmdPlayerMsg(sender, args); break;
			case "bans":         CmdBans(sender); break;
			case "unban":        CmdUnban(sender, args); break;
			case "self.god":     CmdSelfGod(sender, args); break;
			case "self.heal":    CmdSelfHeal(sender); break;
			case "tp.pos":       CmdTpPos(sender, args); break;
			case "tp.list":      CmdTpList(sender); break;
			case "tp.add":       CmdTpAdd(sender, args); break;
			case "tp.del":       CmdTpDel(sender, args); break;
			case "item.spawn":   CmdItemSpawn(sender, args); break;
			case "obj.delete":   CmdObjDelete(sender, args); break;
			case "obj.repair":   CmdObjRepair(sender, args); break;
			case "obj.refuel":   CmdObjRefuel(sender, args); break;
			case "world.time":   CmdWorldTime(sender, args); break;
			case "world.weather": CmdWorldWeather(sender, args); break;
			case "broadcast":    CmdBroadcast(sender, args); break;
			case "exp.unlock":   CmdExp(sender, args, 0); break;
			case "exp.resetkeys": CmdExp(sender, args, 1); break;
			case "exp.key":      CmdExp(sender, args, 2); break;
			case "exp.clearcode": CmdExp(sender, args, 3); break;
			case "exp.readcode": CmdExp(sender, args, 4); break;
			case "exp.atm":      CmdExpAtm(sender, args); break;
			case "reload":       CmdReload(sender); break;
			case "vehicles":     CmdVehicles(sender); break;
			case "veh.tp":       CmdVehicleTp(sender, args); break;
		}
	}

	void SendHello(PlayerIdentity id)
	{
		string steamId = id.GetPlainId();
		array<string> res = new array<string>();
		if (!m_Admins.IsAdmin(steamId))
		{
			res.Insert("0");
			HOD_RPC.ToClient(id, "hello", res);
			return;
		}
		HOD_AdminEntry entry = m_Admins.FindAdmin(steamId);
		res.Insert("1");
		res.Insert(HOD_VERSION);
		res.Insert(m_World);
		res.Insert(HOD_Expansion.Modules());
		res.Insert(entry.Group);
		res.Insert(m_Settings.ServerName);
		foreach (string perm : m_Admins.GetPermissions(steamId))
			res.Insert(perm);
		HOD_RPC.ToClient(id, "hello", res);
	}

	void CmdReload(PlayerIdentity id)
	{
		if (!m_Admins.HasPermission(id.GetPlainId(), "*"))
		{
			Reply(id, "Reload needs the '*' permission");
			return;
		}
		LoadAll();
		Act(id, "Reload", "Configs reloaded");
		Reply(id, "Configs reloaded");
		SendHello(id);
	}

	// stride 8: steamid, name, pos, health%, blood, god, ping, alive
	void CmdPlayers(PlayerIdentity id)
	{
		if (!Check(id, HOD_Perm.PLAYERS_VIEW))
			return;
		array<string> res = new array<string>();
		array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);
		foreach (Man m : players)
		{
			PlayerBase p = PlayerBase.Cast(m);
			if (!p || !p.GetIdentity())
				continue;
			res.Insert(p.GetIdentity().GetPlainId());
			res.Insert(p.GetIdentity().GetName());
			res.Insert(VecStr(p.GetPosition()));
			res.Insert(Math.Round(p.GetHealth01("", "") * 100).ToString());
			res.Insert(Math.Round(p.GetHealth("GlobalHealth", "Blood")).ToString());
			if (IsGod(p))
				res.Insert("1");
			else
				res.Insert("0");
			res.Insert(p.GetIdentity().GetPingAvg().ToString());
			if (p.IsAlive())
				res.Insert("1");
			else
				res.Insert("0");
		}
		HOD_RPC.ToClient(id, "players", res);
	}

	PlayerBase TargetFromArgs(PlayerIdentity id, array<string> args)
	{
		if (args.Count() < 1)
			return null;
		PlayerBase t = FindPlayer(args[0]);
		if (!t)
			Reply(id, "Player not found (disconnected?)");
		return t;
	}

	void CmdPlayerHeal(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.PLAYERS_HEAL))
			return;
		PlayerBase t = TargetFromArgs(id, args);
		if (!t)
			return;
		Heal(t);
		Act(id, "Heal", PlayerLabel(t));
		Reply(id, "Healed " + t.GetIdentity().GetName());
	}

	void CmdPlayerKill(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.PLAYERS_KILL))
			return;
		PlayerBase t = TargetFromArgs(id, args);
		if (!t)
			return;
		SetGod(t, false);
		t.SetHealth("", "", 0);
		Act(id, "Kill", PlayerLabel(t));
		Reply(id, "Killed " + t.GetIdentity().GetName());
	}

	void CmdPlayerTpTo(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.PLAYERS_TELEPORT))
			return;
		PlayerBase t = TargetFromArgs(id, args);
		PlayerBase self = SenderPlayer(id);
		if (!t || !self)
			return;
		if (!TeleportPlayer(id, self, t.GetPosition() + "1 0 1"))
			return;
		Act(id, "Teleport to player", PlayerLabel(t));
	}

	void CmdPlayerBring(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.PLAYERS_TELEPORT))
			return;
		PlayerBase t = TargetFromArgs(id, args);
		PlayerBase self = SenderPlayer(id);
		if (!t || !self)
			return;
		if (!TeleportPlayer(id, t, self.GetPosition() + "1 0 1"))
			return;
		Act(id, "Bring player", PlayerLabel(t));
	}

	bool TeleportPlayer(PlayerIdentity admin, PlayerBase p, vector pos)
	{
		if (p.IsInTransport())
		{
			Reply(admin, "Player is in a vehicle - get them out first");
			return false;
		}
		pos = SafeGround(pos);
		p.SetPosition(pos);
		Reply(admin, "Teleported " + p.GetIdentity().GetName());
		return true;
	}

	void CmdPlayerKick(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.PLAYERS_KICK))
			return;
		PlayerBase t = TargetFromArgs(id, args);
		if (!t)
			return;
		string reason = "Kicked by admin";
		if (args.Count() > 1 && args[1] != "")
			reason = args[1];
		string label = PlayerLabel(t);
		PlayerIdentity tid = t.GetIdentity();
		Notify(tid, "Kicked", reason);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(KickIdentity, 2000, false, tid);
		Act(id, "Kick", label + " - " + reason);
		Reply(id, "Kicking " + tid.GetName());
		if (m_Settings.AnnounceKicksAndBans)
			BroadcastAll(tid.GetName() + " was kicked: " + reason);
	}

	// args: steamid, hours (0 = permanent), reason
	void CmdPlayerBan(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.PLAYERS_BAN))
			return;
		if (args.Count() < 1 || args[0] == "")
			return;
		string target = args[0];
		int hours = 0;
		if (args.Count() > 1)
			hours = args[1].ToInt();
		string reason = "Banned by admin";
		if (args.Count() > 2 && args[2] != "")
			reason = args[2];

		if (m_Admins.IsAdmin(target) && !m_Admins.HasPermission(id.GetPlainId(), "*"))
		{
			Reply(id, "You can't ban another admin");
			return;
		}

		PlayerBase t = FindPlayer(target);
		HOD_BanEntry ban = m_Bans.Find(target);
		if (!ban)
		{
			ban = new HOD_BanEntry();
			m_Bans.Bans.Insert(ban);
		}
		ban.SteamID = target;
		ban.Name = target;
		if (t)
			ban.Name = t.GetIdentity().GetName();
		ban.Reason = reason;
		ban.BannedBy = id.GetName() + " (" + id.GetPlainId() + ")";
		ban.Date = HOD_Time.Stamp();
		ban.ExpiresAt = 0;
		if (hours > 0)
			ban.ExpiresAt = HOD_Time.Now() + hours * 3600;
		SaveBans();

		string len = "permanent";
		if (hours > 0)
			len = hours.ToString() + "h";
		if (t)
		{
			Notify(t.GetIdentity(), "Banned (" + len + ")", reason);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(KickIdentity, 2000, false, t.GetIdentity());
		}
		Act(id, "Ban", ban.Name + " (" + target + ") " + len + " - " + reason);
		Reply(id, "Banned " + ban.Name + " (" + len + ")");
		if (m_Settings.AnnounceKicksAndBans)
			BroadcastAll(ban.Name + " was banned: " + reason);
	}

	// stride 5: steamid, name, reason, by, expires-text
	void CmdBans(PlayerIdentity id)
	{
		if (!Check(id, HOD_Perm.PLAYERS_BAN))
			return;
		if (m_Bans.Prune())
			SaveBans();
		array<string> res = new array<string>();
		int now = HOD_Time.Now();
		foreach (HOD_BanEntry b : m_Bans.Bans)
		{
			res.Insert(b.SteamID);
			res.Insert(b.Name);
			res.Insert(b.Reason);
			res.Insert(b.BannedBy);
			if (b.ExpiresAt <= 0)
				res.Insert("permanent");
			else
				res.Insert(Math.Ceil((b.ExpiresAt - now) / 3600.0).ToString() + "h left");
		}
		HOD_RPC.ToClient(id, "bans", res);
	}

	void CmdUnban(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.PLAYERS_BAN))
			return;
		if (args.Count() < 1)
			return;
		if (m_Bans.Remove(args[0]))
		{
			SaveBans();
			Act(id, "Unban", args[0]);
			Reply(id, "Unbanned " + args[0]);
		}
		else
			Reply(id, "Not in ban list: " + args[0]);
		CmdBans(id);
	}

	void CmdPlayerGod(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.PLAYERS_GODMODE))
			return;
		PlayerBase t = TargetFromArgs(id, args);
		if (!t)
			return;
		bool on = !IsGod(t);
		SetGod(t, on);
		string state = "OFF";
		if (on)
			state = "ON";
		Act(id, "Godmode " + state, PlayerLabel(t));
		Reply(id, "Godmode " + state + " for " + t.GetIdentity().GetName());
	}

	void CmdPlayerMsg(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.PLAYERS_MESSAGE))
			return;
		if (args.Count() < 2 || args[1] == "")
			return;
		PlayerBase t = TargetFromArgs(id, args);
		if (!t)
			return;
		Notify(t.GetIdentity(), "Admin message", args[1]);
		HOD_RPC.ToClient(t.GetIdentity(), "chat", {"[Admin] " + args[1]});
		Act(id, "Message", PlayerLabel(t) + ": " + args[1]);
		Reply(id, "Message sent");
	}

	void CmdSelfGod(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.SELF_GODMODE))
			return;
		PlayerBase self = SenderPlayer(id);
		if (!self)
			return;
		bool on = !IsGod(self);
		SetGod(self, on);
		string state = "OFF";
		if (on)
			state = "ON";
		Act(id, "Self godmode " + state, "");
		Reply(id, "Godmode " + state);
		HOD_RPC.ToClient(id, "god", {state});
	}

	void CmdSelfHeal(PlayerIdentity id)
	{
		if (!Check(id, HOD_Perm.SELF_HEAL))
			return;
		PlayerBase self = SenderPlayer(id);
		if (!self)
			return;
		Heal(self);
		Act(id, "Self heal", "");
		Reply(id, "Healed");
	}

	// args: "x y z"  (y <= 0 -> snap to ground)
	void CmdTpPos(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.TELEPORT_SELF))
			return;
		if (args.Count() < 1)
			return;
		PlayerBase self = SenderPlayer(id);
		if (!self)
			return;
		vector pos = args[0].ToVector();
		if (pos[0] == 0 && pos[2] == 0)
			return;
		if (TeleportPlayer(id, self, pos))
			Act(id, "Teleport", VecStr(pos));
	}

	// stride 2: name, "x y z"
	void CmdTpList(PlayerIdentity id)
	{
		array<string> res = new array<string>();
		foreach (HOD_TeleportPoint tp : m_Teleports.Points)
		{
			res.Insert(tp.Name);
			res.Insert(VecStr(tp.Position));
		}
		HOD_RPC.ToClient(id, "tplist", res);
	}

	void CmdTpAdd(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.TELEPORT_EDIT))
			return;
		PlayerBase self = SenderPlayer(id);
		if (!self || args.Count() < 1 || args[0] == "")
			return;
		m_Teleports.Points.Insert(new HOD_TeleportPoint(args[0], self.GetPosition()));
		SaveTeleports();
		Act(id, "Teleport preset added", args[0] + " @ " + VecStr(self.GetPosition()));
		Reply(id, "Saved preset " + args[0]);
		CmdTpList(id);
	}

	void CmdTpDel(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.TELEPORT_EDIT))
			return;
		if (args.Count() < 1)
			return;
		for (int i = m_Teleports.Points.Count() - 1; i >= 0; i--)
		{
			if (m_Teleports.Points[i].Name == args[0])
			{
				m_Teleports.Points.Remove(i);
				SaveTeleports();
				Act(id, "Teleport preset removed", args[0]);
				Reply(id, "Removed preset " + args[0]);
				break;
			}
		}
		CmdTpList(id);
	}

	// args: classname, count, mode(inv|ground|cursor), targetSteamId, "x y z" cursor pos
	void CmdItemSpawn(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.ITEMS_SPAWN))
			return;
		if (args.Count() < 5)
			return;
		string cls = args[0];
		int count = ClampInt(args[1].ToInt(), 1, m_Settings.MaxSpawnCount);
		string mode = args[2];
		string targetId = args[3];
		vector cursor = args[4].ToVector();

		if (!GetGame().ConfigIsExisting("CfgVehicles " + cls) && !GetGame().ConfigIsExisting("CfgWeapons " + cls) && !GetGame().ConfigIsExisting("CfgMagazines " + cls))
		{
			Reply(id, "Unknown class: " + cls);
			return;
		}

		PlayerBase target = SenderPlayer(id);
		if (targetId != "" && targetId != id.GetPlainId())
		{
			if (!Check(id, HOD_Perm.ITEMS_SPAWN_OTHERS))
				return;
			target = FindPlayer(targetId);
		}
		if (!target)
		{
			Reply(id, "Target player not found");
			return;
		}

		bool isVehicle = GetGame().IsKindOf(cls, "CarScript") || GetGame().IsKindOf(cls, "Transport");
		int spawned = 0;
		int onGround = 0;
		for (int i = 0; i < count; i++)
		{
			EntityAI e = null;
			if (mode == "inv" && !isVehicle)
				e = target.GetInventory().CreateInInventory(cls);

			if (!e)
			{
				vector pos;
				if (mode == "cursor" && (cursor[0] != 0 || cursor[2] != 0))
					pos = cursor;
				else
					pos = target.GetPosition() + target.GetDirection() * 2.0;
				if (isVehicle && mode != "cursor")
					pos = target.GetPosition() + target.GetDirection() * 6.0;
				pos = SafeGround(pos);
				int flags = ECE_PLACE_ON_SURFACE;
				if (isVehicle)
					flags = ECE_PLACE_ON_SURFACE | ECE_CREATEPHYSICS | ECE_UPDATEPATHGRAPH;
				e = EntityAI.Cast(GetGame().CreateObjectEx(cls, pos, flags));
				if (e)
					onGround++;
			}

			if (!e)
				continue;
			spawned++;

			if (isVehicle)
				PrepareVehicle(e);
		}

		string where = "inventory";
		if (onGround > 0)
			where = "ground";
		Act(id, "Spawn", string.Format("%1 x%2 -> %3 [%4]", cls, spawned, PlayerLabel(target), where));
		Reply(id, string.Format("Spawned %1 x%2 (%3 on ground)", cls, spawned, onGround));
	}

	static void PrepareVehicle(EntityAI e)
	{
		e.OnDebugSpawn();
		CarScript car = CarScript.Cast(e);
		if (car)
		{
			car.Fill(CarFluid.FUEL, car.GetFluidCapacity(CarFluid.FUEL));
			car.Fill(CarFluid.OIL, car.GetFluidCapacity(CarFluid.OIL));
			car.Fill(CarFluid.BRAKE, car.GetFluidCapacity(CarFluid.BRAKE));
			car.Fill(CarFluid.COOLANT, car.GetFluidCapacity(CarFluid.COOLANT));
		}
	}

	// args: netLow, netHigh
	Object ObjFromArgs(PlayerIdentity id, array<string> args)
	{
		if (args.Count() < 2)
			return null;
		Object o = FindObject(args[0], args[1]);
		if (!o)
			Reply(id, "Target object not found");
		return o;
	}

	void CmdObjDelete(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.WORLD_DELETE))
			return;
		Object o = ObjFromArgs(id, args);
		if (!o)
			return;
		if (o.IsMan())
		{
			Reply(id, "Won't delete players/AI");
			return;
		}
		string label = o.GetType() + " @ " + VecStr(o.GetPosition());
		GetGame().ObjectDelete(o);
		Act(id, "Delete object", label);
		Reply(id, "Deleted " + label);
	}

	void CmdObjRepair(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.WORLD_REPAIR))
			return;
		Object o = ObjFromArgs(id, args);
		EntityAI e = EntityAI.Cast(o);
		if (!e)
			return;
		RepairEntity(e);
		Act(id, "Repair", e.GetType() + " @ " + VecStr(e.GetPosition()));
		Reply(id, "Repaired " + e.GetType());
	}

	void CmdObjRefuel(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.WORLD_REPAIR))
			return;
		Object o = ObjFromArgs(id, args);
		CarScript car = CarScript.Cast(o);
		if (!car)
		{
			Reply(id, "Not a car");
			return;
		}
		car.Fill(CarFluid.FUEL, car.GetFluidCapacity(CarFluid.FUEL));
		car.Fill(CarFluid.OIL, car.GetFluidCapacity(CarFluid.OIL));
		car.Fill(CarFluid.BRAKE, car.GetFluidCapacity(CarFluid.BRAKE));
		car.Fill(CarFluid.COOLANT, car.GetFluidCapacity(CarFluid.COOLANT));
		Act(id, "Refuel", car.GetType());
		Reply(id, "Refuelled " + car.GetType());
	}

	// args: hour, minute
	void CmdWorldTime(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.WORLD_TIME))
			return;
		if (args.Count() < 2)
			return;
		int y, mo, d, h, mi;
		GetGame().GetWorld().GetDate(y, mo, d, h, mi);
		h = ClampInt(args[0].ToInt(), 0, 23);
		mi = ClampInt(args[1].ToInt(), 0, 59);
		GetGame().GetWorld().SetDate(y, mo, d, h, mi);
		Act(id, "Set time", string.Format("%1:%2", HOD_Time.Pad(h), HOD_Time.Pad(mi)));
		Reply(id, string.Format("Time set to %1:%2", HOD_Time.Pad(h), HOD_Time.Pad(mi)));
	}

	// args: clear|cloudy|rain|storm|fog
	void CmdWorldWeather(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.WORLD_WEATHER))
			return;
		if (args.Count() < 1)
			return;
		Weather w = GetGame().GetWeather();
		float overcast = 0;
		float rain = 0;
		float fog = 0;
		string preset = args[0];
		switch (preset)
		{
			case "clear":  overcast = 0.0; rain = 0.0; fog = 0.0; break;
			case "cloudy": overcast = 0.6; rain = 0.0; fog = 0.05; break;
			case "rain":   overcast = 0.8; rain = 0.6; fog = 0.1; break;
			case "storm":  overcast = 1.0; rain = 1.0; fog = 0.2; break;
			case "fog":    overcast = 0.5; rain = 0.0; fog = 0.8; break;
			default: return;
		}
		float hold = 1800; // keep it at least 30 min
		w.GetOvercast().Set(overcast, 30, hold);
		w.GetRain().Set(rain, 30, hold);
		w.GetFog().Set(fog, 30, hold);
		Act(id, "Weather", preset);
		Reply(id, "Weather -> " + preset + " (fades in over ~30s)");
	}

	void BroadcastAll(string text)
	{
		array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);
		foreach (Man m : players)
		{
			if (m && m.GetIdentity())
			{
				Notify(m.GetIdentity(), m_Settings.ServerName, text);
				HOD_RPC.ToClient(m.GetIdentity(), "chat", {"[" + m_Settings.ServerName + "] " + text});
			}
		}
	}

	void CmdBroadcast(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.SERVER_BROADCAST))
			return;
		if (args.Count() < 1 || args[0] == "")
			return;
		BroadcastAll(args[0]);
		Act(id, "Broadcast", args[0]);
		Reply(id, "Broadcast sent");
	}

	// ------------------------------------------------------------------ vehicles

	// stride 8: netLow, netHigh, type, "x y z", health%, owner, keys, lock
	void CmdVehicles(PlayerIdentity id)
	{
		if (!Check(id, HOD_Perm.VEHICLES_VIEW))
			return;
		HOD_Vehicles.Prune();
		array<string> res = new array<string>();
		foreach (CarScript car : HOD_Vehicles.s_All)
		{
			if (!car)
				continue;
			int lo, hi;
			car.GetNetworkID(lo, hi);
			res.Insert(lo.ToString());
			res.Insert(hi.ToString());
			res.Insert(car.GetType());
			res.Insert(VecStr(car.GetPosition()));
			res.Insert(Math.Round(car.GetHealth01("", "") * 100).ToString());
			array<string> info = new array<string>();
			HOD_Vehicles.ExpansionInfo(car).Split("|", info);
			while (info.Count() < 3)
				info.Insert("-");
			res.Insert(info[0]);
			res.Insert(info[1]);
			res.Insert(info[2]);
		}
		HOD_RPC.ToClient(id, "vehicles", res);
	}

	// args: netLow, netHigh - teleports the admin next to the vehicle
	void CmdVehicleTp(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.VEHICLES_VIEW) || !Check(id, HOD_Perm.TELEPORT_SELF))
			return;
		Object o = ObjFromArgs(id, args);
		PlayerBase self = SenderPlayer(id);
		if (!o || !self)
			return;
		if (TeleportPlayer(id, self, o.GetPosition() + "3 0 3"))
			Act(id, "Teleport to vehicle", o.GetType() + " @ " + VecStr(o.GetPosition()));
	}

	// ------------------------------------------------------------------ Expansion

	void CmdExp(PlayerIdentity id, array<string> args, int op)
	{
		string perm = HOD_Perm.EXP_VEHICLES;
		if (op >= 3)
			perm = HOD_Perm.EXP_CODELOCKS;
		if (!Check(id, perm))
			return;
		Object o = ObjFromArgs(id, args);
		if (!o)
			return;
		EntityAI root = EntityAI.Cast(o);
		if (root && root.GetHierarchyRoot() && op < 3)
			o = root.GetHierarchyRoot();

		string msg;
		bool ok = false;
		string action = "";
		switch (op)
		{
			case 0: ok = HOD_Expansion.UnlockVehicle(o, msg); action = "Expansion unlock vehicle"; break;
			case 1: ok = HOD_Expansion.ResetKeys(o, msg); action = "Expansion reset keys"; break;
			case 2: ok = HOD_Expansion.GiveKey(o, SenderPlayer(id), msg); action = "Expansion give key"; break;
			case 3: ok = HOD_Expansion.ClearCode(o, msg); action = "Expansion clear code"; break;
			case 4: ok = HOD_Expansion.ReadCode(o, msg); action = "Expansion read code"; break;
		}
		if (ok)
			Act(id, action, o.GetType() + " @ " + VecStr(o.GetPosition()) + " - " + msg);
		Reply(id, msg);
	}

	// args: steamid, amount (can be negative)
	void CmdExpAtm(PlayerIdentity id, array<string> args)
	{
		if (!Check(id, HOD_Perm.EXP_ATM))
			return;
		if (args.Count() < 2)
			return;
		string msg;
		int amount = args[1].ToInt();
		if (HOD_Expansion.AddATMMoney(args[0], amount, msg))
			Act(id, "Expansion ATM", args[0] + " " + amount.ToString() + " -> " + msg);
		Reply(id, msg);
	}
}
