/**
 * File:   coin3d_gl.cpp
 * Author: AWTK Develop Team
 * Brief:  coin3d OpenGL 绘制与状态保护
 *
 * Copyright (c) 2026 - 2026 Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 */

/**
 * History:
 * ================================================================
 * 2026-08-16 Li XianJing <xianjimli@hotmail.com> created
 * 2026-08-17 Li XianJing <xianjimli@hotmail.com> rewrite for Coin backend
 * 2026-08-20 Li XianJing <xianjimli@hotmail.com> opaque background clear for snapshot FBO
 * 2026-08-21 Li XianJing <xianjimli@hotmail.com> GLES2 framebuffer save/restore for Raspberry Pi
 *
 */

#include <string.h>
#include "tkc/mem.h"
#include "tkc/utils.h"
#include "base/vgcanvas.h"
#include "base/system_info.h"
#include "base/window_manager.h"
#include "coin3d_gl.h"

#ifdef WITH_GPU_GL
#include "base/opengl.h"

#define COIN3D_GL_OPAQUE_ALPHA 1.0f

#ifdef WITH_GPU_GLES2
#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER 0x8D40
#endif
#ifndef GL_FRAMEBUFFER_BINDING
#define GL_FRAMEBUFFER_BINDING 0x8CA6
#endif
#endif

typedef struct _coin3d_gl_state_t {
  GLint viewport[4];
  GLint program;
  GLint vao;
  GLint array_buffer;
  GLint element_buffer;
  GLint draw_framebuffer;
  GLint read_framebuffer;
  GLint active_texture;
  GLint texture_binding_2d;
  GLboolean depth_test;
  GLboolean blend;
  GLboolean cull_face;
  GLboolean scissor_test;
  GLint scissor_box[4];
  GLboolean stencil_test;
  GLboolean depth_mask;
  GLint depth_func;
  GLint blend_src_rgb;
  GLint blend_dst_rgb;
  GLint blend_src_alpha;
  GLint blend_dst_alpha;
  GLfloat clear_color[4];
} coin3d_gl_state_t;

static ret_t coin3d_gl_save_state(coin3d_gl_state_t* state) {
  return_value_if_fail(state != NULL, RET_BAD_PARAMS);

  glGetIntegerv(GL_VIEWPORT, state->viewport);
  glGetIntegerv(GL_CURRENT_PROGRAM, &state->program);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &state->array_buffer);
  glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &state->element_buffer);
#ifdef WITH_GPU_GLES2
  state->vao = 0;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &state->draw_framebuffer);
  state->read_framebuffer = state->draw_framebuffer;
#else
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &state->vao);
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &state->draw_framebuffer);
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &state->read_framebuffer);
#endif
  glGetIntegerv(GL_ACTIVE_TEXTURE, &state->active_texture);
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &state->texture_binding_2d);
  state->depth_test = glIsEnabled(GL_DEPTH_TEST);
  state->blend = glIsEnabled(GL_BLEND);
  state->cull_face = glIsEnabled(GL_CULL_FACE);
  state->scissor_test = glIsEnabled(GL_SCISSOR_TEST);
  glGetIntegerv(GL_SCISSOR_BOX, state->scissor_box);
  state->stencil_test = glIsEnabled(GL_STENCIL_TEST);
  glGetBooleanv(GL_DEPTH_WRITEMASK, &state->depth_mask);
  glGetIntegerv(GL_DEPTH_FUNC, &state->depth_func);
  glGetIntegerv(GL_BLEND_SRC_RGB, &state->blend_src_rgb);
  glGetIntegerv(GL_BLEND_DST_RGB, &state->blend_dst_rgb);
  glGetIntegerv(GL_BLEND_SRC_ALPHA, &state->blend_src_alpha);
  glGetIntegerv(GL_BLEND_DST_ALPHA, &state->blend_dst_alpha);
  glGetFloatv(GL_COLOR_CLEAR_VALUE, state->clear_color);

  return RET_OK;
}

static ret_t coin3d_gl_restore_state(const coin3d_gl_state_t* state) {
  return_value_if_fail(state != NULL, RET_BAD_PARAMS);

  glViewport(state->viewport[0], state->viewport[1], state->viewport[2], state->viewport[3]);
  glUseProgram((GLuint)state->program);
#ifndef WITH_GPU_GLES2
  glBindVertexArray((GLuint)state->vao);
#endif
  glBindBuffer(GL_ARRAY_BUFFER, (GLuint)state->array_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)state->element_buffer);
#ifdef WITH_GPU_GLES2
  glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)state->draw_framebuffer);
