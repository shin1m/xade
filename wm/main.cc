#include <functional>
#include <stdexcept>
#include <string>
#include <cassert>
#include <cstdlib>
#include <cstdio>
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
#define namespace surface_namespace
#include <wlr/types/wlr_layer_shell_v1.h>
#undef namespace
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
#include <xade/owner.h>

using xade::t_owner;

struct t_server;
struct t_input_method;
struct t_text_input;

struct t_focusable
{
	virtual void f_focus() = 0;
};

struct t_output
{
	wl_list v_link;
	t_server* v_server;
	wlr_output* v_output;
	wl_listener v_frame;
	wl_listener v_commit;
	wl_listener v_request_state;
	wl_listener v_destroy;
	wlr_scene_tree* v_layers[4];

	t_output(t_server* a_server, wlr_output* a_output);
	~t_output()
	{
		for (auto p : v_layers) wlr_scene_node_destroy(&p->node);
		wl_list_remove(&v_link);
		wl_list_remove(&v_frame.link);
		wl_list_remove(&v_commit.link);
		wl_list_remove(&v_request_state.link);
		wl_list_remove(&v_destroy.link);
	}
	void f_configure(zwlr_layer_shell_v1_layer a_layer);
};

struct t_server
{
	t_owner<wl_display*, wl_display_destroy> v_display = wl_display_create();
	t_owner<wlr_backend*, wlr_backend_destroy> v_backend;
	t_owner<wlr_renderer*, wlr_renderer_destroy> v_renderer;
	t_owner<wlr_allocator*, wlr_allocator_destroy> v_allocator;
	wlr_output_layout* v_output_layout;
	wl_list v_outputs;
	wl_listener v_new_output;
	t_owner<wlr_scene*, [](auto a_p)
	{
		wlr_scene_node_destroy(&a_p->tree.node);
	}> v_scene;
	wlr_scene_output_layout* v_scene_layout;
	wlr_scene_tree* v_tree;
	wlr_scene_tree* v_layers[4];

	wlr_xdg_shell* v_xdg_shell;
	wl_listener v_xdg_new_toplevel;
	wl_listener v_xdg_new_popup;
	wl_list v_toplevels;

	wlr_layer_shell_v1* v_layer_shell;
	wl_listener v_layer_new_surface;

	t_owner<wlr_cursor*, wlr_cursor_destroy> v_cursor;
	t_owner<wlr_xcursor_manager*, wlr_xcursor_manager_destroy> v_xcursor_manager;
	wl_listener v_cursor_motion;
	wl_listener v_cursor_motion_absolute;
	wl_listener v_cursor_button;
	wl_listener v_cursor_axis;
	wl_listener v_cursor_frame;

	wlr_seat* v_seat;
	wl_listener v_new_input;
	wl_listener v_request_set_cursor;
	wl_listener v_request_set_selection;
	wl_list v_keyboards;
	static void f_on_unmap_none(wlr_xdg_surface* a_surface)
	{
	}
	std::function<void(wlr_xdg_surface*)> v_on_unmap = f_on_unmap_none;
	void f_on_unmap_ungrab(wlr_surface* a_surface)
	{
		v_on_unmap = [&, a_surface](auto a_unmapped)
		{
			if (a_unmapped->surface == a_surface) f_ungrab_pointer();
		};
	}
	std::function<void(uint32_t)> v_on_cursor_motion;
	std::function<void(uint32_t)> v_on_cursor_motion_forward;
	std::function<void(wl_pointer_button_state)> v_on_cursor_button;
	std::function<void(wl_pointer_button_state)> v_on_cursor_button_grab;
	std::function<void(wl_pointer_button_state)> v_on_cursor_button_ungrab;

	wlr_input_method_manager_v2* v_input_method_manager;
	wl_listener v_input_method_input_method;
	t_input_method* v_input_method = nullptr;
	wlr_text_input_manager_v3* v_text_input_manager;
	wl_listener v_text_input_text_input;
	wl_listener v_keyboard_state_focus_change;
	t_text_input* v_focused_text_input = nullptr;

