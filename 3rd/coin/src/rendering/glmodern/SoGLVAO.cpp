#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "rendering/glmodern/SoGLVAO.h"

#ifdef COIN_GL_MODERN

#include <cstring>

/* GLES2 without glad needs runtime OES lookup. With AWTK glad (Windows),
   use the loaded glGenVertexArrays entry points directly. */
#if defined(COIN_GLES2) && !defined(COIN_USE_GLAD)
#ifndef GL_VERTEX_ARRAY_BINDING
#define GL_VERTEX_ARRAY_BINDING 0x85B5
#endif
#ifndef APIENTRY
#define APIENTRY
#endif

#if defined(HAVE_EGL)
#include <EGL/egl.h>
#endif
#if defined(HAVE_UNISTD_H) || defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h>
#define SOGL_HAVE_DLSYM 1
#endif

typedef void (APIENTRY * SoGLGenVAOProc)(GLsizei, GLuint *);
typedef void (APIENTRY * SoGLBindVAOProc)(GLuint);
typedef void (APIENTRY * SoGLDeleteVAOProc)(GLsizei, const GLuint *);

static SoGLGenVAOProc g_gen = NULL;
static SoGLBindVAOProc g_bind = NULL;
static SoGLDeleteVAOProc g_del = NULL;
static SbBool g_ext_inited = FALSE;
static SbBool g_ext_ok = FALSE;

static void *
sogl_vao_get_proc(const char * name)
{
  void * p = NULL;
#if defined(HAVE_EGL)
  p = (void *)eglGetProcAddress(name);
#endif
#if defined(SOGL_HAVE_DLSYM)
  if (!p) p = dlsym(RTLD_DEFAULT, name);
#endif
  return p;
}

static void
sogl_vao_ensure_ext(void)
{
  if (g_ext_inited) return;
  g_ext_inited = TRUE;
  g_gen = (SoGLGenVAOProc)sogl_vao_get_proc("glGenVertexArraysOES");
  if (!g_gen) g_gen = (SoGLGenVAOProc)sogl_vao_get_proc("glGenVertexArrays");
  g_bind = (SoGLBindVAOProc)sogl_vao_get_proc("glBindVertexArrayOES");
  if (!g_bind) g_bind = (SoGLBindVAOProc)sogl_vao_get_proc("glBindVertexArray");
  g_del = (SoGLDeleteVAOProc)sogl_vao_get_proc("glDeleteVertexArraysOES");
  if (!g_del) g_del = (SoGLDeleteVAOProc)sogl_vao_get_proc("glDeleteVertexArrays");
  g_ext_ok = (g_gen && g_bind && g_del) ? TRUE : FALSE;
}
#endif /* COIN_GLES2 && !COIN_USE_GLAD */

SbBool
sogl_vao_supported(void)
{
#if defined(COIN_GLES2) && !defined(COIN_USE_GLAD)
  sogl_vao_ensure_ext();
  return g_ext_ok;
#else
  return TRUE;
#endif
}

void
sogl_vao_gen(GLsizei n, GLuint * arrays)
{
  if (!arrays || n <= 0) return;
#if defined(COIN_GLES2) && !defined(COIN_USE_GLAD)
  sogl_vao_ensure_ext();
  if (g_ext_ok) {
    g_gen(n, arrays);
    return;
  }
  for (GLsizei i = 0; i < n; ++i) arrays[i] = 0;
#else
  glGenVertexArrays(n, arrays);
#endif
}

void
sogl_vao_delete(GLsizei n, const GLuint * arrays)
{
  if (!arrays || n <= 0) return;
#if defined(COIN_GLES2) && !defined(COIN_USE_GLAD)
  sogl_vao_ensure_ext();
  if (g_ext_ok) g_del(n, arrays);
#else
  glDeleteVertexArrays(n, arrays);
#endif
}

void
sogl_vao_bind_id(GLuint vao)
{
#if defined(COIN_GLES2) && !defined(COIN_USE_GLAD)
  sogl_vao_ensure_ext();
  if (g_ext_ok) g_bind(vao);
#else
  glBindVertexArray(vao);
#endif
}

void
sogl_vao_get_binding(GLint * out)
{
  if (!out) return;
#if defined(COIN_GLES2) && !defined(COIN_USE_GLAD)
  sogl_vao_ensure_ext();
  if (g_ext_ok) {
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, out);
    return;
  }
  *out = 0;
#else
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, out);
#endif
}

void
sogl_vao_init(SoGLVAO * v)
{
  memset(v, 0, sizeof(*v));
}

void
sogl_vao_destroy(SoGLVAO * v)
{
  if (v->vao) sogl_vao_delete(1, &v->vao);
  if (v->ownsVBO && v->vbo) glDeleteBuffers(1, &v->vbo);
  if (v->ownsEBO && v->ebo) glDeleteBuffers(1, &v->ebo);
  memset(v, 0, sizeof(*v));
}

void
sogl_vao_bind(SoGLVAO * v)
{
  if (!v->vao) sogl_vao_gen(1, &v->vao);
  sogl_vao_bind_id(v->vao);
}

void
sogl_vao_unbind(void)
{
  sogl_vao_bind_id(0);
}

void
sogl_vao_set_attrib(int location, int size, GLenum type,
                    GLsizei stride, const void * pointer, GLuint buffer)
{
  if (buffer) glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glEnableVertexAttribArray((GLuint)location);
  glVertexAttribPointer((GLuint)location, size, type, GL_FALSE, stride, pointer);
}

void
sogl_vao_draw_arrays(GLenum mode, int first, int count)
{
  glDrawArrays(mode, first, count);
}

void
sogl_vao_draw_elements(GLenum mode, int count, GLenum type, const void * indices)
{
  glDrawElements(mode, count, type, indices);
}

GLuint
sogl_vao_upload_temp(GLenum target, const void * data, size_t size)
{
  GLuint buf = 0;
  glGenBuffers(1, &buf);
  glBindBuffer(target, buf);
  glBufferData(target, (GLsizeiptr)size, data, GL_STREAM_DRAW);
  return buf;
}

#endif /* COIN_GL_MODERN */
