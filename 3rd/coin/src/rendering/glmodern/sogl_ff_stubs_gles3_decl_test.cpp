/* Compile-only: Ubuntu GLES3/gl3.h declares glReadBuffer as extern and
   defines GL_ES_VERSION_3_0 (and 2.0). FF stubs must not redeclare it
   static. glDrawBuffer / glGetTexLevelParameter* stay stubs. */

#define GL_ES_VERSION_2_0 1
#define GL_ES_VERSION_3_0 1

typedef unsigned int GLenum;

static int g_read_buffer = -1;

#ifdef __cplusplus
extern "C" {
#endif
void glReadBuffer(GLenum src);
#ifdef __cplusplus
}
#endif

#include <Inventor/system/sogl_ff_stubs.h>

#ifdef COIN_SYSTEM_GL_HAS_CORE_BUFFER_DECLS
#error "GLES3 must not skip desktop-only Core buffer stubs"
#endif
#ifndef COIN_SYSTEM_GL_HAS_READ_BUFFER_DECL
#error "GLES3 must keep the system glReadBuffer prototype"
#endif

#ifdef __cplusplus
extern "C" {
#endif
void
glReadBuffer(GLenum src)
{
  g_read_buffer = (int)src;
}
#ifdef __cplusplus
}
#endif

int
main(void)
{
  glReadBuffer(0x0404);
  if (g_read_buffer != 0x0404) {
    return 1;
  }
  glDrawBuffer(0x0405);
  {
    int w = -1;
    glGetTexLevelParameteriv(0, 0, 0, &w);
    if (w != 0) {
      return 1;
    }
  }
  return 0;
}
