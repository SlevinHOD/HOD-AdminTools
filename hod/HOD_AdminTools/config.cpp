class CfgPatches
{
	class HOD_AdminTools
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"JM_CF_Scripts", "JM_COT_Scripts"};
	};
};

class CfgMods
{
	class HOD_AdminTools
	{
		dir = "HOD_AdminTools";
		picture = "";
		action = "https://github.com/SlevinHOD/HOD-AdminTools";
		hideName = 1;
		hidePicture = 1;
		name = "HOD Admin Tools - HOD modules";
		credits = "House of D (Slevin, Scav) | ZenCOT features by Zenarchist";
		author = "House of D";
		authorID = "0";
		version = "0.2.0";
		extra = 0;
		type = "mod";
		inputs = "HOD_AdminTools/data/inputs.xml";
		dependencies[] = {"Game", "World", "Mission"};
		defines[] = {"HOD_ADMINTOOLS"};

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"HOD_AdminTools/scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"HOD_AdminTools/scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"HOD_AdminTools/scripts/5_Mission"};
			};
		};
	};
};
