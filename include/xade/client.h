#ifndef XADE__CLIENT_H
#define XADE__CLIENT_H

#include <list>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <wayland-egl.h>
#include "xdg-shell-client.h"
#include "text-input-unstable-v3-client.h"
#include <xkbcommon/xkbcommon.h>
#include <EGL/egl.h>
#include <suisha/loop.h>

namespace xade
{

class t_surface;
class t_cursor;
class t_input;

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
	wl_display* v_display = NULL;
	wl_registry* v_registry = NULL;
	wl_compositor* v_compositor = NULL;
	wl_shm* v_shm = NULL;
	wl_seat* v_seat = NULL;
	wl_pointer* v_pointer = NULL;
	wl_cursor_theme* v_cursor_theme = NULL;
	wl_keyboard* v_keyboard = NULL;
	xkb_context* v_xkb = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	xkb_state* v_xkb_state = NULL;
	int v_repeat_rate = 0;
	int v_repeat_delay;
	std::shared_ptr<suisha::t_timer> v_repeat;
	xdg_wm_base* v_xdg_wm_base = NULL;
	EGLDisplay v_egl_display = EGL_NO_DISPLAY;
	t_surface* v_pointer_focus = nullptr;
	double v_pointer_x;
	double v_pointer_y;
	uint32_t v_cursor_serial;
	uint32_t v_action_serial;
	const t_cursor* v_cursor = nullptr;
	std::shared_ptr<suisha::t_timer> v_cursor_next;
	t_surface* v_keyboard_focus = nullptr;
	zwp_text_input_manager_v3* v_text_input_manager = NULL;
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
		return v_xkb_state;
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

	wl_surface* v_surface = wl_compositor_create_surface(f_client());
	EGLConfig v_egl_config;
	EGLContext v_egl_context = EGL_NO_CONTEXT;
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
	std::function<void(uint32_t, double)> v_on_scroll;
	std::function<void()> v_on_focus_enter;
	std::function<void()> v_on_focus_leave;
	std::function<void(xkb_keysym_t, wchar_t)> v_on_key_press;
	std::function<void(xkb_keysym_t, wchar_t)> v_on_key_release;
	std::function<void(xkb_keysym_t, wchar_t)> v_on_key_repeat;
	std::function<void()> v_on_input_enable;
	std::function<void()> v_on_input_disable;
	std::function<void()> v_on_input_done;

	t_surface();
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

	xdg_surface* v_xdg_surface = NULL;
	xdg_toplevel* v_xdg_toplevel = NULL;
	int32_t v_width = 0;
	int32_t v_height = 0;
	int32_t v_restore_width = 0;
	int32_t v_restore_height = 0;
	uint16_t v_states = 0;
	uint16_t v_capabilities = 0;

	void f_resize();

public:
	std::function<void(int32_t&, int32_t&)> v_on_measure;
	std::function<void(int32_t, int32_t)> v_on_map;
	std::function<void()> v_on_unmap;
	std::function<void()> v_on_close;

	t_frame();
	~t_frame();
	operator xdg_toplevel*() const
	{
		return v_xdg_toplevel;
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
	void f_resize(uint32_t a_edges)
	{
		auto& client = f_client();
		xdg_toplevel_resize(v_xdg_toplevel, client, client.v_action_serial, a_edges);
	}
	void f_resize(int32_t a_width, int32_t a_height)
	{
		v_width = a_width;
		v_height = a_height;
		if (v_width > 0 && v_height > 0 && !f_is(XDG_TOPLEVEL_STATE_MAXIMIZED) && !f_is(XDG_TOPLEVEL_STATE_FULLSCREEN)) {
			v_restore_width = v_width;
			v_restore_height = v_height;
		}
		f_resize();
	}
};

class t_cursor
{
	friend class t_client;

	wl_surface* v_surface = wl_compositor_create_surface(f_client());
	wl_cursor* v_cursor;

public:
	t_cursor(const char* a_name) : v_cursor(wl_cursor_theme_get_cursor(f_client(), a_name))
	{
		if (!v_surface) throw std::runtime_error("surface");
		if (!v_cursor) throw std::runtime_error(a_name);
	}
	~t_cursor()
	{
		if (v_surface) wl_surface_destroy(v_surface);
	}
};

class t_input
{
	friend class t_client;
	friend class t_surface;

	static zwp_text_input_v3_listener v_zwp_text_input_v3_listener;

	zwp_text_input_v3* v_input = zwp_text_input_manager_v3_get_text_input(f_client().v_text_input_manager, f_client());
	uint32_t v_serial = 0;
	uint32_t v_done = 0;
	std::tuple<std::string, int32_t, int32_t> v_preedit{{}, 0, -1};
	std::string v_commit;
	std::tuple<uint32_t, uint32_t> v_delete;

	void f_reset()
	{
		v_delete = {};
		v_commit = {};
		v_preedit = {{}, 0, -1};
	}
	void f_enable();
	void f_disable();

public:
	t_input()
	{
		if (!v_input) throw std::runtime_error("text input");
		zwp_text_input_v3_set_user_data(v_input, this);
		zwp_text_input_v3_add_listener(v_input, &v_zwp_text_input_v3_listener, this);
	}
	~t_input()
	{
		if (v_input) zwp_text_input_v3_destroy(v_input);
	}
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

}

#endif
