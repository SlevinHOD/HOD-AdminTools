// HOD Admin Tools - quick stats bar (House of D, CC BY-SA 4.0). Written from scratch.
//
// Shown at the bottom of the screen while the admin tools are switched on (COT active).
//   players online | godmode, invisible, ESP, free cam, unlimited ammo | position [copy]
//   | what the crosshair is on [copy] | FPS | in-game time
// Drag it by the grip (cursor must be visible, e.g. with the tools menu open);
// right-click the grip to put it back. Position is saved in $profile:HOD_AdminTools\StatsBar.json.

class HOD_StatsBarSettings
{
	float posX = -1;
	float posY = -1;
}

class HOD_StatsBar : ScriptedWidgetEventHandler
{
	static const string SETTINGS_FILE = "$profile:HOD_AdminTools\\StatsBar.json";
	static const string ICONS = "JM/COT/GUI/textures/icons/lucide/";
	static const int COLOR_ON = 0xFFD4AF37;  // HOD gold
	static const int COLOR_OFF = 0x598C998C; // dim grey-green
	static const int COLOR_ICON = 0xFF2E9E4A; // HOD green

	protected static ref HOD_StatsBar s_Instance;

	protected Widget m_Root;
	protected TextWidget m_Players, m_Pos, m_Target, m_Fps, m_Time;
	protected ImageWidget m_God, m_Invis, m_Esp, m_Cam, m_Ammo;
	protected Widget m_Grip, m_CopyPos, m_CopyTarget;
	protected ref HOD_StatsBarSettings m_Settings;

	protected float m_Refresh;
	protected float m_FpsTime;
	protected int m_FpsFrames;
	protected string m_PosText;
	protected string m_TargetText;

	protected bool m_Dragging;
	protected float m_DragOffX, m_DragOffY;

	static HOD_StatsBar Get()
	{
		if (!s_Instance)
			s_Instance = new HOD_StatsBar();
		return s_Instance;
	}

	static void Destroy()
	{
		s_Instance = null;
	}

	void HOD_StatsBar()
	{
		m_Root = GetGame().GetWorkspace().CreateWidgets("HOD_AdminTools/GUI/layouts/hod_statsbar.layout");
		if (!m_Root)
			return;
		m_Root.Show(false);

		Class.CastTo(m_Players, m_Root.FindAnyWidget("tx_players"));
		Class.CastTo(m_Pos, m_Root.FindAnyWidget("tx_pos"));
		Class.CastTo(m_Target, m_Root.FindAnyWidget("tx_target"));
		Class.CastTo(m_Fps, m_Root.FindAnyWidget("tx_fps"));
		Class.CastTo(m_Time, m_Root.FindAnyWidget("tx_time"));
		m_God = HOD_SetIcon("ic_god", "shield", COLOR_OFF);
		m_Invis = HOD_SetIcon("ic_invis", "eye-off", COLOR_OFF);
		m_Esp = HOD_SetIcon("ic_esp", "radar", COLOR_OFF);
		m_Cam = HOD_SetIcon("ic_cam", "video", COLOR_OFF);
		m_Ammo = HOD_SetIcon("ic_ammo", "infinity", COLOR_OFF);
		HOD_SetIcon("ic_players", "users", COLOR_ICON);
		HOD_SetIcon("ic_pos", "map-pin", COLOR_ICON);
		HOD_SetIcon("ic_target", "crosshair", COLOR_ICON);
		HOD_SetIcon("ic_fps", "gauge", COLOR_ICON);
		HOD_SetIcon("ic_time", "clock", COLOR_ICON);
		HOD_SetIcon("hud_grip_img", "grip-vertical", COLOR_ON);
		HOD_SetIcon("bt_copypos_img", "copy", COLOR_ON);
		HOD_SetIcon("bt_copytarget_img", "copy", COLOR_ON);

		m_Grip = m_Root.FindAnyWidget("hud_grip");
		m_CopyPos = m_Root.FindAnyWidget("bt_copypos");
		m_CopyTarget = m_Root.FindAnyWidget("bt_copytarget");
		m_Root.SetHandler(this);
		if (m_Grip)
			m_Grip.SetHandler(this);
		if (m_CopyPos)
			m_CopyPos.SetHandler(this);
		if (m_CopyTarget)
			m_CopyTarget.SetHandler(this);

		LoadSettings();
	}

