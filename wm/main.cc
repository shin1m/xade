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
enum tinywl_cursor_mode
{
	TINYWL_CURSOR_PASSTHROUGH,
	TINYWL_CURSOR_MOVE,
	TINYWL_CURSOR_RESIZE
};

struct tinywl_toplevel;
struct tinywl_input_method;
struct tinywl_text_input;

struct tinywl_server
{
	struct wl_display* wl_display = wl_display_create();
	wlr_backend* backend;
	wlr_renderer* renderer = NULL;
	wlr_allocator* allocator = NULL;
	wlr_scene* scene = NULL;
	wlr_scene_output_layout* scene_layout;

	wlr_xdg_shell* xdg_shell;
	wl_listener new_xdg_toplevel;
	wl_listener new_xdg_popup;
	wl_list toplevels;

	wlr_cursor* cursor = NULL;
	wlr_xcursor_manager* cursor_mgr = NULL;
	wl_listener cursor_motion;
	wl_listener cursor_motion_absolute;
	wl_listener cursor_button;
	wl_listener cursor_axis;
	wl_listener cursor_frame;

	wlr_seat* seat;
	wl_listener new_input;
	wl_listener request_cursor;
	wl_listener request_set_selection;
	wl_list keyboards;
	tinywl_cursor_mode cursor_mode = TINYWL_CURSOR_PASSTHROUGH;
	tinywl_toplevel* grabbed_toplevel;
	double grab_x, grab_y;
	wlr_box grab_geobox;
	uint32_t resize_edges;

	wlr_output_layout* output_layout;
	wl_list outputs;
	wl_listener new_output;

	wlr_input_method_manager_v2* input_method_manager;
	wl_listener input_method_input_method;
	tinywl_input_method* input_method = nullptr;
	wlr_text_input_manager_v3* text_input_manager;
	wl_listener text_input_text_input;
	wl_listener keyboard_state_focus_change;
	tinywl_text_input* focused_text_input = nullptr;

	tinywl_server();
	~tinywl_server()
	{
		wl_display_destroy_clients(wl_display);
		if (scene) wlr_scene_node_destroy(&scene->tree.node);
		wlr_xcursor_manager_destroy(cursor_mgr);
		if (cursor) wlr_cursor_destroy(cursor);
		wlr_allocator_destroy(allocator);
		wlr_renderer_destroy(renderer);
		wlr_backend_destroy(backend);
		wl_display_destroy(wl_display);
	}
	tinywl_toplevel* desktop_toplevel_at(double lx, double ly, wlr_surface** surface, double* sx, double* sy)
	{
		auto node = wlr_scene_node_at(&scene->tree.node, lx, ly, sx, sy);
		if (!node || node->type != WLR_SCENE_NODE_BUFFER) return nullptr;
		auto scene_buffer = wlr_scene_buffer_from_node(node);
		auto scene_surface = wlr_scene_surface_try_from_buffer(scene_buffer);
		if (!scene_surface) return nullptr;
		*surface = scene_surface->surface;
		auto tree = node->parent;
		while (tree && !tree->node.data) tree = tree->node.parent;
		return tree ? static_cast<tinywl_toplevel*>(tree->node.data) : nullptr;
	}
	void reset_cursor_mode()
	{
		cursor_mode = TINYWL_CURSOR_PASSTHROUGH;
		grabbed_toplevel = nullptr;
	}
	void process_cursor_resize(uint32_t time);
	void process_cursor_motion(uint32_t time);
};

struct tinywl_output
{
	wl_list link;
	tinywl_server* server;
	struct wlr_output* wlr_output;
	wl_listener frame;
	wl_listener request_state;
	wl_listener destroy;

	tinywl_output(tinywl_server* server, struct wlr_output* output);
	~tinywl_output()
	{
		wl_list_remove(&frame.link);
		wl_list_remove(&request_state.link);
		wl_list_remove(&destroy.link);
		wl_list_remove(&link);
	}
};

struct tinywl_toplevel
{
	wl_list link;
	tinywl_server* server;
	wlr_xdg_toplevel* xdg_toplevel;
	wlr_scene_tree* scene_tree;
	wl_listener map;
	wl_listener unmap;
	wl_listener commit;
	wl_listener destroy;
	wl_listener request_move;
	wl_listener request_resize;
	wl_listener request_maximize;
	wl_listener request_fullscreen;

