// HOD Admin Tools - ask before deleting an object from the ESP action menu (House of D, CC BY-SA 4.0)
// Rewritten for COT dev/1.30, where deletion moved to JMESPActionMenu.DoDelete().

modded class JMESPActionMenu
{
	override protected void DoDelete()
	{
		if (!m_Meta || !m_Meta.module || !m_Meta.target)
		{
			super.DoDelete();
			return;
		}

		JMESPForm form;
		if (!Class.CastTo(form, m_Meta.module.GetForm()))
		{
			// ESP window closed - open it so the confirmation has a window to attach to
			m_Meta.module.Show();
			Class.CastTo(form, m_Meta.module.GetForm());
		}

		if (!form || !form.HOD_RequestDelete(m_Meta))
			super.DoDelete();
	}
}

modded class JMESPForm
{
	protected ref JMESPMeta m_HOD_PendingDelete;

	// false = couldn't show a confirmation, caller deletes directly
	bool HOD_RequestDelete(JMESPMeta meta)
	{
		m_HOD_PendingDelete = meta;
		string label = meta.target.GetType();
		JMConfirmation conf = CreateConfirmation_Two(JMConfirmationType.INFO, "Delete object", "Delete " + label + "?", "#STR_COT_GENERIC_NO", "HOD_CancelDelete", "#STR_COT_GENERIC_YES", "HOD_ConfirmDelete");
		if (!conf)
		{
			m_HOD_PendingDelete = null;
			return false;
		}
		return true;
	}

	void HOD_ConfirmDelete(JMConfirmation confirmation)
	{
		JMESPMeta meta = m_HOD_PendingDelete;
		m_HOD_PendingDelete = null;
		if (!meta || !meta.target || !meta.module)
			return;
		meta.module.DeleteObject(meta.target);
		meta.m_TargetDeleted = true;
	}

	void HOD_CancelDelete(JMConfirmation confirmation)
	{
		m_HOD_PendingDelete = null;
	}
}
