// HOD Admin Tools - single-channel RPC transport.
// Every message = direction(int) + command(string) + string args.

class HOD_RPCHandler : Managed
{
	void OnHODRPC(PlayerIdentity sender, string cmd, array<string> args)
	{
	}
}

class HOD_RPC
{
	static const int TO_SERVER = 0;
	static const int TO_CLIENT = 1;
	static const int MAX_ARGS  = 8192;

	// weak references; owners keep the strong ref
	static HOD_RPCHandler s_ServerHandler;
	static HOD_RPCHandler s_ClientHandler;

	static void ToServer(string cmd, array<string> args = null)
	{
		ScriptRPC rpc = new ScriptRPC();
		int dir = TO_SERVER;
		rpc.Write(dir);
		rpc.Write(cmd);
		WriteArgs(rpc, args);
		rpc.Send(null, HOD_RPC_ID, true, null);
	}

	static void ToClient(PlayerIdentity identity, string cmd, array<string> args = null)
	{
		if (!identity)
			return;
		ScriptRPC rpc = new ScriptRPC();
		int dir = TO_CLIENT;
		rpc.Write(dir);
		rpc.Write(cmd);
		WriteArgs(rpc, args);
		rpc.Send(null, HOD_RPC_ID, true, identity);
	}

	protected static void WriteArgs(ScriptRPC rpc, array<string> args)
	{
		int n = 0;
		if (args)
			n = args.Count();
		rpc.Write(n);
		for (int i = 0; i < n; i++)
		{
			string s = args[i];
			rpc.Write(s);
		}
	}

	static void Receive(PlayerIdentity sender, ParamsReadContext ctx)
	{
		int dir;
		string cmd;
		int n;
		if (!ctx.Read(dir))
			return;
		if (!ctx.Read(cmd))
			return;
		if (!ctx.Read(n))
			return;
		if (n < 0 || n > MAX_ARGS)
			return;

		array<string> args = new array<string>();
		for (int i = 0; i < n; i++)
		{
			string s;
			if (!ctx.Read(s))
				return;
			args.Insert(s);
		}

		if (dir == TO_SERVER)
		{
			if (GetGame().IsServer() && s_ServerHandler)
				s_ServerHandler.OnHODRPC(sender, cmd, args);
		}
		else if (dir == TO_CLIENT)
		{
			if (GetGame().IsClient() && s_ClientHandler)
				s_ClientHandler.OnHODRPC(sender, cmd, args);
		}
	}
}

modded class DayZGame
{
	override void OnRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx)
	{
		if (rpc_type == HOD_RPC_ID)
		{
			HOD_RPC.Receive(sender, ctx);
			return;
		}
		super.OnRPC(sender, target, rpc_type, ctx);
	}
}
