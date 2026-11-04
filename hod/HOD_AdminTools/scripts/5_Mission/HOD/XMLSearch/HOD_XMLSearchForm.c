// HOD Admin Tools - XML search window (House of D, CC BY-SA 4.0). Written from scratch.

class HOD_XMLSearchForm : JMFormBase
{
	protected HOD_XMLSearchModule m_Module;
	protected UIActionEditableText m_Search;
	protected UIActionText m_Count;
	protected TextListboxWidget m_Results;
	protected ref array<UIActionText> m_Details = new array<UIActionText>();
	protected int m_Selected = -1;

	protected override bool SetModule(JMRenderableModuleBase mdl)
	{
		return Class.CastTo(m_Module, mdl);
	}

	override void OnInit()
	{
		Widget top = layoutRoot.FindAnyWidget("xml_search");
		Widget row = UIActionManager.CreateGridSpacer(top, 1, 3);
		m_Search = UIActionManager.CreateEditableText(row, "Class:", this, "Change_Search");
		UIActionManager.CreateButton(row, "Search", this, "Click_Search");
		UIActionManager.CreateButton(row, "Re-read files", this, "Click_Reload");
		m_Count = UIActionManager.CreateText(top, "Results:", "type at least 2 letters");

		m_Results = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("xml_results"));

