#include "engine.h"
#include "basic_dictionary.h"
#include "input-method-unstable-v2.h"
#include <xade/client.h>
#include <xade/skia.h>
#include <dirent.h>
#include <sys/mman.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkFont.h>
#include <include/core/SkFontMetrics.h>
#include <include/core/SkFontMgr.h>
#include <include/ports/SkFontMgr_fontconfig.h>
#include <include/ports/SkFontScanner_FreeType.h>

namespace
{

class t_context : public t_engine
{
	static zwp_input_method_v2_listener v_zwp_input_method_v2_listener;
	static zwp_input_method_keyboard_grab_v2_listener v_zwp_input_method_keyboard_grab_v2_listener;

	t_converter<wchar_t, char> v_wctoutf8{"wchar_t", "utf-8"};
	std::vector<wchar_t> v_cs;
	std::vector<t_attribute> v_as;
	bool v_forwarded;
	t_owner<zwp_input_method_v2*, zwp_input_method_v2_destroy> v_input;
	bool v_active = false;
	uint32_t v_serial = 0;
	SkFont v_font;
	SkFontMetrics v_metrics;
	t_surface v_surface{false};
	t_owner<zwp_input_popup_surface_v2*, zwp_input_popup_surface_v2_destroy> v_popup;
	sk_sp<GrDirectContext> v_sk_context;
	sk_sp<SkSurface> v_sk_surface;
	t_owner<zwp_input_method_keyboard_grab_v2*, zwp_input_method_keyboard_grab_v2_release> v_grab;
	t_xkb v_xkb;