#else
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint)state->draw_framebuffer);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, (GLuint)state->read_framebuffer);
#endif
  glActiveTexture((GLenum)state->active_texture);
  glBindTexture(GL_TEXTURE_2D, (GLuint)state->texture_binding_2d);

  if (state->depth_test) {
    glEnable(GL_DEPTH_TEST);
  } else {
    glDisable(GL_DEPTH_TEST);
  }
  if (state->blend) {
    glEnable(GL_BLEND);
  } else {
    glDisable(GL_BLEND);
  }
  if (state->cull_face) {
    glEnable(GL_CULL_FACE);
  } else {
    glDisable(GL_CULL_FACE);
  }
  if (state->scissor_test) {
    glEnable(GL_SCISSOR_TEST);
  } else {
    glDisable(GL_SCISSOR_TEST);
  }
  glScissor(state->scissor_box[0], state->scissor_box[1], state->scissor_box[2],
            state->scissor_box[3]);
  if (state->stencil_test) {
    glEnable(GL_STENCIL_TEST);
  } else {
    glDisable(GL_STENCIL_TEST);
  }

  glDepthMask(state->depth_mask);
  glDepthFunc((GLenum)state->depth_func);
  glBlendFuncSeparate((GLenum)state->blend_src_rgb, (GLenum)state->blend_dst_rgb,
                      (GLenum)state->blend_src_alpha, (GLenum)state->blend_dst_alpha);
  glClearColor(state->clear_color[0], state->clear_color[1], state->clear_color[2],
               state->clear_color[3]);

  return RET_OK;
}

static ret_t coin3d_gl_clip_to_widget(widget_t* widget) {
  int32_t x = 0;
  int32_t y = 0;
  int32_t w = 0;
  int32_t h = 0;
  return_value_if_fail(coin3d_gl_get_widget_gl_rect(widget, &x, &y, &w, &h) == RET_OK, RET_FAIL);

  /* glClear 不受 viewport 限制，必须用 scissor 把背景清屏限制在控件内 */
  glViewport((GLint)x, (GLint)y, (GLsizei)w, (GLsizei)h);
  glEnable(GL_SCISSOR_TEST);
  glScissor((GLint)x, (GLint)y, (GLsizei)w, (GLsizei)h);

  return RET_OK;
}
#endif /*WITH_GPU_GL*/

ret_t coin3d_gl_get_widget_gl_rect(widget_t* widget, int32_t* x, int32_t* y, int32_t* w, int32_t* h) {
  point_t pos = {0, 0};
  float_t ratio = 1.0f;
  int32_t win_h = 0;
  system_info_t* si = system_info();
  widget_t* wm = NULL;
  widget_t* win = NULL;

  return_value_if_fail(widget != NULL && x != NULL && y != NULL && w != NULL && h != NULL,
                       RET_BAD_PARAMS);

  wm = widget_get_window_manager(widget);
  win = widget_get_window(widget);
  if (si != NULL) {
    ratio = si->device_pixel_ratio;
  }

  win_h = widget->h;
  {
    widget_t* iter = widget->parent;
    while (iter != NULL) {
      if (iter->h > win_h) {
        win_h = iter->h;
      }
      iter = iter->parent;
    }
  }
  if (win != NULL && win->h > win_h) {
    win_h = win->h;
  }
  if (wm != NULL && wm->h > win_h) {
    win_h = wm->h;
  }

  widget_to_screen(widget, &pos);
  *x = (int32_t)(pos.x * ratio);
  *y = (int32_t)((win_h - pos.y - widget->h) * ratio);
  *w = (int32_t)(widget->w * ratio);
  *h = (int32_t)(widget->h * ratio);
  if (*w < 1) {
    *w = 1;
  }
  if (*h < 1) {
    *h = 1;
  }

  return RET_OK;
}

ret_t coin3d_gl_init(coin3d_gl_t* gl) {
  return_value_if_fail(gl != NULL, RET_BAD_PARAMS);

#ifdef WITH_GPU_GL
  if (gl->ready) {
    return RET_OK;
  }
  /* 仅加载 GL 入口，避免 opengl_init() 在绘制中途关闭 stencil/scissor */
  opengl_loadGL();
  gl->ready = 1;
#endif /*WITH_GPU_GL*/

  return RET_OK;
}

ret_t coin3d_gl_paint(coin3d_gl_t* gl, coin3d_coin_t* coin, widget_t* widget, canvas_t* c) {
#ifdef WITH_GPU_GL
  vgcanvas_t* vg = NULL;
  coin3d_gl_state_t state;
#endif /*WITH_GPU_GL*/
  return_value_if_fail(gl != NULL && coin != NULL && widget != NULL && c != NULL, RET_BAD_PARAMS);

#ifdef WITH_GPU_GL
  vg = canvas_get_vgcanvas(c);
  return_value_if_fail(vg != NULL, RET_BAD_PARAMS);

  if (!gl->ready) {
    return_value_if_fail(coin3d_gl_init(gl) == RET_OK, RET_FAIL);
  }

  memset(&state, 0x00, sizeof(state));
  coin3d_gl_save_state(&state);
  vgcanvas_flush(vg);
  coin3d_coin_set_viewport(coin, widget);
  coin3d_gl_clip_to_widget(widget);
  glEnable(GL_DEPTH_TEST);
  {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    if (coin3d_coin_get_background_rgb(coin, &r, &g, &b) == RET_OK) {
      glClearColor(r, g, b, COIN3D_GL_OPAQUE_ALPHA);
    }
  }
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  coin3d_coin_render(coin);
  coin3d_gl_restore_state(&state);
#endif /*WITH_GPU_GL*/

  return RET_OK;
}

ret_t coin3d_gl_deinit(coin3d_gl_t* gl) {
  return_value_if_fail(gl != NULL, RET_BAD_PARAMS);
  memset(gl, 0x00, sizeof(*gl));
  return RET_OK;
}
