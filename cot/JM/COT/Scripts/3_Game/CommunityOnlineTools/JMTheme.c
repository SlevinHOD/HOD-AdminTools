// JMTheme - the single source of truth for every colour in COT. Palette is daisyUI
// "dark" converted from OKLCH to sRGB once. Surfaces come from the base ramp, borders
// and muted text are base-content at low alpha. Layers: RAMPS (raw), SEMANTIC, COMPONENT.
// Lives in 3_Game so every layer can read it. @note ARGB order: 0xAARRGGBB.
class JMTheme
{
	// -------------------------------------------------------------------------
	//  1. RAMPS
	// -------------------------------------------------------------------------

	//! Base ladder, darkest to lightest. INK_900..INK_600 are daisyUI's own
	//! neutral / base-300 / base-200 / base-100; the lighter steps are base-100
	//! mixed toward base-content, which is how daisyUI derives its own greys.
	static const int INK_900               = 0xFF080A08;  //!< neutral
	static const int INK_800               = 0xFF121612;  //!< base-300
	static const int INK_700               = 0xFF161B17;  //!< base-200
	static const int INK_600               = 0xFF1A201B;  //!< base-100
	static const int INK_500               = 0xFF30362F;
	static const int INK_400               = 0xFF454D46;
	static const int INK_300               = 0xFF646D65;
	static const int INK_200               = 0xFF8F998F;
	static const int INK_100               = 0xFFB8C4B9;
	static const int INK_50                = 0xFFEEF6EC;  //!< base-content
	static const int WHITE                 = 0xFFFFFFFF;
	static const int BLACK                 = 0xFF000000;

	//! Primary ladder. BLUE_500 is daisyUI's primary; darker steps sink toward
	//! base-300 so they stay usable as fills, lighter steps rise toward
	//! base-content. The names stay BLUE_* so existing call sites keep working -
	//! the hue is violet now.
	static const int BLUE_800              = 0xFF1C3A24;
	static const int BLUE_700              = 0xFF245030;
	static const int BLUE_600              = 0xFF25803C;
	static const int BLUE_500              = 0xFF2E9E4A;  //!< primary
	static const int BLUE_400              = 0xFF3CBF5C;
	static const int BLUE_300              = 0xFFD4AF37;
	static const int BLUE_200              = 0xFFA8E0B6;
	static const int BLUE_100              = 0xFFEEFBF1;  //!< primary-content

	//! Status colours, straight from the theme. Brighter than the old muted set
	//! because daisyUI's dark palette expects them to carry on a dark surface.
	static const int DANGER                = 0xFFFF627D;  //!< error
	static const int DANGER_DIM            = 0xFF4D0218;  //!< error-content
	static const int DANGER_BRIGHT         = 0xFFF98FA4;
	//! Fill for a FILLED destructive control. DANGER is tuned for text/icons and looks
	//! like a slab of pink at button size, so buttons fill with this darker step and
	//! SetColor() derives hover / pressed / ring shades from it.
	static const int DANGER_FILL           = 0xFF8C2438;
	static const int WARNING               = 0xFFFCB700;  //!< warning
	static const int WARNING_DIM           = 0xFF793205;  //!< warning-content
	static const int SUCCESS               = 0xFF00D390;  //!< success
	static const int SUCCESS_DIM           = 0xFF004C39;  //!< success-content
	//! Fill for a FILLED confirming control, the green twin of DANGER_FILL. SUCCESS is
	//! tuned for text/icons and looks like neon at button size, so buttons fill with
	//! this deeper step and SetColor() derives hover / pressed / ring shades from it.
	static const int SUCCESS_FILL          = 0xFF0C6B4F;
	static const int INFO                  = 0xFF00BAFE;  //!< info

	//! The remaining theme roles, kept whole so a call site that genuinely wants
	//! daisyUI's secondary or accent can name it.
	static const int SECONDARY             = 0xFFF43098;  //!< secondary
	static const int SECONDARY_CONTENT     = 0xFFF9E4F0;  //!< secondary-content
	static const int ACCENT_TEAL           = 0xFF00D3BB;  //!< accent
	static const int ACCENT_TEAL_DIM       = 0xFF084D49;  //!< accent-content
	static const int NEUTRAL               = 0xFF080A08;  //!< neutral
	static const int NEUTRAL_CONTENT       = 0xFFE4E4E7;  //!< neutral-content
	static const int INFO_CONTENT          = 0xFF042E49;  //!< info-content
	static const int SUCCESS_CONTENT       = 0xFF004C39;  //!< success-content
	static const int WARNING_CONTENT       = 0xFF793205;  //!< warning-content
	static const int ERROR_CONTENT         = 0xFF4D0218;  //!< error-content