		Widget det = layoutRoot.FindAnyWidget("xml_details");
		array<string> labels = {"Class", "File", "Nominal / Min", "Lifetime", "Restock", "Quantity", "Cost", "Category", "Usage", "Value (tier)", "Tags", "Flags"};
		foreach (string l : labels)
			m_Details.Insert(UIActionManager.CreateText(det, l + ":", ""));
		Widget btns = UIActionManager.CreateGridSpacer(det, 1, 2);
		UIActionManager.CreateButton(btns, "Copy class name", this, "Click_CopyName");
		UIActionManager.CreateButton(btns, "Copy XML block", this, "Click_CopyXml");
	}

	// HOD: results list and details follow the window size
	override void OnResize(float w, float h)
	{
		super.OnResize(w, h);
		HOD_Layout(w, h);
	}

	override void OnShow()
	{
		super.OnShow();
		float lw, lh;
		layoutRoot.GetSize(lw, lh);
		HOD_Layout(lw, lh);
	}

	protected void HOD_Layout(float w, float h)
	{
		if (w < 500 || h < 300)
			return;
		Widget top = layoutRoot.FindAnyWidget("xml_search");
		Widget det = layoutRoot.FindAnyWidget("xml_details");
		float listW = w * 0.5 - 10;
		if (top)
			top.SetSize(w, 30);
		if (m_Results)
		{
			m_Results.SetPos(5, 64);
			m_Results.SetSize(listW, h - 70);
		}
		if (det)
		{
			det.SetPos(listW + 15, 64);
			det.SetSize(w - listW - 20, 30);
		}
	}

	void Change_Search(UIEvent eid, UIActionBase action)
	{
		// search as you type (CHANGE is already debounced by UIActionEditableText)
		if (eid != UIEvent.CHANGE)
			return;
		string q = m_Search.GetText();
		q.TrimInPlace();
		if (q.Length() >= 3)
			DoSearch(q);
	}

	protected void DoSearch(string q)
	{
		q.TrimInPlace();
		if (q.Length() < 2)
		{
			m_Count.SetText("type at least 2 letters");
			return;
		}
		m_Count.SetText("searching '" + q + "' ...");
		m_Module.Client_Search(q);
	}

	void Click_Search(UIEvent eid, UIActionBase action)
	{
		if (eid == UIEvent.CLICK)
			DoSearch(m_Search.GetText());
	}

	void Click_Reload(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;
		m_Count.SetText("re-reading types files ...");
		m_Module.Client_Reload();
	}

	void ShowResults()
	{
		HOD_TypeResults res = m_Module.m_ClientResults;
		m_Results.ClearItems();
		m_Selected = -1;
		foreach (HOD_TypeBrief b : res.entries)
			m_Results.AddItem(b.name + "   n" + Num(b.nominal) + " / m" + Num(b.min), null, 0);

		string info = string.Format("  [%1 types from %2 file(s)", res.indexed, res.files);
		if (res.unreadable > 0)
			info += ", " + res.unreadable.ToString() + " unreadable";
		info += "]";
		if (res.indexed == 0)
		{
			m_Count.SetText("No types loaded - couldn't read db/types.xml or cfgeconomycore.xml in the mission folder");
		}
		else
		{
			string shown = res.total.ToString();
			if (res.total > res.entries.Count())
				shown = res.entries.Count().ToString() + " of " + res.total.ToString();
			m_Count.SetText(shown + " match(es) for '" + res.query + "'" + info);
		}
		if (res.entries.Count() > 0)
		{
			m_Results.SelectRow(0);
			SelectRow(0);
		}
		else
			ShowDetailEntry(null);
	}

	static string Num(int v)
	{
		if (v < 0)
			return "-";
		return v.ToString();
	}

	static string Duration(int seconds)
	{
		if (seconds < 0)
			return "-";
		if (seconds >= 86400)
			return string.Format("%1 s (%2 days)", seconds, Math.Round(seconds / 864.0) / 100.0);
		if (seconds >= 3600)
			return string.Format("%1 s (%2 h)", seconds, Math.Round(seconds / 36.0) / 100.0);
		return seconds.ToString() + " s";
	}

	protected void SelectRow(int i)
	{
		m_Selected = i;
		HOD_TypeResults res = m_Module.m_ClientResults;
		if (i < 0 || i >= res.entries.Count())
		{
			ShowDetailEntry(null);
			return;
		}
		ShowDetailEntry(null);
		m_Details[0].SetText(res.entries[i].name + "  (loading...)");
		m_Module.Client_Detail(res.entries[i].name);
	}

	void ShowDetailEntry(HOD_TypeEntry e)
	{
		if (!e)
		{
			foreach (UIActionText t : m_Details)
				t.SetText("");
			return;
		}
		m_Details[0].SetText(e.name);
		m_Details[1].SetText(e.file);
		m_Details[2].SetText(Num(e.nominal) + " / " + Num(e.min));
		m_Details[3].SetText(Duration(e.lifetime));
		m_Details[4].SetText(Duration(e.restock));
		m_Details[5].SetText(Num(e.quantMin) + " - " + Num(e.quantMax));
		m_Details[6].SetText(Num(e.cost));
		m_Details[7].SetText(e.category);
		m_Details[8].SetText(e.usages);
		m_Details[9].SetText(e.values);
		m_Details[10].SetText(e.tags);
		m_Details[11].SetText(e.flags);
	}

	override bool OnItemSelected(Widget w, int x, int y, int row, int column, int oldRow, int oldColumn)
	{
		if (w == m_Results)
		{
			SelectRow(row);
			return true;
		}
		return false;
	}

	protected HOD_TypeEntry CurrentDetail()
	{
		HOD_TypeEntry e = m_Module.m_ClientDetail;
		HOD_TypeResults res = m_Module.m_ClientResults;
		if (!e || m_Selected < 0 || m_Selected >= res.entries.Count() || res.entries[m_Selected].name != e.name)
			return null;
		return e;
	}

	void Click_CopyName(UIEvent eid, UIActionBase action)
	{
		HOD_TypeEntry e = CurrentDetail();
		if (eid != UIEvent.CLICK || !e)
			return;
		GetGame().CopyToClipboard(e.name);
		COTCreateLocalAdminNotification(new StringLocaliser("Copied " + e.name));
	}

	void Click_CopyXml(UIEvent eid, UIActionBase action)
	{
		HOD_TypeEntry e = CurrentDetail();
		if (eid != UIEvent.CLICK || !e)
			return;
		GetGame().CopyToClipboard(e.raw);
		COTCreateLocalAdminNotification(new StringLocaliser("Copied XML for " + e.name));
	}
}
