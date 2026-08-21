/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
\**************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "rendering/glmodern/SoGLOverlay.h"

#ifdef COIN_GL_MODERN

#include <Inventor/system/gl.h>
#include "rendering/glmodern/SoGLShaderSource.h"
#include "rendering/glmodern/SoGLVAO.h"

static GLuint
sogl_overlay_compile(GLenum type, const char * source)
{
  const char * preamble = sogl_shader_preamble_for(sogl_shader_compile_dialect());
  const char * sources[2] = { preamble, source };
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 2, sources, NULL);
  glCompileShader(shader);
  GLint ok = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

static GLuint
sogl_overlay_program(void)
{
  static GLuint program = 0;
  if (program) return program;

  const SoGLShaderDialect dialect = sogl_shader_compile_dialect();
  GLuint vs = sogl_overlay_compile(GL_VERTEX_SHADER, sogl_shader_overlay_vert_for(dialect));
  GLuint fs = sogl_overlay_compile(GL_FRAGMENT_SHADER, sogl_shader_overlay_frag_for(dialect));
  if (!vs || !fs) {
    if (vs) glDeleteShader(vs);
    if (fs) glDeleteShader(fs);
    return 0;
  }

  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glBindAttribLocation(program, 0, "a_position");
  glBindAttribLocation(program, 1, "a_texcoord");
  glLinkProgram(program);
  glDeleteShader(vs);
  glDeleteShader(fs);

  GLint ok = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &ok);
  if (!ok) {
    glDeleteProgram(program);
    program = 0;
  }
  return program;
}

void
sogl_overlay_draw_rgba(SoState * state,
                       float winx, float winy, float winz,
                       int width, int height,
                       const unsigned char * rgba,
                       SbBool origin_bottom_left)
{
  (void)state;
  if (!rgba || width <= 0 || height <= 0) return;

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  if (viewport[2] <= 0 || viewport[3] <= 0) return;

  GLuint program = sogl_overlay_program();
  if (!program) return;

  const float left = ((winx - viewport[0]) / viewport[2]) * 2.0f - 1.0f;
  const float right = ((winx + width - viewport[0]) / viewport[2]) * 2.0f - 1.0f;
  const float bottom = ((winy - viewport[1]) / viewport[3]) * 2.0f - 1.0f;
  const float top = ((winy + height - viewport[1]) / viewport[3]) * 2.0f - 1.0f;
  const float vbottom = origin_bottom_left ? 0.0f : 1.0f;
  const float vtop = origin_bottom_left ? 1.0f : 0.0f;
  const float vertices[] = {
    left,  bottom, winz, 0.0f, vbottom,
    right, bottom, winz, 1.0f, vbottom,
    right, top,    winz, 1.0f, vtop,
    left,  bottom, winz, 0.0f, vbottom,
    right, top,    winz, 1.0f, vtop,
    left,  top,    winz, 0.0f, vtop
  };

  GLint oldprogram, oldvao, oldvbo, oldtexture, oldactivetexture, oldunpackalignment;
  glGetIntegerv(GL_CURRENT_PROGRAM, &oldprogram);
  sogl_vao_get_binding(&oldvao);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &oldvbo);
  glGetIntegerv(GL_ACTIVE_TEXTURE, &oldactivetexture);
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldunpackalignment);
  glActiveTexture(GL_TEXTURE0);
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldtexture);

  GLuint texture = 0, vao = 0, vbo = 0;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, rgba);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  sogl_vao_gen(1, &vao);
  glGenBuffers(1, &vbo);
  sogl_vao_bind_id(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (const void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (const void *)(3 * sizeof(float)));

  glUseProgram(program);
  glUniform1i(glGetUniformLocation(program, "u_texture"), 0);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glDeleteBuffers(1, &vbo);
  sogl_vao_delete(1, &vao);
  glDeleteTextures(1, &texture);
  glBindBuffer(GL_ARRAY_BUFFER, (GLuint)oldvbo);
  sogl_vao_bind_id((GLuint)oldvao);
  glBindTexture(GL_TEXTURE_2D, (GLuint)oldtexture);
  glPixelStorei(GL_UNPACK_ALIGNMENT, oldunpackalignment);
  glActiveTexture((GLenum)oldactivetexture);
  glUseProgram((GLuint)oldprogram);
}

static GLuint
sogl_overlay_line_program(void)
{
  static GLuint program = 0;
  if (program) return program;
  const SoGLShaderDialect dialect = sogl_shader_compile_dialect();
  GLuint vs = sogl_overlay_compile(GL_VERTEX_SHADER, sogl_shader_overlay_line_vert_for(dialect));
  GLuint fs = sogl_overlay_compile(GL_FRAGMENT_SHADER, sogl_shader_overlay_line_frag_for(dialect));
  if (!vs || !fs) {
    if (vs) glDeleteShader(vs);
    if (fs) glDeleteShader(fs);
    return 0;
  }
  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glBindAttribLocation(program, 0, "a_position");
  glLinkProgram(program);
  glDeleteShader(vs);
  glDeleteShader(fs);
  GLint ok = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &ok);
  if (!ok) {
    glDeleteProgram(program);
    program = 0;
  }
  return program;
}

void
sogl_overlay_draw_lines(SoState * state,
                        const float * xy_pairs,
                        int num_verts,
                        SbBool loop,
                        float r, float g, float b,
                        float linewidth)
{
  (void)state;
  if (!xy_pairs || num_verts < 2) return;

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  if (viewport[2] <= 0 || viewport[3] <= 0) return;

  GLuint program = sogl_overlay_line_program();
  if (!program) return;

  GLint oldprogram, oldvao, oldvbo;
  glGetIntegerv(GL_CURRENT_PROGRAM, &oldprogram);
  sogl_vao_get_binding(&oldvao);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &oldvbo);

  GLuint vao = 0, vbo = 0;
  sogl_vao_gen(1, &vao);
  glGenBuffers(1, &vbo);
  sogl_vao_bind_id(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * num_verts, xy_pairs, GL_STREAM_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);

  glUseProgram(program);
  glUniform2f(glGetUniformLocation(program, "u_viewport"),
              (float)viewport[2], (float)viewport[3]);
  glUniform2f(glGetUniformLocation(program, "u_origin"),
              (float)viewport[0], (float)viewport[1]);
  glUniform3f(glGetUniformLocation(program, "u_color"), r, g, b);
  glLineWidth(linewidth > 0.f ? linewidth : 1.f);
  glDrawArrays(loop ? GL_LINE_LOOP : GL_LINE_STRIP, 0, num_verts);

  glDeleteBuffers(1, &vbo);
  sogl_vao_delete(1, &vao);
  glBindBuffer(GL_ARRAY_BUFFER, (GLuint)oldvbo);
  sogl_vao_bind_id((GLuint)oldvao);
  glUseProgram((GLuint)oldprogram);
}

#else

void
sogl_overlay_draw_rgba(SoState *, float, float, float, int, int,
                       const unsigned char *, SbBool)
{
}

void
sogl_overlay_draw_lines(SoState *, const float *, int, SbBool,
                        float, float, float, float)
{
}

#endif /* COIN_GL_MODERN */