	//! Fully transparent - use instead of a bare 0 so intent is readable.
	static const int TRANSPARENT           = 0x00000000;

	// -------------------------------------------------------------------------
	//  2. SEMANTIC
	// -------------------------------------------------------------------------

	//! Surfaces, from the deepest hole to the highest floating panel. daisyUI
	//! paints these opaque; the mod keeps a little translucency because the game
	//! world sits behind every window.
	static const int SURFACE_SUNKEN        = 0xF2121612;  //!< inside an input field
	static const int SURFACE_BASE          = 0xE6161B17;  //!< a form's body
	static const int SURFACE_RAISED        = 0xF01F2620;  //!< a button, a tile
	static const int SURFACE_HEADER        = 0xF2121612;  //!< section + tab headers
	static const int SURFACE_OVERLAY       = 0xF71A201B;  //!< popups, flyouts, menus

	//! Panel backing for the sidebar and its flyouts. Enfusion cannot blur what
	//! is behind a widget, so this is a near-opaque base rather than a true
	//! frosted pane - it keeps the menu legible over any backdrop.
	static const int SURFACE_GLASS         = 0xF2161B17;
	static const int SURFACE_GLASS_HEADER  = 0xF7121612;
	static const int SURFACE_GLASS_EDGE    = 0x26EEF6EC;  //!< lit hairline under the header

	//! Sidebar entry tint marking a module whose window is currently open.
	static const int MENU_ACTIVE           = 0x592E9E4A;

	//! Borders and rules - base-content at low alpha, per daisyUI.
	static const int BORDER_SUBTLE         = 0x24EEF6EC;
	static const int BORDER_BASE           = 0x2EEEF6EC;
	static const int BORDER_STRONG         = 0x4DEEF6EC;
	static const int BORDER_ACCENT         = 0xFF2E9E4A;

	//! The ring around an interactive pill. daisyUI marks focus and selection
	//! with the primary colour, not with a separate hue.
	static const int BORDER_RING           = 0xFF2E9E4A;
	static const int BORDER_RING_HOVER     = 0xFFD4AF37;

	//! Divider rules, darkest to lightest.
	static const int DIVIDER_DARK          = 0xFF121612;
	static const int DIVIDER_MEDIUM        = 0x1AEEF6EC;
	static const int DIVIDER_LIGHT         = 0x2EEEF6EC;

	//! Text. Secondary / muted / disabled are base-content faded, matching how
	//! daisyUI expresses de-emphasis with opacity.
	static const int TEXT_PRIMARY          = 0xFFEEF6EC;  //!< base-content
	static const int TEXT_SECONDARY        = 0xB3EEF6EC;
	static const int TEXT_MUTED            = 0x80EEF6EC;
	static const int TEXT_DISABLED         = 0x59EEF6EC;
	static const int TEXT_ON_ACCENT        = 0xFFEEFBF1;  //!< primary-content

	//! Interactive accent.
	static const int ACCENT                = 0xFF2E9E4A;
	static const int ACCENT_HOVER          = 0xFF3CBF5C;
	static const int ACCENT_PRESSED        = 0xFF25803C;
	//! Translucent accent wash, for hover backgrounds over a dark surface.
	static const int ACCENT_WASH           = 0x402E9E4A;
	static const int ACCENT_WASH_STRONG    = 0x732E9E4A;

	//! Dim layer drawn over a disabled control.
	static const int DISABLED_OVERLAY      = 0xA6121612;

	// -------------------------------------------------------------------------
	//  3. COMPONENT
	// -------------------------------------------------------------------------

	//! Window title bar, focused / unfocused.
	static const int TITLE_FOCUSED         = 0xFF121612;
	static const int TITLE_UNFOCUSED       = 0xFF080A08;
	//! Close button hover - the one destructive affordance, so it stays red.
	static const int TITLE_CLOSE_HOVER     = 0xCCFF627D;

