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

void t_type_of<xemmaix::xade::t_surface>::f_define(t_library* a_library)
{
	using xemmaix::xade::t_surface;
	t_define{a_library}
		(L"on_frame")
		(L"on_pointer_enter")
		(L"on_pointer_leave")
		(L"on_pointer_move")
		(L"on_button_press")
		(L"on_button_release")
		(L"on_scroll")
		(L"on_focus_enter")
		(L"on_focus_leave")
		(L"on_key_press")
		(L"on_key_release")
		(L"on_key_repeat")
		(L"on_input_enable")
		(L"on_input_disable")
		(L"on_input_done")
		(L"create"sv, t_member<void(t_surface::*)(size_t, size_t), &t_surface::f_create>())
		(L"destroy"sv, t_member<void(t_surface::*)(), &t_surface::f_destroy>())
		(L"make_current"sv, t_member<void(t_surface::*)(), &t_surface::f_make_current>())
		(L"swap_buffers"sv, t_member<void(t_surface::*)(), &t_surface::f_swap_buffers>())
		(L"request_frame"sv, t_member<void(t_surface::*)(), &t_surface::f_request_frame>())
		(L"input"sv, t_member<const std::shared_ptr<::xade::t_input>&(t_surface::*)(), &t_surface::f_input>())
		(L"input__"sv, t_member<void(t_surface::*)(const std::shared_ptr<::xade::t_input>&), &t_surface::f_input__>())
	.f_derive<t_surface, t_object>();
}

t_pvalue t_type_of<xemmaix::xade::t_surface>::f_do_construct(t_pvalue* a_stack, size_t a_n)
{
	return t_construct<bool>::t_bind<xemmaix::xade::t_surface>::f_do(this, a_stack, a_n);
}

void t_type_of<xemmaix::xade::t_frame>::f_define(t_library* a_library)
{
	using xemmaix::xade::t_frame;
	t_define{a_library}
		(L"on_measure")
		(L"on_map")
		(L"on_unmap")
		(L"on_close")
		(L"swap_buffers"sv, t_member<void(t_frame::*)(), &t_frame::f_swap_buffers>())
		//(L"is"sv, t_member<bool(t_frame::*)(xdg_toplevel_state), &t_frame::f_is>())
		//(L"has"sv, t_member<bool(t_frame::*)(xdg_toplevel_wm_capabilities), &t_frame::f_has>())
		(L"show_window_menu"sv, t_member<void(t_frame::*)(int32_t, int32_t), &t_frame::f_show_window_menu>())
		(L"move"sv, t_member<void(t_frame::*)(), &t_frame::f_move>())
		(L"resize"sv,
		 	t_member<void(t_frame::*)(uint32_t), &t_frame::f_resize>(),
		 	t_member<void(t_frame::*)(int32_t, int32_t), &t_frame::f_resize>()
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
		(L"done"sv, t_member<bool(t_input::*)(), &t_input::f_done>())
		(L"commit"sv, t_member<void(t_input::*)(), &t_input::f_commit>())
		(L"preedit"sv, t_member<t_pvalue(t_input::*)(), &t_input::f_preedit>())
		(L"text"sv, t_member<t_pvalue(t_input::*)(), &t_input::f_text>())
		(L"delete"sv, t_member<t_pvalue(t_input::*)(), &t_input::f_delete>())
		(L"spot"sv, t_member<void(t_input::*)(int32_t, int32_t, int32_t, int32_t), &t_input::f_spot>())
	.f_derive<t_input, t_object>();
}

}
