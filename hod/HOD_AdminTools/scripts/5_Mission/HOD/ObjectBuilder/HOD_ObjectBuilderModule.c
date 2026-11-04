// HOD Admin Tools - Object Builder (House of D, CC BY-SA 4.0). Written from scratch.
// Server owns the sets; clients edit through CF RPCs ("HOD_OB" namespace).

class HOD_ObjectBuilderModule : JMRenderableModuleBase
{
	static const string PERM_VIEW = "HOD.ObjectBuilder.View";
	static const string PERM_EDIT = "HOD.ObjectBuilder.Edit";
	static const string RPC_NS = "HOD_OB";

	// ---- server state
	protected ref map<string, ref HOD_OBSet> m_Sets;
	protected ref map<string, ref array<Object>> m_Live;

	// ---- client state
	ref HOD_OBSetList m_ClientList;
	ref HOD_OBSet m_ClientSet;
	int m_SelectedIndex = -1;
	bool m_BuildMode;
	float m_Step = 0.5;   // metres
	float m_RotStep = 5;  // degrees

	void HOD_ObjectBuilderModule()
	{
		GetPermissionsManager().RegisterPermission(PERM_VIEW);
		GetPermissionsManager().RegisterPermission(PERM_EDIT);

		m_Sets = new map<string, ref HOD_OBSet>();
		m_Live = new map<string, ref array<Object>>();
		m_ClientList = new HOD_OBSetList();

		// client -> server
		GetRPCManager().AddRPC(RPC_NS, "ReqList", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RPC_NS, "ReqSet", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RPC_NS, "CreateSet", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RPC_NS, "DeleteSet", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RPC_NS, "SetActive", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RPC_NS, "AddObject", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RPC_NS, "UpdateObject", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RPC_NS, "RemoveObject", this, SingleplayerExecutionType.Server);
		// server -> client
		GetRPCManager().AddRPC(RPC_NS, "List", this, SingleplayerExecutionType.Client);
		GetRPCManager().AddRPC(RPC_NS, "SetData", this, SingleplayerExecutionType.Client);
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission(PERM_VIEW);
	}

	override string GetLayoutRoot()
	{
		return "HOD_AdminTools/GUI/layouts/objectbuilder_form.layout";
	}

	override string GetTitle()
	{
		return "Object Builder";
	}

