/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
\**************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "rendering/glmodern/SoGLShader.h"

#ifdef COIN_GL_MODERN

#include <Inventor/errors/SoDebugError.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "rendering/glmodern/SoGLCaps.h"
#include "rendering/glmodern/SoGLShaderSource.h"

const char *
sogl_shader_preamble(void)
{
  return sogl_shader_preamble_for(sogl_shader_compile_dialect());
}

GLuint
sogl_shader_compile(GLenum type, const char * source)
{
  GLuint s = glCreateShader(type);
  const char * srcs[2] = { sogl_shader_preamble(), source };
  glShaderSource(s, 2, srcs, NULL);
  glCompileShader(s);
  GLint ok = 0;
  glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    char log[1024];
    GLsizei len = 0;
    glGetShaderInfoLog(s, 1023, &len, log);
    log[len] = 0;
    SoDebugError::post("sogl_shader_compile", "compile failed: %s", log);
    glDeleteShader(s);
    return 0;
  }
  return s;
}

GLuint
sogl_shader_link(GLuint vs, GLuint fs)
{
  GLuint p = glCreateProgram();
  glAttachShader(p, vs);
  glAttachShader(p, fs);
  glBindAttribLocation(p, 0, "a_position");
  glBindAttribLocation(p, 1, "a_normal");
  glBindAttribLocation(p, 2, "a_color");
  glBindAttribLocation(p, 3, "a_texcoord0");
  glLinkProgram(p);
  GLint ok = 0;
  glGetProgramiv(p, GL_LINK_STATUS, &ok);
  if (!ok) {
    char log[1024];
    GLsizei len = 0;
    glGetProgramInfoLog(p, 1023, &len, log);
    log[len] = 0;
    SoDebugError::post("sogl_shader_link", "link failed: %s", log);
    glDeleteProgram(p);
    return 0;
  }
  return p;
}

static void
cache_locations(SoGLModernProgram * prog)
{
  GLuint p = prog->program;
  prog->u_modelView = glGetUniformLocation(p, "u_modelView");
  prog->u_projection = glGetUniformLocation(p, "u_projection");
  prog->u_normalMatrix = glGetUniformLocation(p, "u_normalMatrix");
  prog->u_diffuse = glGetUniformLocation(p, "u_diffuse");
  prog->u_ambient = glGetUniformLocation(p, "u_ambient");
  prog->u_specular = glGetUniformLocation(p, "u_specular");
  prog->u_emissive = glGetUniformLocation(p, "u_emissive");
  prog->u_shininess = glGetUniformLocation(p, "u_shininess");
  prog->u_lightModel = glGetUniformLocation(p, "u_lightModel");
  prog->u_numLights = glGetUniformLocation(p, "u_numLights");
  prog->u_useTexture = glGetUniformLocation(p, "u_useTexture");
  prog->u_texEnvMode = glGetUniformLocation(p, "u_texEnvMode");
  prog->u_tex0 = glGetUniformLocation(p, "u_tex0");
  prog->u_fogColor = glGetUniformLocation(p, "u_fogColor");
  prog->u_fogDensity = glGetUniformLocation(p, "u_fogDensity");
  prog->u_fogType = glGetUniformLocation(p, "u_fogType");
  prog->u_numClipPlanes = glGetUniformLocation(p, "u_numClipPlanes");
  for (int i = 0; i < 8; ++i) {
    char name[64];
    snprintf(name, sizeof(name), "u_lightType[%d]", i);
    prog->u_lights_type[i] = glGetUniformLocation(p, name);
    snprintf(name, sizeof(name), "u_lightPosition[%d]", i);
    prog->u_lights_position[i] = glGetUniformLocation(p, name);
    snprintf(name, sizeof(name), "u_lightDirection[%d]", i);
    prog->u_lights_direction[i] = glGetUniformLocation(p, name);
    snprintf(name, sizeof(name), "u_lightDiffuse[%d]", i);
    prog->u_lights_diffuse[i] = glGetUniformLocation(p, name);
    snprintf(name, sizeof(name), "u_lightSpecular[%d]", i);
    prog->u_lights_specular[i] = glGetUniformLocation(p, name);
    snprintf(name, sizeof(name), "u_lightAmbient[%d]", i);
    prog->u_lights_ambient[i] = glGetUniformLocation(p, name);
    snprintf(name, sizeof(name), "u_lightSpotCutoff[%d]", i);
    prog->u_lights_spotCutoff[i] = glGetUniformLocation(p, name);
    snprintf(name, sizeof(name), "u_lightSpotExponent[%d]", i);
    prog->u_lights_spotExponent[i] = glGetUniformLocation(p, name);
    snprintf(name, sizeof(name), "u_lightAttenuation[%d]", i);
    prog->u_lights_attenuation[i] = glGetUniformLocation(p, name);
  }
  for (int i = 0; i < 6; ++i) {
    char name[64];
    snprintf(name, sizeof(name), "u_clipPlanes[%d]", i);
    prog->u_clipPlanes[i] = glGetUniformLocation(p, name);
  }
}

static SoGLModernProgram g_defaultProg;
static SbBool g_defaultOk = FALSE;

SoGLModernProgram *
sogl_shader_get_default(const SoGLShaderKey * key)
{
  (void)key; /* single uber-shader for MVP; variants later */
  if (g_defaultOk) return &g_defaultProg;

  const SoGLShaderDialect dialect = sogl_shader_compile_dialect();
  GLuint vs = sogl_shader_compile(GL_VERTEX_SHADER, sogl_shader_default_vert_for(dialect));
  GLuint fs = sogl_shader_compile(GL_FRAGMENT_SHADER, sogl_shader_default_frag_for(dialect));
  if (!vs || !fs) return NULL;
  GLuint prog = sogl_shader_link(vs, fs);
  glDeleteShader(vs);
  glDeleteShader(fs);
  if (!prog) return NULL;

  memset(&g_defaultProg, 0, sizeof(g_defaultProg));
  g_defaultProg.program = prog;
  if (key) g_defaultProg.key = *key;
  cache_locations(&g_defaultProg);
  g_defaultOk = TRUE;
  return &g_defaultProg;
}

void
sogl_shader_use(SoGLModernProgram * prog)
{
  if (prog) glUseProgram(prog->program);
}

#endif /* COIN_GL_MODERN */
