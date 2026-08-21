/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
\**************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "rendering/glmodern/SoGLUniforms.h"

#ifdef COIN_GL_MODERN

#include <Inventor/system/gl.h>
#include <cstring>
#include "rendering/glmodern/SoGLShader.h"

static SoGLFrameUniforms g_uniforms;
static SbBool g_uniforms_inited = FALSE;

void
sogl_uniforms_reset(SoGLFrameUniforms * u)
{
  memset(u, 0, sizeof(*u));
  u->diffuse[0] = u->diffuse[1] = u->diffuse[2] = u->diffuse[3] = 1.0f;
  u->ambient[0] = u->ambient[1] = u->ambient[2] = 0.2f;
  u->ambient[3] = 1.0f;
  u->specular[0] = u->specular[1] = u->specular[2] = 0.0f;
  u->specular[3] = 1.0f;
  u->emissive[3] = 1.0f;
  u->shininess = 0.2f;
  u->lightModel = 1;
  u->texEnvMode = SOGL_TEXENV_MODULATE;
  /* identity matrices */
  for (int i = 0; i < 16; ++i) {
    u->modelView[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    u->projection[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    u->normalMatrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
  }
}

static void
matrix_to_float(const SbMatrix & m, float out[16])
{
  const float * f = m[0];
  for (int i = 0; i < 16; ++i) out[i] = f[i];
}

void
sogl_uniforms_set_modelview(SoGLFrameUniforms * u, const SbMatrix & mv)
{
  matrix_to_float(mv, u->modelView);
  SbMatrix nrm = mv.inverse().transpose();
  matrix_to_float(nrm, u->normalMatrix);
}

void
sogl_uniforms_set_projection(SoGLFrameUniforms * u, const SbMatrix & p)
{
  matrix_to_float(p, u->projection);
}

SoGLFrameUniforms *
sogl_uniforms_current(void)
{
  if (!g_uniforms_inited) {
    sogl_uniforms_reset(&g_uniforms);
    g_uniforms_inited = TRUE;
  }
  return &g_uniforms;
}

void
sogl_uniforms_upload(const SoGLFrameUniforms * u, unsigned int program)
{
  SoGLShaderKey key;
  key.numLights = u->numLights;
  key.useTexture = u->useTexture;
  key.texEnvMode = u->texEnvMode;
  key.fogType = u->fogType;
  key.numClipPlanes = u->numClipPlanes;
  key.twoSided = 0;

  SoGLModernProgram * prog = sogl_shader_get_default(&key);
  if (!prog) return;
  (void)program;
  sogl_shader_use(prog);

  if (prog->u_modelView >= 0)
    glUniformMatrix4fv(prog->u_modelView, 1, GL_FALSE, u->modelView);
  if (prog->u_projection >= 0)
    glUniformMatrix4fv(prog->u_projection, 1, GL_FALSE, u->projection);
  if (prog->u_normalMatrix >= 0)
    glUniformMatrix4fv(prog->u_normalMatrix, 1, GL_FALSE, u->normalMatrix);
  if (prog->u_diffuse >= 0)
    glUniform4fv(prog->u_diffuse, 1, u->diffuse);
  if (prog->u_ambient >= 0)
    glUniform4fv(prog->u_ambient, 1, u->ambient);
  if (prog->u_specular >= 0)
    glUniform4fv(prog->u_specular, 1, u->specular);
  if (prog->u_emissive >= 0)
    glUniform4fv(prog->u_emissive, 1, u->emissive);
  if (prog->u_shininess >= 0)
    glUniform1f(prog->u_shininess, u->shininess);
  if (prog->u_lightModel >= 0)
    glUniform1i(prog->u_lightModel, u->lightModel);
  if (prog->u_numLights >= 0)
    glUniform1i(prog->u_numLights, u->numLights);
  if (prog->u_useTexture >= 0)
    glUniform1i(prog->u_useTexture, u->useTexture);
  if (prog->u_texEnvMode >= 0)
    glUniform1i(prog->u_texEnvMode, u->texEnvMode);
  if (prog->u_tex0 >= 0)
    glUniform1i(prog->u_tex0, 0);
  if (prog->u_fogColor >= 0)
    glUniform4fv(prog->u_fogColor, 1, u->fogColor);
  if (prog->u_fogDensity >= 0)
    glUniform1f(prog->u_fogDensity, u->fogDensity);
  if (prog->u_fogType >= 0)
    glUniform1i(prog->u_fogType, u->fogType);
  if (prog->u_numClipPlanes >= 0)
    glUniform1i(prog->u_numClipPlanes, u->numClipPlanes);

  for (int i = 0; i < SOGL_MAX_LIGHTS; ++i) {
    if (prog->u_lights_type[i] >= 0)
      glUniform1i(prog->u_lights_type[i], u->lights[i].type);
    if (prog->u_lights_position[i] >= 0)
      glUniform4fv(prog->u_lights_position[i], 1, u->lights[i].position);
    if (prog->u_lights_direction[i] >= 0)
      glUniform3fv(prog->u_lights_direction[i], 1, u->lights[i].direction);
    if (prog->u_lights_diffuse[i] >= 0)
      glUniform4fv(prog->u_lights_diffuse[i], 1, u->lights[i].diffuse);
    if (prog->u_lights_specular[i] >= 0)
      glUniform4fv(prog->u_lights_specular[i], 1, u->lights[i].specular);
    if (prog->u_lights_ambient[i] >= 0)
      glUniform4fv(prog->u_lights_ambient[i], 1, u->lights[i].ambient);
    if (prog->u_lights_spotCutoff[i] >= 0)
      glUniform1f(prog->u_lights_spotCutoff[i], u->lights[i].spotCutoff);
    if (prog->u_lights_spotExponent[i] >= 0)
      glUniform1f(prog->u_lights_spotExponent[i], u->lights[i].spotExponent);
    if (prog->u_lights_attenuation[i] >= 0)
      glUniform3fv(prog->u_lights_attenuation[i], 1, u->lights[i].attenuation);
  }
  for (int i = 0; i < SOGL_MAX_CLIP_PLANES; ++i) {
    if (prog->u_clipPlanes[i] >= 0)
      glUniform4fv(prog->u_clipPlanes[i], 1, u->clipPlanes[i]);
  }
}

#endif /* COIN_GL_MODERN */