	tinywl_toplevel(tinywl_server* server, wlr_xdg_toplevel* toplevel);
	~tinywl_toplevel()
	{
		wl_list_remove(&map.link);
		wl_list_remove(&unmap.link);
		wl_list_remove(&commit.link);
		wl_list_remove(&destroy.link);
		wl_list_remove(&request_move.link);
		wl_list_remove(&request_resize.link);
		wl_list_remove(&request_maximize.link);
		wl_list_remove(&request_fullscreen.link);
	}
	void focus(wlr_surface* surface)
	{
		auto seat = server->seat;
		auto current = seat->keyboard_state.focused_surface;
		if (current == surface) return;
		if (current) if (auto p = wlr_xdg_toplevel_try_from_wlr_surface(current)) wlr_xdg_toplevel_set_activated(p, false);
		wlr_scene_node_raise_to_top(&scene_tree->node);
		wl_list_remove(&link);
		wl_list_insert(&server->toplevels, &link);
		wlr_xdg_toplevel_set_activated(xdg_toplevel, true);
		if (auto p = wlr_seat_get_keyboard(seat)) wlr_seat_keyboard_notify_enter(seat, xdg_toplevel->base->surface, p->keycodes, p->num_keycodes, &p->modifiers);
	}
};

struct tinywl_popup
{
	wlr_xdg_popup* xdg_popup;
	wl_listener commit;
	wl_listener destroy;

	tinywl_popup(wlr_xdg_popup* popup);
	~tinywl_popup()
	{
		wl_list_remove(&commit.link);
		wl_list_remove(&destroy.link);
	}
};

struct tinywl_keyboard
{
	wl_list link;
	tinywl_server* server;
	struct wlr_keyboard* wlr_keyboard;

	wl_listener modifiers;
	wl_listener key;
	wl_listener destroy;

	tinywl_keyboard(tinywl_server* server, wlr_input_device* device);
	~tinywl_keyboard()
	{
		wl_list_remove(&modifiers.link);
		wl_list_remove(&key.link);
		wl_list_remove(&destroy.link);
		wl_list_remove(&link);
	}
};

struct tinywl_input_method
{
	tinywl_server* server;
	wlr_input_method_v2* input_method;
	bool on = false;
	wl_listener commit;
	wl_listener new_popup_surface;
	wl_listener grab_keyboard;
	wl_listener destroy;
	wl_listener grab_destroy;

	tinywl_input_method(tinywl_server* server, wlr_input_method_v2* input_method);
	~tinywl_input_method()
	{
		server->input_method = nullptr;
	}
	operator wlr_input_method_v2*() const
	{
		return input_method;
	}
	void deactivate()
	{
		wlr_input_method_v2_send_deactivate(input_method);
		wlr_input_method_v2_send_done(input_method);
	}
};

struct tinywl_input_popup
{
	tinywl_server* server;
	wlr_input_popup_surface_v2* popup;
	wl_listener commit;
	wl_listener map;
	wl_listener unmap;
	wl_listener destroy;

	tinywl_input_popup(tinywl_input_method* input, wlr_input_popup_surface_v2* popup);
	void move(wlr_text_input_v3* input)
	{
		auto cursor = input->current.cursor_rectangle;
		auto& node0 = static_cast<wlr_scene_tree*>(wlr_xdg_surface_try_from_wlr_surface(input->focused_surface)->data)->node;
		cursor.x += node0.x;
		cursor.y += node0.y;
		wlr_box box;
		wlr_output_layout_get_box(server->output_layout, wlr_output_layout_output_at(server->output_layout, cursor.x, cursor.y), &box);
		auto& buffer = popup->surface->buffer->base;
		auto x = cursor.x;
		if (auto right = box.x + box.width; x + buffer.width > right) x = std::max(right - buffer.width, box.x);
		auto y = cursor.y + cursor.height;
		if (y + buffer.height > box.y + box.height) if (auto y1 = cursor.y - buffer.height; y1 >= box.y) y = y1;
		auto& node1 = static_cast<wlr_scene_surface*>(popup->surface->data)->buffer->node;
		wlr_scene_node_set_position(&node1, x, y);
		cursor.x -= node1.x;
		cursor.y -= node1.y;
		wlr_input_popup_surface_v2_send_text_input_rectangle(popup, &cursor);
	}
};

struct tinywl_text_input
{
	tinywl_server* server;
	wlr_text_input_v3* text_input;
	wl_listener enable;
	wl_listener commit;
	wl_listener disable;
	wl_listener destroy;
	std::string preedit;
	int32_t begin;
	int32_t end;