	void f_send_preedit();

protected:
	virtual void f_on_forward()
	{
		v_forwarded = true;
	}
	virtual void f_on_compose(size_t a_i, size_t a_m, const wchar_t* a_cs, const t_attribute* a_as, size_t a_n)
	{
		{
			auto i = v_cs.begin() + a_i;
			v_cs.insert(v_cs.erase(i, i + a_m), a_cs, a_cs + a_n);
		}
		{
			auto i = v_as.begin() + a_i;
			v_as.insert(v_as.erase(i, i + a_m), a_as, a_as + a_n);
		}
		f_send_preedit();
	}
	virtual void f_on_commit(const wchar_t* a_cs, size_t a_n)
	{
		v_cs.clear();
		v_as.clear();
		std::vector<char> cs;
		v_wctoutf8(a_cs, a_n, f_appender(cs));
		cs.push_back('\0');
		zwp_input_method_v2_commit_string(v_input, cs.data());
		zwp_input_method_v2_commit(v_input, v_serial);
	}
	virtual void f_on_status()
	{
		v_surface.f_request_frame();
	}
	virtual void f_on_candidates()
	{
		v_surface.f_request_frame();
	}
	virtual void f_on_choose()
	{
		v_surface.f_request_frame();
	}

public:
	t_context(t_dictionary& a_dictionary, zwp_input_method_manager_v2* a_manager, const SkFont& a_font) : t_engine(a_dictionary), v_font(a_font)
	{
		v_input = zwp_input_method_manager_v2_get_input_method(a_manager, f_client());
		if (!v_input) throw std::runtime_error("input method");
		zwp_input_method_v2_add_listener(v_input, &v_zwp_input_method_v2_listener, this);
		auto region = wl_compositor_create_region(f_client());
		wl_surface_set_input_region(v_surface, region);
		wl_region_destroy(region);
		v_popup = zwp_input_method_v2_get_input_popup_surface(v_input, v_surface);
		if (!v_popup) throw std::runtime_error("input popup");
		int size = std::ceil(v_font.getMetrics(&v_metrics));
		v_surface.f_create(size, size);
		v_surface.f_make_current();
		v_sk_context = skia::f_new_direct_context();
		v_surface.v_on_frame = [&](auto a_time)
		{
			std::vector rows{1, f_status()};
			size_t current = -1;
			if (f_candidates().empty()) {
				for (auto n = f_states().size() - 1; n > 0;) rows.push_back(L'?' + f_entry(--n));
			} else {
				wchar_t cs[64];
				std::swprintf(cs, 64, L" %zd/%zd", f_chosen() + 1, f_candidates().size());
				rows.back() += cs;
				if (f_choosing()) {
					size_t first = f_chosen() / 8 * 8;
					size_t last = std::min(first + 8, f_candidates().size());
					current = rows.size() + f_chosen() - first;
					for (size_t i = first; i < last; ++i) {
						auto& candidate = f_candidates()[i];
						rows.push_back(candidate.v_text);
						for (auto& x : candidate.v_annotations) rows.back() += L" ;" + x;
					}
				}
			}
			int width = 0;
			for (auto& s : rows) {
				int w = std::ceil(v_font.measureText(s.c_str(), s.size() * sizeof(wchar_t), SkTextEncoding::kUTF32, nullptr));
				if (w > width) width = w;
			}
			int height = std::ceil(v_font.getSize() * rows.size());
			v_surface.f_make_current();
			int width0;
			int height0;
			wl_egl_window_get_attached_size(v_surface, &width0, &height0);
			if (width != width0 || height != height0) {
				v_sk_surface.reset();
				wl_egl_window_resize(v_surface, width, height, 0, 0);
				v_sk_surface = skia::f_new_surface(v_sk_context.get(), 0, width, height);
			}
			auto& canvas = *v_sk_surface->getCanvas();
			SkPaint paint;
			paint.setBlendMode(SkBlendMode::kSrc);
			paint.setColor(SkColorSetARGB(255, 255, 255, 255));
			canvas.drawIRect(SkIRect::MakeXYWH(0, 0, width, height), paint);
			auto y = -v_metrics.fAscent;
			for (size_t i = 0; auto& s : rows) {
				paint.setColor(SkColorSetARGB(255, 0, 0, 0));
				if (i++ == current) {
					canvas.drawIRect(SkIRect::MakeXYWH(0, y + v_metrics.fAscent, width, v_font.getSize()), paint);
					paint.setColor(SkColorSetARGB(255, 255, 255, 255));
				}
				canvas.drawSimpleText(s.c_str(), s.size() * sizeof(wchar_t), SkTextEncoding::kUTF32, 0.0, y, v_font, paint);
				y += v_font.getSize();
			}
			v_sk_context->flushAndSubmit(v_sk_surface.get());
			v_surface.f_swap_buffers();
		};
	}
};

zwp_input_method_v2_listener t_context::v_zwp_input_method_v2_listener = {
	[](auto a_data, auto a_this)
	{
		static_cast<t_context*>(a_data)->v_active = true;
	},
	[](auto a_data, auto a_this)
	{
		static_cast<t_context*>(a_data)->v_active = false;
	},
	[](auto a_data, auto a_this, auto a_text, auto a_cursor, auto a_anchor)
	{
	},
	[](auto a_data, auto a_this, auto a_cause)
	{
	},
	[](auto a_data, auto a_this, auto a_hint, auto a_purpose)
	{
	},
	[](auto a_data, auto a_this)
	{
		auto& self = *static_cast<t_context*>(a_data);
		++self.v_serial;
		if (self.v_active != static_cast<bool>(self.v_grab)) {
			if (self.v_active) {
				self.v_grab = zwp_input_method_v2_grab_keyboard(self.v_input);
				zwp_input_method_keyboard_grab_v2_add_listener(self.v_grab, &v_zwp_input_method_keyboard_grab_v2_listener, &self);
				if (!self.v_cs.empty()) self.f_send_preedit();
				self.v_surface.v_on_frame(0);
			} else {
				self.v_xkb.f_stop();
				self.v_grab = NULL;
			}
		}
	},
	[](auto a_data, auto a_this)
	{
		suisha::f_loop().f_exit();
	}
};
zwp_input_method_keyboard_grab_v2_listener t_context::v_zwp_input_method_keyboard_grab_v2_listener = {
	[](auto a_data, auto a_this, auto a_format, auto a_fd, auto a_size)
	{
		if (a_format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) static_cast<t_context*>(a_data)->v_xkb.f_keymap(a_fd, a_size);
	},
	[](auto a_data, auto a_this, auto a_serial, auto a_time, auto a_key, auto a_state)
	{
		auto& self = *static_cast<t_context*>(a_data);
		self.v_xkb.f_stop();
		auto forward = [&]
		{
			char cs[25];
			std::sprintf(cs, "%08x%08x%08x", a_time, a_key, a_state);
			zwp_input_method_v2_commit_string(self.v_input, cs);
			zwp_input_method_v2_delete_surrounding_text(self.v_input, 0x80000000, 0);
			self.f_send_preedit();
		};
		self.v_xkb.f_key(a_key, a_state, [&](auto sym, auto c)
		{
			self.v_forwarded = false;
			self.f_key_pressed(sym, c);
			if (self.v_forwarded) forward();
		}, [&](auto sym, auto c)
		{
			self.f_key_pressed(sym, c);
		}, [&](auto, auto)
		{
			forward();
		});
	},
	[](auto a_data, auto a_this, auto a_serial, auto a_depressed, auto a_latched, auto a_locked, auto a_group)
	{
		auto& self = *static_cast<t_context*>(a_data);
		if (self.v_xkb) xkb_state_update_mask(self.v_xkb, a_depressed, a_latched, a_locked, a_group, a_group, a_group);
		char cs[33];
		std::sprintf(cs, "%08x%08x%08x%08x", a_depressed, a_latched, a_locked, a_group);
		zwp_input_method_v2_commit_string(self.v_input, cs);
		zwp_input_method_v2_delete_surrounding_text(self.v_input, 0x80000001, 0);
		self.f_send_preedit();
	},
	[](auto a_data, auto a_this, auto a_rate, auto a_delay)
	{
		static_cast<t_context*>(a_data)->v_xkb.f_repeat(a_rate, a_delay);
	}
};

void t_context::f_send_preedit()
{
	std::vector<char> cs;
	size_t begin;
	size_t end;
	if (f_candidates().empty()) {
		auto i = f_caret();
		v_wctoutf8(v_cs.data(), i, f_appender(cs));
		begin = cs.size();
		v_wctoutf8(v_cs.data() + i, v_cs.size() - i, f_appender(cs));
		end = cs.size();
	} else {
		auto i = std::find(v_as.begin(), v_as.end(), c_attribute__CANDIDATE);
		auto j = i - v_as.begin();
		v_wctoutf8(v_cs.data(), j, f_appender(cs));
		begin = cs.size();
		while (i != v_as.end() && *i == c_attribute__CANDIDATE) ++i;
		auto k = i - v_as.begin();
		v_wctoutf8(v_cs.data() + j, k - j, f_appender(cs));
		end = cs.size();
		v_wctoutf8(v_cs.data() + k, v_cs.size() - k, f_appender(cs));
	}
	cs.push_back('\0');
	zwp_input_method_v2_set_preedit_string(v_input, cs.data(), begin, end);
	zwp_input_method_v2_commit(v_input, v_serial);
}

}

