#include "window.h"
#include <sys/ioctl.h>

t_window::t_code t_window::f_code(xkb_keysym_t a_key)
{
	switch (a_key) {
	case XKB_KEY_BackSpace:
		return t_code::c_BACK_SPACE;
	case XKB_KEY_Tab:
		return t_code::c_TAB;
	case XKB_KEY_F1:
		return t_code::c_F1;
	case XKB_KEY_F2:
		return t_code::c_F2;
	case XKB_KEY_F3:
		return t_code::c_F3;
	case XKB_KEY_F4:
		return t_code::c_F4;
	case XKB_KEY_F5:
		return t_code::c_F5;
	case XKB_KEY_F6:
		return t_code::c_F6;
	case XKB_KEY_F7:
		return t_code::c_F7;
	case XKB_KEY_F8:
		return t_code::c_F8;
	case XKB_KEY_F9:
		return t_code::c_F9;
	case XKB_KEY_F10:
		return t_code::c_F10;
	case XKB_KEY_F11:
		return t_code::c_F11;
	case XKB_KEY_F12:
		return t_code::c_F12;
	case XKB_KEY_F13:
		return t_code::c_F13;
	case XKB_KEY_F14:
		return t_code::c_F14;
	case XKB_KEY_F15:
		return t_code::c_F15;
	case XKB_KEY_F16:
		return t_code::c_F16;
	case XKB_KEY_F17:
		return t_code::c_F17;
	case XKB_KEY_F18:
		return t_code::c_F18;
	case XKB_KEY_F19:
		return t_code::c_F19;
	case XKB_KEY_F20:
		return t_code::c_F20;
	case XKB_KEY_F21:
		return t_code::c_F21;
	case XKB_KEY_F22:
		return t_code::c_F22;
	case XKB_KEY_F23:
		return t_code::c_F23;
	case XKB_KEY_F24:
		return t_code::c_F24;
	case XKB_KEY_Delete:
		return t_code::c_DELETE;
	case XKB_KEY_Home:
		return t_code::c_HOME;
	case XKB_KEY_Left:
		return t_code::c_LEFT;
	case XKB_KEY_Up:
		return t_code::c_UP;
	case XKB_KEY_Right:
		return t_code::c_RIGHT;
	case XKB_KEY_Down:
		return t_code::c_DOWN;
	case XKB_KEY_Page_Up:
		return t_code::c_PRIOR;
	case XKB_KEY_Page_Down:
		return t_code::c_NEXT;
	case XKB_KEY_End:
		return t_code::c_END;
	case XKB_KEY_Begin:
		return t_code::c_BEGIN;
	case XKB_KEY_Insert:
		return t_code::c_INSERT;
	case XKB_KEY_KP_Space:
		return t_code::c_KP_SPACE;
	case XKB_KEY_KP_Tab:
		return t_code::c_KP_TAB;
	case XKB_KEY_KP_Enter:
		return t_code::c_KP_ENTER;
	case XKB_KEY_KP_F1:
		return t_code::c_KP_F1;
	case XKB_KEY_KP_F2:
		return t_code::c_KP_F2;
	case XKB_KEY_KP_F3:
		return t_code::c_KP_F3;
	case XKB_KEY_KP_F4:
		return t_code::c_KP_F4;
	case XKB_KEY_KP_Home:
		return t_code::c_KP_HOME;
	case XKB_KEY_KP_Left:
		return t_code::c_KP_LEFT;
	case XKB_KEY_KP_Up:
		return t_code::c_KP_UP;
	case XKB_KEY_KP_Right:
		return t_code::c_KP_RIGHT;
	case XKB_KEY_KP_Down:
		return t_code::c_KP_DOWN;
	case XKB_KEY_KP_Prior:
		return t_code::c_KP_PRIOR;
	case XKB_KEY_KP_Next:
		return t_code::c_KP_NEXT;
	case XKB_KEY_KP_End:
		return t_code::c_KP_END;
	case XKB_KEY_KP_Begin:
		return t_code::c_KP_BEGIN;
	case XKB_KEY_KP_Insert:
		return t_code::c_KP_INSERT;
	case XKB_KEY_KP_Delete:
		return t_code::c_KP_DELETE;
	case XKB_KEY_KP_Equal:
		return t_code::c_KP_EQUAL;
	case XKB_KEY_KP_Multiply:
		return t_code::c_KP_MULTIPLY;
	case XKB_KEY_KP_Add:
		return t_code::c_KP_ADD;
	case XKB_KEY_KP_Separator:
		return t_code::c_KP_SEPARATOR;
	case XKB_KEY_KP_Subtract:
		return t_code::c_KP_SUBTRACT;
	case XKB_KEY_KP_Decimal:
		return t_code::c_KP_DECIMAL;
	case XKB_KEY_KP_Divide:
		return t_code::c_KP_DIVIDE;
	case XKB_KEY_KP_0:
		return t_code::c_KP_0;
	case XKB_KEY_KP_1:
		return t_code::c_KP_1;
	case XKB_KEY_KP_2:
		return t_code::c_KP_2;
	case XKB_KEY_KP_3:
		return t_code::c_KP_3;
	case XKB_KEY_KP_4:
		return t_code::c_KP_4;
	case XKB_KEY_KP_5:
		return t_code::c_KP_5;
	case XKB_KEY_KP_6:
		return t_code::c_KP_6;
	case XKB_KEY_KP_7:
		return t_code::c_KP_7;
	case XKB_KEY_KP_8:
		return t_code::c_KP_8;
	case XKB_KEY_KP_9:
		return t_code::c_KP_9;
	default:
		return t_code::c_NONE;
	}
}