	tinywl_text_input(tinywl_server* server, wlr_text_input_v3* text_input) : server(server), text_input(text_input)
	{
		enable.notify = [](auto a_listener, auto a_data)
		{
			tinywl_text_input* self = wl_container_of(a_listener, self, enable);
			self->server->focused_text_input = self;
			if (auto p = self->server->input_method; p && p->on) {
				wlr_input_method_v2_send_activate(*p);
				self->send();
			}
			wlr_text_input_v3_send_done(self->text_input);
		};
		wl_signal_add(&text_input->events.enable, &enable);
		commit.notify = [](auto a_listener, auto a_data)
		{
			tinywl_text_input* self = wl_container_of(a_listener, self, commit);
			if (auto p = self->server->input_method; p && p->on) self->send();
			wlr_text_input_v3_send_done(self->text_input);
		};
		wl_signal_add(&text_input->events.commit, &commit);
		disable.notify = [](auto a_listener, auto a_data)
		{
			tinywl_text_input* self = wl_container_of(a_listener, self, disable);
			if (auto p = self->server->input_method; p && p->on) p->deactivate();
			self->server->focused_text_input = nullptr;
			self->preedit = {};
			self->begin = self->end = 0;
			wlr_text_input_v3_send_done(self->text_input);
		};
		wl_signal_add(&text_input->events.disable, &disable);
		destroy.notify = [](auto a_listener, auto a_data)
		{
			tinywl_text_input* self = wl_container_of(a_listener, self, destroy);
			delete self;
		};
		wl_signal_add(&text_input->events.destroy, &destroy);
	}
	~tinywl_text_input()
	{
		if (auto p = server->input_method; p && p->on) p->deactivate();
		if (server->focused_text_input == this) server->focused_text_input = nullptr;
	}
	operator wlr_text_input_v3*() const
	{
		return text_input;
	}
	void send()
	{
		auto p = server->input_method;
		auto& state = text_input->current;
		auto& surrounding = state.surrounding;
		wlr_input_method_v2_send_surrounding_text(*p, surrounding.text, surrounding.cursor, surrounding.anchor);
		wlr_input_method_v2_send_text_change_cause(*p, state.text_change_cause);
		auto& type = state.content_type;
		wlr_input_method_v2_send_content_type(*p, type.hint, type.purpose);
		wlr_input_method_v2_send_done(*p);
		wlr_input_popup_surface_v2* popup;
		wl_list_for_each(popup, &p->input_method->popup_surfaces, link) if (popup->surface->data) static_cast<tinywl_input_popup*>(popup->data)->move(text_input);
	}
};

tinywl_input_method::tinywl_input_method(tinywl_server* server, wlr_input_method_v2* input_method) : server(server), input_method(input_method)
{
	commit.notify = [](auto a_listener, auto a_data)
	{
		tinywl_input_method* self = wl_container_of(a_listener, self, commit);
		if (auto p = self->server->focused_text_input) {
			auto& state = self->input_method->current;
			auto& preedit = state.preedit;
			wlr_text_input_v3_send_preedit_string(*p, preedit.text, preedit.cursor_begin, preedit.cursor_end);
			wlr_text_input_v3_send_commit_string(*p, state.commit_text);
			auto& ds = state.delete_surrounding;
			wlr_text_input_v3_send_delete_surrounding_text(*p, ds.before_length, ds.after_length);
			wlr_text_input_v3_send_done(*p);
		}
	};
	wl_signal_add(&input_method->events.commit, &commit);
	new_popup_surface.notify = [](auto a_listener, auto a_data)
	{
		tinywl_input_method* self = wl_container_of(a_listener, self, new_popup_surface);
		new tinywl_input_popup(self, static_cast<wlr_input_popup_surface_v2*>(a_data));
	};
	wl_signal_add(&input_method->events.new_popup_surface, &new_popup_surface);
	grab_keyboard.notify = [](auto a_listener, auto a_data)
	{
		tinywl_input_method* self = wl_container_of(a_listener, self, grab_keyboard);
		auto grab = static_cast<wlr_input_method_keyboard_grab_v2*>(a_data);
		wl_signal_add(&grab->events.destroy, &self->grab_destroy);
	};
	wl_signal_add(&input_method->events.grab_keyboard, &grab_keyboard);
	destroy.notify = [](auto a_listener, auto a_data)
	{
		tinywl_input_method* self = wl_container_of(a_listener, self, destroy);
		delete self;
	};
	wl_signal_add(&input_method->events.destroy, &destroy);
	grab_destroy.notify = [](auto a_listener, auto a_data)
	{
		tinywl_input_method* self = wl_container_of(a_listener, self, grab_destroy);
	};
	server->input_method = this;
}

