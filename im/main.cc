#include "engine.h"
#include "basic_dictionary.h"
#include "input-method-unstable-v2.h"
#include <xade/client.h>
#include <dirent.h>
#include <sys/mman.h>
#include <GLES3/gl3.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkColorSpace.h>
#include <include/core/SkFont.h>
#include <include/core/SkFontMetrics.h>
#include <include/core/SkFontMgr.h>
#include <include/core/SkRegion.h>
#include <include/core/SkSurface.h>
#include <include/gpu/GrBackendSurface.h>
#include <include/gpu/GrDirectContext.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/gpu/ganesh/gl/GrGLBackendSurface.h>
#include <include/gpu/ganesh/gl/GrGLDirectContext.h>
#include <include/ports/SkFontMgr_fontconfig.h>

namespace
{

using namespace std::chrono_literals;

class t_context : public t_engine
{
	static zwp_input_method_v2_listener v_zwp_input_method_v2_listener;
	static zwp_input_method_keyboard_grab_v2_listener v_zwp_input_method_keyboard_grab_v2_listener;

	t_converter<wchar_t, char> v_converter{"wchar_t", "utf-8"};
	std::vector<wchar_t> v_cs;
	std::vector<t_attribute> v_as;
	bool v_forwarded;
	zwp_input_method_v2* v_input;
	bool v_active = false;
	uint32_t v_serial = 0;
	SkFont v_font;
	SkFontMetrics v_metrics;
	t_surface v_surface;
	zwp_input_popup_surface_v2* v_popup = NULL;
	sk_sp<GrDirectContext> v_sk_context;
	sk_sp<SkSurface> v_sk_surface;
	zwp_input_method_keyboard_grab_v2* v_grab = NULL;
	xkb_context* v_xkb = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	xkb_state* v_xkb_state = NULL;
	int v_repeat_rate = 0;
	int v_repeat_delay;
	std::shared_ptr<suisha::t_timer> v_repeat;

