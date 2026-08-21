/* Compile-only: GLES headers provide glDepthRangef, not glDepthRange,
   omit glPolygonMode / glPointSize / GL_FILL / GL_LINE / GL_POINT, and use
   GL_ALIASED_*_RANGE instead of GL_LINE_WIDTH_RANGE / GL_POINT_SIZE_RANGE. */

#define GL_ES_VERSION_2_0 1
#define GL_ALIASED_POINT_SIZE_RANGE 0x846D
#define GL_ALIASED_LINE_WIDTH_RANGE 0x846E

typedef unsigned int GLenum;

static float g_depth_n = -1.0f;
static float g_depth_f = -1.0f;

static void
glDepthRangef(float n, float f)
{
  g_depth_n = n;
  g_depth_f = f;
}

#include <Inventor/system/sogl_es_types.h>
#include <Inventor/system/sogl_ff_stubs.h>

int
main(void)
{
  cc_gl_depth_range(0.0, 1.0);
  cc_gl_polygon_mode(0, GL_FILL);
  cc_gl_polygon_mode(0, GL_LINE);
  cc_gl_polygon_mode(0, GL_POINT);
  cc_gl_point_size(3.0f);
  if (g_depth_n != 0.0f || g_depth_f != 1.0f) {
    return 1;
  }
  if (GL_POINT != 0x1B00 || GL_LINE != 0x1B01 || GL_FILL != 0x1B02) {
    return 1;
  }
  if (GL_LINE_WIDTH_RANGE != GL_ALIASED_LINE_WIDTH_RANGE) {
    return 1;
  }
  if (GL_POINT_SIZE_RANGE != GL_ALIASED_POINT_SIZE_RANGE) {
    return 1;
  }
  if (GL_UNPACK_ROW_LENGTH != 0x0CF2 || GL_UNPACK_SKIP_PIXELS != 0x0CF4 ||
      GL_UNPACK_SKIP_ROWS != 0x0CF3 || GL_PACK_ROW_LENGTH != 0x0D02 ||
      GL_PACK_SKIP_PIXELS != 0x0D04 || GL_PACK_SKIP_ROWS != 0x0D03) {
    return 1;
  }
  if (GL_TEXTURE_WIDTH != 0x1000 || GL_FRONT_LEFT != 0x0400) {
    return 1;
  }
  glDrawBuffer(GL_BACK_LEFT);
  glReadBuffer(GL_FRONT_LEFT);
  {
    int w = -1;
    glGetTexLevelParameteriv(0, 0, GL_TEXTURE_WIDTH, &w);
    if (w != 0) {
      return 1;
    }
  }
  return 0;
}