tinywl_input_popup::tinywl_input_popup(tinywl_input_method* input, wlr_input_popup_surface_v2* popup) : server(input->server), popup(popup)
{
	popup->data = this;
	commit.notify = [](auto a_listener, auto a_data)
	{
		tinywl_input_popup* self = wl_container_of(a_listener, self, commit);
		if (self->popup->surface->data) self->move(self->server->focused_text_input->text_input);
	};
	wl_signal_add(&popup->surface->events.commit, &commit);
	map.notify = [](auto a_listener, auto a_data)
	{
		tinywl_input_popup* self = wl_container_of(a_listener, self, map);
		self->popup->surface->data = wlr_scene_surface_create(&self->server->scene->tree, self->popup->surface);
		self->move(self->server->focused_text_input->text_input);
	};
	wl_signal_add(&popup->surface->events.map, &map);
	unmap.notify = [](auto a_listener, auto a_data)
	{
		tinywl_input_popup* self = wl_container_of(a_listener, self, unmap);
		wlr_scene_node_destroy(&static_cast<wlr_scene_surface*>(self->popup->surface->data)->buffer->node);
		self->popup->surface->data = nullptr;
	};
	wl_signal_add(&popup->surface->events.unmap, &unmap);
	destroy.notify = [](auto a_listener, auto a_data)
	{
		tinywl_input_popup* self = wl_container_of(a_listener, self, destroy);
		delete self;
	};
	wl_signal_add(&popup->events.destroy, &destroy);
}

tinywl_keyboard::tinywl_keyboard(tinywl_server* server, wlr_input_device* device) : server(server), wlr_keyboard(wlr_keyboard_from_input_device(device))
{
	auto context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	auto keymap = xkb_keymap_new_from_names(context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);
	wlr_keyboard_set_keymap(wlr_keyboard, keymap);
	xkb_keymap_unref(keymap);
	xkb_context_unref(context);
	wlr_keyboard_set_repeat_info(wlr_keyboard, 25, 600);
	modifiers.notify = [](auto listener, auto data)
	{
		tinywl_keyboard* keyboard = wl_container_of(listener, keyboard, modifiers);
		if (auto p = keyboard->server->input_method) if (auto q = p->input_method->keyboard_grab) {
			wlr_input_method_keyboard_grab_v2_set_keyboard(q, keyboard->wlr_keyboard);
			wlr_input_method_keyboard_grab_v2_send_modifiers(q, &keyboard->wlr_keyboard->modifiers);
			return;
		}
		wlr_seat_set_keyboard(keyboard->server->seat, keyboard->wlr_keyboard);
		wlr_seat_keyboard_notify_modifiers(keyboard->server->seat, &keyboard->wlr_keyboard->modifiers);
	};
	wl_signal_add(&wlr_keyboard->events.modifiers, &modifiers);
	key.notify = [](auto listener, auto data)
	{
		tinywl_keyboard* keyboard = wl_container_of(listener, keyboard, key);
		auto server = keyboard->server;
		auto event = static_cast<wlr_keyboard_key_event*>(data);
		if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
			auto sym = xkb_state_key_get_one_sym(keyboard->wlr_keyboard->xkb_state, event->keycode + 8);
			if (wlr_keyboard_get_modifiers(keyboard->wlr_keyboard) & WLR_MODIFIER_ALT)
				switch (sym) {
				case XKB_KEY_Escape:
					wl_display_terminate(server->wl_display);
					return;
				case XKB_KEY_F1:
					if (wl_list_length(&server->toplevels) > 1) {
						tinywl_toplevel* p = wl_container_of(server->toplevels.prev, p, link);
						p->focus(p->xdg_toplevel->base->surface);
					}
					return;
				}
			if (sym == XKB_KEY_Kanji || sym == XKB_KEY_Eisu_toggle) if (auto p = server->input_method) if (auto q = server->focused_text_input) {
				p->on ^= true;
				if (p->on) {
					wlr_input_method_v2_send_activate(*p);
					q->send();
				} else {
					p->deactivate();
					wlr_text_input_v3_send_preedit_string(*q, NULL, 0, 0);
					wlr_text_input_v3_send_done(*q);
				}
				return;
			}
		}
		if (auto p = server->input_method) if (auto q = p->input_method->keyboard_grab) {
			wlr_input_method_keyboard_grab_v2_set_keyboard(q, keyboard->wlr_keyboard);
			wlr_input_method_keyboard_grab_v2_send_key(q, event->time_msec, event->keycode, event->state);
			return;
		}
		auto seat = server->seat;
		wlr_seat_set_keyboard(seat, keyboard->wlr_keyboard);
		wlr_seat_keyboard_notify_key(seat, event->time_msec, event->keycode, event->state);
	};
	wl_signal_add(&wlr_keyboard->events.key, &key);
	destroy.notify = [](auto listener, auto data)
	{
		tinywl_keyboard* keyboard = wl_container_of(listener, keyboard, destroy);
		delete keyboard;
	};
	wl_signal_add(&device->events.destroy, &destroy);
	wlr_seat_set_keyboard(server->seat, wlr_keyboard);
	wl_list_insert(&server->keyboards, &link);
}

