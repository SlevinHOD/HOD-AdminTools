modded class DayZGame
{
	private bool m_HOD_IsShiftHeld;
	private bool m_HOD_IsCtrlHeld;
	private bool m_HOD_IsAltHeld;

	override void OnKeyPress( int key )
	{
		super.OnKeyPress( key );

		if ( key == KeyCode.KC_LSHIFT || key == KeyCode.KC_RSHIFT )
			m_HOD_IsShiftHeld = true;

		if ( key == KeyCode.KC_LCONTROL || key == KeyCode.KC_RCONTROL )
			m_HOD_IsCtrlHeld = true;

		if ( key == KeyCode.KC_LMENU || key == KeyCode.KC_RMENU )
			m_HOD_IsAltHeld = true;
	}

	override void OnKeyRelease( int key )
	{
		super.OnKeyRelease( key );

		if ( key == KeyCode.KC_LSHIFT || key == KeyCode.KC_RSHIFT )
			m_HOD_IsShiftHeld = false;

		if ( key == KeyCode.KC_LCONTROL || key == KeyCode.KC_RCONTROL )
			m_HOD_IsCtrlHeld = false;

		if ( key == KeyCode.KC_LMENU || key == KeyCode.KC_RMENU )
			m_HOD_IsAltHeld = false;
	}

	bool HOD_IsShiftHeld()
	{
		return m_HOD_IsShiftHeld;
	}

	bool HOD_IsCtrlHeld()
	{
		return m_HOD_IsCtrlHeld;
	}

	bool HOD_IsAltHeld()
	{
		return m_HOD_IsAltHeld;
	}
}
