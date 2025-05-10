#include <xade/layered.h>

namespace xade
{

zwlr_layer_surface_v1_listener t_layered::v_layer_surface_listener = {
	[](auto a_data, auto a_this, auto a_serial, auto a_width, auto a_height)
	{
		static_cast<t_layered*>(a_data)->f_configure(a_serial, a_width, a_height);
	},
	[](auto a_data, auto a_this)
	{
		if (auto& on = static_cast<t_layered*>(a_data)->v_on_closed) on();
	}
};

void t_layered::f_configure(uint32_t a_serial, uint32_t a_width, uint32_t a_height)
{
	if (auto& on = v_on_measure) on(a_width, a_height);
	auto ack = [&]
	{
		v_configuring = false;
		zwlr_layer_surface_v1_ack_configure(*this, a_serial);
		wl_surface_commit(*this);
	};
	bool map = !static_cast<wl_egl_window*>(*this);
	if (map) {
		if (a_width <= 0 || a_height <= 0) return ack();
		f_create(a_width, a_height);
	} else if (a_width <= 0 || a_height <= 0) {
		if (auto& on = v_on_unmap) on();
		f_destroy();
		return ack();
	} else {
		if (!v_configuring) {
			int width;
			int height;
			wl_egl_window_get_attached_size(*this, &width, &height);
			if (a_width == width && a_height == height) return ack();
		}
		wl_egl_window_resize(*this, a_width, a_height, 0, 0);
	}
	v_configuring = true;
	v_configure_serial = a_serial;
	if (auto& on = v_on_map) on(a_width, a_height);
	if (map)
		v_on_frame(0);
	else
		f_request_frame();
}

t_layered::t_layered(zwlr_layer_shell_v1* a_shell, wl_output* a_output, zwlr_layer_shell_v1_layer a_layer, const char* a_namespace, bool a_depth) : t_surface(a_depth)
{
	v_layer_surface = zwlr_layer_shell_v1_get_layer_surface(a_shell, *this, a_output, a_layer, a_namespace);
	if (!v_layer_surface) throw std::runtime_error("layer surface");
	zwlr_layer_surface_v1_add_listener(*this, &v_layer_surface_listener, this);
}

t_layered::~t_layered()
{
}

}