tinywl_output::tinywl_output(tinywl_server* server, struct wlr_output* output) : server(server), wlr_output(output)
{
	wlr_output_init_render(wlr_output, server->allocator, server->renderer);
	wlr_output_state state;
	wlr_output_state_init(&state);
	wlr_output_state_set_enabled(&state, true);
	if (auto mode = wlr_output_preferred_mode(wlr_output)) wlr_output_state_set_mode(&state, mode);
	wlr_output_commit_state(wlr_output, &state);
	wlr_output_state_finish(&state);
	frame.notify = [](auto listener, auto data)
	{
		tinywl_output* output = wl_container_of(listener, output, frame);
		auto scene = output->server->scene;
		auto scene_output = wlr_scene_get_scene_output(scene, output->wlr_output);
		wlr_scene_output_commit(scene_output, NULL);
		timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		wlr_scene_output_send_frame_done(scene_output, &now);
	};
	wl_signal_add(&wlr_output->events.frame, &frame);
	request_state.notify = [](auto listener, auto data)
	{
		tinywl_output* output = wl_container_of(listener, output, request_state);
		auto event = static_cast<wlr_output_event_request_state*>(data);
		wlr_output_commit_state(output->wlr_output, event->state);
	};
	wl_signal_add(&wlr_output->events.request_state, &request_state);
	destroy.notify = [](auto listener, auto data)
	{
		tinywl_output* output = wl_container_of(listener, output, destroy);
		delete output;
	};
	wl_signal_add(&wlr_output->events.destroy, &destroy);
	wl_list_insert(&server->outputs, &link);
	auto l_output = wlr_output_layout_add_auto(server->output_layout, wlr_output);
	auto scene_output = wlr_scene_output_create(server->scene, wlr_output);
	wlr_scene_output_layout_add_output(server->scene_layout, l_output, scene_output);
}

tinywl_toplevel::tinywl_toplevel(tinywl_server* server, wlr_xdg_toplevel* toplevel) : server(server), xdg_toplevel(toplevel)
{
	scene_tree = wlr_scene_xdg_surface_create(&server->scene->tree, xdg_toplevel->base);
	scene_tree->node.data = this;
	xdg_toplevel->base->data = scene_tree;
	map.notify = [](auto listener, auto data)
	{
		tinywl_toplevel* toplevel = wl_container_of(listener, toplevel, map);
		wl_list_insert(&toplevel->server->toplevels, &toplevel->link);
		toplevel->focus(toplevel->xdg_toplevel->base->surface);
	};
	wl_signal_add(&xdg_toplevel->base->surface->events.map, &map);
	unmap.notify = [](auto listener, auto data)
	{
		tinywl_toplevel* toplevel = wl_container_of(listener, toplevel, unmap);
		if (toplevel == toplevel->server->grabbed_toplevel) toplevel->server->reset_cursor_mode();
		wl_list_remove(&toplevel->link);
	};
	wl_signal_add(&xdg_toplevel->base->surface->events.unmap, &unmap);
	commit.notify = [](auto listener, auto data)
	{
		tinywl_toplevel* toplevel = wl_container_of(listener, toplevel, commit);
		if (toplevel->xdg_toplevel->base->initial_commit) wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, 0, 0);
	};
	wl_signal_add(&xdg_toplevel->base->surface->events.commit, &commit);
	destroy.notify = [](auto listener, auto data)
	{
		tinywl_toplevel* toplevel = wl_container_of(listener, toplevel, destroy);
		delete toplevel;
	};
	wl_signal_add(&xdg_toplevel->events.destroy, &destroy);
	request_move.notify = [](auto listener, auto data)
	{
		/* This event is raised when a client would like to begin an interactive
		 * move, typically because the user clicked on their client-side
		 * decorations. Note that a more sophisticated compositor should check the
		 * provided serial against a list of button press serials sent to this
		 * client, to prevent the client from requesting this whenever they want. */
		tinywl_toplevel* toplevel = wl_container_of(listener, toplevel, request_move);
		auto server = toplevel->server;
		if (toplevel->xdg_toplevel->base->surface != wlr_surface_get_root_surface(server->seat->pointer_state.focused_surface)) return;
		server->grabbed_toplevel = toplevel;
		server->cursor_mode = TINYWL_CURSOR_MOVE;
		server->grab_x = server->cursor->x - toplevel->scene_tree->node.x;
		server->grab_y = server->cursor->y - toplevel->scene_tree->node.y;
	};
	wl_signal_add(&xdg_toplevel->events.request_move, &request_move);
	request_resize.notify = [](auto listener, auto data)
	{
		/* This event is raised when a client would like to begin an interactive
		 * resize, typically because the user clicked on their client-side
		 * decorations. Note that a more sophisticated compositor should check the
		 * provided serial against a list of button press serials sent to this
		 * client, to prevent the client from requesting this whenever they want. */
		tinywl_toplevel* toplevel = wl_container_of(listener, toplevel, request_resize);
		auto server = toplevel->server;
		if (toplevel->xdg_toplevel->base->surface != wlr_surface_get_root_surface(server->seat->pointer_state.focused_surface)) return;
		server->grabbed_toplevel = toplevel;
		server->cursor_mode = TINYWL_CURSOR_RESIZE;
		wlr_box geo_box;
		wlr_xdg_surface_get_geometry(toplevel->xdg_toplevel->base, &geo_box);
		auto edges = static_cast<wlr_xdg_toplevel_resize_event*>(data)->edges;
		double border_x = (toplevel->scene_tree->node.x + geo_box.x) + ((edges & WLR_EDGE_RIGHT) ? geo_box.width : 0);
		double border_y = (toplevel->scene_tree->node.y + geo_box.y) + ((edges & WLR_EDGE_BOTTOM) ? geo_box.height : 0);
		server->grab_x = server->cursor->x - border_x;
		server->grab_y = server->cursor->y - border_y;
		server->grab_geobox = geo_box;
		server->grab_geobox.x += toplevel->scene_tree->node.x;
		server->grab_geobox.y += toplevel->scene_tree->node.y;
		server->resize_edges = edges;
	};
	wl_signal_add(&xdg_toplevel->events.request_resize, &request_resize);
	request_maximize.notify = [](auto listener, auto data)
	{
		tinywl_toplevel* toplevel = wl_container_of(listener, toplevel, request_maximize);
		if (toplevel->xdg_toplevel->base->initialized) wlr_xdg_surface_schedule_configure(toplevel->xdg_toplevel->base);
	};
	wl_signal_add(&xdg_toplevel->events.request_maximize, &request_maximize);
	request_fullscreen.notify = [](auto listener, auto data)
	{
		tinywl_toplevel* toplevel = wl_container_of(listener, toplevel, request_fullscreen);
		if (toplevel->xdg_toplevel->base->initialized) wlr_xdg_surface_schedule_configure(toplevel->xdg_toplevel->base);
	};
	wl_signal_add(&xdg_toplevel->events.request_fullscreen, &request_fullscreen);
}