	//! The rounded pill shared by every clickable surface (buttons, tabs, chips, icon
	//! cells). SELECTED_* is the primary violet, reserved for the one thing that is ON.
	//! BUTTON_* is daisyUI's default button: base-100 surface, faint base-content
	//! border, kept calm so the primary pill carries the meaning.
	static const int SELECTED_FILL         = 0xFF2E9E4A;
	static const int SELECTED_FILL_HOVER   = 0xFF3CBF5C;
	static const int SELECTED_OUTLINE      = 0xFFD4AF37;
	//! An unselected pill in a group draws no fill at all, so only the ON one
	//! carries weight.
	static const int SELECTED_FILL_OFF     = 0x00121612;
	static const int BUTTON_FILL           = 0xF01F2620;
	static const int BUTTON_FILL_HOVER     = 0xF738423A;
	//! Press has to read at a glance, so it drops well below base-300 rather
	//! than one step down from the resting fill, and the ring goes primary.
	static const int BUTTON_FILL_PRESSED   = 0xFF101410;
	static const int BUTTON_OUTLINE_PRESSED = 0xFF2E9E4A;
	static const int BUTTON_OUTLINE        = 0x2EEEF6EC;
	static const int BUTTON_OUTLINE_HOVER  = 0xFF2E9E4A;
	static const int BUTTON_TEXT           = 0xFFEEF6EC;
	static const int BUTTON_TEXT_INACTIVE  = 0x80EEF6EC;

	//! Pin toggle.
	static const int PIN_ACTIVE            = 0xFFFCB700;
	static const int PIN_INACTIVE          = 0x80EEF6EC;

	//! Sidebar rows: primary wash behind, hard primary bar on the left.
	static const int SIDEBAR_HOVER_FILL    = 0x4D2E9E4A;
	static const int SIDEBAR_PRESS_FILL    = 0x7325803C;
	static const int SIDEBAR_ACCENT        = 0xFFD4AF37;
	static const int SIDEBAR_TEXT_IDLE     = 0xFFEEF6EC;
	static const int SIDEBAR_TEXT_HOVER    = 0xFFEEFBF1;

	//! Value readouts that grade good -> caution -> bad (health, blood, ...).
	static const int VALUE_OK              = 0xFFEEF6EC;
	static const int VALUE_CAUTION         = 0xFFFCB700;
	static const int VALUE_BAD             = 0xFFFF627D;
	// -------------------------------------------------------------------------
	//  Helpers
	// -------------------------------------------------------------------------

	//! Same colour at a different opacity. alpha is 0..255.
	static int Alpha( int color, int alpha )
	{
		return ( color & 0x00FFFFFF ) | ( ( alpha & 0xFF ) << 24 );
	}

	//! Same colour at a fractional opacity, 0.0 .. 1.0.
	static int Fade( int color, float opacity )
	{
		int a = (int)Math.Clamp( opacity * 255, 0, 255 );
		return Alpha( color, a );
	}

	//! Linear interpolate every channel (alpha included) of two ARGB colours.
	//! t = 0 returns color, t = 1 returns target.
	//! @note the channel unpack MUST stay in int locals: a float assignment target makes
	//! the shift unresolvable ("Unknown operator '>>'").
	static int Mix( int color, int target, float t )
	{
		int ca = ( color  >> 24 ) & 0xFF;
		int cr = ( color  >> 16 ) & 0xFF;
		int cg = ( color  >>  8 ) & 0xFF;
		int cb =   color          & 0xFF;

		int ta = ( target >> 24 ) & 0xFF;
		int tr = ( target >> 16 ) & 0xFF;
		int tg = ( target >>  8 ) & 0xFF;
		int tb =   target         & 0xFF;

		int a = (int)Math.Clamp( ca + ( ta - ca ) * t, 0, 255 );
		int r = (int)Math.Clamp( cr + ( tr - cr ) * t, 0, 255 );
		int g = (int)Math.Clamp( cg + ( tg - cg ) * t, 0, 255 );
		int b = (int)Math.Clamp( cb + ( tb - cb ) * t, 0, 255 );

		return ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | b;
	}

	static int Lighten( int color, float t )
	{
		return Mix( color, WHITE, t );
	}

	static int Darken( int color, float t )
	{
		return Mix( color, BLACK, t );
	}
}
