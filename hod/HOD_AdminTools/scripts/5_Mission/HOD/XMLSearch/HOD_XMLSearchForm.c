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

	void Change_Search(UIEvent eid, UIActionBase action)
	{
		// search as you type (CHANGE is already debounced by UIActionEditableText)
		if (eid != UIEvent.CHANGE)
			return;
		string q = m_Search.GetText();
		q.TrimInPlace();
		if (q.Length() >= 3)
			m_Module.Client_Search(q);
	}

	void Click_Search(UIEvent eid, UIActionBase action)
	{
		if (eid == UIEvent.CLICK)
			m_Module.Client_Search(m_Search.GetText());
	}

	void Click_Reload(UIEvent eid, UIActionBase action)
	{
		if (eid == UIEvent.CLICK)
			m_Module.Client_Reload();
	}

	void ShowResults()
	{
		HOD_TypeResults res = m_Module.m_ClientResults;
		m_Results.ClearItems();
		m_Selected = -1;
		foreach (HOD_TypeEntry e : res.Entries)
		{
			string line = e.Name + "   n" + Num(e.Nominal) + " / m" + Num(e.Min);
			m_Results.AddItem(line, null, 0);
		}
		string shown = res.Total.ToString();
		if (res.Total > res.Entries.Count())
			shown = res.Entries.Count().ToString() + " of " + res.Total.ToString();
		m_Count.SetText(shown + " match(es) for '" + res.Query + "'");
		if (res.Entries.Count() > 0)
		{
			m_Results.SelectRow(0);
			ShowDetails(0);
		}
		else
			ShowDetails(-1);
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

	protected HOD_TypeEntry Entry(int i)
	{
		HOD_TypeResults res = m_Module.m_ClientResults;
		if (i < 0 || i >= res.Entries.Count())
			return null;
		return res.Entries[i];
	}

	protected void ShowDetails(int i)
	{
		m_Selected = i;
		HOD_TypeEntry e = Entry(i);
		if (!e)
		{
			foreach (UIActionText t : m_Details)
				t.SetText("");
			return;
		}
		m_Details[0].SetText(e.Name);
		m_Details[1].SetText(e.File);
		m_Details[2].SetText(Num(e.Nominal) + " / " + Num(e.Min));
		m_Details[3].SetText(Duration(e.Lifetime));
		m_Details[4].SetText(Duration(e.Restock));
		m_Details[5].SetText(Num(e.QuantMin) + " - " + Num(e.QuantMax));
		m_Details[6].SetText(Num(e.Cost));
		m_Details[7].SetText(e.Category);
		m_Details[8].SetText(e.Usages);
		m_Details[9].SetText(e.Values);
		m_Details[10].SetText(e.Tags);
		m_Details[11].SetText(e.Flags);
	}

	override bool OnItemSelected(Widget w, int x, int y, int row, int column, int oldRow, int oldColumn)
	{
		if (w == m_Results)
		{
			ShowDetails(row);
			return true;
		}
		return false;
	}

	void Click_CopyName(UIEvent eid, UIActionBase action)
	{
		HOD_TypeEntry e = Entry(m_Selected);
		if (eid != UIEvent.CLICK || !e)
			return;
		GetGame().CopyToClipboard(e.Name);
		COTCreateLocalAdminNotification(new StringLocaliser("Copied " + e.Name));
	}

	void Click_CopyXml(UIEvent eid, UIActionBase action)
	{
		HOD_TypeEntry e = Entry(m_Selected);
		if (eid != UIEvent.CLICK || !e)
			return;
		GetGame().CopyToClipboard(e.Raw);
		COTCreateLocalAdminNotification(new StringLocaliser("Copied XML for " + e.Name));
	}
}