tinywl_popup::tinywl_popup(wlr_xdg_popup* popup) : xdg_popup(popup)
{
	auto parent = wlr_xdg_surface_try_from_wlr_surface(xdg_popup->parent);
	assert(parent != NULL);
	auto parent_tree = static_cast<wlr_scene_tree*>(parent->data);
	xdg_popup->base->data = wlr_scene_xdg_surface_create(parent_tree, xdg_popup->base);
	commit.notify = [](auto listener, auto data)
	{
		tinywl_popup* popup = wl_container_of(listener, popup, commit);
		if (popup->xdg_popup->base->initial_commit) wlr_xdg_surface_schedule_configure(popup->xdg_popup->base);
	};
	wl_signal_add(&xdg_popup->base->surface->events.commit, &commit);
	destroy.notify = [](auto listener, auto data)
	{
		tinywl_popup* popup = wl_container_of(listener, popup, destroy);
		delete popup;
	};
	wl_signal_add(&xdg_popup->events.destroy, &destroy);
}

void tinywl_server::process_cursor_resize(uint32_t time)
{
	double border_x = cursor->x - grab_x;
	double border_y = cursor->y - grab_y;
	int new_left = grab_geobox.x;
	int new_right = grab_geobox.x + grab_geobox.width;
	int new_top = grab_geobox.y;
	int new_bottom = grab_geobox.y + grab_geobox.height;
	if (resize_edges & WLR_EDGE_TOP) {
		new_top = border_y;
		if (new_top >= new_bottom) new_top = new_bottom - 1;
	} else if (resize_edges & WLR_EDGE_BOTTOM) {
		new_bottom = border_y;
		if (new_bottom <= new_top) new_bottom = new_top + 1;
	}
	if (resize_edges & WLR_EDGE_LEFT) {
		new_left = border_x;
		if (new_left >= new_right) new_left = new_right - 1;
	} else if (resize_edges & WLR_EDGE_RIGHT) {
		new_right = border_x;
		if (new_right <= new_left) new_right = new_left + 1;
	}
	auto toplevel = grabbed_toplevel;
	wlr_box geo_box;
	wlr_xdg_surface_get_geometry(toplevel->xdg_toplevel->base, &geo_box);
	wlr_scene_node_set_position(&toplevel->scene_tree->node, new_left - geo_box.x, new_top - geo_box.y);
	int new_width = new_right - new_left;
	int new_height = new_bottom - new_top;
	wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, new_width, new_height);
}

