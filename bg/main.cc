#include <xade/layered.h>
#include <xade/skia.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkStream.h>
#include <include/codec/SkCodec.h>
#include <include/codec/SkJpegDecoder.h>
#include <include/codec/SkPngDecoder.h>

using namespace xade;

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
	t_layered background(shell, NULL, ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND, "", false);
	zwlr_layer_shell_v1_destroy(shell);
	sk_sp<GrDirectContext> context;
	sk_sp<SkSurface> surface;
	background.v_on_map = [&](auto a_width, auto a_height)
	{
		background.f_make_current();
		if (!surface)
			context = skia::f_new_direct_context();
		else if (a_width != surface->width() || a_height != surface->height())
			surface.reset();
		if (!surface) surface = skia::f_new_surface(context.get(), 0, a_width, a_height);
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
		client.f_cursor__(&cursor);
	};
	zwlr_layer_surface_v1_set_anchor(background, ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
	wl_surface_commit(background);
	loop.f_run();
	return EXIT_SUCCESS;
}