	t_server();
	~t_server();
	t_output* f_first_output() const
	{
		t_output* p;
		wl_list_for_each(p, &v_outputs, v_link) return p;
		throw std::runtime_error("no output");
	}
	wlr_scene_surface* f_surface_at_cursor(double* a_sx, double* a_sy)
	{
		auto node = wlr_scene_node_at(&v_scene->tree.node, v_cursor->x, v_cursor->y, a_sx, a_sy);
		return node && node->type == WLR_SCENE_NODE_BUFFER ? wlr_scene_surface_try_from_buffer(wlr_scene_buffer_from_node(node)) : NULL;
	}
	t_focusable* f_focusable(wlr_scene_surface* a_surface)
	{
		if (!a_surface) return nullptr;
		auto tree = a_surface->buffer->node.parent;
		while (tree && !tree->node.data) tree = tree->node.parent;
		return tree ? static_cast<t_focusable*>(tree->node.data) : nullptr;
	}
	void f_pointer_motion(uint32_t* a_time)
	{
		double sx, sy;
		auto surface = f_surface_at_cursor(&sx, &sy);
		if (!f_focusable(surface)) wlr_cursor_set_xcursor(v_cursor, v_xcursor_manager, "default");
		if (surface) {
			wlr_seat_pointer_notify_enter(v_seat, surface->surface, sx, sy);
			if (a_time) wlr_seat_pointer_notify_motion(v_seat, *a_time, sx, sy);
		} else {
			wlr_seat_pointer_clear_focus(v_seat);
		}
	};
	void f_ungrab_pointer()
	{
		v_on_unmap = f_on_unmap_none;
		v_on_cursor_motion = v_on_cursor_motion_forward;
		v_on_cursor_button = v_on_cursor_button_grab;
		f_pointer_motion(nullptr);
	}
	void f_emit_cursor_motion()
	{
		wlr_seat_pointer_warp(v_seat, NAN, NAN);
		timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		v_on_cursor_motion(static_cast<int64_t>(now.tv_sec) * 1000 + now.tv_nsec / 1000000);
	}
	void f_blur_text_input();
};

struct t_toplevel : t_focusable
{
	wl_list v_link;
	t_server* v_server;
	wlr_xdg_toplevel* v_toplevel;
	wlr_scene_tree* v_scene_tree;
	wl_listener v_map;
	wl_listener v_unmap;
	wl_listener v_commit;
	wl_listener v_ack_configure;
	wl_listener v_destroy;
	wl_listener v_request_move;
	wl_listener v_request_resize;
	wl_listener v_request_maximize;
	wl_listener v_request_fullscreen;
	wl_listener v_output_commit;
	wl_listener v_output_destroy;
	static void f_on_commit_none()
	{
	};
	std::function<void()> v_on_commit = f_on_commit_none;
	static void f_on_ack_configure_none(uint32_t a_serial)
	{
	};
	std::function<void(uint32_t)> v_on_ack_configure = f_on_ack_configure_none;
	wlr_box v_restore;
	wlr_scene_rect* v_fullscreen = nullptr;

