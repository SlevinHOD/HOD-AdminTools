// HOD Admin Tools - Object Builder data (House of D, CC BY-SA 4.0)
// Saved as $profile:HOD_AdminTools/ObjectBuilder/<set name>.json

const string HOD_OB_DIR = "$profile:HOD_AdminTools/ObjectBuilder";

class HOD_OBObject
{
	string type;
	vector position;
	vector orientation; // yaw pitch roll

	void HOD_OBObject(string cls = "", vector pos = "0 0 0", vector ori = "0 0 0")
	{
		type = cls;
		position = pos;
		orientation = ori;
	}
}

class HOD_OBSet
{
	string name;
	bool active = true;
	ref array<ref HOD_OBObject> objects = new array<ref HOD_OBObject>();

	vector Center()
	{
		vector sum = "0 0 0";
		if (objects.Count() == 0)
			return sum;
		foreach (HOD_OBObject o : objects)
			sum = sum + o.position;
		return sum * (1.0 / objects.Count());
	}

	string ToJson()
	{
		JsonSerializer js = new JsonSerializer();
		string out_json;
		js.WriteToString(this, false, out_json);
		return out_json;
	}

	static HOD_OBSet FromJson(string json)
	{
		HOD_OBSet s = new HOD_OBSet();
		JsonSerializer js = new JsonSerializer();
		string err;
		if (!js.ReadFromString(s, json, err))
		{
			Print("[HOD] Object Builder: bad set data: " + err);
			return null;
		}
		return s;
	}

	// init.c style - paste into a mission's init.c (needs the helper printed in the header)
	string ExportInitC()
	{
		string s = "// HOD Object Builder export: " + name + "\n";
		s += "// void HOD_Spawn(string t, vector p, vector o) { Object obj = GetGame().CreateObjectEx(t, p, ECE_SETUP|ECE_UPDATEPATHGRAPH|ECE_CREATEPHYSICS|ECE_NOLIFETIME); if (obj) { obj.SetPosition(p); obj.SetOrientation(o); obj.Update(); } }\n";
		foreach (HOD_OBObject o : objects)
			s += string.Format("HOD_Spawn(\"%1\", \"%2\", \"%3\");\n", o.type, HOD_OBSet.Vec(o.position), HOD_OBSet.Vec(o.orientation));
		return s;
	}

	// mapgrouppos.xml style lines
	string ExportXml()
	{
		string s = "<!-- HOD Object Builder export: " + name + " -->\n";
		foreach (HOD_OBObject o : objects)
			s += string.Format("<group name=\"%1\" pos=\"%2\" rpy=\"%3 %4 %5\" a=\"%6\" />\n", o.type, HOD_OBSet.Vec(o.position), o.orientation[2], o.orientation[1], o.orientation[0], o.orientation[0]);
		return s;
	}

	static string Vec(vector v)
	{
		return string.Format("%1 %2 %3", v[0], v[1], v[2]);
	}
}

// list entry sent to clients
class HOD_OBSetInfo
{
	string name;
	bool active;
	int count;
}

class HOD_OBSetList
{
	ref array<ref HOD_OBSetInfo> sets = new array<ref HOD_OBSetInfo>();
}
