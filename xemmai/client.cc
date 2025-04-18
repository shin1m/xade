#include "client.h"

namespace xemmai
{

void t_type_of<xemmaix::xade::t_client>::f_define(t_library* a_library)
{
	using xemmaix::xade::t_client;
	t_define{a_library}
	(L"on_idle"sv)
	(L"pointer_focus"sv, t_member<t_object*(t_client::*)(), &t_client::f_pointer_focus>())
	(L"pointer"sv, t_member<t_object*(t_client::*)(), &t_client::f_pointer>())
	(L"keyboard_focus"sv, t_member<t_object*(t_client::*)(), &t_client::f_keyboard_focus>())
	(L"cursor"sv, t_member<t_object*(t_client::*)(), &t_client::f_cursor>())
	(L"cursor__"sv, t_member<void(t_client::*)(const xemmaix::xade::t_cursor*), &t_client::f_cursor__>())
	(L"input_focus"sv, t_member<t_object*(t_client::*)(), &t_client::f_input_focus>())
	(L"new_input"sv, t_member<std::shared_ptr<::xade::t_input>(t_client::*)(), &t_client::f_new_input>())
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