void t_window::f_send(const char* a_cs, size_t a_n)
{
	while (a_n > 0) {
		ssize_t n = write(v_master, a_cs, a_n);
		if (n == ssize_t(-1)) {
std::fprintf(stderr, "write: %s\n", std::strerror(errno));
			break;
		}
		a_cs += n;
		a_n -= n;
	}
}

int t_window::f_draw_row(SkCanvas& a_canvas, int a_x, int a_y, const t_row* a_row)
{
	const auto* cells = a_row->v_cells;
	SkPaint paint;
	paint.setBlendMode(SkBlendMode::kSrc);
	auto y = a_y - v_metrics.fAscent;
	auto u = a_y + v_unit.fHeight - 1;
	unsigned i = 0;
	while (i < a_row->v_size) {
		unsigned n = 0;
		unsigned w = 0;
		auto a = cells[i].v_a;
		do {
			wchar_t c = cells[i++].v_c;
			if (c != L'\0') {
				v_cs[n] = c;
				v_positions[n].fX = w;
				++n;
			}
			w += v_unit.fWidth;
		} while (i < a_row->v_size && cells[i].v_a == a);
		const auto* colors = v_colors;
		if (a.v_faint) colors += 10;
		if (a.v_blink) colors += 20;
		if (a.v_inverse) colors += 40;
		paint.setColor(colors[a.v_background]);
		a_canvas.drawIRect(SkIRect::MakeXYWH(a_x, a_y, w, v_unit.fHeight), paint);
		paint.setColor(colors[a.v_foreground]);
		v_font.unicharsToGlyphs(v_cs, n, v_glyphs);
		a_canvas.drawGlyphs(n, v_glyphs, v_positions, {static_cast<float>(a_x), y}, a.v_bold ? v_bold : v_font, paint);
		if (a.v_underlined) a_canvas.drawIRect(SkIRect::MakeXYWH(a_x, u, w, 1), paint);
		a_x += w;
	}
	return a_x;
}

void t_window::f_draw_row(SkCanvas& a_canvas, int a_y, const t_row* a_row)
{
	if (v_valid.contains(SkIRect::MakeXYWH(0, a_y, v_width, v_unit.fHeight))) return;
	auto x = f_draw_row(a_canvas, 0, a_y, a_row);
	SkPaint paint;
	paint.setBlendMode(SkBlendMode::kSrc);
	paint.setColor(v_colors[1]);
	a_canvas.drawIRect(SkIRect::MakeXYWH(x, a_y, v_width - x, v_unit.fHeight), paint);
}

