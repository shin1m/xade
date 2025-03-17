#include <xade/client.h>
#include <cstring>
#include <sys/mman.h>

namespace xade
{

void t_xkb::f_keymap(int a_fd, size_t a_size)
{
	auto buffer = mmap(0, a_size, PROT_READ, MAP_PRIVATE, a_fd, 0);
	if (buffer == MAP_FAILED) throw std::system_error(errno, std::generic_category());
	auto keymap = xkb_keymap_new_from_buffer(v_context, static_cast<char*>(buffer), a_size, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
	if (!keymap) throw std::runtime_error("xkb_keymap_new_from_buffer");
	munmap(buffer, a_size);
	xkb_state_unref(v_state);
	v_state = xkb_state_new(keymap);
	xkb_keymap_unref(keymap);
	if (!v_state) throw std::runtime_error("xkb_state_new");
}

wl_registry_listener t_client::v_registry_listener = {
	[](auto a_data, auto a_this, auto a_name, auto a_interface, auto a_version)
	{
		auto& self = *static_cast<t_client*>(a_data);
		if (std::strcmp(a_interface, wl_compositor_interface.name) == 0)
			self.v_compositor = static_cast<wl_compositor*>(wl_registry_bind(a_this, a_name, &wl_compositor_interface, std::min<uint32_t>(a_version, wl_compositor_interface.version)));
		else if (std::strcmp(a_interface, wl_shm_interface.name) == 0)
			self.v_shm = static_cast<wl_shm*>(wl_registry_bind(a_this, a_name, &wl_shm_interface, std::min<uint32_t>(a_version, wl_shm_interface.version)));
		else if (std::strcmp(a_interface, wl_seat_interface.name) == 0)
			self.v_seat = static_cast<wl_seat*>(wl_registry_bind(a_this, a_name, &wl_seat_interface, std::min<uint32_t>(a_version, wl_seat_interface.version)));
		else if (std::strcmp(a_interface, xdg_wm_base_interface.name) == 0)
			self.v_xdg_wm_base = static_cast<xdg_wm_base*>(wl_registry_bind(a_this, a_name, &xdg_wm_base_interface, std::min<uint32_t>(a_version, xdg_wm_base_interface.version)));
		else if (std::strcmp(a_interface, zwp_text_input_manager_v3_interface.name) == 0)
			self.v_text_input_manager = static_cast<zwp_text_input_manager_v3*>(wl_registry_bind(a_this, a_name, &zwp_text_input_manager_v3_interface, std::min<uint32_t>(a_version, zwp_text_input_manager_v3_interface.version)));
		else if (auto& on = self.v_on_global)
			on(a_this, a_name, a_interface, a_version);
	},
	[](auto a_data, auto a_this, auto a_name)
	{
	}
};
wl_seat_listener t_client::v_seat_listener = {
	[](auto a_data, auto a_this, auto a_capabilities)
	{
		auto& self = *static_cast<t_client*>(a_data);
		if (a_capabilities & WL_SEAT_CAPABILITY_POINTER) {
			self.v_pointer = wl_seat_get_pointer(a_this);
			wl_pointer_add_listener(self.v_pointer, &v_pointer_listener, &self);
		}
		if (a_capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
			self.v_keyboard = wl_seat_get_keyboard(a_this);
			wl_keyboard_add_listener(self.v_keyboard, &v_keyboard_listener, &self);
		}
	},
	[](auto a_data, auto a_this, auto a_name)
	{
	}
};
wl_pointer_listener t_client::v_pointer_listener = {
	[](auto a_data, auto a_this, auto a_serial, auto a_surface, auto a_x, auto a_y)
	{
		auto& self = *static_cast<t_client*>(a_data);
		auto focus = static_cast<t_surface*>(wl_surface_get_user_data(a_surface));
		self.v_pointer_focus = focus;
		self.v_pointer_x = wl_fixed_to_double(a_x);
		self.v_pointer_y = wl_fixed_to_double(a_y);
		self.v_cursor_serial = a_serial;
		if (auto& on = focus->v_on_pointer_enter) on();
	},
	[](auto a_data, auto a_this, auto a_serial, auto a_surface)
	{
		auto& self = *static_cast<t_client*>(a_data);
		auto focus = static_cast<t_surface*>(wl_surface_get_user_data(a_surface));
		if (auto& on = focus->v_on_pointer_leave) on();
		self.v_pointer_focus = nullptr;
		self.f_cursor__(nullptr);
	},
	[](auto a_data, auto a_this, auto a_time, auto a_x, auto a_y)
	{
		auto& self = *static_cast<t_client*>(a_data);
		self.v_pointer_x = wl_fixed_to_double(a_x);
		self.v_pointer_y = wl_fixed_to_double(a_y);
		if (auto focus = self.v_pointer_focus) if (auto& on = focus->v_on_pointer_move) on();
	},
	[](auto a_data, auto a_this, auto a_serial, auto a_time, auto a_button, auto a_state)
	{
		auto& self = *static_cast<t_client*>(a_data);
		self.v_action_serial = a_serial;
		if (auto focus = self.v_pointer_focus) {
			auto& on = a_state == WL_POINTER_BUTTON_STATE_PRESSED ? focus->v_on_button_press : focus->v_on_button_release;
			if (on) on(a_button);
		}
	},
	[](auto a_data, auto a_this, auto a_time, auto a_axis, auto a_value)
	{
		if (auto focus = static_cast<t_client*>(a_data)->v_pointer_focus)
			if (auto& on = focus->v_on_scroll) on(a_axis, wl_fixed_to_double(a_value));
	},
	[](auto a_data, auto a_this)
	{
	},
	[](auto a_data, auto a_this, auto a_source)
	{
	},
	[](auto a_data, auto a_this, auto a_time, auto a_axis)
	{
	},
	[](auto a_data, auto a_this, auto a_axis, auto a_discrete)
	{
	},
	[](auto a_data, auto a_this, auto a_axis, auto a_value120)
	{
	},
	[](auto a_data, auto a_this, auto a_axis, auto a_direction)
	{
	}
};
wl_keyboard_listener t_client::v_keyboard_listener = {
	[](auto a_data, auto a_this, auto a_format, auto a_fd, auto a_size)
	{
		if (a_format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) static_cast<t_client*>(a_data)->v_xkb.f_keymap(a_fd, a_size);
	},
	[](auto a_data, auto a_this, auto a_serial, auto a_surface, auto a_keys)
	{
		auto focus = static_cast<t_surface*>(wl_surface_get_user_data(a_surface));
		static_cast<t_client*>(a_data)->v_keyboard_focus = focus;
		if (auto& on = focus->v_on_focus_enter) on();
	},
	[](auto a_data, auto a_this, auto a_serial, auto a_surface)
	{
		auto focus = static_cast<t_surface*>(wl_surface_get_user_data(a_surface));
		if (auto& on = focus->v_on_focus_leave) on();
		static_cast<t_client*>(a_data)->v_keyboard_focus = nullptr;
	},
	[](auto a_data, auto a_this, auto a_serial, auto a_time, auto a_key, auto a_state)
	{
		auto& self = *static_cast<t_client*>(a_data);
		self.v_action_serial = a_serial;
		self.v_xkb.f_stop();
		if (auto focus = self.v_keyboard_focus) self.v_xkb.f_key(a_key, a_state, [focus](auto sym, auto c)
		{
			if (auto& on = focus->v_on_key_press) on(sym, c);
		}, [&self](auto sym, auto c)
		{
			if (auto focus = self.v_keyboard_focus) if (auto& on = focus->v_on_key_repeat) on(sym, c);
		}, [focus](auto sym, auto c)
		{
			if (auto& on = focus->v_on_key_release) on(sym, c);
		});
	},
	[](auto a_data, auto a_this, auto a_serial, auto a_depressed, auto a_latched, auto a_locked, auto a_group)
	{
		auto& self = *static_cast<t_client*>(a_data);
		if (self.v_xkb) xkb_state_update_mask(self, a_depressed, a_latched, a_locked, a_group, a_group, a_group);
	},
	[](auto a_data, auto a_this, auto a_rate, auto a_delay)
	{
		static_cast<t_client*>(a_data)->v_xkb.f_repeat(a_rate, a_delay);
	}
};
xdg_wm_base_listener t_client::v_xdg_wm_base_listener = {
	[](auto a_data, auto a_this, auto a_serial)
	{
		xdg_wm_base_pong(a_this, a_serial);
	}
};

t_client::t_client(std::function<void(wl_registry*, uint32_t, const char*, uint32_t)>&& a_on_global) : v_on_global(std::move(a_on_global))
{
	if (v_instance) throw std::runtime_error("already exists");
	v_display = wl_display_connect(NULL);
	if (!v_display) throw std::runtime_error("wl_display_connect");
	v_registry = wl_display_get_registry(v_display);
	wl_registry_add_listener(v_registry, &v_registry_listener, this);
	if (wl_display_roundtrip(v_display) == -1) throw std::runtime_error("wl_display_roundtrip");
	if (!v_compositor) throw std::runtime_error("compositor");
	if (!v_shm) throw std::runtime_error("shm");
	if (!v_seat) throw std::runtime_error("seat");
	wl_seat_add_listener(v_seat, &v_seat_listener, this);
	v_cursor_theme = wl_cursor_theme_load(NULL, 24, v_shm);
	if (!v_cursor_theme) throw std::runtime_error("cursor theme");
	if (!v_xdg_wm_base) throw std::runtime_error("xdg_wm_base");
	xdg_wm_base_add_listener(v_xdg_wm_base, &v_xdg_wm_base_listener, this);
	v_egl_display = eglGetDisplay(v_display);
	if (v_egl_display == EGL_NO_DISPLAY) throw std::runtime_error("eglGetDisplay");
	if (eglInitialize(v_egl_display, NULL, NULL) != EGL_TRUE) throw std::runtime_error("eglInitialize");
	auto& loop = suisha::f_loop();
	loop.f_poll(wl_display_get_fd(v_display), true, false, [](auto, auto)
	{
	});
	loop.v_wait = [this, wait = std::move(loop.v_wait)]
	{
		if (wl_display_prepare_read(v_display) == 0) {
			for (auto& x : v_on_idle) x();
		} else {
			do if (wl_display_dispatch_pending(v_display) == -1) throw std::runtime_error("wl_display_dispatch_pending"); while (wl_display_prepare_read(v_display) != 0);
			suisha::f_loop().f_more();
		}
		suisha::f_loop().f_poll(wl_display_get_fd(v_display), true, wl_display_flush(v_display) == -1 && errno == EAGAIN);
		try {
			wait();
			if (wl_display_read_events(v_display) == -1) throw std::system_error(errno, std::generic_category());
		} catch (...) {
			wl_display_cancel_read(v_display);
			throw;
		}
	};
	v_instance = this;
}

t_client::~t_client()
{
	v_instance = nullptr;
	eglMakeCurrent(v_egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

void t_client::f_cursor__(const t_cursor* a_value)
{
	if (a_value == v_cursor) return;
	if (v_cursor_next) {
		v_cursor_next->f_stop();
		v_cursor_next = {};
	}
	v_cursor = a_value;
	if (!v_cursor) return wl_pointer_set_cursor(v_pointer, v_cursor_serial, NULL, 0, 0);
	auto begin = std::chrono::steady_clock::now();
	auto draw = [this, begin](auto draw, auto a_time) -> void
	{
		uint32_t duration;
		auto image = v_cursor->v_cursor->images[wl_cursor_frame_and_duration(v_cursor->v_cursor, std::chrono::floor<std::chrono::milliseconds>(a_time - begin).count(), &duration)];
		wl_surface_damage(v_cursor->v_surface, 0, 0, image->width, image->height);
		wl_surface_attach(v_cursor->v_surface, wl_cursor_image_get_buffer(image), 0, 0);
		wl_surface_commit(v_cursor->v_surface);
		wl_pointer_set_cursor(v_pointer, v_cursor_serial, v_cursor->v_surface, image->hotspot_x, image->hotspot_y);
		if (duration > 0) v_cursor_next = suisha::f_loop().f_timer([draw]
		{
			draw(draw, std::chrono::steady_clock::now());
		}, std::chrono::milliseconds(duration), true);
	};
	draw(draw, begin);
}

std::shared_ptr<t_input> t_client::f_new_input()
{
	return v_text_input_manager ? std::make_shared<t_input>() : nullptr;
}

wl_callback_listener t_surface::v_frame_listener = {
	[](auto a_data, auto a_this, auto a_time)
	{
		auto& self = *static_cast<t_surface*>(a_data);
		wl_callback_destroy(self.v_frame);
		self.v_frame = NULL;
		if (auto& on = self.v_on_frame) on(a_time);
	}
};

t_surface::t_surface(bool a_depth) : v_surface(wl_compositor_create_surface(f_client()))
{
	if (!v_surface) throw std::runtime_error("surface");
	wl_surface_set_user_data(v_surface, this);
	{
		EGLint attributes[] = {
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_RED_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE, 8,
			EGL_ALPHA_SIZE, 8,
			EGL_DEPTH_SIZE, a_depth ? 16 : 0,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_NONE
		};
		EGLint n;
		eglChooseConfig(f_client(), attributes, &v_egl_config, 1, &n);
		if (n < 1) throw std::runtime_error("eglChooseConfig");
	}
	EGLint attributes[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	v_egl_context = eglCreateContext(f_client(), v_egl_config, EGL_NO_CONTEXT, attributes);
}

t_surface::~t_surface()
{
	if (v_frame) wl_callback_destroy(v_frame);
	f_destroy();
	if (v_egl_context != EGL_NO_CONTEXT) eglDestroyContext(f_client(), v_egl_context);
}

void t_surface::f_create(size_t a_width, size_t a_height)
{
	v_egl_window = wl_egl_window_create(v_surface, a_width, a_height);
	if (!v_egl_window) throw std::runtime_error("wl_egl_window_create");
	v_egl_surface = eglCreateWindowSurface(f_client(), v_egl_config, v_egl_window, NULL);
	if (v_egl_surface == EGL_NO_SURFACE) throw std::runtime_error("eglCreateWindowSurface");
}

void t_surface::f_destroy()
{
	if (v_egl_surface != EGL_NO_SURFACE) {
		eglDestroySurface(f_client(), v_egl_surface);
		v_egl_surface = EGL_NO_SURFACE;
	}
	if (v_egl_window) {
		wl_egl_window_destroy(v_egl_window);
		v_egl_window = NULL;
	}
}

void t_surface::f_input__(const std::shared_ptr<t_input>& a_input)
{
	if (a_input == v_input) return;
	if (v_input && this == f_client().f_input_focus()) {
		v_input->f_disable();
		zwp_text_input_v3_disable(*v_input);
		v_input->f_commit();
	}
	v_input = a_input;
	if (v_input && this == f_client().f_input_focus()) v_input->f_enable();
}

xdg_surface_listener t_frame::v_xdg_surface_listener = {
	[](auto a_data, auto a_this, auto a_serial)
	{
		auto& self = *static_cast<t_frame*>(a_data);
		self.v_configuring = true;
		self.v_configure_serial = a_serial;
		if (!self.f_is(XDG_TOPLEVEL_STATE_MAXIMIZED) && !self.f_is(XDG_TOPLEVEL_STATE_FULLSCREEN)) {
			if (self.v_width <= 0) self.v_width = self.v_restore_width;
			if (self.v_height <= 0) self.v_height = self.v_restore_height;
			if (auto& on = self.v_on_measure) on(self.v_width, self.v_height);
			self.v_restore_width = self.v_width;
			self.v_restore_height = self.v_height;
		}
		self.f_resize();
	}
};
xdg_toplevel_listener t_frame::v_xdg_toplevel_listener = {
	[](auto a_data, auto a_this, auto a_width, auto a_height, auto a_states)
	{
		auto& self = *static_cast<t_frame*>(a_data);
		self.v_width = a_width;
		self.v_height = a_height;
		self.v_states = 0;
		for (auto p = static_cast<xdg_toplevel_state*>(a_states->data); p < static_cast<void*>(static_cast<char*>(a_states->data) + a_states->size); ++p) if (*p < sizeof(v_states) * 8) self.v_states |= 1 << *p;
	},
	[](auto a_data, auto a_this)
	{
		if (auto& on = static_cast<t_frame*>(a_data)->v_on_close) on();
	},
	[](auto a_data, auto a_this, auto a_width, auto a_height)
	{
	},
	[](auto a_data, auto a_this, auto a_capabilities)
	{
		auto& self = *static_cast<t_frame*>(a_data);
		self.v_capabilities = 0;
		for (auto p = static_cast<xdg_toplevel_wm_capabilities*>(a_capabilities->data); p < static_cast<void*>(static_cast<char*>(a_capabilities->data) + a_capabilities->size); ++p) if (*p < sizeof(v_capabilities) * 8) self.v_capabilities |= 1 << *p;
	}
};

void t_frame::f_resize()
{
	auto ack = [&]
	{
		if (!v_configuring) return;
		v_configuring = false;
		xdg_surface_ack_configure(v_xdg_surface, v_configure_serial);
		wl_surface_commit(*this);
	};
	bool map = !static_cast<wl_egl_window*>(*this);
	if (map) {
		if (v_width <= 0 || v_height <= 0) return ack();
		f_create(v_width, v_height);
	} else {
		int width;
		int height;
		wl_egl_window_get_attached_size(*this, &width, &height);
		if (v_width == width && v_height == height) return ack();
		if (v_width <= 0 || v_height <= 0) {
			if (auto& on = v_on_unmap) on();
			f_destroy();
			return ack();
		}
		wl_egl_window_resize(*this, v_width, v_height, 0, 0);
		if (f_is(XDG_TOPLEVEL_STATE_MAXIMIZED)) map = true;
	}
	if (auto& on = v_on_map) on(v_width, v_height);
	if (map)
		v_on_frame(0);
	else
		f_request_frame();
}

t_frame::t_frame(bool a_depth) : t_surface(a_depth), v_xdg_surface(xdg_wm_base_get_xdg_surface(f_client(), *this))
{
	if (!v_xdg_surface) throw std::runtime_error("xdg surface");
	xdg_surface_add_listener(v_xdg_surface, &v_xdg_surface_listener, this);
	v_xdg_toplevel = xdg_surface_get_toplevel(v_xdg_surface);
	if (!v_xdg_toplevel) throw std::runtime_error("xdg toplevel");
	xdg_toplevel_add_listener(v_xdg_toplevel, &v_xdg_toplevel_listener, this);
	wl_surface_commit(*this);
}

t_frame::~t_frame()
{
}

t_cursor::t_cursor(const char* a_name) : v_cursor(wl_cursor_theme_get_cursor(f_client(), a_name))
{
	if (!v_cursor) throw std::runtime_error(a_name);
	v_surface = wl_compositor_create_surface(f_client());
	if (!v_surface) throw std::runtime_error("surface");
}

t_cursor::~t_cursor()
{
	if (this == f_client().f_cursor()) f_client().f_cursor__(nullptr);
}

zwp_text_input_v3_listener t_input::v_zwp_text_input_v3_listener = {
	[](auto a_data, auto a_this, auto a_surface)
	{
		if (f_client().v_input_focus) return;
		auto focus = static_cast<t_surface*>(wl_surface_get_user_data(a_surface));
		f_client().v_input_focus = focus;
		if (auto& p = focus->f_input()) p->f_enable();
	},
	[](auto a_data, auto a_this, auto a_surface)
	{
		if (!f_client().v_input_focus) return;
		auto focus = static_cast<t_surface*>(wl_surface_get_user_data(a_surface));
		if (auto& p = focus->f_input()) p->f_disable();
		f_client().v_input_focus = nullptr;
	},
	[](auto a_data, auto a_this, auto a_text, auto a_begin, auto a_end)
	{
		static_cast<t_input*>(a_data)->v_preedit = {a_text ? a_text : "", a_begin, a_end};
	},
	[](auto a_data, auto a_this, auto a_text)
	{
		static_cast<t_input*>(a_data)->v_commit = a_text ? a_text : "";
	},
	[](auto a_data, auto a_this, auto a_before, auto a_after)
	{
		static_cast<t_input*>(a_data)->v_delete = {a_before, a_after};
	},
	[](auto a_data, auto a_this, auto a_serial)
	{
		auto& self = *static_cast<t_input*>(a_data);
		self.v_done = a_serial;
		if (auto focus = f_client().v_input_focus) if (auto& on = focus->v_on_input_done) on();
		self.f_reset();
	}
};

t_input::t_input() : v_input(zwp_text_input_manager_v3_get_text_input(f_client().v_text_input_manager, f_client()))
{
	if (!v_input) throw std::runtime_error("text input");
	zwp_text_input_v3_add_listener(v_input, &v_zwp_text_input_v3_listener, this);
}

t_input::~t_input()
{
}

void t_input::f_enable()
{
	zwp_text_input_v3_enable(v_input);
	if (auto& on = f_client().v_input_focus->v_on_input_enable) on();
	f_commit();
}

void t_input::f_disable()
{
	f_reset();
	if (auto& on = f_client().v_input_focus->v_on_input_disable) on();
}

}
