#ifndef COIN_SOGLSHADER_MODERN_H
#define COIN_SOGLSHADER_MODERN_H

/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
\**************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Inventor/C/basic.h>
#include <Inventor/system/gl.h>

struct SoGLShaderKey {
  int numLights;
  int useTexture;
  int texEnvMode;
  int fogType;
  int numClipPlanes;
  int twoSided;
};

struct SoGLModernProgram {
  GLuint program;
  SoGLShaderKey key;
  int u_modelView;
  int u_projection;
  int u_normalMatrix;
  int u_diffuse;
  int u_ambient;
  int u_specular;
  int u_emissive;
  int u_shininess;
  int u_lightModel;
  int u_numLights;
  int u_useTexture;
  int u_texEnvMode;
  int u_tex0;
  int u_fogColor;
  int u_fogDensity;
  int u_fogType;
  int u_numClipPlanes;
  int u_lights_type[8];
  int u_lights_position[8];
  int u_lights_direction[8];
  int u_lights_diffuse[8];
  int u_lights_specular[8];
  int u_lights_ambient[8];
  int u_lights_spotCutoff[8];
  int u_lights_spotExponent[8];
  int u_lights_attenuation[8];
  int u_clipPlanes[6];
};

#ifdef COIN_GL_MODERN
GLuint sogl_shader_compile(GLenum type, const char * source);
GLuint sogl_shader_link(GLuint vs, GLuint fs);
SoGLModernProgram * sogl_shader_get_default(const SoGLShaderKey * key);
void sogl_shader_use(SoGLModernProgram * prog);
const char * sogl_shader_preamble(void);
#else
#endif

#endif /* !COIN_SOGLSHADER_MODERN_H */