	override string GetIconName()
	{
		return "OB";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override string GetWebhookTitle()
	{
		return "Object Builder";
	}

	override void RegisterKeyMouseBindings()
	{
		super.RegisterKeyMouseBindings();
		Bind(new JMModuleBinding("Input_Select", "UAHODOBSelect", true));
		Bind(new JMModuleBinding("Input_Deselect", "UAHODOBDeselect", true));
		Bind(new JMModuleBinding("Input_RotateLeft", "UAHODOBRotateLeft", true));
		Bind(new JMModuleBinding("Input_RotateRight", "UAHODOBRotateRight", true));
		Bind(new JMModuleBinding("Input_Delete", "UAHODOBDelete", true));
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();
		if (GetGame().IsServer())
			Server_LoadAll();
	}

	// =============================================================== server

	protected string SetFile(string name)
	{
		return HOD_OB_DIR + "/" + name + ".json";
	}

	protected static bool ValidName(string name)
	{
		if (name == "" || name.Length() > 48)
			return false;
		string bad = "\\/:*?\"<>|.";
		for (int i = 0; i < bad.Length(); i++)
		{
			if (name.IndexOf(bad.Get(i)) >= 0)
				return false;
		}
		return true;
	}

	protected void Server_LoadAll()
	{
		if (!FileExist("$profile:HOD_AdminTools"))
			MakeDirectory("$profile:HOD_AdminTools");
		if (!FileExist(HOD_OB_DIR))
			MakeDirectory(HOD_OB_DIR);

		string fileName;
		FileAttr attr;
		FindFileHandle h = FindFile(HOD_OB_DIR + "/*.json", fileName, attr, FindFileFlags.DIRECTORIES);
		bool more = fileName != "";
		while (more)
		{
			if (fileName != "")
			{
				HOD_OBSet obSet = new HOD_OBSet();
				JsonFileLoader<HOD_OBSet>.JsonLoadFile(HOD_OB_DIR + "/" + fileName, obSet);
				if (obSet.Name == "")
					obSet.Name = fileName.Substring(0, fileName.Length() - 5);
				m_Sets.Set(obSet.Name, obSet);
				if (obSet.Active)
					Server_SpawnSet(obSet);
			}
			more = FindNextFile(h, fileName, attr);
		}
		CloseFindFile(h);
		Print("[HOD] Object Builder loaded " + m_Sets.Count() + " set(s)");
	}

	protected void Server_Save(HOD_OBSet obSet)
	{
		JsonFileLoader<HOD_OBSet>.JsonSaveFile(SetFile(obSet.Name), obSet);
	}

	protected Object Server_SpawnObject(HOD_OBObject o)
	{
		int flags = ECE_SETUP | ECE_UPDATEPATHGRAPH | ECE_CREATEPHYSICS | ECE_NOLIFETIME | ECE_NOPERSISTENCY_WORLD | ECE_KEEPHEIGHT;
		Object obj = GetGame().CreateObjectEx(o.Type, o.Position, flags);
		if (!obj)
			return null;
		obj.SetPosition(o.Position);
		obj.SetOrientation(o.Orientation);
		obj.Update();
		return obj;
	}

	protected void Server_SpawnSet(HOD_OBSet obSet)
	{
		Server_DespawnSet(obSet.Name);
		array<Object> live = new array<Object>();
		foreach (HOD_OBObject o : obSet.Objects)
			live.Insert(Server_SpawnObject(o));
		m_Live.Set(obSet.Name, live);
	}

	protected void Server_DespawnSet(string name)
	{
		array<Object> live;
		if (!m_Live.Find(name, live))
			return;
		foreach (Object obj : live)
		{
			if (obj)
				GetGame().ObjectDelete(obj);
		}
		m_Live.Remove(name);
	}

	// keep the live object for one index in sync with its data
	protected void Server_RespawnIndex(HOD_OBSet obSet, int idx)
	{
		if (!obSet.Active)
			return;
		array<Object> live;
		if (!m_Live.Find(obSet.Name, live))
			return;
		while (live.Count() < obSet.Objects.Count())
			live.Insert(null);
		if (live[idx])
			GetGame().ObjectDelete(live[idx]);
		live[idx] = Server_SpawnObject(obSet.Objects[idx]);
	}

	protected bool Server_Allowed(PlayerIdentity sender, string perm)
	{
		JMPlayerInstance inst;
		return GetPermissionsManager().HasPermission(perm, sender, inst);
	}

	protected void Server_SendList(PlayerIdentity to)
	{
		HOD_OBSetList list = new HOD_OBSetList();
		foreach (string name, HOD_OBSet obSet : m_Sets)
		{
			HOD_OBSetInfo info = new HOD_OBSetInfo();
			info.Name = name;
			info.Active = obSet.Active;
			info.Count = obSet.Objects.Count();
			list.Sets.Insert(info);
		}
		JsonSerializer js = new JsonSerializer();
		string json;
		js.WriteToString(list, false, json);
		GetRPCManager().SendRPC(RPC_NS, "List", new Param1<string>(json), true, to);
	}

	protected void Server_SendSet(PlayerIdentity to, string name)
	{
		HOD_OBSet obSet;
		if (!m_Sets.Find(name, obSet))
			return;
		GetRPCManager().SendRPC(RPC_NS, "SetData", new Param1<string>(obSet.ToJson()), true, to);
	}

	protected void Server_Log(PlayerIdentity sender, string text)
	{
		GetCommunityOnlineToolsBase().Log(sender, "[Object Builder] " + text);
	}

	void ReqList(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server || !Server_Allowed(sender, PERM_VIEW))
			return;
		Server_SendList(sender);
	}

	void ReqSet(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param1<string> p;
		if (type != CallType.Server || !ctx.Read(p) || !Server_Allowed(sender, PERM_VIEW))
			return;
		Server_SendSet(sender, p.param1);
	}

	void CreateSet(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param1<string> p;
		if (type != CallType.Server || !ctx.Read(p) || !Server_Allowed(sender, PERM_EDIT))
			return;
		string name = p.param1;
		name.TrimInPlace();
		if (!ValidName(name) || m_Sets.Contains(name))
		{
			COTCreateNotification(sender, new StringLocaliser("Object Builder: invalid or duplicate set name"));
			return;
		}
		HOD_OBSet obSet = new HOD_OBSet();
		obSet.Name = name;
		m_Sets.Set(name, obSet);
		m_Live.Set(name, new array<Object>());
		Server_Save(obSet);
		Server_Log(sender, "created set " + name);
		Server_SendList(sender);
		Server_SendSet(sender, name);
	}

