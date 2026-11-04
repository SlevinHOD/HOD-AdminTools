// HOD Admin Tools - XML search (House of D, CC BY-SA 4.0). Written from scratch.
// Server reads the mission's types.xml files (db/types.xml + every type="types" file in
// cfgeconomycore.xml) and answers searches. Read-only: never writes economy files.

class HOD_TypeEntry
{
	string name;
	string file;
	int nominal = -1;
	int min = -1;
	int lifetime = -1;
	int restock = -1;
	int quantMin = -1;
	int quantMax = -1;
	int cost = -1;
	string flags;
	string category;
	string usages;
	string values;
	string tags;
	string raw;
}

class HOD_TypeResults
{
	string query;
	int total;
	ref array<ref HOD_TypeEntry> entries = new array<ref HOD_TypeEntry>();
}

class HOD_XMLSearchModule : JMRenderableModuleBase
{
	static const string PERM_VIEW = "HOD.XMLSearch.View";
	static const string RPC_NS = "HOD_XML";
	static const int MAX_RESULTS = 150;

	protected ref array<ref HOD_TypeEntry> m_Types;     // server
	protected ref map<string, int> m_SeenFiles;         // server
	ref HOD_TypeResults m_ClientResults;                // client

	void HOD_XMLSearchModule()
	{
		GetPermissionsManager().RegisterPermission(PERM_VIEW);
		m_ClientResults = new HOD_TypeResults();
		GetRPCManager().AddRPC(RPC_NS, "Search", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RPC_NS, "Reload", this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RPC_NS, "Results", this, SingleplayerExecutionType.Client);
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission(PERM_VIEW);
	}

	override string GetLayoutRoot()
	{
		return "HOD_AdminTools/GUI/layouts/xmlsearch_form.layout";
	}

	override string GetTitle()
	{
		return "XML Search (types)";
	}

