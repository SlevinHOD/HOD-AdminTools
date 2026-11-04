// HOD Admin Tools - action log (file + optional Discord webhook)

class HOD_WebhookCallback : RestCallback
{
	override void OnError(int errorCode)
	{
		Print("[HOD] Discord webhook error: " + errorCode);
	}

	override void OnTimeout()
	{
		Print("[HOD] Discord webhook timeout");
	}

	override void OnSuccess(string data, int dataSize)
	{
	}
}

class HOD_Logger
{
	static bool s_ToFile = true;
	static bool s_ToDiscord = false;
	static string s_Webhook = "";
	static string s_ServerName = "HOD DayZ";
	protected static ref HOD_WebhookCallback s_Callback;

	static void Configure(HOD_Settings settings)
	{
		s_ToFile = settings.LogToFile;
		s_ToDiscord = settings.LogToDiscord && settings.DiscordWebhookURL != "";
		s_Webhook = settings.DiscordWebhookURL;
		s_ServerName = settings.ServerName;
	}

	static void Log(string line)
	{
		Print("[HOD] " + line);
		if (!s_ToFile)
			return;
		if (!FileExist(HOD_LOG_DIR))
			MakeDirectory(HOD_LOG_DIR);
		string path = HOD_LOG_DIR + "/HOD_Admin_" + HOD_Time.DateTag() + ".log";
		FileHandle fh = OpenFile(path, FileMode.APPEND);
		if (fh != 0)
		{
			FPrintln(fh, "[" + HOD_Time.Stamp() + "] " + line);
			CloseFile(fh);
		}
	}

	// Admin action: logs to file and Discord
	static void Action(string adminName, string adminId, string action, string detail)
	{
		Log(string.Format("%1 (%2) %3: %4", adminName, adminId, action, detail));
		if (s_ToDiscord)
			SendDiscord(action, string.Format("**%1** (`%2`)\n%3", adminName, adminId, detail));
	}

	static void SendDiscord(string title, string description)
	{
		RestApi api = GetRestApi();
		if (!api)
			api = CreateRestApi();
		if (!api)
			return;
		RestContext ctx = api.GetRestContext(s_Webhook);
		if (!ctx)
			return;
		if (!s_Callback)
			s_Callback = new HOD_WebhookCallback();
		ctx.SetHeader("application/json");
		string json = "{\"username\":\"HOD Admin Tools\",\"embeds\":[{\"title\":\"" + Escape(title) + "\",\"description\":\"" + Escape(description) + "\",\"color\":13938487,\"footer\":{\"text\":\"" + Escape(s_ServerName) + " | " + HOD_Time.Stamp() + "\"}}]}";
		ctx.POST(s_Callback, "", json);
	}

	static string Escape(string s)
	{
		s.Replace("\\", "\\\\");
		s.Replace("\"", "\\\"");
		s.Replace("\n", "\\n");
		s.Replace("\r", "");
		s.Replace("\t", " ");
		return s;
	}
}