	t_toplevel(t_server* a_server, wlr_xdg_toplevel* a_toplevel);
	~t_toplevel()
	{
		wl_list_remove(&v_map.link);
		wl_list_remove(&v_unmap.link);
		wl_list_remove(&v_commit.link);
		wl_list_remove(&v_ack_configure.link);
		wl_list_remove(&v_destroy.link);
		wl_list_remove(&v_request_move.link);
		wl_list_remove(&v_request_resize.link);
		wl_list_remove(&v_request_maximize.link);
		wl_list_remove(&v_request_fullscreen.link);
		f_detach_output();
	}
	virtual void f_focus()
	{
		auto seat = v_server->v_seat;
		auto current = seat->keyboard_state.focused_surface;
		auto surface = v_toplevel->base->surface;
		if (current == surface) return;
		if (current) if (auto p = wlr_xdg_toplevel_try_from_wlr_surface(current)) wlr_xdg_toplevel_set_activated(p, false);
		wlr_scene_node_raise_to_top(&v_scene_tree->node);
		wl_list_remove(&v_link);
		wl_list_insert(&v_server->v_toplevels, &v_link);
		wlr_xdg_toplevel_set_activated(v_toplevel, true);
		if (auto p = wlr_seat_get_keyboard(seat)) wlr_seat_keyboard_notify_enter(seat, surface, p->keycodes, p->num_keycodes, &p->modifiers);
	}
	void f_detach_output()
	{
		if (!v_output_commit.link.next) return;
		wl_list_remove(&v_output_commit.link);
		wl_list_remove(&v_output_destroy.link);
	}
	void f_try_save_geometry()
	{
		if (auto& x = v_toplevel->current; x.maximized || x.fullscreen) return;
		v_restore = v_toplevel->base->geometry;
		v_restore.x += v_scene_tree->node.x;
		v_restore.y += v_scene_tree->node.y;
	}
	void f_resize(int32_t a_width, int32_t a_height, auto a_do)
	{
		v_on_ack_configure = [this, a_do, serial = wlr_xdg_toplevel_set_size(v_toplevel, a_width, a_height)](auto a_serial)
		{
			v_on_commit = [this, a_do]
			{
				a_do();
				v_on_commit = f_on_commit_none;
			};
			if (a_serial >= serial) v_on_ack_configure = f_on_ack_configure_none;
		};
	}
	void f_restore()
	{
		f_detach_output();
		v_output_commit.link.next = NULL;
		f_resize(v_restore.width, v_restore.height, [this]
		{
			wlr_scene_node_set_position(&v_scene_tree->node, v_restore.x, v_restore.y);
			if (v_fullscreen) {
				wlr_scene_node_destroy(&v_fullscreen->node);
				v_fullscreen = nullptr;
			}
			v_server->f_emit_cursor_motion();
		});
	}
	wlr_output* f_primary_output() const
	{
		wlr_output* output = NULL;
		wlr_scene_node_for_each_buffer(&v_scene_tree->node, [](auto a_buffer, auto, auto, auto a_data)
		{
			auto p = static_cast<wlr_output**>(a_data);
			if (!*p) if (auto q = a_buffer->primary_output) *p = q->output;
		}, &output);
		return output ? output : v_server->f_first_output()->v_output;
	}
	void f_maximize()
	{
		auto output = f_primary_output();
		f_detach_output();
		wl_signal_add(&output->events.commit, &v_output_commit);
		wl_signal_add(&output->events.destroy, &v_output_destroy);
		wlr_box box;
		wlr_output_layout_get_box(v_server->v_output_layout, output, &box);
		f_resize(box.width, box.height, [this, box]
		{
			auto& resized = v_toplevel->base->geometry;
			wlr_scene_node_set_position(&v_scene_tree->node, (box.width - resized.width) / 2, (box.height - resized.height) / 2);
			if (v_fullscreen) {
				wlr_scene_node_destroy(&v_fullscreen->node);
				v_fullscreen = nullptr;
			}
			v_server->f_emit_cursor_motion();
		});
	}
	void f_fullscreen(wlr_output* a_output)
	{
		if (!a_output) a_output = f_primary_output();
		f_detach_output();
		wl_signal_add(&a_output->events.commit, &v_output_commit);
		wl_signal_add(&a_output->events.destroy, &v_output_destroy);
		wlr_box box;
		wlr_output_layout_get_box(v_server->v_output_layout, a_output, &box);
		f_resize(box.width, box.height, [this, box]
		{
			auto& resized = v_toplevel->base->geometry;
			wlr_scene_node_set_position(&v_scene_tree->node, (box.width - resized.width) / 2, (box.height - resized.height) / 2);
			if (v_fullscreen) {
				wlr_scene_rect_set_size(v_fullscreen, box.width, box.height);
			} else {
				float color[] = {0.0f, 0.0f, 0.0f, 1.0f};
				v_fullscreen = wlr_scene_rect_create(v_scene_tree, box.width, box.height, color);
				wlr_scene_node_lower_to_bottom(&v_fullscreen->node);
			}
			v_server->f_emit_cursor_motion();
		});
	}
};

struct t_popup
{
	t_server* v_server;
	wlr_xdg_popup* v_popup;
	wl_listener v_commit;
	wl_listener v_destroy;
	wl_listener v_reposition;

	t_popup(t_server* a_server, wlr_xdg_popup* a_popup);
	~t_popup()
	{
		v_server->v_on_unmap(v_popup->base);
		wl_list_remove(&v_commit.link);
		wl_list_remove(&v_destroy.link);
		wl_list_remove(&v_reposition.link);
	}
	void f_place()
	{
		int x;
		int y;
		wlr_xdg_popup_get_toplevel_coords(v_popup, 0, 0, &x, &y);
		auto layout = v_server->v_output_layout;
		wlr_box box;
		wlr_output_layout_get_box(layout, wlr_output_layout_output_at(layout, x, y), &box);
		wlr_xdg_popup_unconstrain_from_box(v_popup, &box);
	};
};

struct t_layer_surface : t_focusable
{
	t_server* v_server;
	wlr_layer_surface_v1* v_surface;
	wl_listener v_commit;
	wl_listener v_destroy;
	wl_listener v_node_destroy;
	zwlr_layer_shell_v1_layer v_layer = ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND;

