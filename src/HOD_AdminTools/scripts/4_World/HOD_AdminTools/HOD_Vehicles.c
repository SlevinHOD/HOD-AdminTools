// HOD Admin Tools - keeps a list of every vehicle so admins can find them server-wide

class HOD_Vehicles
{
	// non-ref: entries are nulled automatically when the vehicle is deleted
	static ref array<CarScript> s_All = new array<CarScript>();

	static void Add(CarScript car)
	{
		if (s_All.Find(car) < 0)
			s_All.Insert(car);
	}

	static void Remove(CarScript car)
	{
		int i = s_All.Find(car);
		if (i >= 0)
			s_All.Remove(i);
	}

	static void Prune()
	{
		for (int i = s_All.Count() - 1; i >= 0; i--)
		{
			if (!s_All[i])
				s_All.Remove(i);
		}
	}

	// "owner|keys|locked" - Expansion data when available
	static string ExpansionInfo(CarScript car)
	{
		string owner = "-";
		string keys = "-";
		string locked = "-";
#ifdef EXPANSIONMODVEHICLE
		ExpansionVehicle ev = ExpansionVehicle.Get(car);
		if (ev)
		{
			if (ev.GetOwnerName() != "")
				owner = ev.GetOwnerName() + " (" + ev.GetOwnerUID() + ")";
			if (ev.HasKey())
				keys = "paired";
			else
				keys = "none";
			if (ev.IsLocked())
				locked = "LOCKED";
			else
				locked = "open";
		}
#endif
		return owner + "|" + keys + "|" + locked;
	}
}

modded class CarScript
{
	void CarScript()
	{
		HOD_Vehicles.Add(this);
	}

	void ~CarScript()
	{
		HOD_Vehicles.Remove(this);
	}
}
