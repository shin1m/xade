#ifndef XADE__LAYERED_H
#define XADE__LAYERED_H

#include "client.h"
#define namespace surface_namespace
#include "wlr-layer-shell-unstable-v1.h"
#undef namespace

namespace xade
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsubobject-linkage"

class t_layered : public t_surface
{
	static zwlr_layer_surface_v1_listener v_layer_surface_listener;

	t_owner<zwlr_layer_surface_v1*, zwlr_layer_surface_v1_destroy> v_layer_surface;
	bool v_configuring = false;
	uint32_t v_configure_serial;

	void f_configure(uint32_t a_serial, uint32_t a_width, uint32_t a_height);

public:
	std::function<void(uint32_t&, uint32_t&)> v_on_measure;
	std::function<void(uint32_t, uint32_t)> v_on_map;
	std::function<void()> v_on_unmap;
	std::function<void()> v_on_closed;

	t_layered(zwlr_layer_shell_v1* a_shell, wl_output* a_output, zwlr_layer_shell_v1_layer a_layer, const char* a_namespace, bool a_depth);
	~t_layered();
	operator zwlr_layer_surface_v1*() const
	{
		return v_layer_surface;
	}
	void f_swap_buffers()
	{
		if (v_configuring) {
			v_configuring = false;
			zwlr_layer_surface_v1_ack_configure(*this, v_configure_serial);
		}
		t_surface::f_swap_buffers();
	}
};

#pragma GCC diagnostic pop

}

#endif
