#include <stdexcept>
#include <string>
#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <wayland-server-core.h>
extern "C"
{
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_input_device.h>
#define delete delete_surrounding
#include <wlr/types/wlr_input_method_v2.h>
#undef delete
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#define static
#include <wlr/types/wlr_scene.h>
#undef static
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_text_input_v3.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>
}
#include <xkbcommon/xkbcommon.h>

/* For brevity's sake, struct members are annotated where they are used. */
enum t_cursor_mode
{
	c_CURSOR_PASSTHROUGH,
	c_CURSOR_MOVE,
	c_CURSOR_RESIZE
};

struct t_toplevel;
struct t_input_method;
struct t_text_input;

struct t_server
{
	wl_display* v_display = wl_display_create();
	wlr_backend* v_backend;
	wlr_renderer* v_renderer = NULL;
	wlr_allocator* v_allocator = NULL;
	wlr_scene* v_scene = NULL;
	wlr_scene_output_layout* v_scene_layout;

	wlr_xdg_shell* v_xdg_shell;
	wl_listener v_new_xdg_toplevel;
	wl_listener v_new_xdg_popup;
	wl_list v_toplevels;

	wlr_cursor* v_cursor = NULL;
	wlr_xcursor_manager* v_xcursor_manager = NULL;
	wl_listener v_cursor_motion;
	wl_listener v_cursor_motion_absolute;
	wl_listener v_cursor_button;
	wl_listener v_cursor_axis;
	wl_listener v_cursor_frame;

	wlr_seat* v_seat;
	wl_listener v_new_input;
	wl_listener v_request_cursor;
	wl_listener v_request_set_selection;
	wl_list v_keyboards;
	wlr_scene_surface* v_grabbed_surface = NULL;
	t_cursor_mode v_cursor_mode = c_CURSOR_PASSTHROUGH;
	t_toplevel* v_grabbed_toplevel = nullptr;
	double v_grab_x, v_grab_y;
	wlr_box v_grab_geobox;
	uint32_t v_resize_edges;

	wlr_output_layout* v_output_layout;
	wl_list v_outputs;
	wl_listener v_new_output;

	wlr_input_method_manager_v2* v_input_method_manager;
	wl_listener v_input_method_input_method;
	t_input_method* v_input_method = nullptr;
	wlr_text_input_manager_v3* v_text_input_manager;
	wl_listener v_text_input_text_input;
	wl_listener v_keyboard_state_focus_change;
	t_text_input* v_focused_text_input = nullptr;

	t_server();
	~t_server()
	{
		wl_display_destroy_clients(v_display);
		if (v_scene) wlr_scene_node_destroy(&v_scene->tree.node);
		wlr_xcursor_manager_destroy(v_xcursor_manager);
		if (v_cursor) wlr_cursor_destroy(v_cursor);
		wlr_allocator_destroy(v_allocator);
		wlr_renderer_destroy(v_renderer);
		wlr_backend_destroy(v_backend);
		wl_display_destroy(v_display);
	}
	wlr_scene_surface* f_surface_at_cursor(double* a_sx, double* a_sy)
	{
		auto node = wlr_scene_node_at(&v_scene->tree.node, v_cursor->x, v_cursor->y, a_sx, a_sy);
		return node && node->type == WLR_SCENE_NODE_BUFFER ? wlr_scene_surface_try_from_buffer(wlr_scene_buffer_from_node(node)) : nullptr;
	}
	t_toplevel* f_toplevel(wlr_scene_surface* a_surface)
	{
		if (!a_surface) return nullptr;
		auto tree = a_surface->buffer->node.parent;
		while (tree && !tree->node.data) tree = tree->node.parent;
		return tree ? static_cast<t_toplevel*>(tree->node.data) : nullptr;
	}
	void f_ungrab_pointer()
	{
		v_grabbed_surface = NULL;
		double sx, sy;
		if (auto surface = f_surface_at_cursor(&sx, &sy))
			wlr_seat_pointer_notify_enter(v_seat, surface->surface, sx, sy);
		else
			wlr_seat_pointer_clear_focus(v_seat);
	}
	void f_reset_cursor_mode()
	{
		v_cursor_mode = c_CURSOR_PASSTHROUGH;
		v_grabbed_toplevel = nullptr;
	}
	void f_process_cursor_resize(uint32_t a_time);
	void f_process_cursor_motion(uint32_t a_time);
	void f_blur_text_input();
};

struct t_output
{
	wl_list v_link;
	t_server* v_server;
	wlr_output* v_output;
	wl_listener v_frame;
	wl_listener v_request_state;
	wl_listener v_destroy;

	t_output(t_server* a_server, wlr_output* a_output);
	~t_output()
	{
		wl_list_remove(&v_link);
	}
};

