#ifndef XADE__CLIENT_H
#define XADE__CLIENT_H

#include <list>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <wayland-egl.h>
#include "xdg-shell.h"
#include "text-input-unstable-v3.h"
#include <xkbcommon/xkbcommon.h>
#include <EGL/egl.h>
#include <suisha/loop.h>
#include "owner.h"

namespace xade
{

using namespace std::chrono_literals;

class t_surface;
class t_cursor;
class t_input;

class t_xkb
{
	xkb_context* v_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	xkb_state* v_state = NULL;
	int v_rate = 0;
	int v_delay;
	std::shared_ptr<suisha::t_timer> v_timer;

public:
	~t_xkb()
	{
		xkb_state_unref(v_state);
		xkb_context_unref(v_context);
	}
	operator xkb_state*() const
	{
		return v_state;
	}
	void f_keymap(int a_fd, size_t a_size);
	void f_repeat(int a_rate, int a_delay)
	{
		v_rate = a_rate;
		v_delay = a_delay;
	}
	void f_stop()
	{
		if (!v_timer) return;
		v_timer->f_stop();
		v_timer = {};
	}
	void f_key(uint32_t a_key, uint32_t a_state, auto a_press, auto a_repeat, auto a_release)
	{
		if (!v_state) return;
		a_key += 8;
		auto sym = xkb_state_key_get_one_sym(v_state, a_key);
		auto c = xkb_state_key_get_utf32(v_state, a_key);
		switch (a_state) {
		case WL_KEYBOARD_KEY_STATE_RELEASED:
			a_release(sym, c);
			break;
		case WL_KEYBOARD_KEY_STATE_PRESSED:
			if (v_rate > 0 && xkb_keymap_key_repeats(xkb_state_get_keymap(v_state), a_key)) v_timer = suisha::f_loop().f_timer([this, repeat = [a_repeat, sym, c]
			{
				a_repeat(sym, c);
			}]
			{
				if (v_rate <= 0) return;
				repeat();
				v_timer = suisha::f_loop().f_timer(repeat, std::chrono::ceil<std::chrono::milliseconds>(1000000000ns / v_rate));
			}, std::chrono::milliseconds(v_delay), true);
			a_press(sym, c);
			break;
		}
	}
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsubobject-linkage"

class t_client
{
	friend t_client& f_client();
	friend class t_frame;
	friend class t_input;

	static wl_registry_listener v_registry_listener;
	static wl_seat_listener v_seat_listener;
	static wl_pointer_listener v_pointer_listener;
	static wl_keyboard_listener v_keyboard_listener;
	static xdg_wm_base_listener v_xdg_wm_base_listener;
	static inline t_client* v_instance;

	std::function<void(wl_registry*, uint32_t, const char*, uint32_t)> v_on_global;
	t_owner<wl_display*, wl_display_disconnect> v_display;
	t_owner<wl_registry*, wl_registry_destroy> v_registry;
	t_owner<wl_compositor*, wl_compositor_destroy> v_compositor;
	t_owner<wl_shm*, wl_shm_release> v_shm;
	t_owner<wl_seat*, wl_seat_release> v_seat;
	t_owner<wl_pointer*, wl_pointer_release> v_pointer;
	t_owner<wl_cursor_theme*, wl_cursor_theme_destroy> v_cursor_theme;
	t_owner<wl_keyboard*, wl_keyboard_release> v_keyboard;
	t_xkb v_xkb;
	t_owner<xdg_wm_base*, xdg_wm_base_destroy> v_xdg_wm_base;
	t_owner<EGLDisplay, eglTerminate, EGL_NO_DISPLAY> v_egl_display;
	t_surface* v_pointer_focus = nullptr;
	double v_pointer_x;
	double v_pointer_y;
	uint32_t v_cursor_serial;
	uint32_t v_action_serial;
	const t_cursor* v_cursor = nullptr;
	std::shared_ptr<suisha::t_timer> v_cursor_next;
	t_surface* v_keyboard_focus = nullptr;
	t_owner<zwp_text_input_manager_v3*, zwp_text_input_manager_v3_destroy> v_text_input_manager;
	t_surface* v_input_focus = nullptr;

public:
	std::list<std::function<void()>> v_on_idle;

