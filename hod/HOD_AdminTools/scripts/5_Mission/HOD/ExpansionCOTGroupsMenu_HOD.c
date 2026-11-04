// HOD Admin Tools - confirmation before destructive actions in Expansion's COT Groups menu
// (delete group, kick member, delete group marker). Same pattern as the territories tweak.
// House of D, CC BY-SA 4.0.
#ifdef EXPANSIONMODGROUPS
modded class ExpansionCOTGroupsMenu
{
	protected ButtonWidget m_HOD_DeleteGroupButton;
	protected ButtonWidget m_HOD_KickMemberButton;
	protected ButtonWidget m_HOD_DeleteMarkerButton;

	protected int m_HOD_GroupID = -1;
	protected string m_HOD_GroupName;
	protected string m_HOD_MemberName;
	protected string m_HOD_MarkerName;

	override void OnInit()
	{
		super.OnInit();
		m_HOD_DeleteGroupButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("delete_group_button"));
		m_HOD_KickMemberButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("kick_member_button"));
		m_HOD_DeleteMarkerButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("marker_delete_button"));
	}

	override void SetGroupInfo(ExpansionPartyData group)
	{
		super.SetGroupInfo(group);
		if (group)
		{
			m_HOD_GroupID = group.GetPartyID();
			m_HOD_GroupName = group.GetPartyName();
		}
	}

	override void SetMemberInfo(ExpansionPartyPlayerData player)
	{
		super.SetMemberInfo(player);
		if (player)
			m_HOD_MemberName = player.GetName();
	}

#ifdef EXPANSIONMODNAVIGATION
	override void EditMarker(ExpansionCOTGroupsMapMarker marker)
	{
		super.EditMarker(marker);
		m_HOD_MarkerName = "";
		if (marker && marker.GetMarkerData())
			m_HOD_MarkerName = marker.GetMarkerData().GetName();
	}
#endif

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (m_HOD_DeleteGroupButton && w == m_HOD_DeleteGroupButton)
		{
			if (m_HOD_GroupID < 0)
				return true;
			CreateConfirmation_Two(JMConfirmationType.INFO, "Delete group", "Delete group '" + m_HOD_GroupName + "' (ID " + m_HOD_GroupID + ")? This can't be undone.", "#STR_COT_GENERIC_NO", "HOD_CancelGroupAction", "#STR_COT_GENERIC_YES", "HOD_DoDeleteGroup");
			return true;
		}

		if (m_HOD_KickMemberButton && w == m_HOD_KickMemberButton)
		{
			CreateConfirmation_Two(JMConfirmationType.INFO, "Kick member", "Kick '" + m_HOD_MemberName + "' from group '" + m_HOD_GroupName + "'?", "#STR_COT_GENERIC_NO", "HOD_CancelGroupAction", "#STR_COT_GENERIC_YES", "HOD_DoKickMember");
			return true;
		}

		if (m_HOD_DeleteMarkerButton && w == m_HOD_DeleteMarkerButton)
		{
			CreateConfirmation_Two(JMConfirmationType.INFO, "Delete marker", "Delete group marker '" + m_HOD_MarkerName + "'?", "#STR_COT_GENERIC_NO", "HOD_CancelGroupAction", "#STR_COT_GENERIC_YES", "HOD_DoDeleteMarker");
			return true;
		}

		return super.OnClick(w, x, y, button);
	}

	// Calls the module directly: the menu's own DeleteGroup() clears the selection before
	// reading the party ID (Expansion bug), so replaying the click wouldn't delete anything.
	void HOD_DoDeleteGroup(JMConfirmation confirmation)
	{
		ExpansionCOTGroupModule groups;
		if (m_HOD_GroupID < 0 || !Class.CastTo(groups, GetModuleManager().GetModule(ExpansionCOTGroupModule)))
			return;
		groups.DeleteGroup(m_HOD_GroupID);
		m_HOD_GroupID = -1;
		groups.RequestGroups(ExpansionCOTGroupsMenuCallback.GroupsUpdate);
	}

	// Replay through Expansion's own handlers (selection is unchanged while the dialog is up)
	void HOD_DoKickMember(JMConfirmation confirmation)
	{
		super.OnClick(m_HOD_KickMemberButton, 0, 0, 0);
	}

	void HOD_DoDeleteMarker(JMConfirmation confirmation)
	{
		super.OnClick(m_HOD_DeleteMarkerButton, 0, 0, 0);
	}

	void HOD_CancelGroupAction(JMConfirmation confirmation)
	{
	}
}
#endif
