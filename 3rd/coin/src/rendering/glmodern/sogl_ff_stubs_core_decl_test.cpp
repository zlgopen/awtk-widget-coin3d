/* Desktop Core (Apple OpenGL/gl3.h) declares glDrawBuffer / glReadBuffer /
   glGetTexLevelParameter* as extern and defines GL_VERSION_3_0, but omits
   GL_LIGHTING. FF stubs must not redeclare those four as static. */

#define __gl3_h_ 1
#define GL_VERSION_3_0 1

typedef unsigned int GLenum;

static int g_draw_buffer = -1;
static int g_tex_w = -1;

#ifdef __cplusplus
extern "C" {
#endif
void glDrawBuffer(GLenum mode);
void glReadBuffer(GLenum mode);
void glGetTexLevelParameteriv(GLenum, int, GLenum, int * params);
void glGetTexLevelParameterfv(GLenum, int, GLenum, float * params);
#ifdef __cplusplus
}
#endif

#include <Inventor/system/sogl_ff_stubs.h>

#ifdef __cplusplus
extern "C" {
#endif
void
glDrawBuffer(GLenum mode)
{
  g_draw_buffer = (int)mode;
}
void
glReadBuffer(GLenum mode)
{
  (void)mode;
}
void
glGetTexLevelParameteriv(GLenum, int, GLenum, int * params)
{
  if (params) {
    *params = 64;
  }
}
void
glGetTexLevelParameterfv(GLenum, int, GLenum, float * params)
{
  if (params) {
    *params = 1.0f;
  }
}
#ifdef __cplusplus
}
#endif

int
main(void)
{
  glDrawBuffer(0x0402);
  if (g_draw_buffer != 0x0402) {
    return 1;
  }
  glGetTexLevelParameteriv(0, 0, 0, &g_tex_w);
  if (g_tex_w != 64) {
    return 1;
  }
  return 0;
}
