#ifndef XEMMAIX__XADE__LAYERED_H
#define XEMMAIX__XADE__LAYERED_H

#include "client.h"

namespace xemmaix::xade
{

struct t_layered : t_proxy_of<::xade::t_layered>
{
	t_layered(zwlr_layer_shell_v1_layer a_layer, std::wstring_view a_namespace, bool a_depth) : t_base(t_session::f_layer(), nullptr, a_layer, portable::f_convert(a_namespace).c_str(), a_depth)
	{
		XEMMAIX__XADE__SURFACE__ONS
		v_on_measure = [&](auto& a_width, auto& a_height)
		{
			if (auto& on = t_object::f_of(this)->f_fields()[15]) {
				auto size = on(a_width, a_height);
				auto width = size.f_get_at(0);
				xemmai::f_check<int32_t>(width, L"width");
				a_width = f_as<int32_t>(width);
				auto height = size.f_get_at(1);
				xemmai::f_check<int32_t>(height, L"height");
				a_height = f_as<int32_t>(height);
			}
		};
		XEMMAIX__XADE__ON(map, (auto a_width, auto a_height), 16, a_width, a_height)
		XEMMAIX__XADE__ON(unmap, , 17, )
		XEMMAIX__XADE__ON(closed, , 18, )
	}
	void f_swap_buffers()
	{
		f_check();
		::xade::t_layered::f_swap_buffers();
	}
	void f_set_size(uint32_t a_width, uint32_t a_height)
	{
		f_check();
		zwlr_layer_surface_v1_set_size(*this, a_width, a_height);
	}
	void f_set_anchor(uint32_t a_anchor)
	{
		f_check();
		zwlr_layer_surface_v1_set_anchor(*this, a_anchor);
	}
	void f_set_exclusive_zone(int32_t a_zone)
	{
		f_check();
		zwlr_layer_surface_v1_set_exclusive_zone(*this, a_zone);
	}
	void f_set_margin(int32_t a_top, int32_t a_right, int32_t a_bottom, int32_t a_left)
	{
		f_check();
		zwlr_layer_surface_v1_set_margin(*this, a_top, a_right, a_bottom, a_left);
	}
	void f_set_keyboard_interactivity(zwlr_layer_surface_v1_keyboard_interactivity a_interactivity)
	{
		f_check();
		zwlr_layer_surface_v1_set_keyboard_interactivity(*this, a_interactivity);
	}
	void f_set_layer(zwlr_layer_shell_v1_layer a_layer)
	{
		f_check();
		zwlr_layer_surface_v1_set_layer(*this, a_layer);
	}
	void f_set_exclusive_edge(uint32_t a_anchor)
	{
		f_check();
		zwlr_layer_surface_v1_set_exclusive_edge(*this, a_anchor);
	}
};

}

namespace xemmai
{

template<>
struct t_type_of<zwlr_layer_shell_v1_layer> : t_enum_of<zwlr_layer_shell_v1_layer, xemmaix::xade::t_library>
{
	static t_object* f_define(t_library* a_library);

	using t_base::t_base;
};

template<>
struct t_type_of<zwlr_layer_surface_v1_anchor> : t_enum_of<zwlr_layer_surface_v1_anchor, xemmaix::xade::t_library>
{
	static t_object* f_define(t_library* a_library);

	using t_base::t_base;
};

template<>
struct t_type_of<zwlr_layer_surface_v1_keyboard_interactivity> : t_enum_of<zwlr_layer_surface_v1_keyboard_interactivity, xemmaix::xade::t_library>
{
	static t_object* f_define(t_library* a_library);

	using t_base::t_base;
};

template<>
struct t_type_of<xemmaix::xade::t_layered> : t_derivable<t_bears<xemmaix::xade::t_layered, t_type_of<xemmaix::xade::t_surface>>>
{
	static void f_define(t_library* a_library);

	using t_base::t_base;
	t_pvalue f_do_construct(t_pvalue* a_stack, size_t a_n);
};

}

#endif
