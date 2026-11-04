class CfgPatches
{
	class HOD_AdminTools
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
	};
};

class CfgMods
{
	class HOD_AdminTools
	{
		dir = "HOD_AdminTools";
		picture = "";
		action = "";
		hideName = 0;
		hidePicture = 1;
		name = "HOD Admin Tools";
		credits = "House of D";
		author = "HOD";
		authorID = "0";
		version = "0.1.0";
		extra = 0;
		type = "mod";
		inputs = "HOD_AdminTools/data/inputs.xml";
		dependencies[] = {"Game", "World", "Mission"};

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
