#ifdef EXPANSIONMODBASEBUILDING
modded class ExpansionCOTTerritoriesMenu
{
	private ButtonWidget m_HOD_DeleteTerritoryButton;
	private ButtonWidget m_HOD_DeleteObjectButton;
	private string m_HOD_TerritoryName;
	private string m_HOD_CachedObjectName;

	override void OnInit()
	{
		super.OnInit();

		m_HOD_DeleteTerritoryButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "delete_territory_button" ) );
		m_HOD_DeleteObjectButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "delete_object_button" ) );
	}

	override void SetTerritoryInfo( ExpansionTerritory territory )
	{
		super.SetTerritoryInfo( territory );
		m_HOD_TerritoryName = territory.GetTerritoryName();
	}

	override void SetTerritoryObjectInfo( ExpansionEntityMetaData obj )
	{
		super.SetTerritoryObjectInfo( obj );
		m_HOD_CachedObjectName = obj.m_DisplayName;
		if ( m_HOD_CachedObjectName == string.Empty )
			m_HOD_CachedObjectName = obj.m_ClassName;
	}

	override void SetContainerObjectInfo( ExpansionEntityMetaData obj )
	{
		super.SetContainerObjectInfo( obj );
		m_HOD_CachedObjectName = obj.m_DisplayName;
		if ( m_HOD_CachedObjectName == string.Empty )
			m_HOD_CachedObjectName = obj.m_ClassName;
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( m_HOD_DeleteTerritoryButton && w == m_HOD_DeleteTerritoryButton )
		{
			CreateConfirmation_Two(JMConfirmationType.INFO,"Delete Territory","Delete Territory: " + m_HOD_TerritoryName,"Cancel", "HOD_CancelDelete","Confirm", "HOD_DoDeleteTerritory");
			return true;
		}

		if ( m_HOD_DeleteObjectButton && w == m_HOD_DeleteObjectButton )
		{
			CreateConfirmation_Two(JMConfirmationType.INFO,"Delete Object","Delete: " + m_HOD_CachedObjectName,"Cancel", "HOD_CancelDelete","Confirm", "HOD_DoDeleteObject");
			return true;
		}

		return super.OnClick( w, x, y, button );
	}

	// Replay the territory delete click through the original handler.
	// m_CurrentTerritory is still valid since the confirmation modal blocked all input.
	void HOD_DoDeleteTerritory( JMConfirmation confirmation )
	{
		super.OnClick( m_HOD_DeleteTerritoryButton, 0, 0, 0 );
	}

	// Replay the object delete click through the original handler.
	void HOD_DoDeleteObject( JMConfirmation confirmation )
	{
		super.OnClick( m_HOD_DeleteObjectButton, 0, 0, 0 );
	}

	void HOD_CancelDelete( JMConfirmation confirmation )
	{
	}
}

#endif
