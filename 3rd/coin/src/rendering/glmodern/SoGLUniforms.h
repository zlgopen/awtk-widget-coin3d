#ifndef COIN_SOGLUNIFORMS_H
#define COIN_SOGLUNIFORMS_H

/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
\**************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Inventor/SbMatrix.h>
#include <Inventor/SbColor.h>
#include <Inventor/C/basic.h>

enum {
  SOGL_ATTR_POSITION = 0,
  SOGL_ATTR_NORMAL = 1,
  SOGL_ATTR_COLOR = 2,
  SOGL_ATTR_TEXCOORD0 = 3,
  SOGL_ATTR_TEXCOORD1 = 4,
  SOGL_ATTR_TANGENT = 5
};

enum {
  SOGL_MAX_LIGHTS = 8,
  SOGL_MAX_CLIP_PLANES = 6
};

enum SoGLLightType {
  SOGL_LIGHT_NONE = 0,
  SOGL_LIGHT_DIRECTIONAL = 1,
  SOGL_LIGHT_POINT = 2,
  SOGL_LIGHT_SPOT = 3
};

enum SoGLTexEnvMode {
  SOGL_TEXENV_MODULATE = 0,
  SOGL_TEXENV_REPLACE = 1,
  SOGL_TEXENV_DECAL = 2,
  SOGL_TEXENV_BLEND = 3,
  SOGL_TEXENV_ADD = 4
};

struct SoGLLightData {
  int type;
  float position[4];
  float direction[3];
  float diffuse[4];
  float specular[4];
  float ambient[4];
  float spotCutoff; /* degrees, 180 = not spot */
  float spotExponent;
  float attenuation[3]; /* constant, linear, quadratic */
};

struct SoGLFrameUniforms {
  float modelView[16];
  float projection[16];
  float normalMatrix[16];
  float diffuse[4];
  float ambient[4];
  float specular[4];
  float emissive[4];
  float shininess;
  int lightModel; /* 0 = base color, 1 = phong */
  int numLights;
  int useTexture;
  int texEnvMode;
  float fogColor[4];
  float fogDensity;
  int fogType; /* 0 = none, 1 = haze, 2 = fog, 3 = smoke */
  int numClipPlanes;
  float clipPlanes[SOGL_MAX_CLIP_PLANES][4];
  SoGLLightData lights[SOGL_MAX_LIGHTS];
};

#ifdef COIN_GL_MODERN
void sogl_uniforms_reset(SoGLFrameUniforms * u);
void sogl_uniforms_set_modelview(SoGLFrameUniforms * u, const SbMatrix & mv);
void sogl_uniforms_set_projection(SoGLFrameUniforms * u, const SbMatrix & p);
void sogl_uniforms_upload(const SoGLFrameUniforms * u, unsigned int program);
SoGLFrameUniforms * sogl_uniforms_current(void);
#else
static inline SoGLFrameUniforms * sogl_uniforms_current(void) { return NULL; }
#endif

#endif /* !COIN_SOGLUNIFORMS_H */