	t_layer_surface(t_server* a_server, wlr_layer_surface_v1* a_surface);
	~t_layer_surface()
	{
		wl_list_remove(&v_commit.link);
		wl_list_remove(&v_destroy.link);
		wl_list_remove(&v_node_destroy.link);
	}
	virtual void f_focus()
	{
		auto seat = v_server->v_seat;
		auto current = seat->keyboard_state.focused_surface;
		auto surface = v_surface->current.keyboard_interactive == ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE ? NULL : v_surface->surface;
		if (current == surface) return;
		if (current) if (auto p = wlr_xdg_toplevel_try_from_wlr_surface(current)) wlr_xdg_toplevel_set_activated(p, false);
		if (!surface)
			wlr_seat_keyboard_notify_clear_focus(seat);
		else if (auto p = wlr_seat_get_keyboard(seat))
			wlr_seat_keyboard_notify_enter(seat, surface, p->keycodes, p->num_keycodes, &p->modifiers);
	}
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
		wl_list_remove(&v_modifiers.link);
		wl_list_remove(&v_key.link);
		wl_list_remove(&v_destroy.link);
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
		wl_list_remove(&v_commit.link);
		wl_list_remove(&v_new_popup_surface.link);
		wl_list_remove(&v_destroy.link);
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
	~t_input_popup()
	{
		wl_list_remove(&v_commit.link);
		wl_list_remove(&v_map.link);
		wl_list_remove(&v_unmap.link);
		wl_list_remove(&v_destroy.link);
	}
	void f_move(wlr_text_input_v3* a_input)
	{
		auto cursor = a_input->current.cursor_rectangle;
		auto& node0 = static_cast<wlr_scene_tree*>(a_input->focused_surface->data)->node;
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
		wl_list_remove(&v_enable.link);
		wl_list_remove(&v_commit.link);
		wl_list_remove(&v_disable.link);
		wl_list_remove(&v_destroy.link);
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

t_output::t_output(t_server* a_server, wlr_output* a_output) : v_server(a_server), v_output(a_output)
{
	v_output->data = this;
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
		auto scene_output = wlr_scene_get_scene_output(self->v_server->v_scene, self->v_output);
		wlr_scene_output_commit(scene_output, NULL);
		timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		wlr_scene_output_send_frame_done(scene_output, &now);
	};
	wl_signal_add(&v_output->events.frame, &v_frame);
	v_commit.notify = [](auto a_listener, auto a_data)
	{
		auto event = static_cast<wlr_output_event_commit*>(a_data);
		if (!(event->state->committed & WLR_OUTPUT_STATE_MODE)) return;
		t_output* self = wl_container_of(a_listener, self, v_commit);
		self->f_configure(ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND);
		self->f_configure(ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM);
		self->f_configure(ZWLR_LAYER_SHELL_V1_LAYER_TOP);
		self->f_configure(ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY);
	};
	wl_signal_add(&v_output->events.commit, &v_commit);
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
	for (auto x : {
		ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND,
		ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM,
		ZWLR_LAYER_SHELL_V1_LAYER_TOP,
		ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY
	}) v_layers[x] = wlr_scene_tree_create(v_server->v_layers[x]);
	wl_list_insert(&v_server->v_outputs, &v_link);
	auto l_output = wlr_output_layout_add_auto(v_server->v_output_layout, v_output);
	auto scene_output = wlr_scene_output_create(v_server->v_scene, v_output);
	wlr_scene_output_layout_add_output(v_server->v_scene_layout, l_output, scene_output);
}

void t_output::f_configure(zwlr_layer_shell_v1_layer a_layer)
{
	wlr_box full;
	wlr_output_layout_get_box(v_server->v_output_layout, v_output, &full);
	wlr_box usable = full;
	wlr_scene_node* node;
	wl_list_for_each(node, &v_layers[a_layer]->children, link) wlr_scene_layer_surface_v1_configure(static_cast<wlr_scene_layer_surface_v1*>(static_cast<t_layer_surface*>(node->data)->v_surface->data), &full, &usable);
}

t_toplevel::t_toplevel(t_server* a_server, wlr_xdg_toplevel* a_toplevel) : v_server(a_server), v_toplevel(a_toplevel)
{
	v_scene_tree = wlr_scene_xdg_surface_create(v_server->v_tree, v_toplevel->base);
	v_scene_tree->node.data = this;
	v_toplevel->base->surface->data = v_scene_tree;
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
		self->v_server->v_on_unmap(self->v_toplevel->base);
		wl_list_remove(&self->v_link);
	};
	wl_signal_add(&v_toplevel->base->surface->events.unmap, &v_unmap);
	v_commit.notify = [](auto a_listener, auto a_data)
	{
		t_toplevel* self = wl_container_of(a_listener, self, v_commit);
		if (self->v_toplevel->base->initial_commit) {
			wlr_xdg_toplevel_set_wm_capabilities(self->v_toplevel, WLR_XDG_TOPLEVEL_WM_CAPABILITIES_MAXIMIZE | WLR_XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN);
			wlr_xdg_toplevel_set_size(self->v_toplevel, 0, 0);
		}
		self->v_on_commit();
	};
	wl_signal_add(&v_toplevel->base->surface->events.commit, &v_commit);
	v_ack_configure.notify = [](auto a_listener, auto a_data)
	{
		t_toplevel* self = wl_container_of(a_listener, self, v_ack_configure);
		self->v_on_ack_configure(static_cast<wlr_xdg_surface_configure*>(a_data)->serial);
	};
	wl_signal_add(&v_toplevel->base->events.ack_configure, &v_ack_configure);
	v_destroy.notify = [](auto a_listener, auto a_data)
	{
		t_toplevel* self = wl_container_of(a_listener, self, v_destroy);
		delete self;
	};
	wl_signal_add(&v_toplevel->events.destroy, &v_destroy);
	v_request_move.notify = [](auto a_listener, auto a_data)
	{
		// TODO: check the serial.
		t_toplevel* self = wl_container_of(a_listener, self, v_request_move);
		auto server = self->v_server;
		server->f_on_unmap_ungrab(self->v_toplevel->base->surface);
		auto& node = self->v_scene_tree->node;
		auto& cursor = server->v_cursor;
		server->v_on_cursor_motion = [&node, &cursor, x = node.x - cursor->x, y = node.y - cursor->y](auto a_time)
		{
			wlr_scene_node_set_position(&node, x + cursor->x, y + cursor->y);
		};
		server->v_on_cursor_button = server->v_on_cursor_button_ungrab;
		wlr_seat_pointer_clear_focus(server->v_seat);
	};
	wl_signal_add(&v_toplevel->events.request_move, &v_request_move);
	v_request_resize.notify = [](auto a_listener, auto a_data)
	{
		// TODO: check the serial.
		t_toplevel* self = wl_container_of(a_listener, self, v_request_resize);
		auto server = self->v_server;
		server->v_on_unmap = [self, server](auto a_unmapped)
		{
			if (a_unmapped->surface != self->v_toplevel->base->surface) return;
			wlr_xdg_toplevel_set_resizing(self->v_toplevel, false);
			server->f_ungrab_pointer();
		};
		auto box = self->v_toplevel->base->geometry;
		box.x += self->v_scene_tree->node.x;
		box.y += self->v_scene_tree->node.y;
		auto edges = static_cast<wlr_xdg_toplevel_resize_event*>(a_data)->edges;
		auto& cursor = server->v_cursor;
		server->v_on_cursor_motion = [self, box, edges, &cursor,
			x = box.x + (edges & WLR_EDGE_RIGHT ? box.width : 0) - cursor->x,
			y = box.y + (edges & WLR_EDGE_BOTTOM ? box.height : 0) - cursor->y
		](auto a_time)
		{
			int left = edges & WLR_EDGE_LEFT ? x + cursor->x : box.x;
			int right = edges & WLR_EDGE_RIGHT ? x + cursor->x : box.x + box.width;
			int top = edges & WLR_EDGE_TOP ? y + cursor->y : box.y;
			int bottom = edges & WLR_EDGE_BOTTOM ? y + cursor->y : box.y + box.height;
			self->f_resize(std::max(right - left, 1), std::max(bottom - top, 1), [self, box, edges]
			{
				auto& resized = self->v_toplevel->base->geometry;
				wlr_scene_node_set_position(&self->v_scene_tree->node,
					(edges & WLR_EDGE_LEFT ? box.x + box.width - resized.width : box.x) - resized.x,
					(edges & WLR_EDGE_TOP ? box.y + box.height - resized.height : box.y) - resized.y
				);
			});
		};
		server->v_on_cursor_button = [self, server](auto a_state)
		{
			if (a_state != WL_POINTER_BUTTON_STATE_RELEASED) return;
			wlr_xdg_toplevel_set_resizing(self->v_toplevel, false);
			server->f_ungrab_pointer();
		};
		wlr_seat_pointer_clear_focus(server->v_seat);
		wlr_xdg_toplevel_set_resizing(self->v_toplevel, true);
	};
	wl_signal_add(&v_toplevel->events.request_resize, &v_request_resize);
	v_request_maximize.notify = [](auto a_listener, auto a_data)
	{
		t_toplevel* self = wl_container_of(a_listener, self, v_request_maximize);
		if (!self->v_toplevel->base->initialized) return;
		self->f_try_save_geometry();
		auto maximized = self->v_toplevel->requested.maximized;
		wlr_xdg_toplevel_set_maximized(self->v_toplevel, maximized);
		if (self->v_toplevel->current.fullscreen) return;
		if (maximized)
			self->f_maximize();
		else
			self->f_restore();
	};
	wl_signal_add(&v_toplevel->events.request_maximize, &v_request_maximize);
	v_request_fullscreen.notify = [](auto a_listener, auto a_data)
	{
		t_toplevel* self = wl_container_of(a_listener, self, v_request_fullscreen);
		if (!self->v_toplevel->base->initialized) return;
		self->f_try_save_geometry();
		auto fullscreen = self->v_toplevel->requested.fullscreen;
		wlr_xdg_toplevel_set_fullscreen(self->v_toplevel, fullscreen);
		if (fullscreen)
			self->f_fullscreen(self->v_toplevel->requested.fullscreen_output);
		else if (self->v_toplevel->current.maximized)
			self->f_maximize();
		else
			self->f_restore();
	};
	wl_signal_add(&v_toplevel->events.request_fullscreen, &v_request_fullscreen);
	v_output_commit.notify = [](auto a_listener, auto a_data)
	{
		auto event = static_cast<wlr_output_event_commit*>(a_data);
		if (!(event->state->committed & WLR_OUTPUT_STATE_MODE)) return;
		t_toplevel* self = wl_container_of(a_listener, self, v_output_commit);
		if (self->v_toplevel->current.fullscreen)
			self->f_fullscreen(NULL);
		else
			self->f_maximize();
	};
	v_output_commit.link.next = NULL;
	v_output_destroy.notify = [](auto a_listener, auto a_data)
	{
		t_toplevel* self = wl_container_of(a_listener, self, v_output_destroy);
		self->f_restore();
	};
}