struct t_toplevel
{
	wl_list v_link;
	t_server* v_server;
	wlr_xdg_toplevel* v_toplevel;
	wlr_scene_tree* v_scene_tree;
	wl_listener v_map;
	wl_listener v_unmap;
	wl_listener v_commit;
	wl_listener v_destroy;
	wl_listener v_ack_configure;
	wl_listener v_request_move;
	wl_listener v_request_resize;
	wl_listener v_request_maximize;
	wl_listener v_request_fullscreen;
	uint32_t v_resize_edges = 0;
	uint32_t v_resize_serial;
	uint32_t v_move_edges = 0;

	t_toplevel(t_server* a_server, wlr_xdg_toplevel* a_toplevel);
	void f_focus()
	{
		auto seat = v_server->v_seat;
		auto current = seat->keyboard_state.focused_surface;
		if (current == v_toplevel->base->surface) return;
		if (current) if (auto p = wlr_xdg_toplevel_try_from_wlr_surface(current)) wlr_xdg_toplevel_set_activated(p, false);
		wlr_scene_node_raise_to_top(&v_scene_tree->node);
		wl_list_remove(&v_link);
		wl_list_insert(&v_server->v_toplevels, &v_link);
		wlr_xdg_toplevel_set_activated(v_toplevel, true);
		if (auto p = wlr_seat_get_keyboard(seat)) wlr_seat_keyboard_notify_enter(seat, v_toplevel->base->surface, p->keycodes, p->num_keycodes, &p->modifiers);
	}
};

struct t_popup
{
	wlr_xdg_popup* v_popup;
	wl_listener v_commit;
	wl_listener v_destroy;

	t_popup(wlr_xdg_popup* a_popup);
};

struct t_keyboard
{
	wl_list v_link;
	t_server* v_server;
	wlr_keyboard* v_keyboard;
	wl_listener v_modifiers;
	wl_listener v_key;
	wl_listener v_destroy;

	t_keyboard(t_server* a_server, wlr_input_device* a_device);
	~t_keyboard()
	{
		wl_list_remove(&v_link);
	}
};

struct t_input_method
{
	t_server* v_server;
	wlr_input_method_v2* v_input_method;
	bool v_on = false;
	wl_listener v_commit;
	wl_listener v_new_popup_surface;
	wl_listener v_destroy;

	t_input_method(t_server* a_server, wlr_input_method_v2* a_input_method);
	~t_input_method()
	{
		v_server->v_input_method = nullptr;
	}
	operator wlr_input_method_v2*() const
	{
		return v_input_method;
	}
	void f_deactivate()
	{
		wlr_input_method_v2_send_deactivate(v_input_method);
		wlr_input_method_v2_send_done(v_input_method);
	}
};

struct t_input_popup
{
	t_server* v_server;
	wlr_input_popup_surface_v2* v_popup;
	wl_listener v_commit;
	wl_listener v_map;
	wl_listener v_unmap;
	wl_listener v_destroy;

	t_input_popup(t_server* a_server, wlr_input_popup_surface_v2* a_popup);
	void f_move(wlr_text_input_v3* a_input)
	{
		auto cursor = a_input->current.cursor_rectangle;
		auto& node0 = static_cast<wlr_scene_tree*>(wlr_xdg_surface_try_from_wlr_surface(a_input->focused_surface)->data)->node;
		cursor.x += node0.x;
		cursor.y += node0.y;
		wlr_box box;
		wlr_output_layout_get_box(v_server->v_output_layout, wlr_output_layout_output_at(v_server->v_output_layout, cursor.x, cursor.y), &box);
		auto& buffer = v_popup->surface->buffer->base;
		auto x = cursor.x;
		if (auto right = box.x + box.width; x + buffer.width > right) x = std::max(right - buffer.width, box.x);
		auto y = cursor.y + cursor.height;
		if (y + buffer.height > box.y + box.height) if (auto y1 = cursor.y - buffer.height; y1 >= box.y) y = y1;
		auto& node1 = static_cast<wlr_scene_surface*>(v_popup->surface->data)->buffer->node;
		wlr_scene_node_set_position(&node1, x, y);
		cursor.x -= node1.x;
		cursor.y -= node1.y;
		wlr_input_popup_surface_v2_send_text_input_rectangle(v_popup, &cursor);
	}
};

struct t_text_input
{
	t_server* v_server;
	wlr_text_input_v3* v_text_input;
	wl_listener v_enable;
	wl_listener v_commit;
	wl_listener v_disable;
	wl_listener v_destroy;

