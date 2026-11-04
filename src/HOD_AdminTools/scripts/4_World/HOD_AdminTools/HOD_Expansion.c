// HOD Admin Tools - DayZ Expansion bridge.
// Everything here compiles with or without Expansion loaded: Expansion code is
// only referenced inside #ifdef blocks driven by Expansion's own defines.

class HOD_Expansion
{
	static bool IsLoaded()
	{
#ifdef EXPANSIONMODCORE
		return true;
#else
		return false;
#endif
	}

	static string Modules()
	{
		string s = "";
#ifdef EXPANSIONMODCORE
		s += "Core ";
#endif
#ifdef EXPANSIONMODVEHICLE
		s += "Vehicles ";
#endif
#ifdef EXPANSIONMODBASEBUILDING
		s += "BaseBuilding ";
#endif
#ifdef EXPANSIONMODMARKET
		s += "Market ";
#endif
#ifdef EXPANSIONMODGROUPS
		s += "Groups ";
#endif
#ifdef EXPANSIONMODAI
		s += "AI ";
#endif
#ifdef EXPANSIONMODQUESTS
		s += "Quests ";
#endif
#ifdef EXPANSIONMODHARDLINE
		s += "Hardline ";
#endif
#ifdef EXPANSIONMODSPAWNSELECTION
		s += "SpawnSelection ";
#endif
		if (s == "")
			return "none";
		return s;
	}

	// ---------------- Vehicles ----------------

	static bool UnlockVehicle(Object obj, out string msg)
	{
#ifdef EXPANSIONMODVEHICLE
		ExpansionVehicle vehicle = ExpansionVehicle.Get(obj);
		if (!vehicle)
		{
			msg = "Target is not an Expansion-managed vehicle";
			return false;
		}
		vehicle.ForceUnlock(ExpansionVehicleLockState.UNLOCKED);
		msg = "Vehicle unlocked";
		return true;
#else
		msg = "Expansion Vehicles is not loaded";
		return false;
#endif
	}

	static bool ResetKeys(Object obj, out string msg)
	{
#ifdef EXPANSIONMODVEHICLE
		ExpansionVehicle vehicle = ExpansionVehicle.Get(obj);
		if (!vehicle)
		{
			msg = "Target is not an Expansion-managed vehicle";
			return false;
		}
		array<ExpansionCarKey> keys = new array<ExpansionCarKey>();
		ExpansionCarKey.GetKeysForVehicle(vehicle, keys);
		foreach (ExpansionCarKey key : keys)
		{
			if (key)
				key.Unpair(true);
		}
		vehicle.ResetKeyPairing();
		msg = "Unpaired " + keys.Count().ToString() + " key(s), vehicle pairing reset";
		return true;
#else
		msg = "Expansion Vehicles is not loaded";
		return false;
#endif
	}

	static bool GiveKey(Object obj, PlayerBase admin, out string msg)
	{
		if (!admin)
		{
			msg = "Admin player not found";
			return false;
		}
#ifdef EXPANSIONMODVEHICLE
		ExpansionVehicle vehicle = ExpansionVehicle.Get(obj);
		if (!vehicle)
		{
			msg = "Target is not an Expansion-managed vehicle";
			return false;
		}
		ExpansionCarKey key = ExpansionCarKey.Cast(admin.GetInventory().CreateInInventory("ExpansionCarKey"));
		if (!key)
		{
			msg = "No inventory space for a key";
			return false;
		}
		vehicle.PairKey(key);
		msg = "Paired key created in your inventory";
		return true;
#else
		msg = "Expansion Vehicles is not loaded";
		return false;
#endif
	}

	// ---------------- Code locks ----------------

	static bool ClearCode(Object obj, out string msg)
	{
#ifdef EXPANSIONMODBASEBUILDING
		ItemBase item = ItemBase.Cast(obj);
		if (!item)
		{
			msg = "Target can't hold an Expansion code";
			return false;
		}
		string old = item.GetCode();
		item.SetCode("");
		if (old == "")
			msg = "No code was set - lock cleared/unlocked";
		else
			msg = "Code removed (was " + old + ")";
		return true;
#else
		msg = "Expansion BaseBuilding is not loaded";
		return false;
#endif
	}

	static bool ReadCode(Object obj, out string msg)
	{
#ifdef EXPANSIONMODBASEBUILDING
		ItemBase item = ItemBase.Cast(obj);
		if (!item)
		{
			msg = "Target can't hold an Expansion code";
			return false;
		}
		string code = item.GetCode();
		if (code == "")
			msg = "No Expansion code on " + obj.GetType();
		else
			msg = obj.GetType() + " code: " + code;
		return true;
#else
		msg = "Expansion BaseBuilding is not loaded";
		return false;
#endif
	}

	// ---------------- Market / ATM ----------------

	static bool AddATMMoney(string steamId, int amount, out string msg)
	{
#ifdef EXPANSIONMODMARKET
		ExpansionMarketModule market = ExpansionMarketModule.GetInstance();
		if (!market)
		{
			msg = "Expansion Market module not running";
			return false;
		}
		ExpansionMarketATM_Data data = market.GetPlayerATMData(steamId);
		if (!data)
		{
			msg = "Player has no ATM account yet (they must open an ATM once)";
			return false;
		}
		data.AddMoney(amount);
		if (data.MoneyDeposited < 0)
			data.MoneyDeposited = 0;
		data.Save();
		msg = "ATM balance now " + data.MoneyDeposited.ToString();
		return true;
#else
		msg = "Expansion Market is not loaded";
		return false;
#endif
	}

	// ---------------- Notifications ----------------

	// Returns true if an Expansion toast was sent (so the caller can skip the vanilla one)
	static bool Notify(PlayerIdentity identity, string title, string text)
	{
#ifdef EXPANSIONMODCORE
		if (!identity)
			return false;
		ExpansionNotification(title, text, "", HOD_COL_GOLD, 6).Create(identity);
		return true;
#else
		return false;
#endif
	}
}
