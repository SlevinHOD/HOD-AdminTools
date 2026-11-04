// HOD Admin Tools - double-click a COT window's title bar to maximize / restore it.
// House of D, CC BY-SA 4.0. Applies the new size the same way COT's drag-resize does,
// so every form re-lays itself out through OnResize().

modded class JMWindowBase
{
	static const float HOD_MAX_MARGIN = 16;

	protected bool m_HOD_Maximized;
	protected float m_HOD_PrevX;
	protected float m_HOD_PrevY;
	protected float m_HOD_PrevW;
	protected float m_HOD_PrevH;

	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		if (w && w == m_TitleWrapper && button == MouseState.LEFT)
		{
			HOD_ToggleMaximize();
			return true;
		}
		return super.OnDoubleClick(w, x, y, button);
	}

	bool HOD_IsMaximized()
	{
		return m_HOD_Maximized;
	}

	void HOD_ToggleMaximize()
	{
		if (m_IsMinimized || m_IsAnimatingMinimize || m_IsAnimatingRestore)
			return;

		if (m_HOD_Maximized)
		{
			m_HOD_Maximized = false;
			HOD_ApplyRect(m_HOD_PrevX, m_HOD_PrevY, m_HOD_PrevW, m_HOD_PrevH);
			GetCOTWindowManager().SaveWindowLayout(this);
			return;
		}

		layoutRoot.GetPos(m_HOD_PrevX, m_HOD_PrevY);
		layoutRoot.GetSize(m_HOD_PrevW, m_HOD_PrevH);

		float screenW, screenH;
		GetGame().GetWorkspace().GetScreenSize(screenW, screenH);

		// keep COT's sidebar uncovered (it sits on the right edge when open)
		float sideBarW = 0;
		COTModule cotModule;
		if (CF_Modules<COTModule>.Get(cotModule))
		{
			JMCOTSideBar sideBar = cotModule.GetCOTSidebar();
			if (sideBar && sideBar.GetLayoutRoot() && sideBar.GetLayoutRoot().IsVisible())
			{
				float sbX, sbY;
				sideBar.GetLayoutRoot().GetScreenPos(sbX, sbY);
				if (sbX > 0 && sbX < screenW)
					sideBarW = screenW - sbX;
			}
		}

		float margin = JMUILayout.Px(HOD_MAX_MARGIN);
		float w = screenW - sideBarW - margin * 2;
		float h = screenH - margin * 2;
		float minW = JMUILayout.Px(RESIZE_MIN_WIDTH);
		if (w < minW)
			w = minW;

		m_HOD_Maximized = true;
		HOD_ApplyRect(margin, margin, w, h);
	}

	// Forms built with fixed pixel panels (e.g. Expansion's territory menu has a 1081x600
	// map container) don't follow the window. Any fixed-size descendant whose on-screen size
	// equals the old form size is resized with it; percentage children then follow on their own.
	protected void HOD_ResizePinned(Widget parent, float oldW, float oldH, float newW, float newH, int depth)
	{
		if (!parent || depth > 3)
			return;
		Widget child = parent.GetChildren();
		while (child)
		{
			int flags = child.GetFlags();
			bool exactW = (flags & WidgetFlags.HEXACTSIZE) != 0;
			bool exactH = (flags & WidgetFlags.VEXACTSIZE) != 0;
			if (exactW || exactH)
			{
				float sw, sh;
				child.GetScreenSize(sw, sh);
				if (Math.AbsFloat(sw - oldW) < 3 && Math.AbsFloat(sh - oldH) < 3)
				{
					float cw, ch;
					child.GetSize(cw, ch);
					if (exactW)
						cw = newW;
					if (exactH)
						ch = newH;
					child.SetSize(cw, ch);
				}
			}
			HOD_ResizePinned(child, oldW, oldH, newW, newH, depth + 1);
			child = child.GetSibling();
		}
	}

	protected void HOD_ApplyRect(float x, float y, float w, float h)
	{
		float oldW = 0;
		float oldH = 0;
		if (m_FormRoot)
			m_FormRoot.GetScreenSize(oldW, oldH);

		layoutRoot.SetSize(w, h);
		layoutRoot.SetPos(x, y, true);

		float contentH = h - m_TitleBarHeight;
		if (m_ContentWidget)
			m_ContentWidget.SetSize(w, contentH);
		if (m_FormRoot)
		{
			m_FormRoot.SetSize(w, contentH);
			if (oldW > 0 && oldH > 0)
				HOD_ResizePinned(m_FormRoot, oldW, oldH, w, contentH, 0);
		}
		if (m_ConfirmationPanel)
			m_ConfirmationPanel.SetSize(w, contentH);
		if (m_TitleWrapper)
			m_TitleWrapper.SetPos(0, 0, true);

		m_RestoreWidth = w;
		m_RestoreHeight = h;

		if (m_Form)
			m_Form.OnResize(w, contentH);
	}
}
