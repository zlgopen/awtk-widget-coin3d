/* Desktop <GL/gl.h> is OpenGL 1.1. Core VBO / glGetStringi live in glext
   and may be missing when GLX includes gl.h before GL_GLEXT_PROTOTYPES.
   GLES and Apple gl3 already declare these; glad maps them to macros. */

#ifndef COIN_SOGL_MODERN_CORE_DECLS_H
#define COIN_SOGL_MODERN_CORE_DECLS_H

#if !defined(COIN_USE_GLAD) && !defined(GLAD_GL) && !defined(glad_glBindBuffer)
#if !defined(GL_ES_VERSION_2_0) && !defined(__gl3_h_) && !defined(__gl2_h_)

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(GL_VERSION_1_5)
#include <stddef.h>
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;
#endif

void glBindBuffer(GLenum target, GLuint buffer);
void glDeleteBuffers(GLsizei n, const GLuint *buffers);
void glGenBuffers(GLsizei n, GLuint *buffers);
GLboolean glIsBuffer(GLuint buffer);
void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
void glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params);
const GLubyte * glGetStringi(GLenum name, GLuint index);

#ifdef __cplusplus
}
#endif

#define COIN_SOGL_PROVIDED_CORE_DECLS 1

#endif /* !GLES && !Apple gl3 */
#endif /* !glad */

#endif /* ! COIN_SOGL_MODERN_CORE_DECLS_H */
