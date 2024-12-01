#ifndef XADE__SKIA_H
#define XADE__SKIA_H

#include <GLES3/gl3.h>
#include <include/core/SkSurface.h>
#include <include/gpu/ganesh/GrDirectContext.h>

namespace xade::skia
{

sk_sp<GrDirectContext> f_new_direct_context();
sk_sp<SkSurface> f_new_surface(GrRecordingContext* a_context, GLuint a_framebuffer, int a_width, int a_height);

}

#endif