	void ~HOD_StatsBar()
	{
		if (m_Root)
			m_Root.Unlink();
	}

	protected ImageWidget HOD_SetIcon(string name, string icon, int color)
	{
		ImageWidget img;
		if (!Class.CastTo(img, m_Root.FindAnyWidget(name)))
			return null;
		img.LoadImageFile(0, ICONS + icon + ".edds");
		img.SetImage(0);
		img.SetColor(color);
		return img;
	}

	// ---- settings ----

	protected void LoadSettings()
	{
		m_Settings = new HOD_StatsBarSettings();
		if (FileExist(SETTINGS_FILE))
			JsonFileLoader<HOD_StatsBarSettings>.JsonLoadFile(SETTINGS_FILE, m_Settings);
		ApplyPosition();
	}

	protected void SaveSettings()
	{
		if (!FileExist("$profile:HOD_AdminTools"))
			MakeDirectory("$profile:HOD_AdminTools");
		JsonFileLoader<HOD_StatsBarSettings>.JsonSaveFile(SETTINGS_FILE, m_Settings);
	}

	protected void ApplyPosition()
	{
		if (!m_Root || !m_Settings)
			return;
		int sw, sh;
		GetScreenSize(sw, sh);
		float w, h;
		m_Root.GetSize(w, h);
		float x = (sw - w) * 0.5; // default: bottom centre
		float y = sh - h - 14;
		if (m_Settings.posX >= 0 && m_Settings.posY >= 0)
		{
			x = m_Settings.posX;
			y = m_Settings.posY;
		}
		x = Math.Clamp(x, 0, Math.Max(sw - w, 0));
		y = Math.Clamp(y, 0, Math.Max(sh - h, 0));
		m_Root.SetPos(x, y);
	}

	// ---- per frame ----

	void Tick(float dt)
	{
		if (!m_Root)
			return;

		bool show = ShouldShow();
		if (show != m_Root.IsVisible())
		{
			m_Root.Show(show);
			if (show)
				ApplyPosition(); // picks up resolution changes
		}
		if (!show)
		{
			m_Dragging = false;
			return;
		}

		m_FpsFrames++;
		m_FpsTime += dt;

		if (m_Dragging)
			UpdateDrag();

		m_Refresh -= dt;
		if (m_Refresh > 0)
			return;
		m_Refresh = 0.2;
		Refresh();
	}

	protected bool ShouldShow()
	{
		if (!GetGame() || !GetGame().GetPlayer())
			return false;
		CommunityOnlineToolsBase cot = GetCommunityOnlineToolsBase();
		return cot && cot.IsActive();
	}

