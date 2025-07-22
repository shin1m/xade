#ifndef XADEVT__WINDOW_H
#define XADEVT__WINDOW_H

#include "terminal.h"
#include "theme.h"
#include <xade/converter.h>
#include <xade/skia.h>
#include <include/core/SkFontMetrics.h>
#include <include/core/SkRegion.h>

class t_window
{
	friend class t_buffer<t_window>;
	friend class t_terminal<t_window>;
	using t_attribute = t_buffer<t_window>::t_attribute;
	using t_cell = t_buffer<t_window>::t_cell;
	using t_row = t_buffer<t_window>::t_row;
	using t_code = t_terminal<t_window>::t_code;

	enum t_part
	{
		c_part__NONE,
		c_part__CONTENT,
		c_part__BUTTON_UP,
		c_part__BUTTON_DOWN,
		c_part__GAP_UP,
		c_part__GAP_DOWN,
		c_part__THUMB,
		c_part__OTHER
	};

	static t_code f_code(xkb_keysym_t a_key);

	t_converter<char32_t, char> v_utf32tomb{"utf-32", ""};
	t_converter<char, char> v_utf8tomb{"utf-8", ""};
	t_converter<char, wchar_t> v_utf8towc{"utf-8", "wchar_t"};
	t_terminal<t_window> v_buffer;
	int v_master;
	char v_mbs[MB_LEN_MAX * 256];
	size_t v_mbn = 0;
	std::mbstate_t v_mbstate{};
	SkColor v_colors[80];
	SkFont v_font;
	SkFont v_bold;
	SkFontMetrics v_metrics;
	SkISize v_unit;
	size_t v_width;
	size_t v_height;
	std::list<std::function<void()>>::iterator v_idle = f_client().v_on_idle.end();
	std::vector<wchar_t> v_preedit_text;
	size_t v_preedit_begin;
	size_t v_preedit_end;
	std::vector<std::unique_ptr<t_row>> v_preedit_rows;
	std::tuple<int, size_t> v_preedit_cursor;
	bool v_preedit_valid = false;
	int v_position = 0;
	GLuint v_renderbuffer = 0;
	GLuint v_framebuffer = 0;
	sk_sp<GrDirectContext> v_context;
	sk_sp<SkSurface> v_surface;
	SkUnichar* v_cs;
	SkGlyphID* v_glyphs;
	SkPoint* v_positions;
	SkRegion v_valid;
	bool v_bar_valid = false;
	std::function<void()> v_on_hover;
	t_part v_hovered = c_part__NONE;
	t_part v_pressed = c_part__NONE;
	const t_cursor* v_cursor_content = nullptr;
	std::shared_ptr<suisha::t_timer> v_repeat;

	void f_send(const char* a_cs, size_t a_n);
	int f_draw_row(SkCanvas& a_canvas, int a_x, int a_y, const t_row* a_row);
	void f_draw_row(SkCanvas& a_canvas, int a_y, const t_row* a_row);
	void f_draw_content(SkCanvas& a_canvas);
	void f_draw_bar(SkCanvas& a_canvas);
	void f__invalidate(int a_y, unsigned a_height)
	{
		v_valid.op(SkIRect::MakeXYWH(0, a_y, v_width, a_height), SkRegion::kDifference_Op);
		v_host.v_surface.f_request_frame();
	}
	void f_invalidate(int a_y, unsigned a_height);
	void f__scroll(int a_y, unsigned a_height, int a_dy);
	void f_scroll(int a_y, unsigned a_height, int a_dy);
	void f_log();
	int f_width(wchar_t a_c) const
	{
		return wcwidth(a_c);
	}
	void f_bell()
	{
	}
	unsigned f_content() const
	{
		return v_buffer.f_log_size() * v_unit.fHeight + v_height;
	}
	void f_position__(int a_position);
	void f_input_state();

public:
	struct t_host
	{
		t_surface& v_surface;
		std::function<void(int32_t&, int32_t&)>& v_on_measure;
		std::function<void(int32_t, int32_t)>& v_on_map;
		std::function<void()>& v_on_unmap;
		std::function<void()> v_swap_buffers;
		std::function<SkIRect()> v_measure_frame;
		std::function<void(SkCanvas&, size_t, size_t)> v_draw_frame;
		const SkColor v_bar_color;
		const SkISize v_bar_unit;
		std::function<void(SkCanvas&, size_t, int, int, size_t)> v_draw_button;
		const t_cursor& v_cursor_text;
		const t_cursor& v_cursor_arrow;
	};
	t_host& v_host;

	t_window(unsigned a_log, unsigned a_width, unsigned a_height, int a_master, const SkFont& a_font, t_host& a_host);
	~t_window();
};

#endif