void tinywl_server::process_cursor_motion(uint32_t time)
{
	if (cursor_mode == TINYWL_CURSOR_MOVE) return wlr_scene_node_set_position(&grabbed_toplevel->scene_tree->node, cursor->x - grab_x, cursor->y - grab_y);
	if (cursor_mode == TINYWL_CURSOR_RESIZE) return process_cursor_resize(time);
	wlr_surface* surface = NULL;
	double sx, sy;
	if (!desktop_toplevel_at(cursor->x, cursor->y, &surface, &sx, &sy)) wlr_cursor_set_xcursor(cursor, cursor_mgr, "default");
	if (surface) {
		wlr_seat_pointer_notify_enter(seat, surface, sx, sy);
		wlr_seat_pointer_notify_motion(seat, time, sx, sy);
	} else {
		wlr_seat_pointer_clear_focus(seat);
	}
}

tinywl_server::tinywl_server()
{
	backend = wlr_backend_autocreate(wl_display_get_event_loop(wl_display), NULL);
	if (!backend) throw std::runtime_error("failed to create wlr_backend");
	renderer = wlr_renderer_autocreate(backend);
	if (!renderer) throw std::runtime_error("failed to create wlr_renderer");
	wlr_renderer_init_wl_display(renderer, wl_display);
	allocator = wlr_allocator_autocreate(backend, renderer);
	if (!allocator) throw std::runtime_error("failed to create wlr_allocator");
	wlr_compositor_create(wl_display, 5, renderer);
	wlr_subcompositor_create(wl_display);
	wlr_data_device_manager_create(wl_display);
	output_layout = wlr_output_layout_create(wl_display);
	wl_list_init(&outputs);
	new_output.notify = [](auto listener, auto data)
	{
		tinywl_server* server = wl_container_of(listener, server, new_output);
		new tinywl_output(server, static_cast<struct wlr_output*>(data));
	};
	wl_signal_add(&backend->events.new_output, &new_output);
	scene = wlr_scene_create();
	scene_layout = wlr_scene_attach_output_layout(scene, output_layout);
	wl_list_init(&toplevels);
	xdg_shell = wlr_xdg_shell_create(wl_display, 3);
	new_xdg_toplevel.notify = [](auto listener, auto data)
	{
		tinywl_server* server = wl_container_of(listener, server, new_xdg_toplevel);
		new tinywl_toplevel(server, static_cast<wlr_xdg_toplevel*>(data));
	};
	wl_signal_add(&xdg_shell->events.new_toplevel, &new_xdg_toplevel);
	new_xdg_popup.notify = [](auto listener, auto data)
	{
		new tinywl_popup(static_cast<wlr_xdg_popup*>(data));
	};
	wl_signal_add(&xdg_shell->events.new_popup, &new_xdg_popup);
	cursor = wlr_cursor_create();
	wlr_cursor_attach_output_layout(cursor, output_layout);
	cursor_mgr = wlr_xcursor_manager_create(NULL, 24);
	cursor_motion.notify = [](auto listener, auto data)
	{
		tinywl_server* server = wl_container_of(listener, server, cursor_motion);
		auto event = static_cast<wlr_pointer_motion_event*>(data);
		wlr_cursor_move(server->cursor, &event->pointer->base, event->delta_x, event->delta_y);
		server->process_cursor_motion(event->time_msec);
	};
	wl_signal_add(&cursor->events.motion, &cursor_motion);
	cursor_motion_absolute.notify = [](auto listener, auto data)
	{
		tinywl_server* server = wl_container_of(listener, server, cursor_motion_absolute);
		auto event = static_cast<wlr_pointer_motion_absolute_event*>(data);
		wlr_cursor_warp_absolute(server->cursor, &event->pointer->base, event->x, event->y);
		server->process_cursor_motion(event->time_msec);
	};
	wl_signal_add(&cursor->events.motion_absolute, &cursor_motion_absolute);
	cursor_button.notify = [](auto listener, auto data)
	{
		tinywl_server* server = wl_container_of(listener, server, cursor_button);
		auto event = static_cast<wlr_pointer_button_event*>(data);
		wlr_seat_pointer_notify_button(server->seat, event->time_msec, event->button, event->state);
		if (event->state == WL_POINTER_BUTTON_STATE_RELEASED) return server->reset_cursor_mode();
		wlr_surface* surface;
		double sx, sy;
		if (auto p = server->desktop_toplevel_at(server->cursor->x, server->cursor->y, &surface, &sx, &sy)) p->focus(surface);
	};
	wl_signal_add(&cursor->events.button, &cursor_button);
	cursor_axis.notify = [](wl_listener* listener, void* data)
	{
		tinywl_server* server = wl_container_of(listener, server, cursor_axis);
		auto event = static_cast<wlr_pointer_axis_event*>(data);
		wlr_seat_pointer_notify_axis(server->seat, event->time_msec, event->orientation, event->delta, event->delta_discrete, event->source, event->relative_direction);
	};
	wl_signal_add(&cursor->events.axis, &cursor_axis);
	cursor_frame.notify = [](auto listener, auto data)
	{
		tinywl_server* server = wl_container_of(listener, server, cursor_frame);
		wlr_seat_pointer_notify_frame(server->seat);
	};
	wl_signal_add(&cursor->events.frame, &cursor_frame);
	wl_list_init(&keyboards);
	new_input.notify = [](auto listener, auto data)
	{
		tinywl_server* server = wl_container_of(listener, server, new_input);
		auto device = static_cast<wlr_input_device*>(data);
		switch (device->type) {
		case WLR_INPUT_DEVICE_KEYBOARD:
			new tinywl_keyboard(server, device);
			break;
		case WLR_INPUT_DEVICE_POINTER:
			wlr_cursor_attach_input_device(server->cursor, device);
			break;
		}
		uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
		if (!wl_list_empty(&server->keyboards)) caps |= WL_SEAT_CAPABILITY_KEYBOARD;
		wlr_seat_set_capabilities(server->seat, caps);
	};
	wl_signal_add(&backend->events.new_input, &new_input);
	seat = wlr_seat_create(wl_display, "seat0");
	request_cursor.notify = [](auto listener, auto data)
	{
		tinywl_server* server = wl_container_of(listener, server, request_cursor);
		auto event = static_cast<wlr_seat_pointer_request_set_cursor_event*>(data);
		auto focused_client = server->seat->pointer_state.focused_client;
		if (focused_client == event->seat_client) wlr_cursor_set_surface(server->cursor, event->surface, event->hotspot_x, event->hotspot_y);
	};
	wl_signal_add(&seat->events.request_set_cursor, &request_cursor);
	request_set_selection.notify = [](auto listener, auto data)
	{
		tinywl_server* server = wl_container_of(listener, server, request_set_selection);
		auto event = static_cast<wlr_seat_request_set_selection_event*>(data);
		wlr_seat_set_selection(server->seat, event->source, event->serial);
	};
	wl_signal_add(&seat->events.request_set_selection, &request_set_selection);
	input_method_manager = wlr_input_method_manager_v2_create(wl_display);
	input_method_input_method.notify = [](auto a_listener, auto a_data)
	{
		tinywl_server* server = wl_container_of(a_listener, server, input_method_input_method);
		auto input_method = static_cast<wlr_input_method_v2*>(a_data);
		if (server->input_method)
			wlr_input_method_v2_send_unavailable(input_method);
		else
			new tinywl_input_method(server, input_method);
	};
	wl_signal_add(&input_method_manager->events.input_method, &input_method_input_method);
	text_input_manager = wlr_text_input_manager_v3_create(wl_display);
	text_input_text_input.notify = [](auto a_listener, auto a_data)
	{
		tinywl_server* server = wl_container_of(a_listener, server, text_input_text_input);
		new tinywl_text_input(server, static_cast<wlr_text_input_v3*>(a_data));
	};
	wl_signal_add(&text_input_manager->events.text_input, &text_input_text_input);
	keyboard_state_focus_change.notify = [](auto a_listener, auto a_data)
	{
		tinywl_server* server = wl_container_of(a_listener, server, keyboard_state_focus_change);
		auto event = static_cast<wlr_seat_keyboard_focus_change_event*>(a_data);
		wlr_text_input_v3* input;
		if (event->old_surface) wl_list_for_each(input, &server->text_input_manager->text_inputs, link) {
			if (input->focused_surface == event->old_surface) {
				if (auto p = server->input_method; p && p->on) p->deactivate();
				wlr_text_input_v3_send_leave(input);
			}
		}
		if (event->new_surface) wl_list_for_each(input, &server->text_input_manager->text_inputs, link) {
			if (input->seat == event->seat && wl_resource_get_client(input->resource) == wl_resource_get_client(event->new_surface->resource)) wlr_text_input_v3_send_enter(input, event->new_surface);
		}
	};
	wl_signal_add(&seat->keyboard_state.events.focus_change, &keyboard_state_focus_change);
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
	tinywl_server server;
	auto socket = wl_display_add_socket_auto(server.wl_display);
	if (!socket) return 1;
	if (!wlr_backend_start(server.backend)) return 1;
	setenv("WAYLAND_DISPLAY", socket, true);
	if (startup_cmd && fork() == 0) execl("/bin/sh", "/bin/sh", "-c", startup_cmd, (void*)NULL);
	wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s", socket);
	wl_display_run(server.wl_display);
	return 0;
}