t_popup::t_popup(t_server* a_server, wlr_xdg_popup* a_popup) : v_server(a_server), v_popup(a_popup)
{
	auto parent_tree = static_cast<wlr_scene_tree*>(v_popup->parent->data);
	v_popup->base->surface->data = wlr_scene_xdg_surface_create(parent_tree, v_popup->base);
	v_commit.notify = [](auto a_listener, auto a_data)
	{
		t_popup* self = wl_container_of(a_listener, self, v_commit);
		if (self->v_popup->base->initial_commit) self->f_place();
	};
	wl_signal_add(&v_popup->base->surface->events.commit, &v_commit);
	v_destroy.notify = [](auto a_listener, auto a_data)
	{
		t_popup* self = wl_container_of(a_listener, self, v_destroy);
		delete self;
	};
	wl_signal_add(&v_popup->events.destroy, &v_destroy);
	v_reposition.notify = [](auto a_listener, auto a_data)
	{
		t_popup* self = wl_container_of(a_listener, self, v_reposition);
		self->f_place();
	};
	wl_signal_add(&v_popup->events.reposition, &v_reposition);
}

t_layer_surface::t_layer_surface(t_server* a_server, wlr_layer_surface_v1* a_surface) : v_server(a_server), v_surface(a_surface)
{
	if (!v_surface->output) v_surface->output = v_server->f_first_output()->v_output;
	v_commit.notify = [](auto a_listener, auto a_data)
	{
		t_layer_surface* self = wl_container_of(a_listener, self, v_commit);
		auto surface = self->v_surface;
		auto output = static_cast<t_output*>(surface->output->data);
		auto& current = surface->current;
		if (current.committed & WLR_LAYER_SURFACE_V1_STATE_LAYER && current.layer != self->v_layer) {
			wlr_scene_node_reparent(&static_cast<wlr_scene_tree*>(surface->surface->data)->node, output->v_layers[current.layer]);
			output->f_configure(self->v_layer);
			self->v_layer = current.layer;
		}
		if (current.committed & (
			WLR_LAYER_SURFACE_V1_STATE_DESIRED_SIZE |
			WLR_LAYER_SURFACE_V1_STATE_ANCHOR |
			WLR_LAYER_SURFACE_V1_STATE_EXCLUSIVE_ZONE |
			WLR_LAYER_SURFACE_V1_STATE_MARGIN |
			WLR_LAYER_SURFACE_V1_STATE_LAYER
		)) output->f_configure(current.layer);
	};
	wl_signal_add(&v_surface->surface->events.commit, &v_commit);
	v_destroy.notify = [](auto a_listener, auto a_data)
	{
		t_layer_surface* self = wl_container_of(a_listener, self, v_destroy);
		auto surface = self->v_surface;
		delete self;
		auto& node = static_cast<wlr_scene_tree*>(surface->surface->data)->node;
		wl_list_remove(&node.link);
		wl_list_init(&node.link);
		static_cast<t_output*>(surface->output->data)->f_configure(surface->current.layer);
	};
	wl_signal_add(&v_surface->events.destroy, &v_destroy);
	auto scene_surface = wlr_scene_layer_surface_v1_create(static_cast<t_output*>(v_surface->output->data)->v_layers[v_surface->current.layer], v_surface);
	scene_surface->tree->node.data = this;
	v_surface->data = scene_surface;
	v_surface->surface->data = scene_surface->tree;
	v_node_destroy.notify = [](auto a_listener, auto a_data)
	{
		t_layer_surface* self = wl_container_of(a_listener, self, v_node_destroy);
		auto surface = self->v_surface;
		delete self;
		wlr_layer_surface_v1_destroy(surface);
	};
	wl_signal_add(&scene_surface->tree->node.events.destroy, &v_node_destroy);
}

