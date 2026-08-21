/* Compile-only: GLES2 + HAVE_GLX includes desktop <GL/gl.h> first.
   That header declares glDepthRange / glPolygonMode / glPointSize as
   extern and may omit GL_VERSION_1_0. Shims must not redeclare them. */

#define GL_ES_VERSION_2_0 1
#define __gl_h_ 1

typedef unsigned int GLenum;
typedef double GLclampd;
typedef float GLfloat;

#ifdef __cplusplus
extern "C" {
#endif
void glDepthRange(GLclampd n, GLclampd f);
void glPolygonMode(GLenum face, GLenum mode);
void glPointSize(GLfloat size);
void glDepthRangef(float n, float f);
#ifdef __cplusplus
}
#endif

#include <Inventor/system/sogl_es_types.h>

#ifdef __cplusplus
extern "C" {
#endif
void
glDepthRange(GLclampd n, GLclampd f)
{
  (void)n;
  (void)f;
}
void
glPolygonMode(GLenum face, GLenum mode)
{
  (void)face;
  (void)mode;
}
void
glPointSize(GLfloat size)
{
  (void)size;
}
void
glDepthRangef(float n, float f)
{
  (void)n;
  (void)f;
}
#ifdef __cplusplus
}
#endif

int
main(void)
{
  cc_gl_depth_range(0.0, 1.0);
  cc_gl_polygon_mode(0, GL_FILL);
  cc_gl_point_size(2.0f);
  return 0;
}