	protected void Refresh()
	{
		// players online (vanilla list is synced to every client, COT's roster is the fallback)
		int players = 0;
		if (ClientData.m_PlayerList && ClientData.m_PlayerList.m_PlayerList)
			players = ClientData.m_PlayerList.m_PlayerList.Count();
		if (players == 0 && GetPermissionsManager() && GetPermissionsManager().Players)
			players = GetPermissionsManager().Players.Count();
		m_Players.SetText(players.ToString());

		// own toggles
		JMPlayerInstance me;
		if (GetPermissionsManager())
			me = GetPermissionsManager().GetClientPlayer();
		bool god, invis, ammo;
		if (me)
		{
			god = me.HasGodMode();
			invis = me.HasInvisibility();
			ammo = me.HasUnlimitedAmmo();
		}
		SetState(m_God, god);
		SetState(m_Invis, invis);
		SetState(m_Ammo, ammo);
		SetState(m_Cam, CurrentActiveCamera != null);
		JMESPModule esp;
		bool espOn;
		if (Class.CastTo(esp, GetModuleManager().GetModule(JMESPModule)))
			espOn = esp.GetState() == JMESPState.View || esp.GetState() == JMESPState.Update;
		SetState(m_Esp, espOn);

		// position (free cam position while flying)
		vector pos = GetGame().GetPlayer().GetPosition();
		if (CurrentActiveCamera)
			pos = CurrentActiveCamera.GetPosition();
		m_PosText = string.Format("%1 %2 %3", F2(pos[0]), F2(pos[1]), F2(pos[2]));
		m_Pos.SetText(m_PosText);

		// what the crosshair is on
		m_TargetText = "";
		vector from = GetGame().GetCurrentCameraPosition();
		vector to = from + GetGame().GetCurrentCameraDirection() * 1000;
		vector hitPos, hitDir;
		int comp;
		set<Object> hits = new set<Object>();
		Object ignore = GetGame().GetPlayer();
		if (DayZPhysics.RaycastRV(from, to, hitPos, hitDir, comp, hits, null, ignore, true, false, ObjIntersectView, 0.1))
		{
			foreach (Object o : hits)
			{
				if (!o || o == ignore)
					continue;
				m_TargetText = ObjectName(o);
				if (m_TargetText != "")
					break;
			}
			if (m_TargetText != "")
				m_Target.SetText(m_TargetText + "  (" + Math.Round(vector.Distance(from, hitPos)).ToString() + " m)");
		}
		if (m_TargetText == "")
			m_Target.SetText("-");

		// fps over the last refresh window
		if (m_FpsTime > 0)
			m_Fps.SetText(Math.Round(m_FpsFrames / m_FpsTime).ToString());
		m_FpsFrames = 0;
		m_FpsTime = 0;

		int year, month, day, hour, minute;
		GetGame().GetWorld().GetDate(year, month, day, hour, minute);
		m_Time.SetText(Pad2(hour) + ":" + Pad2(minute));
	}

	static string ObjectName(Object o)
	{
		string n = o.GetType();
		if (n != "")
			return n;
		// map objects have no config class: show the model file
		n = o.GetShapeName();
		int slash = n.LastIndexOf("\\");
		if (slash < 0)
			slash = n.LastIndexOf("/");
		if (slash >= 0)
			n = n.Substring(slash + 1, n.Length() - slash - 1);
		return n;
	}

	static string F2(float v)
	{
		return (Math.Round(v * 100) / 100).ToString();
	}

	static string Pad2(int v)
	{
		if (v < 10)
			return "0" + v.ToString();
		return v.ToString();
	}

	protected void SetState(ImageWidget img, bool on)
	{
		if (!img)
			return;
		if (on)
			img.SetColor(COLOR_ON);
		else
			img.SetColor(COLOR_OFF);
	}

	// ---- mouse ----

	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (w != m_Grip)
			return false;
		if (button == MouseState.RIGHT)
		{
			m_Settings.posX = -1;
			m_Settings.posY = -1;
			ApplyPosition();
			SaveSettings();
			return true;
		}
		if (button != MouseState.LEFT)
			return false;
		float rx, ry;
		m_Root.GetScreenPos(rx, ry);
		m_DragOffX = x - rx;
		m_DragOffY = y - ry;
		m_Dragging = true;
		return true;
	}

	protected void UpdateDrag()
	{
		int mx, my;
		GetMousePos(mx, my);
		m_Settings.posX = Math.Max(mx - m_DragOffX, 0);
		m_Settings.posY = Math.Max(my - m_DragOffY, 0);
		ApplyPosition();
		if (!(GetMouseState(MouseState.LEFT) & MB_PRESSED_MASK))
		{
			m_Dragging = false;
			SaveSettings();
		}
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_CopyPos)
		{
			if (m_PosText != "")
			{
				GetGame().CopyToClipboard(m_PosText);
				COTCreateLocalAdminNotification(new StringLocaliser("Copied position " + m_PosText));
			}
			return true;
		}
		if (w == m_CopyTarget)
		{
			if (m_TargetText != "")
			{
				GetGame().CopyToClipboard(m_TargetText);
				COTCreateLocalAdminNotification(new StringLocaliser("Copied " + m_TargetText));
			}
			return true;
		}
		return w == m_Grip;
	}
}

modded class MissionGameplay
{
	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		HOD_StatsBar.Get().Tick(timeslice);
	}

	override void OnMissionFinish()
	{
		HOD_StatsBar.Destroy();
		super.OnMissionFinish();
	}
}
