/* Compile-only: AWTK glad defines GL_ES_VERSION_2_0 and maps glXxx to
   glad_glXxx function pointers. ES shims must not redeclare those names. */

#define GL_ES_VERSION_2_0 1

typedef unsigned int GLenum;

static void glad_glDepthRangef(float n, float f)
{
  (void)n;
  (void)f;
}
static void glad_glPolygonMode(GLenum face, GLenum mode)
{
  (void)face;
  (void)mode;
}
static void glad_glPointSize(float size)
{
  (void)size;
}

#define glDepthRangef glad_glDepthRangef
#define glPolygonMode glad_glPolygonMode
#define glPointSize glad_glPointSize

#include <Inventor/system/sogl_es_types.h>

int
main(void)
{
  cc_gl_depth_range(0.0, 1.0);
  cc_gl_polygon_mode(0, GL_FILL);
  cc_gl_point_size(2.0f);
  return 0;
}