	void DeleteSet(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param1<string> p;
		if (type != CallType.Server || !ctx.Read(p) || !Server_Allowed(sender, PERM_EDIT))
			return;
		if (!m_Sets.Contains(p.param1))
			return;
		Server_DespawnSet(p.param1);
		m_Sets.Remove(p.param1);
		DeleteFile(SetFile(p.param1));
		Server_Log(sender, "deleted set " + p.param1);
		Server_SendList(sender);
	}

	void SetActive(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param2<string, bool> p;
		if (type != CallType.Server || !ctx.Read(p) || !Server_Allowed(sender, PERM_EDIT))
			return;
		HOD_OBSet obSet;
		if (!m_Sets.Find(p.param1, obSet))
			return;
		obSet.Active = p.param2;
		if (obSet.Active)
			Server_SpawnSet(obSet);
		else
			Server_DespawnSet(obSet.Name);
		Server_Save(obSet);
		Server_Log(sender, string.Format("set %1 active=%2", obSet.Name, obSet.Active));
		Server_SendList(sender);
		Server_SendSet(sender, obSet.Name);
	}

	void AddObject(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param4<string, string, vector, vector> p;
		if (type != CallType.Server || !ctx.Read(p) || !Server_Allowed(sender, PERM_EDIT))
			return;
		HOD_OBSet obSet;
		if (!m_Sets.Find(p.param1, obSet))
			return;
		string cls = p.param2;
		if (!GetGame().ConfigIsExisting("CfgVehicles " + cls) && !GetGame().ConfigIsExisting("CfgNonAIVehicles " + cls))
		{
			COTCreateNotification(sender, new StringLocaliser("Object Builder: unknown class " + cls));
			return;
		}
		obSet.Objects.Insert(new HOD_OBObject(cls, p.param3, p.param4));
		Server_RespawnIndex(obSet, obSet.Objects.Count() - 1);
		Server_Save(obSet);
		Server_Log(sender, string.Format("%1: added %2 at %3", obSet.Name, cls, p.param3));
		Server_SendSet(sender, obSet.Name);
	}

	void UpdateObject(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param4<string, int, vector, vector> p;
		if (type != CallType.Server || !ctx.Read(p) || !Server_Allowed(sender, PERM_EDIT))
			return;
		HOD_OBSet obSet;
		if (!m_Sets.Find(p.param1, obSet))
			return;
		int idx = p.param2;
		if (idx < 0 || idx >= obSet.Objects.Count())
			return;
		obSet.Objects[idx].Position = p.param3;
		obSet.Objects[idx].Orientation = p.param4;
		Server_RespawnIndex(obSet, idx);
		Server_Save(obSet);
		Server_SendSet(sender, obSet.Name);
	}

	void RemoveObject(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param2<string, int> p;
		if (type != CallType.Server || !ctx.Read(p) || !Server_Allowed(sender, PERM_EDIT))
			return;
		HOD_OBSet obSet;
		if (!m_Sets.Find(p.param1, obSet))
			return;
		int idx = p.param2;
		if (idx < 0 || idx >= obSet.Objects.Count())
			return;
		array<Object> live;
		if (m_Live.Find(obSet.Name, live) && idx < live.Count())
		{
			if (live[idx])
				GetGame().ObjectDelete(live[idx]);
			live.Remove(idx);
		}
		Server_Log(sender, string.Format("%1: removed %2", obSet.Name, obSet.Objects[idx].Type));
		obSet.Objects.Remove(idx);
		Server_Save(obSet);
		Server_SendSet(sender, obSet.Name);
	}

	// =============================================================== client

	void List(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param1<string> p;
		if (type != CallType.Client || !ctx.Read(p))
			return;
		HOD_OBSetList list = new HOD_OBSetList();
		JsonSerializer js = new JsonSerializer();
		string err;
		if (js.ReadFromString(list, p.param1, err))
			m_ClientList = list;
		RefreshForm();
	}

	void SetData(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param1<string> p;
		if (type != CallType.Client || !ctx.Read(p))
			return;
		HOD_OBSet obSet = HOD_OBSet.FromJson(p.param1);
		if (!obSet)
			return;
		if (!m_ClientSet || m_ClientSet.Name != obSet.Name)
			m_SelectedIndex = -1;
		m_ClientSet = obSet;
		if (m_SelectedIndex >= obSet.Objects.Count())
			m_SelectedIndex = obSet.Objects.Count() - 1;
		RefreshForm();
	}

	protected void RefreshForm()
	{
		HOD_ObjectBuilderForm form;
		if (Class.CastTo(form, GetForm()))
			form.RefreshAll();
	}

