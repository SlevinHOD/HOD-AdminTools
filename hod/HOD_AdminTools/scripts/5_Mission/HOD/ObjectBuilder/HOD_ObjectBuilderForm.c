// HOD Admin Tools - Object Builder window (House of D, CC BY-SA 4.0). Written from scratch.

class HOD_ObjectBuilderForm : JMFormBase
{
	protected HOD_ObjectBuilderModule m_Module;

	protected TextListboxWidget m_SetsList;
	protected TextListboxWidget m_ObjectsList;
	protected TextListboxWidget m_ClassList;

	protected UIActionEditableText m_NewSetName;
	protected UIActionText m_ObjectsTitle;
	protected UIActionEditableText m_Search;
	protected UIActionText m_SelectedText;
	protected UIActionEditableVector m_PosEdit;
	protected UIActionEditableVector m_OriEdit;
	protected UIActionSelectBox m_StepBox;
	protected UIActionCheckbox m_BuildMode;

	protected ref array<string> m_ClassRows = new array<string>();
	protected static ref array<string> s_AllClasses;
	protected static ref array<string> s_AllLower;
	protected bool m_Refreshing;
	protected int m_LastSetRow = -2;
	protected int m_LastObjRow = -2;

	protected ref array<string> STEP_LABELS = {"Step 0.1 m / 1 deg", "Step 0.5 m / 5 deg", "Step 1 m / 15 deg", "Step 5 m / 45 deg"};
	protected ref array<float> STEP_M = {0.1, 0.5, 1.0, 5.0};
	protected ref array<float> STEP_DEG = {1.0, 5.0, 15.0, 45.0};

	protected override bool SetModule(JMRenderableModuleBase mdl)
	{
		return Class.CastTo(m_Module, mdl);
	}

