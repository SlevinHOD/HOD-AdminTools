modded class JMESPForm
{
	private JMESPMeta m_HOD_PendingDeleteMeta;

	// Called from modded JMESPMeta when "Delete Object" is clicked on an individual ESP widget.
	// The form may have been auto-opened; meta.m_HOD_WasAutoShown tracks this so we can auto-close.
	void HOD_RequestDeleteObject( JMESPMeta meta )
	{
		m_HOD_PendingDeleteMeta = meta;

		string message = "Delete: " + meta.GetType();

		JMConfirmation conf = CreateConfirmation_Two(JMConfirmationType.INFO,"Delete Object",message,"Cancel", "HOD_CancelDeleteObject","Confirm", "HOD_ConfirmDeleteObject");

		if ( !conf )
		{
			// CF_WINDOWS or window unavailable — delete immediately and clean up.
			m_HOD_PendingDeleteMeta = null;

			if ( IsMissionOffline() || ( !meta.networkLow && !meta.networkHigh ) )
				meta.module.DeleteObject( meta.target );
			else
				meta.module.DeleteObject( meta.networkLow, meta.networkHigh );

			meta.m_TargetDeleted = true;
			HOD_AutoHideModule( meta );
		}
	}

	// Confirmation callback — perform the delete.
	void HOD_ConfirmDeleteObject( JMConfirmation confirmation )
	{
		if ( !m_HOD_PendingDeleteMeta )
			return;

		JMESPMeta meta = m_HOD_PendingDeleteMeta;
		m_HOD_PendingDeleteMeta = null;

		if ( IsMissionOffline() || ( !meta.networkLow && !meta.networkHigh ) )
			meta.module.DeleteObject( meta.target );
		else
			meta.module.DeleteObject( meta.networkLow, meta.networkHigh );

		meta.m_TargetDeleted = true;
		HOD_AutoHideModule( meta );
	}

	// Cancel callback — abort delete, hide module if it was auto-opened.
	void HOD_CancelDeleteObject( JMConfirmation confirmation )
	{
		if ( !m_HOD_PendingDeleteMeta )
			return;

		JMESPMeta meta = m_HOD_PendingDeleteMeta;
		m_HOD_PendingDeleteMeta = null;

		HOD_AutoHideModule( meta );
	}

	// Hides the ESP module window if it was auto-opened for the confirmation.
	// Deferred one tick so the confirmation fully closes before the window disappears.
	private void HOD_AutoHideModule( JMESPMeta meta )
	{
		if ( meta.m_HOD_WasAutoShown )
		{
			meta.m_HOD_WasAutoShown = false;
			g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).Call( meta.module.Hide );
		}
	}

	// Override "Delete Selected" button to always show confirmation.
	// Single object: shows classname. Multiple: shows count.
	override void Click_DeleteSelected( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		auto selected = JM_GetSelected();
		if ( !selected )
			return;

		auto objects = selected.GetObjects();
		int count = objects.Count();

		if ( count == 0 )
			return;

		if ( count == 1 )
		{
			JMSelectedObject firstObj = objects[0];

			string className;
			if ( firstObj && firstObj.obj )
				className = JMESPMeta.GetObjectType( firstObj.obj );

			if ( className == string.Empty )
				className = "Unknown";

			CreateConfirmation_Two(JMConfirmationType.INFO,"Delete Object","Delete: " + className,"Cancel", "","Confirm", "DeleteSelected");
		}
		else
		{
			CreateConfirmation_Two(JMConfirmationType.INFO,"#STR_COT_WARNING_OBJECTS_MESSAGE_HEADER",string.Format( Widget.TranslateString( "#STR_COT_WARNING_OBJECTS_MESSAGE_BODY" ), count.ToString() ),"#STR_COT_GENERIC_CANCEL", "","#STR_COT_GENERIC_CONFIRM", "DeleteSelected");
		}
	}
}
