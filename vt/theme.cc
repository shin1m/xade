#include "theme.h"

t_theme::t_theme(const SkFont& a_font, size_t a_border) : v_font(a_font), v_border(a_border),
v_color_foreground(SkColorSetARGB(255, 255, 255, 255)),
v_color_background_active(SkColorSetARGB(63, 0, 63, 127)),
v_color_background_inactive(SkColorSetARGB(0, 0, 0, 0)),
v_color_hovered(SkColorSetARGB(127, 63, 127, 255)),
v_color_pressed(SkColorSetARGB(255, 191, 127, 63)),
v_unit(v_font.getSize(), v_font.getSize()),
v_part_cursors{
	{"nw-resize"},
	{"n-resize"},
	{"ne-resize"},
	{"w-resize"},
	{"move"},
	{"e-resize"},
	{"sw-resize"},
	{"s-resize"},
	{"se-resize"}
},
v_cursor_text("text"), v_cursor_arrow("default")
{
	SkUnichar cs[] = {L'\ue5d2', L'\ue931', L'\ue930', L'\ue5d0', L'\ue5cd'};
	v_font.unicharsToGlyphs(cs, 5, v_glyphs);
}

void t_theme::f_draw(SkCanvas& a_canvas, SkGlyphID a_glyph, int a_x, int a_y, size_t a_state) const
{
	SkPaint paint;
	if (a_state) {
		paint.setColor(a_state == 1 ? v_color_hovered : v_color_pressed);
		a_canvas.drawIRect(SkIRect::MakePtSize({a_x, a_y}, v_unit), paint);
	}
	SkPoint point{};
	paint.setColor(v_color_foreground);
	a_canvas.drawGlyphs(1, &a_glyph, &point, {static_cast<float>(a_x), static_cast<float>(a_y + v_unit.fHeight)}, v_font, paint);
}

t_decoration::t_part t_decoration::f_part(t_frame& a_frame) const
{
	int width;
	int height;
	wl_egl_window_get_attached_size(a_frame, &width, &height);
	auto y = f_client().f_pointer_y();
	if (a_frame.f_is(XDG_TOPLEVEL_STATE_FULLSCREEN)) return y < 1 ? c_part__FULLSCREEN : c_part__CONTENT;
	auto [b, c] = v_theme.f_border(a_frame);
	auto x = f_client().f_pointer_x();
	if (x >= b && x < width - b && y >= b && y < height - b) {
		if (y >= c) return c_part__CONTENT;
		if (a_frame.f_has(XDG_TOPLEVEL_WM_CAPABILITIES_WINDOW_MENU) && x < b + v_theme.v_unit.fWidth) return c_part__MENU;
		auto bx = width - b - v_theme.v_unit.fWidth;
		if (x >= bx) return c_part__CLOSE;
		if (a_frame.f_has(XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN)) {
			bx -= v_theme.v_unit.fWidth;
			if (x >= bx) return c_part__FULLSCREEN;
		}
		if (a_frame.f_has(XDG_TOPLEVEL_WM_CAPABILITIES_MAXIMIZE)) {
			bx -= v_theme.v_unit.fWidth;
			if (x >= bx) return c_part__MAXIMIZE;
		}
		if (a_frame.f_has(XDG_TOPLEVEL_WM_CAPABILITIES_MINIMIZE)) {
			bx -= v_theme.v_unit.fWidth;
			if (x >= bx) return c_part__MINIMIZE;
		}
		return c_part__BAR;
	}
	auto part = [&](auto a_size, auto a_value)
	{
		return a_value < c ? 0 : a_value < a_size - c ? 1 : 2;
	};
	return static_cast<t_part>(part(width, x) + part(height, y) * 3);
}

void t_decoration::f_draw(t_frame& a_frame, SkCanvas& a_canvas, size_t a_width, size_t a_height)
{
	if (v_valid) return;
	v_valid = true;
	if (a_frame.f_is(XDG_TOPLEVEL_STATE_FULLSCREEN)) return;
	SkPaint paint;
	paint.setBlendMode(SkBlendMode::kSrc);
	paint.setColor(a_frame.f_is(XDG_TOPLEVEL_STATE_ACTIVATED) ? v_theme.v_color_background_active : v_theme.v_color_background_inactive);
	auto [b, c] = v_theme.f_border(a_frame);
	a_canvas.drawIRect(SkIRect::MakeXYWH(0, 0, a_width, c), paint);
	auto height = a_height - c - b;
	a_canvas.drawIRect(SkIRect::MakeXYWH(0, c, b, height), paint);
	a_canvas.drawIRect(SkIRect::MakeXYWH(a_width - b, c, b, height), paint);
	a_canvas.drawIRect(SkIRect::MakeXYWH(0, a_height - b, a_width, b), paint);
	auto draw = [&](auto a_part, int32_t a_x, int32_t a_y)
	{
		auto hovered = a_part == v_hovered ? 1 : 0;
		v_theme.f_draw(a_canvas, v_theme.f_glyphs()[a_part - c_part__MENU], a_x, a_y, v_pressed < c_part__CONTENT ? a_part == v_pressed ? hovered + 1 : 0 : hovered);
	};
	if (a_frame.f_has(XDG_TOPLEVEL_WM_CAPABILITIES_WINDOW_MENU)) draw(c_part__MENU, b, b);
	auto x = a_width - b - v_theme.v_unit.fWidth;
	draw(c_part__CLOSE, x, b);
	if (a_frame.f_has(XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN)) draw(c_part__FULLSCREEN, x -= v_theme.v_unit.fWidth, b);
	if (a_frame.f_has(XDG_TOPLEVEL_WM_CAPABILITIES_MAXIMIZE)) draw(c_part__MAXIMIZE, x -= v_theme.v_unit.fWidth, b);
	if (a_frame.f_has(XDG_TOPLEVEL_WM_CAPABILITIES_MINIMIZE)) draw(c_part__MINIMIZE, x -= v_theme.v_unit.fWidth, b);
}