	t_client(std::function<void(wl_registry*, uint32_t, const char*, uint32_t)>&& a_on_global = {});
	~t_client();
	operator wl_compositor*() const
	{
		return v_compositor;
	}
	operator wl_seat*() const
	{
		return v_seat;
	}
	operator wl_cursor_theme*() const
	{
		return v_cursor_theme;
	}
	operator xkb_state*() const
	{
		return v_xkb;
	}
	operator xdg_wm_base*() const
	{
		return v_xdg_wm_base;
	}
	operator EGLDisplay() const
	{
		return v_egl_display;
	}
	t_surface* f_pointer_focus() const
	{
		return v_pointer_focus;
	}
	double f_pointer_x() const
	{
		return v_pointer_x;
	}
	double f_pointer_y() const
	{
		return v_pointer_y;
	}
	t_surface* f_keyboard_focus() const
	{
		return v_keyboard_focus;
	}
	const t_cursor* f_cursor() const
	{
		return v_cursor;
	}
	void f_cursor__(const t_cursor* a_value);
	t_surface* f_input_focus() const
	{
		return v_input_focus;
	}
	std::shared_ptr<t_input> f_new_input();
};

inline t_client& f_client()
{
	return *t_client::v_instance;
}

class t_surface
{
	static wl_callback_listener v_frame_listener;

	t_owner<wl_surface*, wl_surface_destroy> v_surface;
	EGLConfig v_egl_config;
	EGLContext v_egl_context;
	wl_egl_window* v_egl_window = NULL;
	EGLSurface v_egl_surface = EGL_NO_SURFACE;
	wl_callback* v_frame = NULL;
	std::shared_ptr<t_input> v_input;

public:
	std::function<void(uint32_t)> v_on_frame;
	std::function<void()> v_on_pointer_enter;
	std::function<void()> v_on_pointer_leave;
	std::function<void()> v_on_pointer_move;
	std::function<void(uint32_t)> v_on_button_press;
	std::function<void(uint32_t)> v_on_button_release;
	std::function<void(wl_pointer_axis, double)> v_on_scroll;
	std::function<void()> v_on_focus_enter;
	std::function<void()> v_on_focus_leave;
	std::function<void(xkb_keysym_t, char32_t)> v_on_key_press;
	std::function<void(xkb_keysym_t, char32_t)> v_on_key_release;
	std::function<void(xkb_keysym_t, char32_t)> v_on_key_repeat;
	std::function<void()> v_on_input_enable;
	std::function<void()> v_on_input_disable;
	std::function<void()> v_on_input_done;

	t_surface(bool a_depth);
	~t_surface();
	operator wl_surface*() const
	{
		return v_surface;
	}
	operator wl_egl_window*() const
	{
		return v_egl_window;
	}
	void f_create(size_t a_width, size_t a_height);
	void f_destroy();
	void f_make_current()
	{
		if (!eglMakeCurrent(f_client(), v_egl_surface, v_egl_surface, v_egl_context)) throw std::runtime_error("eglMakeCurrent");;
	}
	void f_swap_buffers()
	{
		if (!eglSwapBuffers(f_client(), v_egl_surface)) throw std::runtime_error("eglSwapBuffers");
	}
	void f_request_frame()
	{
		if (v_frame) return;
		v_frame = wl_surface_frame(v_surface);
		wl_callback_add_listener(v_frame, &v_frame_listener, this);
		wl_surface_commit(v_surface);
	}
	const std::shared_ptr<t_input>& f_input() const
	{
		return v_input;
	}
	void f_input__(const std::shared_ptr<t_input>& a_input);
};

class t_frame : public t_surface
{
	static xdg_surface_listener v_xdg_surface_listener;
	static xdg_toplevel_listener v_xdg_toplevel_listener;

