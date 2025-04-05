#include "layered.h"
#include <cstring>

namespace xemmaix::xade
{

void t_entry::f_dispose()
{
	v_previous->v_next = v_next;
	v_next->v_previous = v_previous;
}

t_session::t_session() : t_client([&](auto a_this, auto a_name, auto a_interface, auto a_version)
{
	if (std::strcmp(a_interface, zwlr_layer_shell_v1_interface.name) == 0) v_layer = static_cast<zwlr_layer_shell_v1*>(wl_registry_bind(a_this, a_name, &zwlr_layer_shell_v1_interface, std::min<uint32_t>(a_version, zwlr_layer_shell_v1_interface.version)));
})
{
	v_on_idle.push_back([&]
	{
		if (auto& on = v_object->f_fields()[/*on_idle*/0]) on();
	});
	v_instance = this;
}

t_session::~t_session()
{
	while (v_next != this) v_next->f_dispose();
	if (v_layer) {
		zwlr_layer_shell_v1_destroy(v_layer);
		v_layer = NULL;
	}
	v_object->f_as<xemmaix::xade::t_client>().v_client = v_instance = nullptr;
}

void t_proxy::f_dispose()
{
	t_entry::f_dispose();
	v_session = nullptr;
	v_object = nullptr;
}

void t_library::f_main(t_library* a_library, const t_pvalue& a_callable)
{
	t_session session;
	session.v_object = f_new<t_client>(a_library);
	a_callable(session.v_object);
}

void t_library::f_scan(t_scan a_scan)
{
	a_scan(v_type_client);
	a_scan(v_type_pointer_axis);
	a_scan(v_type_proxy);
	a_scan(v_type_surface);
	a_scan(v_type_frame_state);
	a_scan(v_type_frame_wm_capabilities);
	a_scan(v_type_frame_resize_edge);
	a_scan(v_type_frame);
	a_scan(v_type_cursor);
	a_scan(v_type_input);
	a_scan(v_type_layer);
	a_scan(v_type_anchor);
	a_scan(v_type_keyboard_interactivity);
	a_scan(v_type_layered);
}

std::vector<std::pair<t_root, t_rvalue>> t_library::f_define()
{
	t_type_of<t_client>::f_define(this);
	t_type_of<t_proxy>::f_define(this);
	t_type_of<t_surface>::f_define(this);
	t_type_of<t_frame>::f_define(this);
	t_define{this}.f_derive<t_cursor, t_proxy>();
	t_type_of<t_input>::f_define(this);
	t_type_of<t_layered>::f_define(this);
	return t_define(this)
		(L"Client"sv, static_cast<t_object*>(v_type_client))
		(L"PointerAxis"sv, t_type_of<wl_pointer_axis>::f_define(this))
		(L"Proxy"sv, static_cast<t_object*>(v_type_proxy))
		(L"Surface"sv, static_cast<t_object*>(v_type_surface))
		(L"FrameState"sv, t_type_of<xdg_toplevel_state>::f_define(this))
		(L"FrameWMCapabilities"sv, t_type_of<xdg_toplevel_wm_capabilities>::f_define(this))
		(L"FrameResizeEdge"sv, t_type_of<xdg_toplevel_resize_edge>::f_define(this))
		(L"Frame"sv, static_cast<t_object*>(v_type_frame))
		(L"Cursor"sv, static_cast<t_object*>(v_type_cursor))
		(L"Input"sv, static_cast<t_object*>(v_type_input))
		(L"Layer"sv, t_type_of<zwlr_layer_shell_v1_layer>::f_define(this))
		(L"Anchor"sv, t_type_of<zwlr_layer_surface_v1_anchor>::f_define(this))
		(L"KeyboardInteractivity"sv, t_type_of<zwlr_layer_surface_v1_keyboard_interactivity>::f_define(this))
		(L"Layered"sv, static_cast<t_object*>(v_type_layered))
		(L"main"sv, t_static<void(*)(t_library*, const t_pvalue&), f_main>())
		(L"client"sv, t_static<t_object*(*)(), []
		{
			return static_cast<t_object*>(t_session::f_instance()->v_object);
		}>())
	;
}

}

namespace xemmai
{

void t_type_of<xemmaix::xade::t_proxy>::f_define(t_library* a_library)
{
	using xemmaix::xade::t_proxy;
	t_define{a_library}
		(L"dispose"sv, t_member<void(t_proxy::*)(), &t_proxy::f_dispose>())
	.f_derive<t_proxy, t_object>();
}

}

XEMMAI__MODULE__FACTORY(xemmai::t_library::t_handle* a_handle)
{
	return xemmai::f_new<xemmaix::xade::t_library>(a_handle);
}