	t_text_input(t_server* a_server, wlr_text_input_v3* a_text_input) : v_server(a_server), v_text_input(a_text_input)
	{
		v_enable.notify = [](auto a_listener, auto a_data)
		{
			t_text_input* self = wl_container_of(a_listener, self, v_enable);
			if (self->v_text_input->focused_surface) self->f_focus();
			wlr_text_input_v3_send_done(*self);
		};
		wl_signal_add(&v_text_input->events.enable, &v_enable);
		v_commit.notify = [](auto a_listener, auto a_data)
		{
			t_text_input* self = wl_container_of(a_listener, self, v_commit);
			if (self->v_text_input->focused_surface && self->v_text_input->current_enabled) {
				if (!self->v_server->v_focused_text_input)
					self->f_focus();
				else if (auto p = self->v_server->v_input_method; p && p->v_on)
					self->f_send();
			}
			wlr_text_input_v3_send_done(*self);
		};
		wl_signal_add(&v_text_input->events.commit, &v_commit);
		v_disable.notify = [](auto a_listener, auto a_data)
		{
			t_text_input* self = wl_container_of(a_listener, self, v_disable);
			if (self->v_server->v_focused_text_input == self) self->v_server->f_blur_text_input();
			wlr_text_input_v3_send_done(*self);
		};
		wl_signal_add(&v_text_input->events.disable, &v_disable);
		v_destroy.notify = [](auto a_listener, auto a_data)
		{
			t_text_input* self = wl_container_of(a_listener, self, v_destroy);
			delete self;
		};
		wl_signal_add(&v_text_input->events.destroy, &v_destroy);
	}
	~t_text_input()
	{
		if (v_server->v_focused_text_input == this) v_server->f_blur_text_input();
	}
	operator wlr_text_input_v3*() const
	{
		return v_text_input;
	}
	void f_send()
	{
		auto p = v_server->v_input_method;
		auto& state = v_text_input->current;
		auto& surrounding = state.surrounding;
		wlr_input_method_v2_send_surrounding_text(*p, surrounding.text, surrounding.cursor, surrounding.anchor);
		wlr_input_method_v2_send_text_change_cause(*p, state.text_change_cause);
		auto& type = state.content_type;
		wlr_input_method_v2_send_content_type(*p, type.hint, type.purpose);
		wlr_input_method_v2_send_done(*p);
		wlr_input_popup_surface_v2* popup;
		wl_list_for_each(popup, &p->v_input_method->popup_surfaces, link) if (popup->surface->data) static_cast<t_input_popup*>(popup->data)->f_move(v_text_input);
	}
	void f_focus()
	{
		v_server->v_focused_text_input = this;
		if (auto p = v_server->v_input_method; p && p->v_on) {
			wlr_input_method_v2_send_activate(*p);
			f_send();
		}
	}
};

t_input_method::t_input_method(t_server* a_server, wlr_input_method_v2* a_input_method) : v_server(a_server), v_input_method(a_input_method)
{
	v_commit.notify = [](auto a_listener, auto a_data)
	{
		t_input_method* self = wl_container_of(a_listener, self, v_commit);
		if (auto p = self->v_server->v_focused_text_input) {
			auto& state = self->v_input_method->current;
			auto& preedit = state.preedit;
			wlr_text_input_v3_send_preedit_string(*p, preedit.text, preedit.cursor_begin, preedit.cursor_end);
			wlr_text_input_v3_send_commit_string(*p, state.commit_text);
			auto& ds = state.delete_surrounding;
			wlr_text_input_v3_send_delete_surrounding_text(*p, ds.before_length, ds.after_length);
			wlr_text_input_v3_send_done(*p);
		}
	};
	wl_signal_add(&v_input_method->events.commit, &v_commit);
	v_new_popup_surface.notify = [](auto a_listener, auto a_data)
	{
		t_input_method* self = wl_container_of(a_listener, self, v_new_popup_surface);
		new t_input_popup(self->v_server, static_cast<wlr_input_popup_surface_v2*>(a_data));
	};
	wl_signal_add(&v_input_method->events.new_popup_surface, &v_new_popup_surface);
	v_destroy.notify = [](auto a_listener, auto a_data)
	{
		t_input_method* self = wl_container_of(a_listener, self, v_destroy);
		delete self;
	};
	wl_signal_add(&v_input_method->events.destroy, &v_destroy);
	v_server->v_input_method = this;
}

t_input_popup::t_input_popup(t_server* a_server, wlr_input_popup_surface_v2* a_popup) : v_server(a_server), v_popup(a_popup)
{
	v_popup->data = this;
	v_commit.notify = [](auto a_listener, auto a_data)
	{
		t_input_popup* self = wl_container_of(a_listener, self, v_commit);
		if (self->v_popup->surface->data) self->f_move(*self->v_server->v_focused_text_input);
	};
	wl_signal_add(&v_popup->surface->events.commit, &v_commit);
	v_map.notify = [](auto a_listener, auto a_data)
	{
		t_input_popup* self = wl_container_of(a_listener, self, v_map);
		self->v_popup->surface->data = wlr_scene_surface_create(&self->v_server->v_scene->tree, self->v_popup->surface);
		self->f_move(*self->v_server->v_focused_text_input);
	};
	wl_signal_add(&v_popup->surface->events.map, &v_map);
	v_unmap.notify = [](auto a_listener, auto a_data)
	{
		t_input_popup* self = wl_container_of(a_listener, self, v_unmap);
		wlr_scene_node_destroy(&static_cast<wlr_scene_surface*>(self->v_popup->surface->data)->buffer->node);
		self->v_popup->surface->data = nullptr;
	};
	wl_signal_add(&v_popup->surface->events.unmap, &v_unmap);
	v_destroy.notify = [](auto a_listener, auto a_data)
	{
		t_input_popup* self = wl_container_of(a_listener, self, v_destroy);
		delete self;
	};
	wl_signal_add(&v_popup->events.destroy, &v_destroy);
}