	string CurrentSetName()
	{
		if (m_ClientSet)
			return m_ClientSet.Name;
		return "";
	}

	HOD_OBObject Selected()
	{
		if (!m_ClientSet || m_SelectedIndex < 0 || m_SelectedIndex >= m_ClientSet.Objects.Count())
			return null;
		return m_ClientSet.Objects[m_SelectedIndex];
	}

	void Client_RequestList()
	{
		GetRPCManager().SendRPC(RPC_NS, "ReqList", null, true);
	}

	void Client_OpenSet(string name)
	{
		GetRPCManager().SendRPC(RPC_NS, "ReqSet", new Param1<string>(name), true);
	}

	void Client_CreateSet(string name)
	{
		GetRPCManager().SendRPC(RPC_NS, "CreateSet", new Param1<string>(name), true);
	}

	void Client_DeleteSet(string name)
	{
		GetRPCManager().SendRPC(RPC_NS, "DeleteSet", new Param1<string>(name), true);
		if (m_ClientSet && m_ClientSet.Name == name)
		{
			m_ClientSet = null;
			m_SelectedIndex = -1;
		}
	}

	void Client_SetActive(string name, bool active)
	{
		GetRPCManager().SendRPC(RPC_NS, "SetActive", new Param2<string, bool>(name, active), true);
	}

	void Client_AddObject(string cls, vector pos, vector ori)
	{
		if (!m_ClientSet)
		{
			COTCreateLocalAdminNotification(new StringLocaliser("Object Builder: open or create a set first"));
			return;
		}
		m_SelectedIndex = m_ClientSet.Objects.Count(); // select the new one once it arrives
		GetRPCManager().SendRPC(RPC_NS, "AddObject", new Param4<string, string, vector, vector>(m_ClientSet.Name, cls, pos, ori), true);
	}

	void Client_UpdateSelected(vector pos, vector ori)
	{
		HOD_OBObject o = Selected();
		if (!o)
			return;
		o.Position = pos;
		o.Orientation = ori;
		GetRPCManager().SendRPC(RPC_NS, "UpdateObject", new Param4<string, int, vector, vector>(m_ClientSet.Name, m_SelectedIndex, pos, ori), true);
		RefreshForm();
	}

	void Client_RemoveSelected()
	{
		if (!Selected())
			return;
		GetRPCManager().SendRPC(RPC_NS, "RemoveObject", new Param2<string, int>(m_ClientSet.Name, m_SelectedIndex), true);
		m_SelectedIndex = -1;
	}

	void Nudge(vector delta, float yaw)
	{
		HOD_OBObject o = Selected();
		if (!o)
			return;
		vector ori = o.Orientation;
		ori[0] = Math.ModFloat(ori[0] + yaw + 360.0, 360.0);
		Client_UpdateSelected(o.Position + delta, ori);
	}

	// Select the set object nearest to whatever the crosshair hits
	bool SelectAtCursor()
	{
		if (!m_ClientSet)
			return false;
		vector hit = GetCursorPos();
		int best = -1;
		float bestDist = 25.0;
		for (int i = 0; i < m_ClientSet.Objects.Count(); i++)
		{
			float d = vector.Distance(m_ClientSet.Objects[i].Position, hit);
			if (d < bestDist)
			{
				bestDist = d;
				best = i;
			}
		}
		m_SelectedIndex = best;
		RefreshForm();
		return best >= 0;
	}

	protected bool BuildInputAllowed(UAInput input)
	{
		if (!m_BuildMode || !input.LocalPress())
			return false;
		if (!GetPermissionsManager().HasPermission(PERM_EDIT))
			return false;
		return GetCommunityOnlineToolsBase().IsActive();
	}

	void Input_Select(UAInput input)
	{
		if (BuildInputAllowed(input))
			SelectAtCursor();
	}

	void Input_Deselect(UAInput input)
	{
		if (!BuildInputAllowed(input))
			return;
		m_SelectedIndex = -1;
		RefreshForm();
	}

	void Input_RotateLeft(UAInput input)
	{
		if (BuildInputAllowed(input))
			Nudge("0 0 0", -m_RotStep);
	}

	void Input_RotateRight(UAInput input)
	{
		if (BuildInputAllowed(input))
			Nudge("0 0 0", m_RotStep);
	}

	void Input_Delete(UAInput input)
	{
		if (BuildInputAllowed(input))
			Client_RemoveSelected();
	}
}
