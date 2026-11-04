// HOD Admin Tools - client mission hooks (keybinds, admin handshake)

modded class MissionGameplay
{
	protected ref HOD_Client m_HODClient;
	protected int m_HODHelloTries;

	override void OnMissionStart()
	{
		super.OnMissionStart();
		m_HODClient = HOD_Client.Create();
		m_HODHelloTries = 0;
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(HOD_Hello, 4000, false);
	}

	// ask the server whether we're an admin; retry until it answers
	void HOD_Hello()
	{
		if (!m_HODClient || m_HODClient.m_HelloReceived)
			return;
		m_HODHelloTries++;
		m_HODClient.Send("hello");
		if (m_HODHelloTries < 6)
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(HOD_Hello, 10000, false);
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		if (!m_HODClient || !m_HODClient.m_IsAdmin)
			return;

		UAInput toggle = GetUApi().GetInputByName("UAHODToggleMenu");
		if (toggle && toggle.LocalPress())
			HOD_OpenMenu();

		UAInput tp = GetUApi().GetInputByName("UAHODTeleportCursor");
		if (tp && tp.LocalPress() && !GetGame().GetUIManager().GetMenu())
		{
			if (m_HODClient.Has(HOD_Perm.TELEPORT_SELF))
				HOD_AdminMenu.TeleportToCursor();
		}
	}

	void HOD_OpenMenu()
	{
		UIScriptedMenu top = GetGame().GetUIManager().GetMenu();
		if (top)
			return; // our menu closes itself; don't stack on other menus
		if (!GetGame().GetPlayer())
			return;
		HOD_AdminMenu menu = new HOD_AdminMenu();
		GetGame().GetUIManager().ShowScriptedMenu(menu, null);
	}
}