t_keyboard::t_keyboard(t_server* a_server, wlr_input_device* a_device) : v_server(a_server), v_keyboard(wlr_keyboard_from_input_device(a_device))
{
	auto context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	auto keymap = xkb_keymap_new_from_names(context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);
	wlr_keyboard_set_keymap(v_keyboard, keymap);
	xkb_keymap_unref(keymap);
	xkb_context_unref(context);
	wlr_keyboard_set_repeat_info(v_keyboard, 25, 600);
	v_modifiers.notify = [](auto a_listener, auto a_data)
	{
		t_keyboard* self = wl_container_of(a_listener, self, v_modifiers);
		if (auto p = self->v_server->v_input_method) if (auto q = p->v_input_method->keyboard_grab) {
			wlr_input_method_keyboard_grab_v2_set_keyboard(q, self->v_keyboard);
			wlr_input_method_keyboard_grab_v2_send_modifiers(q, &self->v_keyboard->modifiers);
			return;
		}
		wlr_seat_set_keyboard(self->v_server->v_seat, self->v_keyboard);
		wlr_seat_keyboard_notify_modifiers(self->v_server->v_seat, &self->v_keyboard->modifiers);
	};
	wl_signal_add(&v_keyboard->events.modifiers, &v_modifiers);
	v_key.notify = [](auto a_listener, auto a_data)
	{
		t_keyboard* self = wl_container_of(a_listener, self, v_key);
		auto server = self->v_server;
		auto event = static_cast<wlr_keyboard_key_event*>(a_data);
		if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
			auto sym = xkb_state_key_get_one_sym(self->v_keyboard->xkb_state, event->keycode + 8);
			if (wlr_keyboard_get_modifiers(self->v_keyboard) & WLR_MODIFIER_ALT)
				switch (sym) {
				case XKB_KEY_Escape:
					wl_display_terminate(server->v_display);
					return;
				case XKB_KEY_F1:
					if (wl_list_length(&server->v_toplevels) > 1) {
						t_toplevel* p = wl_container_of(server->v_toplevels.prev, p, v_link);
						p->f_focus();
					}
					return;
				}
			if (sym == XKB_KEY_Kanji || sym == XKB_KEY_Eisu_toggle) if (auto p = server->v_input_method) if (auto q = server->v_focused_text_input) {
				p->v_on ^= true;
				if (p->v_on) {
					wlr_input_method_v2_send_activate(*p);
					q->f_send();
				} else {
					p->f_deactivate();
					wlr_text_input_v3_send_preedit_string(*q, NULL, 0, 0);
					wlr_text_input_v3_send_done(*q);
				}
				return;
			}
		}
		if (auto p = server->v_input_method) if (auto q = p->v_input_method->keyboard_grab) {
			wlr_input_method_keyboard_grab_v2_set_keyboard(q, self->v_keyboard);
			wlr_input_method_keyboard_grab_v2_send_key(q, event->time_msec, event->keycode, event->state);
			return;
		}
		auto seat = server->v_seat;
		wlr_seat_set_keyboard(seat, self->v_keyboard);
		wlr_seat_keyboard_notify_key(seat, event->time_msec, event->keycode, event->state);
	};
	wl_signal_add(&v_keyboard->events.key, &v_key);
	v_destroy.notify = [](auto a_listener, auto a_data)
	{
		t_keyboard* self = wl_container_of(a_listener, self, v_destroy);
		delete self;
	};
	wl_signal_add(&a_device->events.destroy, &v_destroy);
	wlr_seat_set_keyboard(v_server->v_seat, v_keyboard);
	wl_list_insert(&v_server->v_keyboards, &v_link);
}

t_output::t_output(t_server* a_server, wlr_output* a_output) : v_server(a_server), v_output(a_output)
{
	wlr_output_init_render(v_output, v_server->v_allocator, v_server->v_renderer);
	wlr_output_state state;
	wlr_output_state_init(&state);
	wlr_output_state_set_enabled(&state, true);
	if (auto mode = wlr_output_preferred_mode(v_output)) wlr_output_state_set_mode(&state, mode);
	wlr_output_commit_state(v_output, &state);
	wlr_output_state_finish(&state);
	v_frame.notify = [](auto a_listener, auto a_data)
	{
		t_output* self = wl_container_of(a_listener, self, v_frame);
		auto scene = self->v_server->v_scene;
		auto scene_output = wlr_scene_get_scene_output(scene, self->v_output);
		wlr_scene_output_commit(scene_output, NULL);
		timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		wlr_scene_output_send_frame_done(scene_output, &now);
	};
	wl_signal_add(&v_output->events.frame, &v_frame);
	v_request_state.notify = [](auto a_listener, auto a_data)
	{
		t_output* self = wl_container_of(a_listener, self, v_request_state);
		auto event = static_cast<wlr_output_event_request_state*>(a_data);
		wlr_output_commit_state(self->v_output, event->state);
	};
	wl_signal_add(&v_output->events.request_state, &v_request_state);
	v_destroy.notify = [](auto a_listener, auto a_data)
	{
		t_output* self = wl_container_of(a_listener, self, v_destroy);
		delete self;
	};
	wl_signal_add(&v_output->events.destroy, &v_destroy);
	wl_list_insert(&v_server->v_outputs, &v_link);
	auto l_output = wlr_output_layout_add_auto(v_server->v_output_layout, v_output);
	auto scene_output = wlr_scene_output_create(v_server->v_scene, v_output);
	wlr_scene_output_layout_add_output(v_server->v_scene_layout, l_output, scene_output);
}