int main(int argc, char* argv[])
{
	using namespace std::literals;
	auto home = std::getenv("HOME") + "/.xadeim"s;
	auto path = home + "/public";
	std::vector<std::string> publics;
	if (DIR* dp = opendir(path.c_str())) {
		while (auto d = readdir(dp)) publics.push_back(path + '/' + d->d_name);
		closedir(dp);
	}
	std::sort(publics.begin(), publics.end());
	auto fm = SkFontMgr_New_FontConfig(nullptr, SkFontScanner_Make_FreeType());
	auto typeface = fm->matchFamilyStyle(nullptr, {});
	suisha::t_loop loop;
	zwp_input_method_manager_v2* manager = NULL;
	t_client client([&](auto a_this, auto a_name, auto a_interface, auto a_version)
	{
		if (std::strcmp(a_interface, zwp_input_method_manager_v2_interface.name) == 0) manager = static_cast<zwp_input_method_manager_v2*>(wl_registry_bind(a_this, a_name, &zwp_input_method_manager_v2_interface, std::min<uint32_t>(a_version, zwp_input_method_manager_v2_interface.version)));
	});
	if (!manager) throw std::runtime_error("input method manager");
	t_basic_dictionary dictionary(publics, home + "/private");
	dictionary.f_load();
	t_context context(dictionary, manager, {typeface, 16});
	zwp_input_method_manager_v2_destroy(manager);
	loop.f_run();
	dictionary.f_save();
	return EXIT_SUCCESS;
}