	override void OnInit()
	{
		m_SetsList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("sets_list"));
		m_ObjectsList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("objects_list"));
		m_ClassList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("class_list"));
		// route list events straight to this form (they don't always bubble up)
		if (m_SetsList)
			m_SetsList.SetHandler(this);
		if (m_ObjectsList)
			m_ObjectsList.SetHandler(this);
		if (m_ClassList)
			m_ClassList.SetHandler(this);

		// --- sets column
		Widget setsHeader = layoutRoot.FindAnyWidget("sets_header");
		UIActionManager.CreateText(setsHeader, "BUILDING SETS", "");
		Widget newRow = UIActionManager.CreateGridSpacer(setsHeader, 1, 2);
		m_NewSetName = UIActionManager.CreateEditableText(newRow, "", this);
		UIActionButton create = UIActionManager.CreateButton(newRow, "Create set", this, "Click_CreateSet");
		create.SetColor(COLOR_GREEN);

		Widget sets = layoutRoot.FindAnyWidget("sets_wrapper");
		Widget setRow1 = UIActionManager.CreateGridSpacer(sets, 1, 2);
		UIActionManager.CreateButton(setRow1, "Open", this, "Click_OpenSet");
		UIActionManager.CreateButton(setRow1, "Refresh", this, "Click_Refresh");
		Widget setRow2 = UIActionManager.CreateGridSpacer(sets, 1, 2);
		UIActionManager.CreateButton(setRow2, "Spawn / Despawn", this, "Click_ToggleActive");
		UIActionManager.CreateButton(setRow2, "Teleport to", this, "Click_TeleportSet");
		UIActionButton del = UIActionManager.CreateButton(sets, "Delete set", this, "Click_DeleteSet");
		del.SetColor(COLOR_RED);

		// --- objects column
		Widget objHeader = layoutRoot.FindAnyWidget("objects_header");
		m_ObjectsTitle = UIActionManager.CreateText(objHeader, "OBJECTS", "no set open");

		Widget objs = layoutRoot.FindAnyWidget("objects_wrapper");
		Widget objRow1 = UIActionManager.CreateGridSpacer(objs, 1, 2);
		UIActionManager.CreateButton(objRow1, "Select at crosshair", this, "Click_SelectCursor");
		UIActionManager.CreateButton(objRow1, "Duplicate", this, "Click_Duplicate");
		UIActionButton delObj = UIActionManager.CreateButton(objs, "Delete object", this, "Click_DeleteObject");
		delObj.SetColor(COLOR_RED);
		Widget objRow2 = UIActionManager.CreateGridSpacer(objs, 1, 2);
		UIActionManager.CreateButton(objRow2, "Copy init.c", this, "Click_ExportInit");
		UIActionManager.CreateButton(objRow2, "Copy XML", this, "Click_ExportXml");
		m_BuildMode = UIActionManager.CreateCheckbox(objs, "Build mode hotkeys (click / wheel / Backspace)", this, "Click_BuildMode", m_Module.m_BuildMode);

		// --- add / edit column
		Widget search = layoutRoot.FindAnyWidget("search_wrapper");
		m_Search = UIActionManager.CreateEditableText(search, "Search:", this, "Change_Search");

		Widget edit = layoutRoot.FindAnyWidget("edit_wrapper");
		Widget addRow = UIActionManager.CreateGridSpacer(edit, 1, 2);
		UIActionButton place = UIActionManager.CreateButton(addRow, "Place at crosshair", this, "Click_Place");
		place.SetColor(COLOR_GREEN);
		UIActionButton copyObj = UIActionManager.CreateButton(addRow, "Copy object at crosshair", this, "Click_CopyCursorObject");
		m_SelectedText = UIActionManager.CreateText(edit, "Selected:", "none");
		m_PosEdit = UIActionManager.CreateEditableVector(edit, "Pos", this, "Click_SetPos", "Set");
		m_OriEdit = UIActionManager.CreateEditableVector(edit, "Ori", this, "Click_SetOri", "Set");
		m_StepBox = UIActionManager.CreateSelectionBox(edit, "", STEP_LABELS, this, "Change_Step");
		m_StepBox.SetSelectorWidth(1.0);
		m_StepBox.SetSelection(1, false);
		Widget nudge1 = UIActionManager.CreateGridSpacer(edit, 1, 4);
		UIActionManager.CreateButton(nudge1, "X-", this, "Click_XMinus");
		UIActionManager.CreateButton(nudge1, "X+", this, "Click_XPlus");
		UIActionManager.CreateButton(nudge1, "Z-", this, "Click_ZMinus");
		UIActionManager.CreateButton(nudge1, "Z+", this, "Click_ZPlus");
		Widget nudge2 = UIActionManager.CreateGridSpacer(edit, 1, 4);
		UIActionManager.CreateButton(nudge2, "Down", this, "Click_YMinus");
		UIActionManager.CreateButton(nudge2, "Up", this, "Click_YPlus");
		UIActionManager.CreateButton(nudge2, "Yaw-", this, "Click_YawMinus");
		UIActionManager.CreateButton(nudge2, "Yaw+", this, "Click_YawPlus");
		Widget nudge3 = UIActionManager.CreateGridSpacer(edit, 1, 2);
		UIActionManager.CreateButton(nudge3, "Snap to ground", this, "Click_Snap");
		UIActionManager.CreateButton(nudge3, "Move to crosshair", this, "Click_MoveCursor");

		if (!GetPermissionsManager().HasPermission(HOD_ObjectBuilderModule.PERM_EDIT))
		{
			m_NewSetName.Disable();
			place.Disable();
		}
	}

	// HOD: columns and lists follow the window size (drag-resize or title-bar double-click)
	override void OnResize(float w, float h)
	{
		super.OnResize(w, h);
		HOD_Layout(w, h);
	}

	protected void Place(string name, float x, float y, float w, float h)
	{
		Widget wd = layoutRoot.FindAnyWidget(name);
		if (!wd)
			return;
		wd.SetPos(x, y);
		wd.SetSize(w, h);
	}

	protected void HOD_Layout(float w, float h)
	{
		if (w < 600 || h < 400)
			return;
		float gap = 10;
		float colL = (w - gap * 2) * 0.31;
		float colM = (w - gap * 2) * 0.34;
		float colR = w - gap * 2 - colL - colM;
		float xM = colL + gap;
		float xR = xM + colM + gap;

		float setsListH = Math.Max(120, h - 64 - 130);
		Place("sets_header", 0, 0, colL, 30);
		Place("sets_list", 5, 64, colL - 10, setsListH);
		Place("sets_wrapper", 0, 68 + setsListH, colL, 30);

		float objListH = Math.Max(150, h - 34 - 175);
		Place("objects_header", xM, 0, colM, 30);
		Place("objects_list", xM + 5, 34, colM - 10, objListH);
		Place("objects_wrapper", xM, 38 + objListH, colM, 30);

		float classH = Math.Max(120, h - 34 - 350);
		Place("search_wrapper", xR, 0, colR, 30);
		Place("class_list", xR + 5, 34, colR - 10, classH);
		Place("edit_wrapper", xR, 38 + classH, colR, 30);
	}

	override void OnShow()
	{
		super.OnShow();
		float lw, lh;
		layoutRoot.GetSize(lw, lh);
		HOD_Layout(lw, lh);
		m_Module.Client_RequestList();
		if (m_Module.m_ClientSet)
			m_Module.Client_OpenSet(m_Module.m_ClientSet.name);
		BuildClassIndex();
		RefreshAll();
	}

	// ------------------------------------------------------------ data -> UI

	void RefreshAll()
	{
		if (!m_SetsList)
			return;
		m_Refreshing = true;

		m_SetsList.ClearItems();
		int selSet = -1;
		foreach (HOD_OBSetInfo info : m_Module.m_ClientList.sets)
		{
			string state = "  [off]";
			if (info.active)
				state = "  [live]";
			int row = m_SetsList.AddItem(info.name + "  (" + info.count + ")" + state, null, 0);
			if (info.name == m_Module.CurrentSetName())
				selSet = row;
		}
		if (selSet >= 0)
			m_SetsList.SelectRow(selSet);

		m_ObjectsList.ClearItems();
		HOD_OBSet obSet = m_Module.m_ClientSet;
		if (obSet)
		{
			string live = "despawned";
			if (obSet.active)
				live = "live";
			m_ObjectsTitle.SetText(obSet.name + " - " + obSet.objects.Count() + " objects, " + live);
			for (int i = 0; i < obSet.objects.Count(); i++)
			{
				HOD_OBObject o = obSet.objects[i];
				m_ObjectsList.AddItem(string.Format("%1  %2", i + 1, o.type), null, 0);
			}
			if (m_Module.m_SelectedIndex >= 0 && m_Module.m_SelectedIndex < m_ObjectsList.GetNumItems())
			{
				m_ObjectsList.SelectRow(m_Module.m_SelectedIndex);
				m_ObjectsList.EnsureVisible(m_Module.m_SelectedIndex);
			}
		}
		else
		{
			m_ObjectsTitle.SetText("no set open");
		}

		HOD_OBObject sel = m_Module.Selected();
		if (sel)
		{
			m_SelectedText.SetText(sel.type);
			m_PosEdit.SetValue(sel.position);
			m_OriEdit.SetValue(sel.orientation);
		}
		else
		{
			m_SelectedText.SetText("none");
		}
		m_LastSetRow = m_SetsList.GetSelectedRow();
		m_LastObjRow = m_ObjectsList.GetSelectedRow();
		m_Refreshing = false;
	}

	// Polled every frame by COT's window: selecting a set opens it, selecting an object selects it.
	// Doesn't depend on click events reaching the form.
	override void Update()
	{
		super.Update();
		if (!m_SetsList || m_Refreshing)
			return;

		int setRow = m_SetsList.GetSelectedRow();
		if (setRow != m_LastSetRow)
		{
			m_LastSetRow = setRow;
			string name = SelectedSetName();
			if (name != "" && name != m_Module.CurrentSetName())
				m_Module.Client_OpenSet(name);
		}

		int objRow = m_ObjectsList.GetSelectedRow();
		if (objRow != m_LastObjRow)
		{
			m_LastObjRow = objRow;
			if (objRow >= 0 && objRow != m_Module.m_SelectedIndex)
			{
				m_Module.m_SelectedIndex = objRow;
				RefreshAll();
			}
		}
	}

	override bool OnItemSelected(Widget w, int x, int y, int row, int column, int oldRow, int oldColumn)
	{
		if (m_Refreshing)
			return false;
		if (w == m_ObjectsList)
		{
			m_Module.m_SelectedIndex = row;
			RefreshAll();
			return true;
		}
		return false;
	}

	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		if (w == m_SetsList)
		{
			OpenSelectedSet();
			return true;
		}
		if (w == m_ClassList)
		{
			PlaceSelectedClass();
			return true;
		}
		return false;
	}

	protected string SelectedSetName()
	{
		int row = m_SetsList.GetSelectedRow();
		if (row < 0 || row >= m_Module.m_ClientList.sets.Count())
			return "";
		return m_Module.m_ClientList.sets[row].name;
	}

	protected bool SelectedSetActive()
	{
		int row = m_SetsList.GetSelectedRow();
		if (row < 0 || row >= m_Module.m_ClientList.sets.Count())
			return false;
		return m_Module.m_ClientList.sets[row].active;
	}

	protected void OpenSelectedSet()
	{
		string name = SelectedSetName();
		if (name != "")
			m_Module.Client_OpenSet(name);
	}

	// ------------------------------------------------------------ class search

	protected void BuildClassIndex()
	{
		if (s_AllClasses)
			return;
		s_AllClasses = new array<string>();
		s_AllLower = new array<string>();
		AddClasses("CfgVehicles");
		AddClasses("CfgNonAIVehicles");
	}

	protected void AddClasses(string root)
	{
		int n = GetGame().ConfigGetChildrenCount(root);
		for (int i = 0; i < n; i++)
		{
			string name;
			GetGame().ConfigGetChildName(root, i, name);
			if (GetGame().ConfigGetInt(root + " " + name + " scope") < 1)
				continue;
			string model;
			GetGame().ConfigGetText(root + " " + name + " model", model);
			if (model == "")
				continue;
			string lower = name;
			lower.ToLower();
			s_AllClasses.Insert(name);
			s_AllLower.Insert(lower);
		}
	}

	void Change_Search(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CHANGE)
			return;
		string q = m_Search.GetText();
		q.ToLower();
		q.TrimInPlace();
		m_ClassList.ClearItems();
		m_ClassRows.Clear();
		if (q.Length() < 2 || !s_AllClasses)
			return;
		for (int i = 0; i < s_AllClasses.Count(); i++)
		{
			if (s_AllLower[i].IndexOf(q) < 0)
				continue;
			m_ClassList.AddItem(s_AllClasses[i], null, 0);
			m_ClassRows.Insert(s_AllClasses[i]);
			if (m_ClassRows.Count() >= 300)
				break;
		}
	}

	// selected search result, or the typed text if it's an exact class name
	protected string ChosenClass()
	{
		int row = m_ClassList.GetSelectedRow();
		if (row >= 0 && row < m_ClassRows.Count())
			return m_ClassRows[row];
		string typed = m_Search.GetText();
		typed.TrimInPlace();
		if (typed != "" && (GetGame().ConfigIsExisting("CfgVehicles " + typed) || GetGame().ConfigIsExisting("CfgNonAIVehicles " + typed)))
			return typed;
		return "";
	}

	protected void PlaceSelectedClass()
	{
		string cls = ChosenClass();
		if (cls == "")
		{
			COTCreateLocalAdminNotification(new StringLocaliser("Object Builder: pick a class from the list, or type an exact class name"));
			return;
		}
		if (!m_Module.m_ClientSet)
		{
			COTCreateLocalAdminNotification(new StringLocaliser("Object Builder: open or create a set first (double-click it in the list)"));
			return;
		}
		vector ori = "0 0 0";
		Man player = GetGame().GetPlayer();
		if (player)
		{
			vector pOri = player.GetOrientation();
			ori[0] = pOri[0];
		}
		m_Module.Client_AddObject(cls, GetCursorPos(), ori);
		COTCreateLocalAdminNotification(new StringLocaliser("Object Builder: placing " + cls + " in " + m_Module.m_ClientSet.name));
	}

	// Add a copy of the world object under the crosshair (same class, position and rotation)
	void Click_CopyCursorObject(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;
		vector from = GetGame().GetCurrentCameraPosition();
		vector to = from + GetGame().GetCurrentCameraDirection() * 300;
		vector hitPos;
		vector hitNormal;
		int component;
		set<Object> hits = new set<Object>();
		DayZPhysics.RaycastRV(from, to, hitPos, hitNormal, component, hits, null, GetGame().GetPlayer(), false, false, ObjIntersectView, 0.0);
		Object obj = null;
		if (hits.Count() > 0)
			obj = hits.Get(0);
		if (!obj || obj.IsMan())
		{
			COTCreateLocalAdminNotification(new StringLocaliser("Object Builder: no object under the crosshair"));
			return;
		}
		string cls = obj.GetType();
		if (cls == "" || (!GetGame().ConfigIsExisting("CfgVehicles " + cls) && !GetGame().ConfigIsExisting("CfgNonAIVehicles " + cls)))
		{
			COTCreateLocalAdminNotification(new StringLocaliser("Object Builder: '" + obj.GetDebugNameNative() + "' has no spawnable class (map-baked object)"));
			return;
		}
		if (!m_Module.m_ClientSet)
		{
			COTCreateLocalAdminNotification(new StringLocaliser("Object Builder: open or create a set first"));
			return;
		}
		m_Module.Client_AddObject(cls, obj.GetPosition(), obj.GetOrientation());
		COTCreateLocalAdminNotification(new StringLocaliser("Object Builder: copied " + cls));
	}

	// ------------------------------------------------------------ buttons

	void Click_CreateSet(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;
		string name = m_NewSetName.GetText();
		name.TrimInPlace();
		if (name == "")
			return;
		m_Module.Client_CreateSet(name);
		m_NewSetName.SetText("");
	}

	void Click_OpenSet(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;
		if (SelectedSetName() == "")
		{
			COTCreateLocalAdminNotification(new StringLocaliser("Object Builder: click a set in the list first"));
			return;
		}
		OpenSelectedSet();
		COTCreateLocalAdminNotification(new StringLocaliser("Object Builder: opening " + SelectedSetName()));
	}

	void Click_Refresh(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;
		m_Module.Client_RequestList();
		if (m_Module.m_ClientSet)
			m_Module.Client_OpenSet(m_Module.m_ClientSet.name);
	}

	void Click_ToggleActive(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;
		string name = SelectedSetName();
		if (name != "")
			m_Module.Client_SetActive(name, !SelectedSetActive());
	}

	void Click_TeleportSet(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK || !m_Module.m_ClientSet || m_Module.m_ClientSet.objects.Count() == 0)
			return;
		JMTeleportModule tp;
		if (Class.CastTo(tp, GetModuleManager().GetModule(JMTeleportModule)))
			tp.Position(m_Module.m_ClientSet.Center() + "0 1 0");
	}

	void Click_DeleteSet(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;
		string name = SelectedSetName();
		if (name == "")
			return;
		CreateConfirmation_Two(JMConfirmationType.INFO, "Delete set", "Delete building set '" + name + "' and despawn its objects?", "#STR_COT_GENERIC_NO", "", "#STR_COT_GENERIC_YES", "DeleteSet_Confirmed");
	}

	void DeleteSet_Confirmed(JMConfirmation confirmation)
	{
		string name = SelectedSetName();
		if (name != "")
			m_Module.Client_DeleteSet(name);
	}

	void Click_SelectCursor(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;
		if (!m_Module.SelectAtCursor())
			COTCreateLocalAdminNotification(new StringLocaliser("No set object near the crosshair"));
	}

	void Click_Duplicate(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;
		HOD_OBObject o = m_Module.Selected();
		if (o)
			m_Module.Client_AddObject(o.type, o.position + Vector(StepM(), 0, 0), o.orientation);
	}

	void Click_DeleteObject(UIEvent eid, UIActionBase action)
	{
		if (eid == UIEvent.CLICK)
			m_Module.Client_RemoveSelected();
	}

	void Click_ExportInit(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK || !m_Module.m_ClientSet)
			return;
		GetGame().CopyToClipboard(m_Module.m_ClientSet.ExportInitC());
		COTCreateLocalAdminNotification(new StringLocaliser("init.c lines copied to clipboard"));
	}

	void Click_ExportXml(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK || !m_Module.m_ClientSet)
			return;
		GetGame().CopyToClipboard(m_Module.m_ClientSet.ExportXml());
		COTCreateLocalAdminNotification(new StringLocaliser("mapgrouppos XML copied to clipboard"));
	}

	void Click_BuildMode(UIEvent eid, UIActionBase action)
	{
		if (eid == UIEvent.CLICK)
			m_Module.m_BuildMode = m_BuildMode.IsChecked();
	}

	void Click_Place(UIEvent eid, UIActionBase action)
	{
		if (eid == UIEvent.CLICK)
			PlaceSelectedClass();
	}

	void Click_SetPos(UIEvent eid, UIActionBase action)
	{
		HOD_OBObject o = m_Module.Selected();
		if (eid == UIEvent.CLICK && o)
			m_Module.Client_UpdateSelected(m_PosEdit.GetValue(), o.orientation);
	}

	void Click_SetOri(UIEvent eid, UIActionBase action)
	{
		HOD_OBObject o = m_Module.Selected();
		if (eid == UIEvent.CLICK && o)
			m_Module.Client_UpdateSelected(o.position, m_OriEdit.GetValue());
	}

	void Change_Step(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CHANGE)
			return;
		m_Module.m_Step = StepM();
		m_Module.m_RotStep = StepDeg();
	}

	protected float StepM()
	{
		int i = m_StepBox.GetSelection();
		if (i < 0 || i >= STEP_M.Count())
			return 0.5;
		return STEP_M[i];
	}

	protected float StepDeg()
	{
		int i = m_StepBox.GetSelection();
		if (i < 0 || i >= STEP_DEG.Count())
			return 5;
		return STEP_DEG[i];
	}

	void Click_XMinus(UIEvent eid, UIActionBase action) { if (eid == UIEvent.CLICK) m_Module.Nudge(Vector(-StepM(), 0, 0), 0); }
	void Click_XPlus(UIEvent eid, UIActionBase action)  { if (eid == UIEvent.CLICK) m_Module.Nudge(Vector(StepM(), 0, 0), 0); }
	void Click_ZMinus(UIEvent eid, UIActionBase action) { if (eid == UIEvent.CLICK) m_Module.Nudge(Vector(0, 0, -StepM()), 0); }
	void Click_ZPlus(UIEvent eid, UIActionBase action)  { if (eid == UIEvent.CLICK) m_Module.Nudge(Vector(0, 0, StepM()), 0); }
	void Click_YMinus(UIEvent eid, UIActionBase action) { if (eid == UIEvent.CLICK) m_Module.Nudge(Vector(0, -StepM(), 0), 0); }
	void Click_YPlus(UIEvent eid, UIActionBase action)  { if (eid == UIEvent.CLICK) m_Module.Nudge(Vector(0, StepM(), 0), 0); }
	void Click_YawMinus(UIEvent eid, UIActionBase action) { if (eid == UIEvent.CLICK) m_Module.Nudge("0 0 0", -StepDeg()); }
	void Click_YawPlus(UIEvent eid, UIActionBase action)  { if (eid == UIEvent.CLICK) m_Module.Nudge("0 0 0", StepDeg()); }

	void Click_Snap(UIEvent eid, UIActionBase action)
	{
		HOD_OBObject o = m_Module.Selected();
		if (eid != UIEvent.CLICK || !o)
			return;
		vector p = o.position;
		p[1] = GetGame().SurfaceY(p[0], p[2]);
		m_Module.Client_UpdateSelected(p, o.orientation);
	}

	void Click_MoveCursor(UIEvent eid, UIActionBase action)
	{
		HOD_OBObject o = m_Module.Selected();
		if (eid == UIEvent.CLICK && o)
			m_Module.Client_UpdateSelected(GetCursorPos(), o.orientation);
	}
}