t_toplevel::t_toplevel(t_server* a_server, wlr_xdg_toplevel* a_toplevel) : v_server(a_server), v_toplevel(a_toplevel)
{
	v_scene_tree = wlr_scene_xdg_surface_create(&v_server->v_scene->tree, v_toplevel->base);
	v_scene_tree->node.data = this;
	v_toplevel->base->data = v_scene_tree;
	v_map.notify = [](auto a_listener, auto a_data)
	{
		t_toplevel* self = wl_container_of(a_listener, self, v_map);
		wl_list_insert(&self->v_server->v_toplevels, &self->v_link);
		self->f_focus();
	};
	wl_signal_add(&v_toplevel->base->surface->events.map, &v_map);
	v_unmap.notify = [](auto a_listener, auto a_data)
	{
		t_toplevel* self = wl_container_of(a_listener, self, v_unmap);
		if (self->v_server->v_grabbed_surface && self->v_toplevel->base->surface == self->v_server->v_grabbed_surface->surface) self->v_server->f_ungrab_pointer();
		if (self == self->v_server->v_grabbed_toplevel) self->v_server->f_reset_cursor_mode();
		wl_list_remove(&self->v_link);
	};
	wl_signal_add(&v_toplevel->base->surface->events.unmap, &v_unmap);
	v_commit.notify = [](auto a_listener, auto a_data)
	{
		t_toplevel* self = wl_container_of(a_listener, self, v_commit);
		if (self->v_toplevel->base->initial_commit) wlr_xdg_toplevel_set_size(self->v_toplevel, 0, 0);
		if (auto edges = self->v_move_edges) {
			self->v_move_edges = 0;
			wlr_box box;
			wlr_xdg_surface_get_geometry(self->v_toplevel->base, &box);
			auto& grab = self->v_server->v_grab_geobox;
			wlr_scene_node_set_position(&self->v_scene_tree->node, edges & WLR_EDGE_LEFT ? grab.x + grab.width - box.width : grab.x, edges & WLR_EDGE_TOP ? grab.y + grab.height - box.height : grab.y);
		}
	};
	wl_signal_add(&v_toplevel->base->surface->events.commit, &v_commit);
	v_destroy.notify = [](auto a_listener, auto a_data)
	{
		t_toplevel* self = wl_container_of(a_listener, self, v_destroy);
		delete self;
	};
	wl_signal_add(&v_toplevel->events.destroy, &v_destroy);
	v_ack_configure.notify = [](auto a_listener, auto a_data)
	{
		t_toplevel* self = wl_container_of(a_listener, self, v_ack_configure);
		if (self->v_resize_edges) {
			self->v_move_edges = self->v_resize_edges;
			if (static_cast<wlr_xdg_surface_configure*>(a_data)->serial >= self->v_resize_serial) self->v_resize_edges = 0;
		}
	};
	wl_signal_add(&v_toplevel->base->events.ack_configure, &v_ack_configure);
	v_request_move.notify = [](auto a_listener, auto a_data)
	{
		// TODO: check the serial.
		t_toplevel* self = wl_container_of(a_listener, self, v_request_move);
		auto server = self->v_server;
		server->v_grabbed_toplevel = self;
		server->v_cursor_mode = c_CURSOR_MOVE;
		server->v_grab_x = server->v_cursor->x - self->v_scene_tree->node.x;
		server->v_grab_y = server->v_cursor->y - self->v_scene_tree->node.y;
	};
	wl_signal_add(&v_toplevel->events.request_move, &v_request_move);
	v_request_resize.notify = [](auto a_listener, auto a_data)
	{
		// TODO: check the serial.
		t_toplevel* self = wl_container_of(a_listener, self, v_request_resize);
		auto server = self->v_server;
		server->v_grabbed_toplevel = self;
		server->v_cursor_mode = c_CURSOR_RESIZE;
		wlr_box box;
		wlr_xdg_surface_get_geometry(self->v_toplevel->base, &box);
		auto edges = static_cast<wlr_xdg_toplevel_resize_event*>(a_data)->edges;
		auto& node = self->v_scene_tree->node;
		server->v_grab_x = server->v_cursor->x - (node.x + box.x + (edges & WLR_EDGE_RIGHT ? box.width : 0));
		server->v_grab_y = server->v_cursor->y - (node.y + box.y + (edges & WLR_EDGE_BOTTOM ? box.height : 0));
		server->v_grab_geobox = box;
		server->v_grab_geobox.x += node.x;
		server->v_grab_geobox.y += node.y;
		server->v_resize_edges = edges;
	};
	wl_signal_add(&v_toplevel->events.request_resize, &v_request_resize);
	v_request_maximize.notify = [](auto a_listener, auto a_data)
	{
		t_toplevel* self = wl_container_of(a_listener, self, v_request_maximize);
		if (self->v_toplevel->base->initialized) wlr_xdg_surface_schedule_configure(self->v_toplevel->base);
	};
	wl_signal_add(&v_toplevel->events.request_maximize, &v_request_maximize);
	v_request_fullscreen.notify = [](auto a_listener, auto a_data)
	{
		t_toplevel* self = wl_container_of(a_listener, self, v_request_fullscreen);
		if (self->v_toplevel->base->initialized) wlr_xdg_surface_schedule_configure(self->v_toplevel->base);
	};
	wl_signal_add(&v_toplevel->events.request_fullscreen, &v_request_fullscreen);
}