	override string GetIconName()
	{
		return "XML";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	// =============================================================== server: parsing

	protected void Server_EnsureLoaded()
	{
		if (m_Types)
			return;
		m_Types = new array<ref HOD_TypeEntry>();
		m_SeenFiles = new map<string, int>();
		ParseTypesFile("db/types.xml");
		ParseEconomyCore();
		Print("[HOD] XML search indexed " + m_Types.Count() + " types");
	}

	protected string ReadFile(string path)
	{
		FileHandle fh = OpenFile(path, FileMode.READ);
		if (fh == 0)
			return "";
		string content = "";
		string line;
		while (FGets(fh, line) >= 0)
			content += line + "\n";
		CloseFile(fh);
		return content;
	}

	// value of attribute `attr` inside tag text like: name="X" min="1"
	static string Attr(string tag, string attr)
	{
		string key = attr + "=\"";
		int i = tag.IndexOf(key);
		if (i < 0)
			return "";
		int start = i + key.Length();
		int end = tag.IndexOfFrom(start, "\"");
		if (end < 0)
			return "";
		return tag.Substring(start, end - start);
	}

	// text between <el> and </el> inside block, -1 if missing
	static int ElemInt(string block, string el)
	{
		int i = block.IndexOf("<" + el + ">");
		if (i < 0)
			return -1;
		int start = i + el.Length() + 2;
		int end = block.IndexOfFrom(start, "</" + el + ">");
		if (end < 0)
			return -1;
		string v = block.Substring(start, end - start);
		v.TrimInPlace();
		return v.ToInt();
	}

	// all name="" values of <el .../> tags, comma separated
	static string ElemNames(string block, string el)
	{
		string res = "";
		int pos = 0;
		while (true)
		{
			int i = block.IndexOfFrom(pos, "<" + el + " ");
			if (i < 0)
				break;
			int end = block.IndexOfFrom(i, ">");
			if (end < 0)
				break;
			string n = Attr(block.Substring(i, end - i), "name");
			if (n != "")
			{
				if (res != "")
					res += ", ";
				res += n;
			}
			pos = end;
		}
		return res;
	}

	protected void ParseTypesFile(string relPath)
	{
		string key = relPath;
		key.ToLower();
		if (m_SeenFiles.Contains(key))
			return;
		m_SeenFiles.Set(key, 1);

		string xml = ReadFile("$mission:" + relPath);
		if (xml == "")
		{
			Print("[HOD] XML search: can't read $mission:" + relPath);
			return;
		}

		int pos = 0;
		while (true)
		{
			int start = xml.IndexOfFrom(pos, "<type ");
			if (start < 0)
				break;
			int headEnd = xml.IndexOfFrom(start, ">");
			if (headEnd < 0)
				break;
			string head = xml.Substring(start, headEnd - start + 1);
			int end;
			if (head.IndexOf("/>") >= 0)
				end = headEnd + 1;
			else
			{
				end = xml.IndexOfFrom(headEnd, "</type>");
				if (end < 0)
					break;
				end += 7;
			}
			string block = xml.Substring(start, end - start);
			pos = end;

			HOD_TypeEntry e = new HOD_TypeEntry();
			e.name = Attr(head, "name");
			if (e.name == "")
				continue;
			e.file = relPath;
			e.nominal = ElemInt(block, "nominal");
			e.min = ElemInt(block, "min");
			e.lifetime = ElemInt(block, "lifetime");
			e.restock = ElemInt(block, "restock");
			e.quantMin = ElemInt(block, "quantmin");
			e.quantMax = ElemInt(block, "quantmax");
			e.cost = ElemInt(block, "cost");
			e.category = ElemNames(block, "category");
			e.usages = ElemNames(block, "usage");
			e.values = ElemNames(block, "value");
			e.tags = ElemNames(block, "tag");
			int fi = block.IndexOf("<flags ");
			if (fi >= 0)
			{
				int fe = block.IndexOfFrom(fi, ">");
				if (fe > fi)
					e.flags = block.Substring(fi + 7, fe - fi - 7);
				e.flags.Replace("/", "");
				e.flags.TrimInPlace();
			}
			e.raw = block;
			m_Types.Insert(e);
		}
	}

	// <ce folder="x"> <file name="y.xml" type="types"/> </ce>
	protected void ParseEconomyCore()
	{
		string xml = ReadFile("$mission:cfgeconomycore.xml");
		if (xml == "")
			return;
		int pos = 0;
		while (true)
		{
			int ce = xml.IndexOfFrom(pos, "<ce ");
			if (ce < 0)
				break;
			int ceHeadEnd = xml.IndexOfFrom(ce, ">");
			int ceEnd = xml.IndexOfFrom(ce, "</ce>");
			if (ceHeadEnd < 0 || ceEnd < 0)
				break;
			string folder = Attr(xml.Substring(ce, ceHeadEnd - ce), "folder");
			string body = xml.Substring(ceHeadEnd, ceEnd - ceHeadEnd);
			int fpos = 0;
			while (true)
			{
				int f = body.IndexOfFrom(fpos, "<file ");
				if (f < 0)
					break;
				int fEnd = body.IndexOfFrom(f, ">");
				if (fEnd < 0)
					break;
				string tag = body.Substring(f, fEnd - f);
				if (Attr(tag, "type") == "types")
				{
					string path = Attr(tag, "name");
					if (folder != "")
						path = folder + "/" + path;
					ParseTypesFile(path);
				}
				fpos = fEnd;
			}
			pos = ceEnd + 5;
		}
	}

	// =============================================================== server: RPC

	void Search(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param1<string> p;
		if (type != CallType.Server || !ctx.Read(p))
			return;
		JMPlayerInstance inst;
		if (!GetPermissionsManager().HasPermission(PERM_VIEW, sender, inst))
			return;
		Server_EnsureLoaded();

		string q = p.param1;
		q.ToLower();
		q.TrimInPlace();
		HOD_TypeResults res = new HOD_TypeResults();
		res.query = p.param1;
		if (q.Length() >= 2)
		{
			foreach (HOD_TypeEntry e : m_Types)
			{
				string n = e.name;
				n.ToLower();
				if (n.IndexOf(q) < 0)
					continue;
				res.total++;
				if (res.entries.Count() < MAX_RESULTS)
					res.entries.Insert(e);
			}
		}
		JsonSerializer js = new JsonSerializer();
		string json;
		js.WriteToString(res, false, json);
		GetRPCManager().SendRPC(RPC_NS, "Results", new Param1<string>(json), true, sender);
	}

	void Reload(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		JMPlayerInstance inst;
		if (!GetPermissionsManager().HasPermission(PERM_VIEW, sender, inst))
			return;
		m_Types = null;
		Server_EnsureLoaded();
		COTCreateNotification(sender, new StringLocaliser("XML search: indexed " + m_Types.Count() + " types from " + m_SeenFiles.Count() + " file(s)"));
	}

	// =============================================================== client

	void Results(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param1<string> p;
		if (type != CallType.Client || !ctx.Read(p))
			return;
		HOD_TypeResults res = new HOD_TypeResults();
		JsonSerializer js = new JsonSerializer();
		string err;
		if (!js.ReadFromString(res, p.param1, err))
			return;
		m_ClientResults = res;
		HOD_XMLSearchForm form;
		if (Class.CastTo(form, GetForm()))
			form.ShowResults();
	}

	void Client_Search(string q)
	{
		GetRPCManager().SendRPC(RPC_NS, "Search", new Param1<string>(q), true);
	}

	void Client_Reload()
	{
		GetRPCManager().SendRPC(RPC_NS, "Reload", null, true);
	}
}
