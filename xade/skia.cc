#include <xade/skia.h>
#include <include/core/SkColorSpace.h>
#include <include/gpu/ganesh/GrBackendSurface.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/gpu/ganesh/gl/GrGLBackendSurface.h>
#include <include/gpu/ganesh/gl/GrGLDirectContext.h>
#include <include/gpu/ganesh/gl/GrGLInterface.h>
#include <include/gpu/ganesh/gl/egl/GrGLMakeEGLInterface.h>

namespace xade::skia
{

sk_sp<GrDirectContext> f_new_direct_context()
{
	//auto interface = GrGLInterfaces::MakeEGL();
	//if (!interface) throw std::runtime_error("GrGLInterfaces::MakeEGL");
	auto interface = GrGLMakeNativeInterface();
	if (!interface) throw std::runtime_error("GrGLMakeNativeInterface");
	auto context = GrDirectContexts::MakeGL(interface);
	if (!context) throw std::runtime_error("GrDirectContexts::MakeGL");
	return context;
}

sk_sp<SkSurface> f_new_surface(GrRecordingContext* a_context, GLuint a_framebuffer, int a_width, int a_height)
{
	GrGLFramebufferInfo fi;
	fi.fFBOID = a_framebuffer;
	fi.fFormat = GL_RGBA8;
	auto target = GrBackendRenderTargets::MakeGL(a_width, a_height, 0, 0, fi);
	if (!target.isValid()) throw std::runtime_error("GrBackendRenderTargets::MakeGL");
	auto surface = SkSurfaces::WrapBackendRenderTarget(a_context, target, kBottomLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType, {}, nullptr);
	if (!surface) throw std::runtime_error("SkSurfaces::WrapBackendRenderTarget");
	return surface;
}

}
