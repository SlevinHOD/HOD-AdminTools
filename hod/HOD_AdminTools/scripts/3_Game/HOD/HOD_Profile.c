// HOD Admin Tools - every profile file lives under $profile:HOD_AdminTools (House of D, CC BY-SA 4.0)
//
//   HOD_AdminTools\COT\           COT settings, logs, bans, map editor, anticheat, window layout
//   HOD_AdminTools\Permissions\   roles, players (was $profile:PermissionsFramework)
//   HOD_AdminTools\ObjectBuilder\ building sets
//
// On first start the old COT folders are COPIED across (originals are left as a backup).

class HOD_Profile
{
	static const string DIR_ROOT = "$profile:HOD_AdminTools\\";
	static const string DIR_COT = "$profile:HOD_AdminTools\\COT\\";
	static const string DIR_PERMISSIONS = "$profile:HOD_AdminTools\\Permissions\\";
	static const string OLD_COT = "$profile:CommunityOnlineTools";
	static const string OLD_PF = "$profile:PermissionsFramework";

	static bool s_Done;

	static void Init()
	{
		if (s_Done)
			return;
		s_Done = true;

		EnsureDir("$profile:HOD_AdminTools");
		Migrate(OLD_COT, "$profile:HOD_AdminTools\\COT");
		Migrate(OLD_PF, "$profile:HOD_AdminTools\\Permissions");
		EnsureDir("$profile:HOD_AdminTools\\COT");
		EnsureDir("$profile:HOD_AdminTools\\Permissions");
	}

	static void EnsureDir(string path)
	{
		if (!FileExist(path))
			MakeDirectory(path);
	}

	protected static void Migrate(string oldDir, string newDir)
	{
		if (FileExist(newDir) || !FileExist(oldDir))
			return;
		int n = CopyDir(oldDir, newDir, 0);
		Print("[HOD] Migrated " + n + " file(s) from " + oldDir + " to " + newDir + " (original kept)");
		FileHandle fh = OpenFile(oldDir + "\\MOVED_TO_HOD_AdminTools.txt", FileMode.WRITE);
		if (fh != 0)
		{
			FPrintln(fh, "HOD Admin Tools now reads and writes " + newDir);
			FPrintln(fh, "This folder was copied there on first start and is no longer used. Safe to delete once you've checked the copy.");
			CloseFile(fh);
		}
	}

	// FileAttr bit values aren't documented, so probe: a folder lists children, a file doesn't
	protected static bool IsDir(string path)
	{
		string child;
		FileAttr a;
		FindFileHandle h = FindFile(path + "\\*", child, a, FindFileFlags.DIRECTORIES);
		bool dir = child != "";
		if (h)
			CloseFindFile(h);
		return dir;
	}

	// recursive copy, returns files copied
	protected static int CopyDir(string src, string dst, int depth)
	{
		if (depth > 6)
			return 0;
		EnsureDir(dst);
		int copied = 0;
		string fileName;
		FileAttr attr;
		FindFileHandle h = FindFile(src + "\\*", fileName, attr, FindFileFlags.DIRECTORIES);
		if (!h)
			return 0;
		bool more = fileName != "";
		while (more)
		{
			if (fileName != "." && fileName != ".." && fileName != "")
			{
				string from = src + "\\" + fileName;
				string to = dst + "\\" + fileName;
				if (IsDir(from))
					copied += CopyDir(from, to, depth + 1);
				else if (CopyFile(from, to))
					copied++;
			}
			more = FindNextFile(h, fileName, attr);
		}
		CloseFindFile(h);
		return copied;
	}
}

modded class DayZGame
{
	void DayZGame()
	{
		HOD_Profile.Init();
	}
}