t_popup::t_popup(wlr_xdg_popup* a_popup) : v_popup(a_popup)
{
	auto parent = wlr_xdg_surface_try_from_wlr_surface(v_popup->parent);
	assert(parent != NULL);
	auto parent_tree = static_cast<wlr_scene_tree*>(parent->data);
	v_popup->base->data = wlr_scene_xdg_surface_create(parent_tree, v_popup->base);
	v_commit.notify = [](auto a_listener, auto a_data)
	{
		t_popup* self = wl_container_of(a_listener, self, v_commit);
		// TODO: adjust the position.
		if (self->v_popup->base->initial_commit) wlr_xdg_surface_schedule_configure(self->v_popup->base);
	};
	wl_signal_add(&v_popup->base->surface->events.commit, &v_commit);
	v_destroy.notify = [](auto a_listener, auto a_data)
	{
		t_popup* self = wl_container_of(a_listener, self, v_destroy);
		delete self;
	};
	wl_signal_add(&v_popup->events.destroy, &v_destroy);
}

void t_server::f_process_cursor_resize(uint32_t a_time)
{
	double x = v_cursor->x - v_grab_x;
	int left = v_grab_geobox.x;
	int right = v_grab_geobox.x + v_grab_geobox.width;
	if (v_resize_edges & WLR_EDGE_LEFT) {
		left = x;
		if (left >= right) left = right - 1;
	} else if (v_resize_edges & WLR_EDGE_RIGHT) {
		right = x;
		if (right <= left) right = left + 1;
	}
	double y = v_cursor->y - v_grab_y;
	int top = v_grab_geobox.y;
	int bottom = v_grab_geobox.y + v_grab_geobox.height;
	if (v_resize_edges & WLR_EDGE_TOP) {
		top = y;
		if (top >= bottom) top = bottom - 1;
	} else if (v_resize_edges & WLR_EDGE_BOTTOM) {
		bottom = y;
		if (bottom <= top) bottom = top + 1;
	}
	v_grabbed_toplevel->v_resize_edges = v_resize_edges;
	v_grabbed_toplevel->v_resize_serial = wlr_xdg_toplevel_set_size(v_grabbed_toplevel->v_toplevel, right - left, bottom - top);
}

void t_server::f_process_cursor_motion(uint32_t a_time)
{
	if (v_cursor_mode == c_CURSOR_MOVE) return wlr_scene_node_set_position(&v_grabbed_toplevel->v_scene_tree->node, v_cursor->x - v_grab_x, v_cursor->y - v_grab_y);
	if (v_cursor_mode == c_CURSOR_RESIZE) return f_process_cursor_resize(a_time);
	if (v_grabbed_surface) {
		int x, y;
		wlr_scene_node_coords(&v_grabbed_surface->buffer->node, &x, &y);
		wlr_seat_pointer_notify_motion(v_seat, a_time, v_cursor->x - x, v_cursor->y - y);
		return;
	}
	double sx, sy;
	auto surface = f_surface_at_cursor(&sx, &sy);
	if (!f_toplevel(surface)) wlr_cursor_set_xcursor(v_cursor, v_xcursor_manager, "default");
	if (surface) {
		wlr_seat_pointer_notify_enter(v_seat, surface->surface, sx, sy);
		wlr_seat_pointer_notify_motion(v_seat, a_time, sx, sy);
	} else {
		wlr_seat_pointer_clear_focus(v_seat);
	}
}

