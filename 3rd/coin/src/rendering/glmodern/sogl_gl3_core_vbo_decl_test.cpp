/* Compile-only: Apple OpenGL/gl3.h already declares VBO as extern.
   sogl_modern_core_decls.h must not redeclare them (would be fine as
   compatible extern, but the skip keeps GLES/Core headers authoritative). */

#define __gl3_h_ 1
#define GL_VERSION_3_0 1

typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef int GLsizei;
typedef unsigned char GLboolean;
typedef unsigned char GLubyte;
typedef int GLint;
typedef long GLsizeiptr;
typedef long GLintptr;

#ifdef __cplusplus
extern "C" {
#endif
void glBindBuffer(GLenum target, GLuint buffer);
const GLubyte * glGetStringi(GLenum name, GLuint index);
#ifdef __cplusplus
}
#endif

#include <Inventor/system/sogl_modern_core_decls.h>

#ifdef COIN_SOGL_PROVIDED_CORE_DECLS
#error "Apple/Core gl3 must not receive fallback VBO prototypes"
#endif

#ifdef __cplusplus
extern "C" {
#endif
void
glBindBuffer(GLenum target, GLuint buffer)
{
  (void)target;
  (void)buffer;
}
const GLubyte *
glGetStringi(GLenum name, GLuint index)
{
  (void)name;
  (void)index;
  return 0;
}
#ifdef __cplusplus
}
#endif

int
main(void)
{
  glBindBuffer(0, 0);
  (void)glGetStringi(0, 0);
  return 0;
}
