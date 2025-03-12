#include <xade/client.h>
#include <xade/skia.h>
#define namespace surface_namespace
#include "wlr-layer-shell-unstable-v1.h"
#undef namespace
#include <include/core/SkCanvas.h>
#include <include/core/SkStream.h>
#include <include/codec/SkCodec.h>
#include <include/codec/SkJpegDecoder.h>
#include <include/codec/SkPngDecoder.h>

using namespace xade;

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

	t_layered(zwlr_layer_shell_v1* a_shell, wl_output* a_output, uint32_t a_layer, const char* a_namespace);
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
	} else {
		int width;
		int height;
		wl_egl_window_get_attached_size(*this, &width, &height);
		if (a_width == width && a_height == height) return ack();
		if (a_width <= 0 || a_height <= 0) {
			if (auto& on = v_on_unmap) on();
			f_destroy();
			return ack();
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

t_layered::t_layered(zwlr_layer_shell_v1* a_shell, wl_output* a_output, uint32_t a_layer, const char* a_namespace)
{
	v_layer_surface = zwlr_layer_shell_v1_get_layer_surface(a_shell, *this, a_output, a_layer, a_namespace);
	if (!v_layer_surface) throw std::runtime_error("layer surface");
	zwlr_layer_surface_v1_add_listener(*this, &v_layer_surface_listener, this);
}

t_layered::~t_layered()
{
}

int main(int argc, char* argv[])
{
	if (argc != 2) return EXIT_FAILURE;
	auto input = SkFILEStream::Make(argv[1]);
	if (!input) throw std::runtime_error("SkFILEStream::Make");
	SkCodec::Result result0;
	auto codec = SkCodec::MakeFromStream(std::move(input), {
		SkJpegDecoder::Decoder(),
		SkPngDecoder::Decoder()
	}, &result0);
	if (!codec) throw std::runtime_error("SkCodec::MakeFromStream");
	auto [image, result1] = codec->getImage();
	suisha::t_loop loop;
	zwlr_layer_shell_v1* shell = NULL;
	t_client client([&](auto a_this, auto a_name, auto a_interface, auto a_version)
	{
		if (std::strcmp(a_interface, zwlr_layer_shell_v1_interface.name) == 0) shell = static_cast<zwlr_layer_shell_v1*>(wl_registry_bind(a_this, a_name, &zwlr_layer_shell_v1_interface, std::min<uint32_t>(a_version, zwlr_layer_shell_v1_interface.version)));
	});
	if (!shell) throw std::runtime_error("layer shell");
	t_cursor cursor("default");
	t_layered background(shell, NULL, ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND, "");
	zwlr_layer_shell_v1_destroy(shell);
	sk_sp<GrDirectContext> context;
	sk_sp<SkSurface> surface;
	background.v_on_map = [&](auto a_width, auto a_height)
	{
		background.f_make_current();
		if (surface)
			surface.reset();
		else
			context = skia::f_new_direct_context();
		surface = skia::f_new_surface(context.get(), 0, a_width, a_height);
	};
	background.v_on_unmap = [&]
	{
		background.f_make_current();
		surface.reset();
		context.reset();
	};
	background.v_on_closed = []
	{
		suisha::f_loop().f_exit();
	};
	background.v_on_frame = [&](auto a_time)
	{
		auto& canvas = *surface->getCanvas();
		auto s = SkRect::MakeWH(image->width(), image->height());
		auto d = SkRect::MakeWH(surface->width(), surface->height());
		auto a = d.width() / d.height();
		if (s.width() / s.height() < a)
			s.inset(0.0f, (s.height() - s.width() / a) * 0.5f);
		else
			s.inset((s.width() - a * s.height()) * 0.5f, 0.0f);
		canvas.drawImageRect(image, s, d, SkFilterMode::kLinear, nullptr, SkCanvas::kStrict_SrcRectConstraint);
		context->flushAndSubmit(surface.get());
		background.f_swap_buffers();
	};
	background.v_on_pointer_enter = [&]
	{
		f_client().f_cursor__(&cursor);
	};
	zwlr_layer_surface_v1_set_anchor(background, ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
	wl_surface_commit(background);
	loop.f_run();
	return EXIT_SUCCESS;
}
