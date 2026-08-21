#ifndef COIN_SOGLCAPS_H
#define COIN_SOGLCAPS_H

/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
\**************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Inventor/C/basic.h>

struct SoGLCaps {
  SbBool isES;
  SbBool isModernImplemented;
  int major;
  int minor;
  SbBool hasUBO;
  SbBool hasVAO;
  SbBool hasClipDistance;
  SbBool has3DTexture;
  int maxLights;
  int maxTextureUnits;
  const char * profileName;
};

#ifdef COIN_GL_MODERN
const SoGLCaps * sogl_caps(void);
void sogl_caps_init_from_context(void);
SbBool sogl_caps_validate_context(void);
#else
static inline const SoGLCaps * sogl_caps(void) { return NULL; }
#endif

#endif /* !COIN_SOGLCAPS_H */
