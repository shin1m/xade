#include "client.h"

namespace xemmaix::xade
{

void f_receive(t_data_offer& a_offer, std::wstring_view a_mime_type, const t_pvalue& a_done)
{
	auto mime_type = portable::f_convert(a_mime_type);
	if (!a_offer.f_mime_types().contains(mime_type.c_str())) f_throw(L"no mime type"sv);
	auto data = std::make_shared<std::tuple<t_root, size_t>>(t_bytes::f_instantiate(t_object::f_size_to_capacity<t_bytes, unsigned char>(sizeof(t_object))), 0);
	a_offer.f_receive(mime_type.c_str(), [data](auto a_fd) mutable
	{
		auto& [buffer, size] = *data;
		if (auto& bytes = buffer->f_as<t_bytes>(); size >= bytes.f_size()) {
			auto p = t_bytes::f_instantiate(t_object::f_size_to_capacity<t_bytes, unsigned char>(t_object::f_capacity_to_size<t_bytes, unsigned char>(size) * 2));
			std::copy_n(&bytes[0], size, &p->f_as<t_bytes>()[0]);
			buffer = p;
		}
		auto& bytes = buffer->f_as<t_bytes>();
		auto n = read(a_fd, &bytes[size], bytes.f_size() - size);
		if (n < 0) return n;
		size += n;
		return n;
	}, [done = t_rvalue(a_done), data]
	{
		auto& [buffer, size] = *data;
		done(buffer, size);
	});
}

std::unique_ptr<t_data_source> f_new_data_source(t_map& a_offers)
{
	auto source = std::make_unique<t_data_source>();
	a_offers.f_owned_or_shared<t_shared_lock_with_safe_region>([&]
	{
		for (t_map::t_iterator i(a_offers); i.f_entry(); i.f_next()) {
			auto& x = i.f_entry()->f_key();
			xemmai::f_check<t_string>(x, L"key");
			source->f_offer(portable::f_convert(x->f_as<t_string>()).c_str(), [get = t_rvalue(i.f_entry()->v_value)]
			{
				auto data = get();
				auto bytes = data.f_get_at(0);
				xemmai::f_check<t_bytes>(bytes, L"data[0]");
				auto size = data.f_get_at(1);
				xemmai::f_check<size_t>(size, L"data[1]");
				return [bytes = t_root(bytes), p = &bytes->f_as<t_bytes>()[0], size = f_as<size_t>(size)](auto a_fd) mutable -> ssize_t
				{
					auto n = write(a_fd, p, size);
					if (n == -1) return n;
					p += n;
					return size -= n;
				};
			});
		}
	});
	return source;
}

}