t_keyboard::t_keyboard(t_server* a_server, wlr_input_device* a_device) : v_server(a_server), v_keyboard(wlr_keyboard_from_input_device(a_device))
{
	{
		t_owner<xkb_context*, xkb_context_unref> context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
		if (!context) throw std::runtime_error("failed to create xkb_context");
		t_owner<xkb_keymap*, xkb_keymap_unref> keymap = xkb_keymap_new_from_names(context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);
		if (!keymap) throw std::runtime_error("failed to create xkb_keymap");
		wlr_keyboard_set_keymap(v_keyboard, keymap);
	}
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
		self->v_popup->surface->data = wlr_scene_surface_create(self->v_server->v_tree, self->v_popup->surface);
		self->f_move(*self->v_server->v_focused_text_input);
	};
	wl_signal_add(&v_popup->surface->events.map, &v_map);
	v_unmap.notify = [](auto a_listener, auto a_data)
	{
		t_input_popup* self = wl_container_of(a_listener, self, v_unmap);
		wlr_scene_node_destroy(&static_cast<wlr_scene_surface*>(self->v_popup->surface->data)->buffer->node);
		self->v_popup->surface->data = NULL;
	};
	wl_signal_add(&v_popup->surface->events.unmap, &v_unmap);
	v_destroy.notify = [](auto a_listener, auto a_data)
	{
		t_input_popup* self = wl_container_of(a_listener, self, v_destroy);
		delete self;
	};
	wl_signal_add(&v_popup->events.destroy, &v_destroy);
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
	v_layers[ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND] = wlr_scene_tree_create(&v_scene->tree);
	v_layers[ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM] = wlr_scene_tree_create(&v_scene->tree);
	v_tree = wlr_scene_tree_create(&v_scene->tree);
	v_layers[ZWLR_LAYER_SHELL_V1_LAYER_TOP] = wlr_scene_tree_create(&v_scene->tree);
	v_layers[ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY] = wlr_scene_tree_create(&v_scene->tree);
	v_xdg_shell = wlr_xdg_shell_create(v_display, 5);
	wl_list_init(&v_toplevels);
	v_xdg_new_toplevel.notify = [](auto a_listener, auto a_data)
	{
		t_server* self = wl_container_of(a_listener, self, v_xdg_new_toplevel);
		new t_toplevel(self, static_cast<wlr_xdg_toplevel*>(a_data));
	};
	wl_signal_add(&v_xdg_shell->events.new_toplevel, &v_xdg_new_toplevel);
	v_xdg_new_popup.notify = [](auto a_listener, auto a_data)
	{
		t_server* self = wl_container_of(a_listener, self, v_xdg_new_popup);
		new t_popup(self, static_cast<wlr_xdg_popup*>(a_data));
	};
	wl_signal_add(&v_xdg_shell->events.new_popup, &v_xdg_new_popup);
	v_layer_shell = wlr_layer_shell_v1_create(v_display, 5);
	v_layer_new_surface.notify = [](auto a_listener, auto a_data)
	{
		t_server* self = wl_container_of(a_listener, self, v_layer_new_surface);
		new t_layer_surface(self, static_cast<wlr_layer_surface_v1*>(a_data));
	};
	wl_signal_add(&v_layer_shell->events.new_surface, &v_layer_new_surface);
	v_cursor = wlr_cursor_create();
	wlr_cursor_attach_output_layout(v_cursor, v_output_layout);
	v_xcursor_manager = wlr_xcursor_manager_create(NULL, 24);
	v_cursor_motion.notify = [](auto a_listener, auto a_data)
	{
		t_server* self = wl_container_of(a_listener, self, v_cursor_motion);
		auto event = static_cast<wlr_pointer_motion_event*>(a_data);
		wlr_cursor_move(self->v_cursor, &event->pointer->base, event->delta_x, event->delta_y);
		self->v_on_cursor_motion(event->time_msec);
	};
	wl_signal_add(&v_cursor->events.motion, &v_cursor_motion);
	v_cursor_motion_absolute.notify = [](auto a_listener, auto a_data)
	{
		t_server* self = wl_container_of(a_listener, self, v_cursor_motion_absolute);
		auto event = static_cast<wlr_pointer_motion_absolute_event*>(a_data);
		wlr_cursor_warp_absolute(self->v_cursor, &event->pointer->base, event->x, event->y);
		self->v_on_cursor_motion(event->time_msec);
	};
	wl_signal_add(&v_cursor->events.motion_absolute, &v_cursor_motion_absolute);
	v_cursor_button.notify = [](auto a_listener, auto a_data)
	{
		t_server* self = wl_container_of(a_listener, self, v_cursor_button);
		auto event = static_cast<wlr_pointer_button_event*>(a_data);
		wlr_seat_pointer_notify_button(self->v_seat, event->time_msec, event->button, event->state);
		self->v_on_cursor_button(event->state);
	};
	wl_signal_add(&v_cursor->events.button, &v_cursor_button);
	v_cursor_axis.notify = [](auto a_listener, void* a_data)
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
	v_request_set_cursor.notify = [](auto a_listener, auto a_data)
	{
		t_server* self = wl_container_of(a_listener, self, v_request_set_cursor);
		auto event = static_cast<wlr_seat_pointer_request_set_cursor_event*>(a_data);
		auto focused_client = self->v_seat->pointer_state.focused_client;
		if (focused_client == event->seat_client) wlr_cursor_set_surface(self->v_cursor, event->surface, event->hotspot_x, event->hotspot_y);
	};
	wl_signal_add(&v_seat->events.request_set_cursor, &v_request_set_cursor);
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
	v_on_cursor_motion = v_on_cursor_motion_forward = [&](auto a_time)
	{
		f_pointer_motion(&a_time);
	};
	v_on_cursor_button = v_on_cursor_button_grab = [&](auto a_state)
	{
		auto surface = f_surface_at_cursor(NULL, NULL);
		if (auto p = f_focusable(surface)) {
			p->f_focus();
		} else {
			if (auto p = v_seat->keyboard_state.focused_surface) if (auto q = wlr_xdg_toplevel_try_from_wlr_surface(p)) wlr_xdg_toplevel_set_activated(q, false);
			wlr_seat_keyboard_notify_clear_focus(v_seat);
		}
		if (!surface) return;
		f_on_unmap_ungrab(surface->surface);
		v_on_cursor_motion = [&, surface](auto a_time)
		{
			int x, y;
			wlr_scene_node_coords(&surface->buffer->node, &x, &y);
			wlr_seat_pointer_notify_motion(v_seat, a_time, v_cursor->x - x, v_cursor->y - y);
		};
		v_on_cursor_button = v_on_cursor_button_ungrab;
	};
	v_on_cursor_button_ungrab = [&](auto a_state)
	{
		if (a_state == WL_POINTER_BUTTON_STATE_RELEASED) f_ungrab_pointer();
	};
}

t_server::~t_server()
{
	wl_display_destroy_clients(v_display);
	t_output* p;
	t_output* q;
	wl_list_for_each_safe(p, q, &v_outputs, v_link) delete p;
	wl_list_remove(&v_new_output.link);
	wl_list_remove(&v_xdg_new_toplevel.link);
	wl_list_remove(&v_xdg_new_popup.link);
	wl_list_remove(&v_layer_new_surface.link);
	wl_list_remove(&v_cursor_motion.link);
	wl_list_remove(&v_cursor_motion_absolute.link);
	wl_list_remove(&v_cursor_button.link);
	wl_list_remove(&v_cursor_axis.link);
	wl_list_remove(&v_cursor_frame.link);
	wl_list_remove(&v_new_input.link);
	wl_list_remove(&v_request_set_cursor.link);
	wl_list_remove(&v_request_set_selection.link);
	wl_list_remove(&v_input_method_input_method.link);
	wl_list_remove(&v_text_input_text_input.link);
	wl_list_remove(&v_keyboard_state_focus_change.link);
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
			std::printf("Usage: %s [-s startup command]\n", argv[0]);
			return 0;
		}
	}
	if (optind < argc) {
		std::printf("Usage: %s [-s startup command]\n", argv[0]);
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
