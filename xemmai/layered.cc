#include "layered.h"

namespace xemmai
{

t_object* t_type_of<zwlr_layer_shell_v1_layer>::f_define(t_library* a_library)
{
	return t_base::f_define(a_library, [](auto a_fields)
	{
		a_fields
		(L"BACKGROUND"sv, ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND)
		(L"BOTTOM"sv, ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM)
		(L"TOP"sv, ZWLR_LAYER_SHELL_V1_LAYER_TOP)
		(L"OVERLAY"sv, ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY)
		;
	});
}

t_object* t_type_of<zwlr_layer_surface_v1_anchor>::f_define(t_library* a_library)
{
	return t_base::f_define(a_library, [](auto a_fields)
	{
		a_fields
		(L"TOP"sv, ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP)
		(L"BOTTOM"sv, ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM)
		(L"LEFT"sv, ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT)
		(L"RIGHT"sv, ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT)
		;
	});
}

t_object* t_type_of<zwlr_layer_surface_v1_keyboard_interactivity>::f_define(t_library* a_library)
{
	return t_base::f_define(a_library, [](auto a_fields)
	{
		a_fields
		(L"NONE"sv, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE)
		(L"EXCLUSIVE"sv, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_EXCLUSIVE)
		(L"ON_DEMAND"sv, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND)
		;
	});
}

void t_type_of<xemmaix::xade::t_layered>::f_define(t_library* a_library)
{
	using xemmaix::xade::t_layered;
	t_define{a_library}
		(L"on_measure"sv)
		(L"on_map"sv)
		(L"on_unmap"sv)
		(L"on_closed"sv)
		(L"swap_buffers"sv, t_member<void(t_layered::*)(), &t_layered::f_swap_buffers>())
		(L"set_size"sv, t_member<void(t_layered::*)(uint32_t, uint32_t), &t_layered::f_set_size>())
		(L"set_anchor"sv, t_member<void(t_layered::*)(uint32_t), &t_layered::f_set_anchor>())
		(L"set_exclusive_zone"sv, t_member<void(t_layered::*)(int32_t), &t_layered::f_set_exclusive_zone>())
		(L"set_margin"sv, t_member<void(t_layered::*)(int32_t, int32_t, int32_t, int32_t), &t_layered::f_set_margin>())
		(L"set_keyboard_interactivity"sv, t_member<void(t_layered::*)(zwlr_layer_surface_v1_keyboard_interactivity), &t_layered::f_set_keyboard_interactivity>())
		(L"set_layer"sv, t_member<void(t_layered::*)(zwlr_layer_shell_v1_layer), &t_layered::f_set_layer>())
		(L"set_exclusive_edge"sv, t_member<void(t_layered::*)(uint32_t), &t_layered::f_set_exclusive_edge>())
	.f_derive<t_layered, xemmaix::xade::t_surface>();
}

t_pvalue t_type_of<xemmaix::xade::t_layered>::f_do_construct(t_pvalue* a_stack, size_t a_n)
{
	return t_construct<zwlr_layer_shell_v1_layer, std::wstring_view, bool>::t_bind<xemmaix::xade::t_layered>::f_do(this, a_stack, a_n);
}

}