namespace xemmai
{

void t_type_of<xemmaix::xade::t_client>::f_define(t_library* a_library)
{
	using xemmaix::xade::t_client;
	t_define{a_library}
	(L"on_idle"sv)
	(L"on_selection"sv)
	(L"pointer_focus"sv, t_member<t_object*(t_client::*)() const, &t_client::f_pointer_focus>())
	(L"pointer"sv, t_member<t_object*(t_client::*)() const, &t_client::f_pointer>())
	(L"keyboard_focus"sv, t_member<t_object*(t_client::*)() const, &t_client::f_keyboard_focus>())
	(L"cursor"sv, t_member<t_object*(t_client::*)() const, &t_client::f_cursor>())
	(L"cursor__"sv, t_member<void(t_client::*)(const xemmaix::xade::t_cursor*), &t_client::f_cursor__>())
	(L"input_focus"sv, t_member<t_object*(t_client::*)() const, &t_client::f_input_focus>())
	(L"new_input"sv, t_member<std::shared_ptr<::xade::t_input>(t_client::*)(), &t_client::f_new_input>())
	(L"drag_offers"sv, t_member<bool(t_client::*)() const, &t_client::f_drag_offers>())
	(L"drag_has"sv, t_member<bool(t_client::*)(std::wstring_view) const, &t_client::f_drag_has>())
	(L"drag_accept"sv, t_member<void(t_client::*)(const t_string*), &t_client::f_drag_accept>())
	(L"drag_source_actions"sv, t_member<uint32_t(t_client::*)() const, &t_client::f_drag_source_actions>())
	(L"drag_dnd_action"sv, t_member<uint32_t(t_client::*)() const, &t_client::f_drag_dnd_action>())
	(L"drag_set_actions"sv, t_member<void(t_client::*)(uint32_t, uint32_t), &t_client::f_drag_set_actions>())
	(L"drag_receive"sv, t_member<void(t_client::*)(std::wstring_view, const t_pvalue&) const, &t_client::f_drag_receive>())
	(L"drag_finish"sv, t_member<void(t_client::*)(), &t_client::f_drag_finish>())
	(L"drag_cancel"sv, t_member<void(t_client::*)(), &t_client::f_drag_finish>())
	(L"selection_has"sv, t_member<bool(t_client::*)(std::wstring_view) const, &t_client::f_selection_has>())
	(L"selection_receive"sv, t_member<void(t_client::*)(std::wstring_view, const t_pvalue&) const, &t_client::f_selection_receive>())
	(L"selection_set"sv, t_member<void(t_client::*)(t_map&, const t_pvalue&), &t_client::f_selection_set>())
	(L"selection_clear"sv, t_member<void(t_client::*)(), &t_client::f_selection_clear>())
	.f_derive<t_client, t_object>();
}

t_object* t_type_of<wl_pointer_axis>::f_define(t_library* a_library)
{
	return t_base::f_define(a_library, [](auto a_fields)
	{
		a_fields
		(L"VERTICAL_SCROLL"sv, WL_POINTER_AXIS_VERTICAL_SCROLL)
		(L"HORIZONTAL_SCROLL"sv, WL_POINTER_AXIS_HORIZONTAL_SCROLL)
		;
	});
}

void t_type_of<xemmaix::xade::t_surface>::f_define(t_library* a_library)
{
	using xemmaix::xade::t_surface;
	t_define{a_library}
	(L"on_frame"sv)
	(L"on_pointer_enter"sv)
	(L"on_pointer_leave"sv)
	(L"on_pointer_move"sv)
	(L"on_button_press"sv)
	(L"on_button_release"sv)
	(L"on_scroll"sv)
	(L"on_focus_enter"sv)
	(L"on_focus_leave"sv)
	(L"on_key_press"sv)
	(L"on_key_release"sv)
	(L"on_key_repeat"sv)
	(L"on_input_enable"sv)
	(L"on_input_disable"sv)
	(L"on_input_done"sv)
	(L"on_drag_enter"sv)
	(L"on_drag_leave"sv)
	(L"on_drag_motion"sv)
	(L"on_drag_drop"sv)
	(L"commit"sv, t_member<void(*)(t_surface&), [](auto a_this)
	{
		wl_surface_commit(a_this);
	}>())
	(L"create"sv, t_member<void(xade::t_surface::*)(size_t, size_t), &t_surface::f_create>())
	(L"destroy"sv, t_member<void(xade::t_surface::*)(), &t_surface::f_destroy>())
	(L"make_current"sv, t_member<void(xade::t_surface::*)(), &t_surface::f_make_current>())
	(L"swap_buffers"sv, t_member<void(xade::t_surface::*)(), &t_surface::f_swap_buffers>())
	(L"request_frame"sv, t_member<void(xade::t_surface::*)(), &t_surface::f_request_frame>())
	(L"input"sv, t_member<const std::shared_ptr<xade::t_input>&(xade::t_surface::*)() const, &t_surface::f_input>())
	(L"input__"sv, t_member<void(xade::t_surface::*)(const std::shared_ptr<xade::t_input>&), &t_surface::f_input__>())
	(L"drag_start"sv, t_member<void(*)(t_surface&, t_surface*), [](auto a_this, auto a_icon)
	{
		a_this.f_start_drag({}, a_icon);
	}>(), t_member<void(*)(t_library*, t_surface&, t_map&, uint32_t, const t_pvalue&, const t_pvalue&, const t_pvalue&, const t_pvalue&, const t_pvalue&, t_surface*), [](auto a_library, auto a_this, auto a_offers, auto a_actions, auto a_on_target, auto a_on_cancelled, auto a_on_dnd_drop_performed, auto a_on_dnd_finished, auto a_on_action, auto a_icon)
	{
		auto source = xemmaix::xade::f_new_data_source(a_offers);
		source->f_set_actions(a_actions);
		if (a_on_target) source->v_on_target = [=, on = t_rvalue(a_on_target)](auto a_mime_type)
		{
			on(a_library->f_as(portable::f_convert(a_mime_type)));
		};
		if (a_on_cancelled) source->v_on_cancelled = t_rvalue(a_on_cancelled);
		if (a_on_dnd_drop_performed) source->v_on_dnd_drop_performed = t_rvalue(a_on_dnd_drop_performed);
		if (a_on_dnd_finished) source->v_on_dnd_finished = t_rvalue(a_on_dnd_finished);
		if (a_on_action) source->v_on_action = [=, on = t_rvalue(a_on_action)](auto a_dnd_action)
		{
			on(a_library->f_as(a_dnd_action));
		};
		a_this.f_start_drag(std::move(source), a_icon);
	}>())
	.f_derive<t_surface, xemmaix::xade::t_proxy>();
}

t_pvalue t_type_of<xemmaix::xade::t_surface>::f_do_construct(t_pvalue* a_stack, size_t a_n)
{
	return t_construct<bool>::t_bind<xemmaix::xade::t_surface>::f_do(this, a_stack, a_n);
}

t_object* t_type_of<xdg_toplevel_state>::f_define(t_library* a_library)
{
	return t_base::f_define(a_library, [](auto a_fields)
	{
		a_fields
		(L"MAXIMIZED"sv, XDG_TOPLEVEL_STATE_MAXIMIZED)
		(L"FULLSCREEN"sv, XDG_TOPLEVEL_STATE_FULLSCREEN)
		(L"RESIZING"sv, XDG_TOPLEVEL_STATE_RESIZING)
		(L"ACTIVATED"sv, XDG_TOPLEVEL_STATE_ACTIVATED)
		(L"TILED_LEFT"sv, XDG_TOPLEVEL_STATE_TILED_LEFT)
		(L"TILED_RIGHT"sv, XDG_TOPLEVEL_STATE_TILED_RIGHT)
		(L"TILED_TOP"sv, XDG_TOPLEVEL_STATE_TILED_TOP)
		(L"TILED_BOTTOM"sv, XDG_TOPLEVEL_STATE_TILED_BOTTOM)
		(L"SUSPENDED"sv, XDG_TOPLEVEL_STATE_SUSPENDED)
		;
	});
}

t_object* t_type_of<xdg_toplevel_wm_capabilities>::f_define(t_library* a_library)
{
	return t_base::f_define(a_library, [](auto a_fields)
	{
		a_fields
		(L"WINDOW_MENU"sv, XDG_TOPLEVEL_WM_CAPABILITIES_WINDOW_MENU)
		(L"MAXIMIZE"sv, XDG_TOPLEVEL_WM_CAPABILITIES_MAXIMIZE)
		(L"FULLSCREEN"sv, XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN)
		(L"MINIMIZE"sv, XDG_TOPLEVEL_WM_CAPABILITIES_MINIMIZE)
		;
	});
}

t_object* t_type_of<xdg_toplevel_resize_edge>::f_define(t_library* a_library)
{
	return t_base::f_define(a_library, [](auto a_fields)
	{
		a_fields
		(L"NONE"sv, XDG_TOPLEVEL_RESIZE_EDGE_NONE)
		(L"TOP"sv, XDG_TOPLEVEL_RESIZE_EDGE_TOP)
		(L"BOTTOM"sv, XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM)
		(L"LEFT"sv, XDG_TOPLEVEL_RESIZE_EDGE_LEFT)
		(L"TOP_LEFT"sv, XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT)
		(L"BOTTOM_LEFT"sv, XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT)
		(L"RIGHT"sv, XDG_TOPLEVEL_RESIZE_EDGE_RIGHT)
		(L"TOP_RIGHT"sv, XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT)
		(L"BOTTOM_RIGHT"sv, XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT)
		;
	});
}

void t_type_of<xemmaix::xade::t_frame>::f_define(t_library* a_library)
{
	using xemmaix::xade::t_frame;
	t_define{a_library}
	(L"on_measure"sv)
	(L"on_map"sv)
	(L"on_unmap"sv)
	(L"on_close"sv)
	(L"swap_buffers"sv, t_member<void(xade::t_frame::*)(), &t_frame::f_swap_buffers>())
	(L"is"sv, t_member<bool(xade::t_frame::*)(xdg_toplevel_state) const, &t_frame::f_is>())
	(L"has"sv, t_member<bool(xade::t_frame::*)(xdg_toplevel_wm_capabilities) const, &t_frame::f_has>())
	(L"show_window_menu"sv, t_member<void(xade::t_frame::*)(int32_t, int32_t), &t_frame::f_show_window_menu>())
	(L"move"sv, t_member<void(xade::t_frame::*)(), &t_frame::f_move>())
	(L"resize"sv,
		t_member<void(xade::t_frame::*)(xdg_toplevel_resize_edge), &t_frame::f_resize>(),
		t_member<void(xade::t_frame::*)(int32_t, int32_t), &t_frame::f_resize>()
	)
	(L"set_maximized"sv, t_member<void(*)(xade::t_frame&), [](auto a_this)
	{
		xdg_toplevel_set_maximized(a_this);
	}>())
	(L"unset_maximized"sv, t_member<void(*)(xade::t_frame&), [](auto a_this)
	{
		xdg_toplevel_unset_maximized(a_this);
	}>())
	(L"set_fullscreen"sv, t_member<void(*)(xade::t_frame&), [](auto a_this)
	{
		xdg_toplevel_set_fullscreen(a_this, NULL);
	}>())
	(L"unset_fullscreen"sv, t_member<void(*)(xade::t_frame&), [](auto a_this)
	{
		xdg_toplevel_unset_fullscreen(a_this);
	}>())
	(L"set_minimized"sv, t_member<void(*)(xade::t_frame&), [](auto a_this)
	{
		xdg_toplevel_set_minimized(a_this);
	}>())
	.f_derive<t_frame, xemmaix::xade::t_surface>();
}

t_pvalue t_type_of<xemmaix::xade::t_frame>::f_do_construct(t_pvalue* a_stack, size_t a_n)
{
	return t_construct<bool>::t_bind<xemmaix::xade::t_frame>::f_do(this, a_stack, a_n);
}

t_pvalue t_type_of<xemmaix::xade::t_cursor>::f_do_construct(t_pvalue* a_stack, size_t a_n)
{
	return t_construct<std::wstring_view>::t_bind<xemmaix::xade::t_cursor>::f_do(this, a_stack, a_n);
}

void t_type_of<xemmaix::xade::t_input>::f_define(t_library* a_library)
{
	using xemmaix::xade::t_input;
	t_define{a_library}
	(L"done"sv, t_member<bool(*)(const std::shared_ptr<xade::t_input>&), [](auto a_this)
	{
		return a_this->f_done();
	}>())
	(L"commit"sv, t_member<void(*)(const std::shared_ptr<xade::t_input>&), [](auto a_this)
	{
		a_this->f_commit();
	}>())
	(L"preedit"sv, t_member<t_object*(t_input::*)(), &t_input::f_preedit>())
	(L"text"sv, t_member<t_object*(t_input::*)(), &t_input::f_text>())
	(L"delete"sv, t_member<t_object*(*)(const std::shared_ptr<xade::t_input>&), [](auto a_this)
	{
		auto [before, after] = a_this->f_delete();
		return f_tuple(before, after);
	}>())
	(L"spot"sv, t_member<void(*)(const std::shared_ptr<xade::t_input>&, int32_t, int32_t, int32_t, int32_t), [](auto a_this, auto a_x, auto a_y, auto a_width, auto a_height)
	{
		zwp_text_input_v3_set_cursor_rectangle(*a_this, a_x, a_y, a_width, a_height);
	}>())
	.f_derive<t_input, xemmaix::xade::t_proxy>();
}

}