t_server::t_server() : v_backend(wlr_backend_autocreate(wl_display_get_event_loop(v_display), NULL))
{
	if (!v_backend) throw std::runtime_error("failed to create wlr_backend");
	v_renderer = wlr_renderer_autocreate(v_backend);
	if (!v_renderer) throw std::runtime_error("failed to create wlr_renderer");
	wlr_renderer_init_wl_display(v_renderer, v_display);
	v_allocator = wlr_allocator_autocreate(v_backend, v_renderer);
	if (!v_allocator) throw std::runtime_error("failed to create wlr_allocator");
	wlr_compositor_create(v_display, 5, v_renderer);
	wlr_subcompositor_create(v_display);
	wlr_data_device_manager_create(v_display);
	v_output_layout = wlr_output_layout_create(v_display);
	wl_list_init(&v_outputs);
	v_new_output.notify = [](auto a_listener, auto a_data)
	{
		t_server* self = wl_container_of(a_listener, self, v_new_output);
		new t_output(self, static_cast<wlr_output*>(a_data));
	};
	wl_signal_add(&v_backend->events.new_output, &v_new_output);
	v_scene = wlr_scene_create();
	v_scene_layout = wlr_scene_attach_output_layout(v_scene, v_output_layout);
	wl_list_init(&v_toplevels);
	v_xdg_shell = wlr_xdg_shell_create(v_display, 3);
	v_new_xdg_toplevel.notify = [](auto a_listener, auto a_data)
	{
		t_server* self = wl_container_of(a_listener, self, v_new_xdg_toplevel);
		new t_toplevel(self, static_cast<wlr_xdg_toplevel*>(a_data));
	};
	wl_signal_add(&v_xdg_shell->events.new_toplevel, &v_new_xdg_toplevel);
	v_new_xdg_popup.notify = [](auto a_listener, auto a_data)
	{
		new t_popup(static_cast<wlr_xdg_popup*>(a_data));
	};
	wl_signal_add(&v_xdg_shell->events.new_popup, &v_new_xdg_popup);
	v_cursor = wlr_cursor_create();
	wlr_cursor_attach_output_layout(v_cursor, v_output_layout);
	v_xcursor_manager = wlr_xcursor_manager_create(NULL, 24);
	v_cursor_motion.notify = [](auto a_listener, auto a_data)
	{
		t_server* self = wl_container_of(a_listener, self, v_cursor_motion);
		auto event = static_cast<wlr_pointer_motion_event*>(a_data);
		wlr_cursor_move(self->v_cursor, &event->pointer->base, event->delta_x, event->delta_y);
		self->f_process_cursor_motion(event->time_msec);
	};
	wl_signal_add(&v_cursor->events.motion, &v_cursor_motion);
	v_cursor_motion_absolute.notify = [](auto a_listener, auto a_data)
	{
		t_server* self = wl_container_of(a_listener, self, v_cursor_motion_absolute);
		auto event = static_cast<wlr_pointer_motion_absolute_event*>(a_data);
		wlr_cursor_warp_absolute(self->v_cursor, &event->pointer->base, event->x, event->y);
		self->f_process_cursor_motion(event->time_msec);
	};
	wl_signal_add(&v_cursor->events.motion_absolute, &v_cursor_motion_absolute);
	v_cursor_button.notify = [](auto a_listener, auto a_data)
	{
		t_server* self = wl_container_of(a_listener, self, v_cursor_button);
		auto event = static_cast<wlr_pointer_button_event*>(a_data);
		wlr_seat_pointer_notify_button(self->v_seat, event->time_msec, event->button, event->state);
		if (event->state == WL_POINTER_BUTTON_STATE_RELEASED) {
			if (self->v_grabbed_surface) self->f_ungrab_pointer();
			self->f_reset_cursor_mode();
		} else if (!self->v_grabbed_surface) {
			auto surface = self->f_surface_at_cursor(NULL, NULL);
			self->v_grabbed_surface = surface;
			if (auto p = self->f_toplevel(surface)) {
				p->f_focus();
			} else {
				if (auto p = self->v_seat->keyboard_state.focused_surface) if (auto q = wlr_xdg_toplevel_try_from_wlr_surface(p)) wlr_xdg_toplevel_set_activated(q, false);
				wlr_seat_keyboard_notify_clear_focus(self->v_seat);
			}
		}
	};
	wl_signal_add(&v_cursor->events.button, &v_cursor_button);
	v_cursor_axis.notify = [](wl_listener* a_listener, void* a_data)
	{
		t_server* self = wl_container_of(a_listener, self, v_cursor_axis);
		auto event = static_cast<wlr_pointer_axis_event*>(a_data);
		wlr_seat_pointer_notify_axis(self->v_seat, event->time_msec, event->orientation, event->delta, event->delta_discrete, event->source, event->relative_direction);
	};
	wl_signal_add(&v_cursor->events.axis, &v_cursor_axis);
	v_cursor_frame.notify = [](auto a_listener, auto a_data)
	{
		t_server* self = wl_container_of(a_listener, self, v_cursor_frame);
		wlr_seat_pointer_notify_frame(self->v_seat);
	};
	wl_signal_add(&v_cursor->events.frame, &v_cursor_frame);
	wl_list_init(&v_keyboards);
	v_new_input.notify = [](auto a_listener, auto a_data)
	{
		t_server* self = wl_container_of(a_listener, self, v_new_input);
		auto device = static_cast<wlr_input_device*>(a_data);
		switch (device->type) {
		case WLR_INPUT_DEVICE_KEYBOARD:
			new t_keyboard(self, device);
			break;
		case WLR_INPUT_DEVICE_POINTER:
			wlr_cursor_attach_input_device(self->v_cursor, device);
			break;
		}
		uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
		if (!wl_list_empty(&self->v_keyboards)) caps |= WL_SEAT_CAPABILITY_KEYBOARD;
		wlr_seat_set_capabilities(self->v_seat, caps);
	};
	wl_signal_add(&v_backend->events.new_input, &v_new_input);
	v_seat = wlr_seat_create(v_display, "seat0");
	v_request_cursor.notify = [](auto a_listener, auto a_data)
	{
		t_server* self = wl_container_of(a_listener, self, v_request_cursor);
		auto event = static_cast<wlr_seat_pointer_request_set_cursor_event*>(a_data);
		auto focused_client = self->v_seat->pointer_state.focused_client;
		if (focused_client == event->seat_client) wlr_cursor_set_surface(self->v_cursor, event->surface, event->hotspot_x, event->hotspot_y);
	};
	wl_signal_add(&v_seat->events.request_set_cursor, &v_request_cursor);
	v_request_set_selection.notify = [](auto a_listener, auto a_data)
	{
		t_server* self = wl_container_of(a_listener, self, v_request_set_selection);
		auto event = static_cast<wlr_seat_request_set_selection_event*>(a_data);
		wlr_seat_set_selection(self->v_seat, event->source, event->serial);
	};
	wl_signal_add(&v_seat->events.request_set_selection, &v_request_set_selection);
	v_input_method_manager = wlr_input_method_manager_v2_create(v_display);
	v_input_method_input_method.notify = [](auto a_listener, auto a_data)
	{
		t_server* self = wl_container_of(a_listener, self, v_input_method_input_method);
		auto input_method = static_cast<wlr_input_method_v2*>(a_data);
		if (self->v_input_method)
			wlr_input_method_v2_send_unavailable(input_method);
		else
			new t_input_method(self, input_method);
	};
	wl_signal_add(&v_input_method_manager->events.input_method, &v_input_method_input_method);
	v_text_input_manager = wlr_text_input_manager_v3_create(v_display);
	v_text_input_text_input.notify = [](auto a_listener, auto a_data)
	{
		t_server* self = wl_container_of(a_listener, self, v_text_input_text_input);
		new t_text_input(self, static_cast<wlr_text_input_v3*>(a_data));
	};
	wl_signal_add(&v_text_input_manager->events.text_input, &v_text_input_text_input);
	v_keyboard_state_focus_change.notify = [](auto a_listener, auto a_data)
	{
		t_server* self = wl_container_of(a_listener, self, v_keyboard_state_focus_change);
		auto event = static_cast<wlr_seat_keyboard_focus_change_event*>(a_data);
		wlr_text_input_v3* input;
		if (event->old_surface) wl_list_for_each(input, &self->v_text_input_manager->text_inputs, link) {
			if (input->focused_surface == event->old_surface) {
				self->f_blur_text_input();
				wlr_text_input_v3_send_leave(input);
			}
		}
		if (event->new_surface) wl_list_for_each(input, &self->v_text_input_manager->text_inputs, link) {
			if (input->seat == event->seat && wl_resource_get_client(input->resource) == wl_resource_get_client(event->new_surface->resource)) wlr_text_input_v3_send_enter(input, event->new_surface);
		}
	};
	wl_signal_add(&v_seat->keyboard_state.events.focus_change, &v_keyboard_state_focus_change);
}

void t_server::f_blur_text_input()
{
	if (auto p = v_input_method; p && p->v_on) p->f_deactivate();
	v_focused_text_input = nullptr;
}

int main(int argc, char* argv[])
{
	wlr_log_init(WLR_DEBUG, NULL);
	char* startup_cmd = NULL;
	for (int c; (c = getopt(argc, argv, "s:h")) != -1;) {
		switch (c) {
		case 's':
			startup_cmd = optarg;
			break;
		default:
			printf("Usage: %s [-s startup command]\n", argv[0]);
			return 0;
		}
	}
	if (optind < argc) {
		printf("Usage: %s [-s startup command]\n", argv[0]);
		return 0;
	}
	t_server server;
	auto socket = wl_display_add_socket_auto(server.v_display);
	if (!socket) return 1;
	if (!wlr_backend_start(server.v_backend)) return 1;
	setenv("WAYLAND_DISPLAY", socket, true);
	if (startup_cmd && fork() == 0) execl("/bin/sh", "/bin/sh", "-c", startup_cmd, (void*)NULL);
	wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s", socket);
	wl_display_run(server.v_display);
	return 0;
}
