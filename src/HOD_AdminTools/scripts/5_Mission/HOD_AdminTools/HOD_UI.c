// HOD Admin Tools - tiny widget builder on top of the primitive layouts

class HOD_UI
{
	static const string L = "HOD_AdminTools/GUI/layouts/";

	static Widget Make(string layout, Widget parent, float x, float y, float w, float h)
	{
		Widget wd = GetGame().GetWorkspace().CreateWidgets(L + layout + ".layout", parent);
		if (wd)
		{
			wd.SetPos(x, y);
			wd.SetSize(w, h);
		}
		return wd;
	}

	static Widget Panel(Widget parent, float x, float y, float w, float h, int color)
	{
		Widget p = Make("hod_panel", parent, x, y, w, h);
		if (p)
			p.SetColor(color);
		return p;
	}

	static Widget Frame(Widget parent, float x, float y, float w, float h)
	{
		return Make("hod_frame", parent, x, y, w, h);
	}

	static TextWidget Label(Widget parent, float x, float y, float w, float h, string text, int size = 16, int color = HOD_COL_WHITE, bool center = false)
	{
		string layout = "hod_label";
		if (center)
			layout = "hod_label_center";
		TextWidget t = TextWidget.Cast(Make(layout, parent, x, y, w, h));
		if (t)
		{
			t.SetText(text);
			t.SetTextExactSize(size);
			t.SetColor(color);
		}
		return t;
	}

	static ButtonWidget Button(Widget parent, float x, float y, float w, float h, string text)
	{
		ButtonWidget b = ButtonWidget.Cast(Make("hod_button", parent, x, y, w, h));
		if (b)
		{
			TextWidget lbl = TextWidget.Cast(b.FindAnyWidget("hod_btn_lbl"));
			if (lbl)
				lbl.SetText(text);
		}
		return b;
	}

	static void SetButtonText(ButtonWidget b, string text)
	{
		if (!b)
			return;
		TextWidget lbl = TextWidget.Cast(b.FindAnyWidget("hod_btn_lbl"));
		if (lbl)
			lbl.SetText(text);
	}

	static void SetButtonColor(ButtonWidget b, int color)
	{
		if (!b)
			return;
		Widget bg = b.FindAnyWidget("hod_btn_bg");
		if (bg)
			bg.SetColor(color);
	}

	static EditBoxWidget Edit(Widget parent, float x, float y, float w, float h, string text = "")
	{
		Widget bg = Make("hod_edit", parent, x, y, w, h);
		if (!bg)
			return null;
		EditBoxWidget e = EditBoxWidget.Cast(bg.FindAnyWidget("hod_edit_box"));
		if (e)
			e.SetText(text);
		return e;
	}

	static TextListboxWidget List(Widget parent, float x, float y, float w, float h)
	{
		Widget bg = Make("hod_list", parent, x, y, w, h);
		if (!bg)
			return null;
		return TextListboxWidget.Cast(bg.FindAnyWidget("hod_list_box"));
	}

	static ImageWidget Image(Widget parent, float x, float y, float w, float h, string path)
	{
		ImageWidget img = ImageWidget.Cast(Make("hod_image", parent, x, y, w, h));
		if (img)
		{
			img.LoadImageFile(0, path);
			img.SetImage(0);
		}
		return img;
	}

	static MapWidget Map(Widget parent, float x, float y, float w, float h)
	{
		return MapWidget.Cast(Make("hod_map", parent, x, y, w, h));
	}

	static string Trim(string s)
	{
		s.TrimInPlace();
		return s;
	}
}