void t_decoration::f_hook(t_frame& a_frame)
{
	auto hovered = [&](auto a_part)
	{
		if (a_part == v_hovered) return;
		v_hovered = a_part;
		v_valid = false;
		a_frame.f_request_frame();
	};
	auto hover = [&, hovered]
	{
		auto part = f_part(a_frame);
		hovered(part);
		if (part >= c_part__CONTENT) return false;
		f_client().f_cursor__(part < c_part__MENU ? &v_theme.v_part_cursors[part] : &v_theme.v_cursor_arrow);
		return true;
	};
	auto enter = a_frame.v_on_pointer_enter;
	a_frame.v_on_pointer_enter = [&, hover, enter]
	{
		if (!hover()) enter();
	};
	auto leave = a_frame.v_on_pointer_leave;
	a_frame.v_on_pointer_leave = [&, hovered, leave]
	{
		if (v_hovered < c_part__CONTENT)
			hovered(c_part__CONTENT);
		else
			leave();
	};
	a_frame.v_on_pointer_move = [&, hover, enter, leave, move = a_frame.v_on_pointer_move]
	{
		auto hovered = v_hovered;
		if (hover()) {
			if (hovered >= c_part__CONTENT) leave();
		} else {
			if (hovered < c_part__CONTENT)
				enter();
			else
				move();
		}
	};
	a_frame.v_on_button_press = [&, hovered, press = a_frame.v_on_button_press](auto a_button)
	{
		if (v_hovered >= c_part__CONTENT) return press(a_button);
		if (v_hovered < c_part__MENU) {
			uint32_t edges = XDG_TOPLEVEL_RESIZE_EDGE_NONE;
			switch (v_hovered) {
			case c_part__LEFT_TOP:
			case c_part__TOP:
			case c_part__RIGHT_TOP:
				edges |= XDG_TOPLEVEL_RESIZE_EDGE_TOP;
				break;
			case c_part__LEFT_BOTTOM:
			case c_part__BOTTOM:
			case c_part__RIGHT_BOTTOM:
				edges |= XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM;
				break;
			}
			switch (v_hovered) {
			case c_part__LEFT_TOP:
			case c_part__LEFT:
			case c_part__LEFT_BOTTOM:
				edges |= XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
				break;
			case c_part__RIGHT_TOP:
			case c_part__RIGHT:
			case c_part__RIGHT_BOTTOM:
				edges |= XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;
				break;
			}
			if (edges)
				a_frame.f_resize(static_cast<xdg_toplevel_resize_edge>(edges));
			else
				a_frame.f_move();
			return;
		}
		v_pressed = v_hovered;
		v_valid = false;
		a_frame.f_request_frame();
		a_frame.v_on_button_release = [&, move = a_frame.v_on_pointer_move, release = a_frame.v_on_button_release](auto)
		{
			auto pressed = v_pressed;
			v_pressed = c_part__CONTENT;
			v_valid = false;
			a_frame.f_request_frame();
			if (v_hovered == pressed) switch (v_hovered) {
			case c_part__MENU:
				{
					auto [b, c] = v_theme.f_border(a_frame);
					a_frame.f_show_window_menu(b, c);
				}
				break;
			case c_part__MINIMIZE:
				xdg_toplevel_set_minimized(a_frame);
				break;
			case c_part__MAXIMIZE:
				if (a_frame.f_is(XDG_TOPLEVEL_STATE_MAXIMIZED))
					xdg_toplevel_unset_maximized(a_frame);
				else
					xdg_toplevel_set_maximized(a_frame);
				break;
			case c_part__FULLSCREEN:
				if (a_frame.f_is(XDG_TOPLEVEL_STATE_FULLSCREEN))
					xdg_toplevel_unset_fullscreen(a_frame);
				else
					xdg_toplevel_set_fullscreen(a_frame, NULL);
				break;
			case c_part__CLOSE:
				if (auto& on = a_frame.v_on_close) on();
				break;
			}
			a_frame.v_on_pointer_move = move;
			move();
			a_frame.v_on_button_release = release;
		};
		a_frame.v_on_pointer_move = [&, hovered]
		{
			hovered(f_part(a_frame));
		};
	};
}