	t_owner<xdg_surface*, xdg_surface_destroy> v_xdg_surface;
	t_owner<xdg_toplevel*, xdg_toplevel_destroy> v_xdg_toplevel;
	int32_t v_width = 0;
	int32_t v_height = 0;
	int32_t v_restore_width = 0;
	int32_t v_restore_height = 0;
	uint16_t v_states = 0;
	uint16_t v_capabilities = 0;
	bool v_configuring = false;
	uint32_t v_configure_serial;

	void f_resize();

public:
	std::function<void(int32_t&, int32_t&)> v_on_measure;
	std::function<void(int32_t, int32_t)> v_on_map;
	std::function<void()> v_on_unmap;
	std::function<void()> v_on_close;

	t_frame(bool a_depth);
	~t_frame();
	operator xdg_toplevel*() const
	{
		return v_xdg_toplevel;
	}
	void f_swap_buffers()
	{
		if (v_configuring) {
			v_configuring = false;
			xdg_surface_ack_configure(v_xdg_surface, v_configure_serial);
		}
		t_surface::f_swap_buffers();
	}
	bool f_is(xdg_toplevel_state a_state) const
	{
		return v_states & 1 << a_state;
	}
	bool f_has(xdg_toplevel_wm_capabilities a_capability) const
	{
		return v_capabilities & 1 << a_capability;
	}
	void f_show_window_menu(int32_t a_x, int32_t a_y)
	{
		auto& client = f_client();
		xdg_toplevel_show_window_menu(v_xdg_toplevel, client, client.v_action_serial, a_x, a_y);
	}
	void f_move()
	{
		auto& client = f_client();
		xdg_toplevel_move(v_xdg_toplevel, client, client.v_action_serial);
	}
	void f_resize(xdg_toplevel_resize_edge a_edges)
	{
		auto& client = f_client();
		xdg_toplevel_resize(v_xdg_toplevel, client, client.v_action_serial, a_edges);
	}
	void f_resize(int32_t a_width, int32_t a_height)
	{
		if (a_width > 0 && a_height > 0) {
			v_restore_width = a_width;
			v_restore_height = a_height;
		}
		if (f_is(XDG_TOPLEVEL_STATE_MAXIMIZED) || f_is(XDG_TOPLEVEL_STATE_FULLSCREEN)) return;
		v_width = a_width;
		v_height = a_height;
		f_resize();
	}
};

class t_cursor
{
	friend class t_client;

	wl_cursor* v_cursor;
	t_owner<wl_surface*, wl_surface_destroy> v_surface;

public:
	t_cursor(const char* a_name);
	~t_cursor();
};

class t_input
{
	friend class t_client;
	friend class t_surface;

	static zwp_text_input_v3_listener v_zwp_text_input_v3_listener;

	t_owner<zwp_text_input_v3*, zwp_text_input_v3_destroy> v_input;
	uint32_t v_serial = 0;
	uint32_t v_done = 0;
	std::tuple<std::string, int32_t, int32_t> v_preedit;
	std::string v_commit;
	std::tuple<uint32_t, uint32_t> v_delete;

	void f_reset()
	{
		v_preedit = {};
		v_commit = {};
		v_delete = {};
	}
	void f_enable();
	void f_disable();

public:
	t_input();
	~t_input();
	operator zwp_text_input_v3*() const
	{
		return v_input;
	}
	bool f_done() const
	{
		return v_done == v_serial;
	}
	void f_commit()
	{
		++v_serial;
		zwp_text_input_v3_commit(v_input);
	}
	const std::tuple<std::string, int32_t, int32_t>& f_preedit() const
	{
		return v_preedit;
	}
	const std::string& f_text() const
	{
		return v_commit;
	}
	const std::tuple<uint32_t, uint32_t>& f_delete() const
	{
		return v_delete;
	}
};

#pragma GCC diagnostic pop

}

#endif