	operator xkb_state*() const
	{
		return v_xkb_state;
	}
	void f_send_preedit();
	void f_process(xkb_keysym_t a_key, char a_ascii);

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
		v_converter(a_cs, a_cs + a_n, std::back_inserter(cs));
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
		zwp_input_method_v2_add_listener(v_input, &v_zwp_input_method_v2_listener, this);
		auto region = wl_compositor_create_region(f_client());
		wl_surface_set_input_region(v_surface, region);
		wl_region_destroy(region);
		v_popup = zwp_input_method_v2_get_input_popup_surface(v_input, v_surface);
		int size = std::ceil(v_font.getMetrics(&v_metrics));
		v_surface.f_create(size, size);
		v_surface.f_make_current();
		v_sk_context = GrDirectContexts::MakeGL();
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
				GrGLFramebufferInfo fi;
				fi.fFBOID = 0;
				fi.fFormat = GL_RGBA8;
				auto target = GrBackendRenderTargets::MakeGL(width, height, 1, 0, fi);
				v_sk_surface = SkSurfaces::WrapBackendRenderTarget(v_sk_context.get(), target, kBottomLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType, {}, nullptr);
				if (!v_sk_surface) throw std::runtime_error("SkSurfaces::WrapBackendRenderTarget");
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
	~t_context()
	{
		if (v_grab) zwp_input_method_keyboard_grab_v2_release(v_grab);
		if (v_popup) zwp_input_popup_surface_v2_destroy(v_popup);
		if (v_input) zwp_input_method_v2_destroy(v_input);
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
				if (self.v_repeat) {
					self.v_repeat->f_stop();
					self.v_repeat = {};
				}
				zwp_input_method_keyboard_grab_v2_release(self.v_grab);
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
		if (a_format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
			auto buffer = mmap(0, a_size, PROT_READ, MAP_PRIVATE, a_fd, 0);
			if (buffer == MAP_FAILED) throw std::system_error(errno, std::generic_category());
			auto& self = *static_cast<t_context*>(a_data);
			auto keymap = xkb_keymap_new_from_buffer(self.v_xkb, static_cast<char*>(buffer), a_size, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
			if (!keymap) throw std::runtime_error("xkb_keymap_new_from_buffer");
			munmap(buffer, a_size);
			xkb_state_unref(self);
			self.v_xkb_state = xkb_state_new(keymap);
			xkb_keymap_unref(keymap);
			if (!self.v_xkb_state) throw std::runtime_error("xkb_state_new");
		}
	},
	[](auto a_data, auto a_this, auto a_serial, auto a_time, auto a_key, auto a_state)
	{
		auto& self = *static_cast<t_context*>(a_data);
		if (self.v_repeat) {
			self.v_repeat->f_stop();
			self.v_repeat = {};
		}
		if (self.v_xkb_state) {
			a_key += 8;
			auto sym = xkb_state_key_get_one_sym(self, a_key);
			auto c = xkb_state_key_get_utf32(self, a_key);
			switch (a_state) {
			case WL_KEYBOARD_KEY_STATE_PRESSED:
				if (self.v_repeat_rate > 0 && xkb_keymap_key_repeats(xkb_state_get_keymap(self), a_key)) {
					self.v_repeat = suisha::f_loop().f_timer([&self, sym, c]
					{
						self.f_process(sym, c);
						if (self.v_repeat_rate > 0) self.v_repeat = suisha::f_loop().f_timer([&self, sym, c]
						{
							self.f_process(sym, c);
						}, std::chrono::ceil<std::chrono::milliseconds>(1000000000ns / self.v_repeat_rate));
					}, std::chrono::milliseconds(self.v_repeat_delay), true);
				}
				self.f_process(sym, c);
				break;
			}
		}
	},
	[](auto a_data, auto a_this, auto a_serial, auto a_depressed, auto a_latched, auto a_locked, auto a_group)
	{
		auto& self = *static_cast<t_context*>(a_data);
		if (self.v_xkb_state) xkb_state_update_mask(self, a_depressed, a_latched, a_locked, a_group, a_group, a_group);
	},
	[](auto a_data, auto a_this, auto a_rate, auto a_delay)
	{
		auto& self = *static_cast<t_context*>(a_data);
		self.v_repeat_rate = a_rate;
		self.v_repeat_delay = a_delay;
	}
};

void t_context::f_send_preedit()
{
	std::vector<char> cs;
	size_t begin;
	size_t end;
	if (f_candidates().empty()) {
		auto i = v_cs.begin() + f_caret();
		v_converter(v_cs.begin(), i, std::back_inserter(cs));
		begin = cs.size();
		v_converter(i, v_cs.end(), std::back_inserter(cs));
		end = cs.size();
	} else {
		auto i = std::find(v_as.begin(), v_as.end(), c_attribute__CANDIDATE);
		auto j = v_cs.begin() + (i - v_as.begin());
		v_converter(v_cs.begin(), j, std::back_inserter(cs));
		begin = cs.size();
		while (i != v_as.end() && *i == c_attribute__CANDIDATE) ++i;
		auto k = v_cs.begin() + (i - v_as.begin());
		v_converter(j, k, std::back_inserter(cs));
		end = cs.size();
		v_converter(k, v_cs.end(), std::back_inserter(cs));
	}
	cs.push_back('\0');
	zwp_input_method_v2_set_preedit_string(v_input, cs.data(), begin, end);
	zwp_input_method_v2_commit(v_input, v_serial);
}

void t_context::f_process(xkb_keysym_t a_key, char a_ascii)
{
	v_forwarded = false;
	f_key_pressed(a_key, a_ascii);
	if (!v_forwarded) return;
	char cs[] = {a_ascii, '\0'};
	zwp_input_method_v2_commit_string(v_input, cs);
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
	auto fm = SkFontMgr_New_FontConfig(nullptr);
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
