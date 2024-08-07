#ifndef XADEVT__THEME_H
#define XADEVT__THEME_H

#include <xade/client.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkFont.h>

using namespace xade;

class t_theme
{
	const size_t v_border;
	SkGlyphID v_glyphs[5];

public:
	const SkFont v_font;
	const SkColor v_color_foreground;
	const SkColor v_color_background;
	const SkColor v_color_hovered;
	const SkColor v_color_pressed;
	const SkISize v_unit;
	const t_cursor v_part_cursors[9];
	const t_cursor v_cursor_text;
	const t_cursor v_cursor_arrow;

	t_theme(const SkFont& a_font, size_t a_border);
	std::tuple<size_t, size_t> f_border(t_frame& a_frame) const
	{
		if (a_frame.f_is(XDG_TOPLEVEL_STATE_FULLSCREEN)) return {};
		auto b = a_frame.f_is(XDG_TOPLEVEL_STATE_MAXIMIZED) ? 0 : v_border;
		return {b, b + v_unit.fHeight};
	}
	SkIRect f_frame(t_frame& a_frame) const
	{
		auto [b, c] = f_border(a_frame);
		return SkIRect::MakeLTRB(b, c, b, b);
	}
	const SkGlyphID* f_glyphs() const
	{
		return v_glyphs;
	}
	void f_draw(SkCanvas& a_canvas, SkGlyphID a_glyph, int a_x, int a_y, size_t a_state) const;
};

class t_decoration
{
	enum t_part
	{
		e_part__LEFT_TOP,
		e_part__TOP,
		e_part__RIGHT_TOP,
		e_part__LEFT,
		e_part__BAR,
		e_part__RIGHT,
		e_part__LEFT_BOTTOM,
		e_part__BOTTOM,
		e_part__RIGHT_BOTTOM,
		e_part__MENU,
		e_part__MINIMIZE,
		e_part__MAXIMIZE,
		e_part__FULLSCREEN,
		e_part__CLOSE,
		e_part__CONTENT
	};

	t_part v_hovered = e_part__CONTENT;
	t_part v_pressed = e_part__CONTENT;
	bool v_valid = false;

	t_part f_part(t_frame& a_frame) const;

public:
	const t_theme& v_theme;

	t_decoration(const t_theme& a_theme) : v_theme(a_theme)
	{
	}
	void f_invalidate()
	{
		v_valid = false;
	}
	void f_draw(t_frame& a_frame, SkCanvas& a_canvas, size_t a_width, size_t a_height);
	void f_hook(t_frame& a_frame);
};

#endif
