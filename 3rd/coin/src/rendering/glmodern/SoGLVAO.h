#ifndef COIN_SOGLVAO_H
#define COIN_SOGLVAO_H

/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
\**************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Inventor/C/basic.h>
#include <Inventor/system/gl.h>

struct SoGLVAO {
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
  SbBool ownsVBO;
  SbBool ownsEBO;
  int vertexCount;
  int indexCount;
};

#ifdef COIN_GL_MODERN
SbBool sogl_vao_supported(void);
void sogl_vao_gen(GLsizei n, GLuint * arrays);
void sogl_vao_delete(GLsizei n, const GLuint * arrays);
void sogl_vao_bind_id(GLuint vao);
void sogl_vao_get_binding(GLint * out);
void sogl_vao_init(SoGLVAO * v);
void sogl_vao_destroy(SoGLVAO * v);
void sogl_vao_bind(SoGLVAO * v);
void sogl_vao_unbind(void);
void sogl_vao_set_attrib(int location, int size, GLenum type,
                         GLsizei stride, const void * pointer, GLuint buffer);
void sogl_vao_draw_arrays(GLenum mode, int first, int count);
void sogl_vao_draw_elements(GLenum mode, int count, GLenum type, const void * indices);
/* Upload client data into a temporary VBO (Core/ES forbid client pointers). */
GLuint sogl_vao_upload_temp(GLenum target, const void * data, size_t size);
#else
#endif

#endif /* !COIN_SOGLVAO_H */