void t_window::f_draw_content(SkCanvas& a_canvas)
{
	SkRegion invalid(SkIRect::MakeXYWH(0, 0, v_width, v_height));
	invalid.op(v_valid, SkRegion::kDifference_Op);
	auto bounds = invalid.getBounds();
	auto bottom = bounds.fBottom;
	auto uh = v_unit.fHeight;
	unsigned i = (bounds.fTop + v_position) / uh;
	int y = i * uh - v_position;
	while (i < v_buffer.f_log_size()) {
		f_draw_row(a_canvas, y, v_buffer.f_log(i));
		if ((y += uh) >= bottom) break;
		++i;
	}
	if (y < bottom) {
		i -= v_buffer.f_log_size();
		while (i < v_buffer.f_height()) {
			f_draw_row(a_canvas, y, v_buffer.f_at(i));
			if ((y += uh) >= bottom) break;
			++i;
		}
	}
	SkPaint paint;
	paint.setBlendMode(SkBlendMode::kSrc);
	paint.setColor(v_colors[1]);
	y = (v_buffer.f_log_size() + v_buffer.f_height()) * uh - v_position;
	if (y < bottom) if (auto bounds = SkIRect::MakeXYWH(0, y, v_width, v_height - y); !v_valid.contains(bounds)) a_canvas.drawIRect(bounds, paint);
	y = (v_buffer.f_log_size() + v_buffer.f_cursor_y()) * uh - v_position;
	if (y + uh > bounds.fTop && y < bottom && !v_valid.contains(SkIRect::MakeXYWH(0, y, v_width, uh))) {
		int x = v_buffer.f_cursor_x() * v_unit.fWidth;
		const t_row* row = v_buffer.f_at(v_buffer.f_cursor_y());
		wchar_t c;
		t_attribute a;
		if (v_buffer.f_cursor_x() < row->v_size) {
			auto& cell = row->v_cells[v_buffer.f_cursor_x()];
			c = cell.v_c;
			a = cell.v_a;
		} else {
			c = L' ';
			a = {false, false, false, false, false, 0, 1};
		}
		const auto* colors = v_colors;
		if (a.v_faint) colors += 10;
		if (a.v_blink) colors += 20;
		if (a.v_inverse) colors += 40;
		paint.setColor(colors[a.v_foreground]);
		unsigned width = wcwidth(c) * v_unit.fWidth;
		if (&v_frame == f_client().f_keyboard_focus()) {
			a_canvas.drawIRect(SkIRect::MakeXYWH(x, y, width, uh), paint);
			paint.setColor(colors[a.v_background]);
			auto glyph = v_font.unicharToGlyph(c);
			SkPoint point{};
			a_canvas.drawGlyphs(1, &glyph, &point, {static_cast<float>(x), y - v_metrics.fAscent}, a.v_bold ? v_bold : v_font, paint);
			if (a.v_underlined) a_canvas.drawIRect(SkIRect::MakeXYWH(x, y + uh - 1, width, 1), paint);
		} else {
			paint.setStyle(SkPaint::kStroke_Style);
			a_canvas.drawIRect(SkIRect::MakeXYWH(x + 1, y + 1, width - 2, uh - 2), paint);
		}
	}
}

void t_window::f_draw_bar(SkCanvas& a_canvas)
{
	int x = v_width;
	auto bw = v_decoration.v_theme.v_unit.fWidth;
	auto content = f_content();
	SkPaint paint;
	paint.setBlendMode(SkBlendMode::kSrc);
	if (content <= v_height) {
		paint.setColor(v_colors[1]);
		a_canvas.drawIRect(SkIRect::MakeXYWH(x, 0, bw, v_height), paint);
		return;
	}
	paint.setColor(v_decoration.v_theme.v_color_background);
	auto bh = v_decoration.v_theme.v_unit.fHeight;
	if (v_height > bh * 3) {
		unsigned gap = v_height - bh * 2;
		unsigned thumb = static_cast<double>(gap) * v_height / content;
		if (thumb < bh) thumb = bh;
		unsigned thumb_begin = bh + static_cast<unsigned>(static_cast<double>(gap - thumb) * v_position / (content - v_height));
		a_canvas.drawIRect(SkIRect::MakeXYWH(x, 0, bw, thumb_begin), paint);
		unsigned thumb_end = thumb_begin + thumb;
		a_canvas.drawIRect(SkIRect::MakeXYWH(x, thumb_end, bw, v_height - thumb_end), paint);
		paint.setColor(v_colors[1]);
		a_canvas.drawIRect(SkIRect::MakeXYWH(x, thumb_begin, bw, thumb), paint);
	} else {
		a_canvas.drawIRect(SkIRect::MakeXYWH(x, 0, bw, v_height), paint);
	}
	auto draw = [&](auto a_part, auto a_y)
	{
		auto hovered = a_part == v_hovered ? 1 : 0;
		v_decoration.v_theme.f_draw(a_canvas, v_bar_glyphs[a_part - c_part__BUTTON_UP], x, a_y, v_pressed != c_part__NONE ? a_part == v_pressed ? hovered + 1 : 0 : hovered);
	};
	draw(c_part__BUTTON_UP, 0);
	draw(c_part__BUTTON_DOWN, v_height - bh);
}

