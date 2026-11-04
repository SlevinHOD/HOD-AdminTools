// HOD Admin Tools - server mission hooks

modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		HOD_Server.Create();
	}

	override void OnClientPrepareEvent(PlayerIdentity identity, out bool useDB, out vector pos, out float yaw, out int preloadTimeout)
	{
		super.OnClientPrepareEvent(identity, useDB, pos, yaw, preloadTimeout);
		HOD_Server srv = HOD_Server.Get();
		if (srv)
			srv.CheckBan(identity);
	}
}
