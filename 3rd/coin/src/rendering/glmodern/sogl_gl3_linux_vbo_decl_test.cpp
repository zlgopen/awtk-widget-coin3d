/* Compile-only: Ubuntu desktop GL3 uses <GL/gl.h> (OpenGL 1.1). gl.cpp
   includes <GL/glx.h> first, so glext may be seen without
   GL_GLEXT_PROTOTYPES. VBO / glGetStringi must still be declared so
   COIN_GL_MODERN linked-symbol fallbacks compile. */

#define __gl_h_ 1
#define GL_VERSION_1_1 1
#define GL_VERSION_1_5 1

typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef int GLsizei;
typedef unsigned char GLboolean;
typedef unsigned char GLubyte;
typedef int GLint;
typedef long GLsizeiptr;
typedef long GLintptr;

/* Intentionally no glBindBuffer / glGetStringi prototypes. */

#include <Inventor/system/sogl_modern_core_decls.h>

#ifndef COIN_SOGL_PROVIDED_CORE_DECLS
#error "Linux desktop GL 1.1 must receive fallback VBO / glGetStringi prototypes"
#endif

typedef void (*coin_bindbuffer_proc)(GLenum, GLuint);
typedef const GLubyte *(*coin_getstringi_proc)(GLenum, GLuint);

int
main(void)
{
  coin_bindbuffer_proc pBind = (coin_bindbuffer_proc)glBindBuffer;
  coin_getstringi_proc pGetStringi = (coin_getstringi_proc)glGetStringi;
  if (!pBind || !pGetStringi) {
    return 1;
  }
  pBind(0, 0);
  (void)pGetStringi(0, 0);
  return 0;
}

#ifdef __cplusplus
extern "C" {
#endif
void glBindBuffer(GLenum target, GLuint buffer) { (void)target; (void)buffer; }
void glDeleteBuffers(GLsizei n, const GLuint *buffers) { (void)n; (void)buffers; }
void glGenBuffers(GLsizei n, GLuint *buffers) { (void)n; (void)buffers; }
GLboolean glIsBuffer(GLuint buffer) { (void)buffer; return 0; }
void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage)
{
  (void)target; (void)size; (void)data; (void)usage;
}
void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data)
{
  (void)target; (void)offset; (void)size; (void)data;
}
void glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
  (void)target; (void)pname; (void)params;
}
const GLubyte * glGetStringi(GLenum name, GLuint index)
{
  (void)name; (void)index; return 0;
}
#ifdef __cplusplus
}
#endif
