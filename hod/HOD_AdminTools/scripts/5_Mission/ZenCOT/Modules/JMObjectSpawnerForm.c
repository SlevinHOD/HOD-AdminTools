modded class JMObjectSpawnerForm
{
	string m_ZenDeleteAllEntityType;
	UIActionButton m_ZenShowOnMapButton;
	UIActionButton m_ZenDeleteAllButton;
	
	override void OnInit()
	{
		super.OnInit();

		// HOD: adapted to COT dev/1.30 (UIActionItemList + filter row); original placed these next to the old search box
		if (!m_ZenShowOnMapButton && m_FilterWrapper)
		{
			Widget zenRow = UIActionManager.CreateWrapSpacerCompact(m_FilterWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER);
			m_ZenShowOnMapButton = UIActionManager.CreateButton(zenRow, "#STR_ZenCOT_Map", this, "OnClick_ZenShowOnMap");
			m_ZenShowOnMapButton.SetWidth(0.3);
			m_ZenDeleteAllButton = UIActionManager.CreateButton(zenRow, "#STR_ZenCOT_MapClear", this, "OnClick_ZenDeleteAllButton");
			m_ZenDeleteAllButton.SetWidth(0.4);
			m_ZenDeleteAllButton.SetColor(COLOR_RED);
		}
	}
	
	void OnClick_ZenDeleteAllButton(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK) 
			return;

		JMMapModule mapMod;
		if (Class.CastTo(mapMod, GetModuleManager().GetModule(JMMapModule)))
		{
			mapMod.SetTempItemMarks(new array<ref JMMapTempMark>, "Markers");
		}
	}

	void OnClick_ZenShowOnMap(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK) 
			return;

		string selected = GetCurrentSelection(); // HOD: dev/1.30 list API
		if (selected == "" && m_SearchBox)
			selected = m_SearchBox.GetText();

		if (selected != "" && m_Module)
		{
			GetRPCManager().SendRPC("ZenCOT_RPC", "RPC_ZenShowOnMapRequest", new Param1<string>(selected), true);
		}
	}

	private void ZenDeleteAll_Yes(JMConfirmation confirmation)
	{
		GetRPCManager().SendRPC("ZenCOT_RPC", "RPC_ZenDeleteAllOnMap", new Param1<string>(m_ZenDeleteAllEntityType), true, null);
		m_ZenDeleteAllEntityType = "";

		if (m_Module && m_Module.m_AutoShow)
			m_Module.Hide();
	}

	private void ZenDeleteAll_No(JMConfirmation confirmation)
	{
		m_ZenDeleteAllEntityType = "";

		if (m_Module && m_Module.m_AutoShow)
			m_Module.Hide();
	}
}