void t_window::f__scroll(int a_y, unsigned a_height, int a_dy)
{
	auto bounds = SkIRect::MakeXYWH(0, a_y, v_width, a_height);
	SkRegion region(bounds);
	region.op(v_valid, SkRegion::kIntersect_Op);
	v_valid.op(bounds, SkRegion::kDifference_Op);
	region.translate(0, a_dy);
	region.op(bounds, SkRegion::kIntersect_Op);
	v_valid.op(region, SkRegion::kUnion_Op);
	v_frame.f_request_frame();
	if (region.isEmpty()) return;
	auto frame = v_decoration.v_theme.f_frame(v_frame);
	int sy1 = v_height - a_y + frame.fBottom;
	int sy0 = sy1 - a_height;
	int dy0 = sy0;
	int dy1 = sy1;
	if (a_dy < 0) {
		sy1 += a_dy;
		dy0 -= a_dy;
	} else {
		sy0 += a_dy;
		dy1 -= a_dy;
	}
	glBindFramebuffer(GL_READ_FRAMEBUFFER, v_framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	auto x0 = frame.fLeft;
	auto x1 = x0 + v_width;
	glBlitFramebuffer(x0, sy0, x1, sy1, x0, dy0, x1, dy1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, v_framebuffer);
	glBlitFramebuffer(x0, dy0, x1, dy1, x0, dy0, x1, dy1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void t_window::f_invalidate(int a_y, unsigned a_height)
{
	if (a_height <= 0) return;
	a_y = (a_y + v_buffer.f_log_size()) * v_unit.fHeight - v_position;
	a_height *= v_unit.fHeight;
	int bottom = a_y + a_height;
	if (bottom < 0 || a_y >= static_cast<int>(v_height)) return;
	if (a_y < 0) a_y = 0;
	if (bottom > static_cast<int>(v_height)) bottom = v_height;
	f__invalidate(a_y, bottom - a_y);
}

void t_window::f_scroll(int a_y, unsigned a_height, int a_dy)
{
	a_y = (a_y + v_buffer.f_log_size()) * v_unit.fHeight - v_position;
	a_height *= v_unit.fHeight;
	int bottom = a_y + a_height;
	if (bottom < 0 || a_y >= static_cast<int>(v_height)) return;
	if (a_y < 0) a_y = 0;
	if (bottom > static_cast<int>(v_height)) bottom = v_height;
	f__scroll(a_y, bottom - a_y, a_dy * v_unit.fHeight);
}

void t_window::f_log()
{
	v_position += v_unit.fHeight;
	v_preedit_valid = v_bar_valid = false;
	v_frame.f_request_frame();
	v_on_hover();
}

void t_window::f_position__(int a_position)
{
	unsigned content = f_content();
	if (a_position < 0)
		a_position = 0;
	else if (a_position > static_cast<int>(content - v_height))
		a_position = content - v_height;
	if (a_position == v_position) return;
	f__scroll(0, v_height, v_position - a_position);
	v_position = a_position;
	v_preedit_valid = v_bar_valid = false;
	v_on_hover();
}

t_window::t_window(unsigned a_log, unsigned a_width, unsigned a_height, int a_master, const SkFont& a_font, const t_theme& a_theme, const char* a_name) :
v_buffer(*this, a_log, a_width, a_height), v_master(a_master),
v_colors{
	SkColorSetARGB(255, 255, 255, 255),
	SkColorSetARGB(127, 0, 0, 0),
	SkColorSetARGB(255, 0, 0, 0),
	SkColorSetARGB(255, 255, 63, 63),
	SkColorSetARGB(255, 63, 255, 63),
	SkColorSetARGB(255, 255, 191, 63),
	SkColorSetARGB(255, 63, 127, 255),
	SkColorSetARGB(255, 255, 63, 255),
	SkColorSetARGB(255, 63, 255, 255),
	SkColorSetARGB(255, 255, 255, 255),

	SkColorSetARGB(255, 127, 127, 127),
	SkColorSetARGB(255, 0, 0, 0),
	SkColorSetARGB(255, 63, 63, 63),
	SkColorSetARGB(255, 127, 31, 31),
	SkColorSetARGB(255, 31, 127, 31),
	SkColorSetARGB(255, 127, 95, 31),
	SkColorSetARGB(255, 31, 63, 127),
	SkColorSetARGB(255, 127, 31, 127),
	SkColorSetARGB(255, 31, 127, 127),
	SkColorSetARGB(255, 127, 127, 127),

	SkColorSetARGB(255, 255, 255, 255),
	SkColorSetARGB(255, 0, 0, 0),
	SkColorSetARGB(255, 0, 0, 0),
	SkColorSetARGB(255, 255, 63, 63),
	SkColorSetARGB(255, 63, 255, 63),
	SkColorSetARGB(255, 255, 191, 63),
	SkColorSetARGB(255, 63, 127, 255),
	SkColorSetARGB(255, 255, 63, 255),
	SkColorSetARGB(255, 63, 255, 255),
	SkColorSetARGB(255, 255, 255, 255),

	SkColorSetARGB(255, 127, 127, 127),
	SkColorSetARGB(255, 0, 0, 0),
	SkColorSetARGB(255, 63, 63, 63),
	SkColorSetARGB(255, 127, 31, 31),
	SkColorSetARGB(255, 31, 127, 31),
	SkColorSetARGB(255, 127, 95, 31),
	SkColorSetARGB(255, 31, 63, 127),
	SkColorSetARGB(255, 127, 31, 127),
	SkColorSetARGB(255, 31, 127, 127),
	SkColorSetARGB(255, 127, 127, 127),

	SkColorSetARGB(255, 0, 0, 0),
	SkColorSetARGB(255, 255, 255, 255),
	SkColorSetARGB(255, 0, 0, 0),
	SkColorSetARGB(255, 255, 63, 63),
	SkColorSetARGB(255, 63, 255, 63),
	SkColorSetARGB(255, 255, 191, 63),
	SkColorSetARGB(255, 63, 127, 255),
	SkColorSetARGB(255, 255, 63, 255),
	SkColorSetARGB(255, 63, 255, 255),
	SkColorSetARGB(255, 255, 255, 255),

	SkColorSetARGB(255, 0, 0, 0),
	SkColorSetARGB(255, 127, 127, 127),
	SkColorSetARGB(255, 63, 63, 63),
	SkColorSetARGB(255, 127, 31, 31),
	SkColorSetARGB(255, 31, 127, 31),
	SkColorSetARGB(255, 127, 95, 31),
	SkColorSetARGB(255, 31, 63, 127),
	SkColorSetARGB(255, 127, 31, 127),
	SkColorSetARGB(255, 31, 127, 127),
	SkColorSetARGB(255, 127, 127, 127),

	SkColorSetARGB(255, 0, 0, 0),
	SkColorSetARGB(255, 255, 255, 255),
	SkColorSetARGB(255, 0, 0, 0),
	SkColorSetARGB(255, 255, 63, 63),
	SkColorSetARGB(255, 63, 255, 63),
	SkColorSetARGB(255, 255, 191, 63),
	SkColorSetARGB(255, 63, 127, 255),
	SkColorSetARGB(255, 255, 63, 255),
	SkColorSetARGB(255, 63, 255, 255),
	SkColorSetARGB(255, 255, 255, 255),

	SkColorSetARGB(255, 0, 0, 0),
	SkColorSetARGB(255, 127, 127, 127),
	SkColorSetARGB(255, 63, 63, 63),
	SkColorSetARGB(255, 127, 31, 31),
	SkColorSetARGB(255, 31, 127, 31),
	SkColorSetARGB(255, 127, 95, 31),
	SkColorSetARGB(255, 31, 63, 127),
	SkColorSetARGB(255, 127, 31, 127),
	SkColorSetARGB(255, 31, 127, 127),
	SkColorSetARGB(255, 127, 127, 127)
},
v_font(a_font), v_bold(a_font.makeWithSize(a_font.getSize())),
v_unit(SkISize::Make(std::ceil(v_font.measureText(L"M", sizeof(wchar_t), SkTextEncoding::kUTF32, nullptr)), std::ceil(v_font.getMetrics(&v_metrics)))),
v_decoration(a_theme),
v_cs(new SkUnichar[a_width]), v_glyphs(new SkGlyphID[a_width]), v_positions(new SkPoint[a_width])
{
	SkUnichar cs[] = {L'\ue5c7', L'\ue5c5'};
	v_decoration.v_theme.v_font.unicharsToGlyphs(cs, 2, v_bar_glyphs);
	std::fill_n(v_positions, a_width, SkPoint::Make(0.0f, 0.0f));
	v_bold.setEmbolden(true);
	v_frame.v_on_measure = [&](auto& a_width, auto& a_height)
	{
		auto frame = v_decoration.v_theme.f_frame(v_frame);
		auto dx = v_decoration.v_theme.v_unit.fWidth + frame.fLeft + frame.fRight;
		auto dy = frame.fTop + frame.fBottom;
		a_width = (a_width > 0 ? (a_width - dx) / v_unit.fWidth : v_buffer.f_width()) * v_unit.fWidth + dx;
		a_height = (a_height > 0 ? (a_height - dy) / v_unit.fHeight : v_buffer.f_height()) * v_unit.fHeight + dy;
	};
	v_frame.v_on_map = [&](auto a_width, auto a_height)
	{
		v_frame.f_make_current();
		if (v_surface) {
			v_surface.reset();
		} else {
			glGenRenderbuffers(1, &v_renderbuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, v_renderbuffer);
			glGenFramebuffers(1, &v_framebuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, v_framebuffer);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, v_renderbuffer);
			v_context = skia::f_new_direct_context();
		}
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, a_width, a_height);
		v_surface = skia::f_new_surface(v_context.get(), v_framebuffer, a_width, a_height);
		auto frame = v_decoration.v_theme.f_frame(v_frame);
		v_width = a_width - v_decoration.v_theme.v_unit.fWidth - frame.fLeft - frame.fRight;
		v_height = a_height - frame.fTop - frame.fBottom;
		auto width = v_buffer.f_width();
		auto height = v_buffer.f_height();
		v_buffer.f_resize(v_width / v_unit.fWidth, v_height / v_unit.fHeight);
		if (v_buffer.f_width() != width) {
			auto n = v_buffer.f_width();
			delete[] v_cs;
			v_cs = new SkUnichar[n];
			delete[] v_glyphs;
			v_glyphs = new SkGlyphID[n];
			delete[] v_positions;
			v_positions = new SkPoint[n];
			std::fill_n(v_positions, n, SkPoint::Make(0.0f, 0.0f));
		}
		if (v_buffer.f_width() != width || v_buffer.f_height() != height) {
			//f_invalidate(v_buffer.f_cursor_y(), 1);
			struct winsize ws;
			ws.ws_col = v_buffer.f_width();
			ws.ws_row = v_buffer.f_height();
			ws.ws_xpixel = ws.ws_ypixel = 0;
			ioctl(v_master, TIOCSWINSZ, &ws);
		}
		v_decoration.f_invalidate();
		v_valid.setEmpty();
		v_preedit_valid = v_bar_valid = false;
	};
	v_frame.v_on_unmap = [&]
	{
		v_frame.f_make_current();
		v_surface.reset();
		v_context.reset();
		glDeleteFramebuffers(1, &v_framebuffer);
		glDeleteRenderbuffers(1, &v_renderbuffer);
	};
	v_frame.v_on_close = []
	{
		suisha::f_loop().f_exit();
	};
	v_frame.v_on_frame = [&](auto a_time)
	{
		auto& canvas = *v_surface->getCanvas();
		auto cw = v_width + v_decoration.v_theme.v_unit.fWidth;
		auto frame = v_decoration.v_theme.f_frame(v_frame);
		auto width = cw + frame.fLeft + frame.fRight;
		auto height = v_height + frame.fTop + frame.fBottom;
		v_decoration.f_draw(v_frame, canvas, width, height);
		canvas.save();
		canvas.translate(frame.fLeft, frame.fTop);
		canvas.clipIRect(SkIRect::MakeXYWH(0, 0, cw, v_height));
		auto bounds = SkIRect::MakeXYWH(0, 0, v_width, v_height);
		if (!v_valid.contains(bounds)) {
			f_draw_content(canvas);
			auto x = v_buffer.f_cursor_x() * v_unit.fWidth;
			auto y = (v_buffer.f_log_size() + v_buffer.f_cursor_y()) * v_unit.fHeight - v_position;
			for (auto& row : v_preedit_rows) {
				f_draw_row(canvas, x, y, row.get());
				x = 0;
				y += v_unit.fHeight;
			}
			v_valid.setRect(bounds);
		}
		if (!v_bar_valid) {
			f_draw_bar(canvas);
			v_bar_valid = true;
		}
		canvas.restore();
		v_context->flushAndSubmit(v_surface.get());
		glBindFramebuffer(GL_READ_FRAMEBUFFER, v_framebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, v_framebuffer);
		v_frame.f_swap_buffers();
	};
	v_frame.v_on_focus_enter = v_frame.v_on_focus_leave = [&]
	{
		f_invalidate(v_buffer.f_cursor_y(), 1);
	};
	v_frame.v_on_key_press = v_frame.v_on_key_repeat = [&](auto a_key, auto a_c)
	{
		if (v_pressed == c_part__NONE && v_hovered == c_part__CONTENT) f_client().f_cursor__(v_cursor_content = nullptr);
		f_position__(INT_MAX);
		auto code = f_code(a_key);
		if (code != t_code::c_NONE) {
			size_t i = 0;
			if (xkb_state_mod_name_is_active(f_client(), XKB_MOD_NAME_SHIFT, XKB_STATE_MODS_EFFECTIVE) > 0) i |= 1;
			if (xkb_state_mod_name_is_active(f_client(), XKB_MOD_NAME_CTRL, XKB_STATE_MODS_EFFECTIVE) > 0) i |= 2;
			const char* cs = v_buffer.f_code(code, i);
			f_send(cs, std::strlen(cs));
		} else if (a_c != L'\0') {
			char mbs[MB_LEN_MAX];
			std::mbstate_t state{};
			size_t n = std::wcrtomb(mbs, a_c, &state);
			if (n != size_t(-1)) f_send(mbs, n);
			n = std::wcrtomb(mbs, L'\0', &state);
			if (n != size_t(-1)) f_send(mbs, n - 1);
		}
	};
	v_frame.v_on_input_enable = [&]
	{
		auto [x, y] = v_preedit_cursor;
		zwp_text_input_v3_set_cursor_rectangle(*v_frame.f_input(), x, y, v_unit.fWidth, v_unit.fHeight);
	};
	v_frame.v_on_input_disable = [&]
	{
		v_preedit_valid = false;
		v_preedit_text.clear();
	};
	auto convert = [](auto a_p, auto a_q, auto& a_mbstate, auto& a_put)
	{
		while (a_p < a_q) {
			wchar_t c;
			size_t n = std::mbrtowc(&c, a_p, a_q - a_p, &a_mbstate);
			if (n == size_t(-2)) break;
			if (n == size_t(-1)) {
				a_mbstate = {};
				++a_p;
			} else if (n == 0) {
				a_put(L'\0');
				++a_p;
			} else {
				a_put(c);
				a_p += n;
			}
		}
		return a_p;
	};
	v_frame.v_on_input_done = [&, convert]
	{
		auto& text = v_frame.f_input()->f_text();
		f_send(text.data(), text.size());
		auto [preedit, begin, end] = v_frame.f_input()->f_preedit();
		if (begin > end) return;
		v_preedit_valid = false;
		v_preedit_text.clear();
		if (preedit.empty()) return;
		auto p = preedit.c_str();
		std::mbstate_t mbstate{};
		auto put = [&](auto a_c)
		{
			v_preedit_text.push_back(a_c);
		};
		convert(p, p + begin, mbstate, put);
		v_preedit_begin = v_preedit_text.size();
		convert(p + begin, p + end, mbstate, put);
		v_preedit_end = v_preedit_text.size();
		convert(p + end, p + preedit.size(), mbstate, put);
	};
	f_client().v_on_idle.push_back([&]
	{
		auto input = v_frame.f_input();
		if (!input || !input->f_done()) return;
		if (v_preedit_valid) return;
		v_preedit_valid = true;
		f_invalidate(v_buffer.f_cursor_y(), v_preedit_rows.size());
		v_preedit_rows.clear();
		auto cursor = std::make_tuple(v_buffer.f_cursor_x(), v_buffer.f_cursor_y());
		if (!v_preedit_text.empty()) {
			auto size = sizeof(t_row) + sizeof(t_cell) * v_buffer.f_width();
			auto row = v_preedit_rows.emplace_back(new(new char[size]) t_row).get();
			auto [x, y] = cursor;
			auto put = [&](auto a_i, auto a_j, t_attribute a_a)
			{
				while (a_i < a_j) {
					auto c = v_preedit_text[a_i++];
					int n = f_width(c);
					if (n <= 0) continue;
					if (x + n > v_buffer.f_width()) {
						row = v_preedit_rows.emplace_back(new(new char[size]) t_row).get();
						x = 0;
						++y;
					}
					auto p = row->v_cells + row->v_size;
					*p = {c, a_a};
					std::fill(p + 1, p + n, t_cell{L'\0', a_a});
					row->v_size += n;
					x += n;
				}
			};
			put(0, v_preedit_begin, {false, false, true, false, false, 0, 1});
			cursor = std::make_tuple(x, y);
			put(v_preedit_begin, v_preedit_end, {false, false, true, false, true, 0, 1});
			put(v_preedit_end, v_preedit_text.size(), {false, false, true, false, false, 0, 1});
			f_invalidate(v_buffer.f_cursor_y(), v_preedit_rows.size());
		}
		auto frame = v_decoration.v_theme.f_frame(v_frame);
		v_preedit_cursor = {
			frame.fLeft + std::get<0>(cursor) * v_unit.fWidth,
			frame.fTop + (v_buffer.f_log_size() + std::get<1>(cursor)) * v_unit.fHeight - v_position
		};
		if (&v_frame != f_client().f_input_focus()) return;
		v_frame.v_on_input_enable();
		input->f_commit();
	});
	--v_idle;
	v_frame.f_input__(f_client().f_new_input());
	auto hovered = [&](auto a_part)
	{
		if (a_part == v_hovered) return;
		v_hovered = a_part;
		v_bar_valid = false;
		v_frame.f_request_frame();
	};
	auto hover = [&, hovered]
	{
		auto frame = v_decoration.v_theme.f_frame(v_frame);
		auto y = f_client().f_pointer_y() - frame.fTop;
		if (y < 0.0) return hovered(c_part__NONE);
		auto x = f_client().f_pointer_x() - frame.fLeft;
		if (x < v_width) return hovered(x >= 0.0 && y < v_height ? c_part__CONTENT : c_part__NONE);
		if (x >= v_width + v_decoration.v_theme.v_unit.fWidth) return hovered(c_part__NONE);
		auto content = f_content();
		if (content <= v_height) return hovered(c_part__OTHER);
		auto bh = v_decoration.v_theme.v_unit.fHeight;
		if (v_height < bh * 2) return hovered(y < v_height / 2 ? c_part__BUTTON_UP : c_part__BUTTON_DOWN);
		if (y < bh) return hovered(c_part__BUTTON_UP);
		int gap_end = v_height - bh;
		int gap = gap_end - bh;
		if (gap > bh) {
			unsigned thumb = static_cast<double>(gap) * v_height / content;
			if (thumb < bh) thumb = bh;
			int thumb_begin = bh + static_cast<int>(static_cast<double>(gap - thumb) * v_position / (content - v_height));
			if (y < thumb_begin) return hovered(c_part__GAP_UP);
			if (y < thumb_begin + thumb) return hovered(c_part__THUMB);
			if (y < gap_end) return hovered(c_part__GAP_DOWN);
		} else {
			if (y < gap_end) return hovered(c_part__OTHER);
		}
		hovered(y < v_height ? c_part__BUTTON_DOWN : c_part__NONE);
	};
	auto hover_cursor = [&, hover]
	{
		hover();
		f_client().f_cursor__(v_hovered == c_part__CONTENT ? v_cursor_content : &v_decoration.v_theme.v_cursor_arrow);
	};
	v_on_hover = [&, hover_cursor]
	{
		if (v_hovered != c_part__NONE) hover_cursor();
	};
	v_frame.v_on_pointer_enter = v_frame.v_on_pointer_move = [&, hover_cursor]
	{
		v_cursor_content = &v_decoration.v_theme.v_cursor_text;
		hover_cursor();
	};
	v_frame.v_on_pointer_leave = [&, hovered]
	{
		hovered(c_part__NONE);
	};
	v_frame.v_on_button_press = [&, hover](auto a_button)
	{
		if (v_hovered == c_part__NONE) return;
		v_pressed = v_hovered;
		v_bar_valid = false;
		v_frame.f_request_frame();
		auto release = [&, hover = v_on_hover, move = v_frame.v_on_pointer_move](auto a_button)
		{
			v_pressed = c_part__NONE;
			v_bar_valid = false;
			v_frame.f_request_frame();
			v_on_hover = hover;
			v_frame.v_on_pointer_move = move;
			move();
			v_frame.v_on_button_release = {};
		};
		v_on_hover = v_frame.v_on_pointer_move = hover;
		int delta;
		switch (v_hovered) {
		case c_part__BUTTON_UP:
			delta = -v_unit.fHeight;
			break;
		case c_part__GAP_UP:
			delta = -v_height;
			break;
		case c_part__GAP_DOWN:
			delta = v_height;
			break;
		case c_part__BUTTON_DOWN:
			delta = v_unit.fHeight;
			break;
		case c_part__THUMB:
			{
				auto w2v = [&]
				{
					auto content = f_content();
					auto bh = v_decoration.v_theme.v_unit.fHeight;
					auto gap = v_height - bh * 2;
					unsigned thumb = static_cast<double>(gap) * v_height / content;
					if (thumb < bh) thumb = bh;
					return static_cast<double>(gap - thumb) / (content - v_height);
				};
				v_frame.v_on_pointer_move = [&, w2v, thumb0__y0 = v_position * w2v() - f_client().f_pointer_y()]
				{
					f_position__((thumb0__y0 + f_client().f_pointer_y()) / w2v());
				};
			}
		default:
			v_frame.v_on_button_release = release;
			return;
		}
		v_frame.v_on_button_release = [&, release](auto a_button)
		{
			v_repeat->f_stop();
			v_repeat = {};
			release(a_button);
		};
		f_position__(v_position + delta);
		v_repeat = suisha::f_loop().f_timer([&, repeat = [&, delta]
		{
			if (v_hovered == v_pressed) f_position__(v_position + delta);
		}]
		{
			repeat();
			v_repeat = suisha::f_loop().f_timer(repeat, 40ms);
		}, 400ms, true);
	};
	v_decoration.f_hook(v_frame);
	v_frame.v_on_scroll = [&](auto a_axis, auto a_value)
	{
		if (a_axis == WL_POINTER_AXIS_VERTICAL_SCROLL) f_position__(v_position + std::copysign(v_unit.fHeight * 4, a_value));
	};
	suisha::f_loop().f_poll(a_master, true, false, [&, convert](auto a_readable, auto)
	{
		if (!a_readable) return;
		ssize_t n = read(v_master, v_mbs + v_mbn, sizeof(v_mbs) - v_mbn);
		if (n == ssize_t(-1)) {
			std::fprintf(stderr, "read: %s\n", std::strerror(errno));
			return;
		}
		v_mbn += n;
		if (v_mbn <= 0) return suisha::f_loop().f_exit();
		f_invalidate(v_buffer.f_cursor_y(), 1);
		auto q = v_mbs + v_mbn;
		auto p = convert(v_mbs, q, v_mbstate, v_buffer);
		if (p < q) {
			std::copy(p, q, v_mbs);
			v_mbstate = {};
		}
		v_mbn = q - p;
		f_invalidate(v_buffer.f_cursor_y(), 1);
		v_preedit_valid = false;
	});
	xdg_toplevel_set_title(v_frame, a_name);
}

t_window::~t_window()
{
	if (auto& on = f_client().v_on_idle; v_idle != on.end()) on.erase(v_idle);
	if (v_context) {
		glDeleteFramebuffers(1, &v_framebuffer);
		glDeleteRenderbuffers(1, &v_renderbuffer);
	}
	delete[] v_cs;
	delete[] v_glyphs;
	delete[] v_positions;
}
