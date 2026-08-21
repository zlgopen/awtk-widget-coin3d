/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
\**************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "rendering/glmodern/SoGLDevice.h"

#ifdef COIN_GL_MODERN

#include <Inventor/system/gl.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/elements/SoGLCacheContextElement.h>
#include <Inventor/errors/SoDebugError.h>
#include <cstring>

#include "rendering/glmodern/SoGLCaps.h"
#include "rendering/glmodern/SoGLUniforms.h"
#include "rendering/glmodern/SoGLShader.h"
#include "rendering/glmodern/SoGLVAO.h"

static SbBool g_active = FALSE;
static int g_contextid = -1;
static unsigned int g_boundTexture = 0;
static unsigned int g_defaultWhiteTex = 0;
static SbBool g_dirty = TRUE;

static void
sogl_device_ensure_default_texture(void)
{
  if (g_defaultWhiteTex) return;
  const unsigned char white[4] = { 255, 255, 255, 255 };
  glGenTextures(1, &g_defaultWhiteTex);
  glBindTexture(GL_TEXTURE_2D, g_defaultWhiteTex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
}

static SbBool
sogl_gl_loader_init_impl(void)
{
#if defined(COIN_USE_GLAD)
  static int loaded = 0;
  if (loaded) {
    return TRUE;
  }
  if (!gladLoadGL()) {
    return FALSE;
  }
  loaded = 1;
#endif
  return TRUE;
}

SbBool
sogl_gl_loader_init(void)
{
  return sogl_gl_loader_init_impl();
}

SbBool
sogl_device_bind(int contextid)
{
  if (!sogl_gl_loader_init_impl()) {
    SoDebugError::post("sogl_device_bind",
                       "GL loader init failed (no current GL context?).");
    g_active = FALSE;
    return FALSE;
  }
  sogl_caps_init_from_context();
  if (!sogl_caps_validate_context()) {
    g_active = FALSE;
    return FALSE;
  }
  g_contextid = contextid;
  g_active = TRUE;
  /* Do not reset frame uniforms here. SoMaterialBundle::sendFirst()
     writes diffuse/ambient before the first draw, which then calls
     begin_frame → bind. Resetting would wipe that to white and make
     single-shape scenes (cube.iv, rotating_cube.iv) render gray. */
  g_dirty = TRUE;
  /* Touch default shader early to fail fast. */
  SoGLShaderKey key;
  memset(&key, 0, sizeof(key));
  if (!sogl_shader_get_default(&key)) {
    g_active = FALSE;
    return FALSE;
  }
  sogl_device_ensure_default_texture();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, g_defaultWhiteTex);
  g_boundTexture = g_defaultWhiteTex;
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  while (glGetError() != GL_NO_ERROR) { }
  return TRUE;
}

SbBool
sogl_device_is_active(void)
{
  return g_active;
}

int
sogl_device_context_id(void)
{
  return g_contextid;
}

void
sogl_device_disable_auto_cache(SoState * state)
{
  if (state) {
    SoGLCacheContextElement::shouldAutoCache(
      state, SoGLCacheContextElement::DONT_AUTO_CACHE);
  }
}

void
sogl_device_begin_frame(SoState * state)
{
  if (!g_active) {
    int ctx = state ? SoGLCacheContextElement::get(state) : 0;
    sogl_device_bind(ctx);
  }
  sogl_device_disable_auto_cache(state);
  g_dirty = TRUE;
}

void
sogl_device_end_frame(void)
{
  glUseProgram(0);
  sogl_vao_unbind();
}

void
sogl_device_set_modelview(const SbMatrix & mv)
{
  sogl_uniforms_set_modelview(sogl_uniforms_current(), mv);
  g_dirty = TRUE;
}

void
sogl_device_set_projection(const SbMatrix & p)
{
  sogl_uniforms_set_projection(sogl_uniforms_current(), p);
  g_dirty = TRUE;
}

void
sogl_device_set_material(const SbColor & diffuse, float alpha,
                         const SbColor & ambient,
                         const SbColor & specular,
                         const SbColor & emissive,
                         float shininess)
{
  SoGLFrameUniforms * u = sogl_uniforms_current();
  diffuse.getValue(u->diffuse[0], u->diffuse[1], u->diffuse[2]);
  u->diffuse[3] = alpha;
  ambient.getValue(u->ambient[0], u->ambient[1], u->ambient[2]);
  u->ambient[3] = 1.0f;
  specular.getValue(u->specular[0], u->specular[1], u->specular[2]);
  u->specular[3] = 1.0f;
  emissive.getValue(u->emissive[0], u->emissive[1], u->emissive[2]);
  u->emissive[3] = 1.0f;
  u->shininess = shininess;
  g_dirty = TRUE;
}

void
sogl_device_set_light_model(int phong)
{
  sogl_uniforms_current()->lightModel = phong ? 1 : 0;
  g_dirty = TRUE;
}

void
sogl_device_clear_lights(void)
{
  SoGLFrameUniforms * u = sogl_uniforms_current();
  u->numLights = 0;
  memset(u->lights, 0, sizeof(u->lights));
  g_dirty = TRUE;
}

void
sogl_device_add_light(const SoGLLightData * light)
{
  SoGLFrameUniforms * u = sogl_uniforms_current();
  if (u->numLights >= SOGL_MAX_LIGHTS) return;
  u->lights[u->numLights++] = *light;
  g_dirty = TRUE;
}

void
sogl_device_set_texture(SbBool enabled, int texEnvMode, unsigned int textureId)
{
  SoGLFrameUniforms * u = sogl_uniforms_current();
  u->useTexture = enabled ? 1 : 0;
  u->texEnvMode = texEnvMode;
  sogl_device_ensure_default_texture();
  g_boundTexture = (enabled && textureId) ? textureId : g_defaultWhiteTex;
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, g_boundTexture);
  g_dirty = TRUE;
}

void
sogl_device_set_fog(int type, const SbColor & color, float density)
{
  SoGLFrameUniforms * u = sogl_uniforms_current();
  u->fogType = type;
  color.getValue(u->fogColor[0], u->fogColor[1], u->fogColor[2]);
  u->fogColor[3] = 1.0f;
  u->fogDensity = density;
  g_dirty = TRUE;
}

void
sogl_device_set_clip_plane(int index, const float plane[4])
{
  if (index < 0 || index >= SOGL_MAX_CLIP_PLANES) return;
  SoGLFrameUniforms * u = sogl_uniforms_current();
  u->clipPlanes[index][0] = plane[0];
  u->clipPlanes[index][1] = plane[1];
  u->clipPlanes[index][2] = plane[2];
  u->clipPlanes[index][3] = plane[3];
  g_dirty = TRUE;
}

void
sogl_device_set_num_clip_planes(int n)
{
  SoGLFrameUniforms * u = sogl_uniforms_current();
  if (n < 0) n = 0;
  if (n > SOGL_MAX_CLIP_PLANES) n = SOGL_MAX_CLIP_PLANES;
  u->numClipPlanes = n;
  g_dirty = TRUE;
}

void
sogl_device_mark_dirty(void)
{
  g_dirty = TRUE;
}

void
sogl_device_apply_state(void)
{
  if (!g_active) return;
  if (g_dirty) {
    sogl_device_ensure_default_texture();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_boundTexture ? g_boundTexture : g_defaultWhiteTex);
    sogl_uniforms_upload(sogl_uniforms_current(), 0);
    g_dirty = FALSE;
  }
}

#endif /* COIN_GL_MODERN */
