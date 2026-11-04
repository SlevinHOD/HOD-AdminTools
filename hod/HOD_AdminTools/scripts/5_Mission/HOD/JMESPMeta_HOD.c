modded class JMESPMeta
{
	bool m_HOD_WasAutoShown;

	override void Action_Delete( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMESPForm espForm;

		if ( !Class.CastTo( espForm, module.GetForm() ) )
		{
			// ESP settings panel is closed — open it so the confirmation dialog can attach to its window.
			// This mirrors the pattern used by JMObjectSpawnerModule.DeleteCursor().
			module.Show();

			if ( !Class.CastTo( espForm, module.GetForm() ) )
			{
				// Still no form (e.g. no permission to open module). Fall back to direct delete.
				super.Action_Delete( eid, action );
				return;
			}

			m_HOD_WasAutoShown = true;
		}
		else
		{
			m_HOD_WasAutoShown = false;
		}

		espForm.HOD_RequestDeleteObject( this );
	}
}
