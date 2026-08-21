/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
\**************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "rendering/glmodern/SoGLCaps.h"

#ifdef COIN_GL_MODERN

#include <Inventor/system/gl.h>
#include <Inventor/errors/SoDebugError.h>
#include <cstring>
#include "rendering/glmodern/SoGLVAO.h"

static SoGLCaps g_caps;
static SbBool g_caps_inited = FALSE;

const SoGLCaps *
sogl_caps(void)
{
  return &g_caps;
}

void
sogl_caps_init_from_context(void)
{
  memset(&g_caps, 0, sizeof(g_caps));
  g_caps.maxLights = 8;
  g_caps.maxTextureUnits = 8;

#if defined(COIN_GLES2)
  g_caps.isES = TRUE;
  g_caps.isModernImplemented = TRUE;
  g_caps.profileName = "GLES2";
  g_caps.hasUBO = FALSE;
  g_caps.hasVAO = FALSE;
  g_caps.hasClipDistance = FALSE;
  g_caps.has3DTexture = FALSE;
#elif defined(COIN_GLES3)
  g_caps.isES = TRUE;
  g_caps.isModernImplemented = TRUE;
  g_caps.profileName = "GLES3";
  g_caps.hasUBO = TRUE;
  g_caps.hasVAO = TRUE;
  g_caps.hasClipDistance = FALSE; /* use discard */
  g_caps.has3DTexture = TRUE;
#elif defined(COIN_GL3_CORE)
  g_caps.isES = FALSE;
  g_caps.isModernImplemented = TRUE;
  g_caps.profileName = "GL3";
  g_caps.hasUBO = TRUE;
  g_caps.hasVAO = TRUE;
  g_caps.hasClipDistance = FALSE; /* unified discard path */
  g_caps.has3DTexture = TRUE;
#else
  g_caps.profileName = "UNKNOWN";
  g_caps.isModernImplemented = FALSE;
#endif

  const char * ver = (const char *)glGetString(GL_VERSION);
  if (ver) {
    /* ES strings look like "OpenGL ES 3.0 ..." */
    const char * p = ver;
    while (*p && (*p < '0' || *p > '9')) ++p;
    if (*p) {
      g_caps.major = 0;
      g_caps.minor = 0;
      while (*p >= '0' && *p <= '9') {
        g_caps.major = g_caps.major * 10 + (*p - '0');
        ++p;
      }
      if (*p == '.') {
        ++p;
        while (*p >= '0' && *p <= '9') {
          g_caps.minor = g_caps.minor * 10 + (*p - '0');
          ++p;
        }
      }
    }
  }

  GLint units = 0;
  glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &units);
  if (units > 0) g_caps.maxTextureUnits = units;

#if defined(COIN_GLES2)
  g_caps.hasVAO = sogl_vao_supported();
#endif

  g_caps_inited = TRUE;
}

SbBool
sogl_caps_validate_context(void)
{
  if (!g_caps_inited) sogl_caps_init_from_context();

#if defined(COIN_GLES2)
  if (g_caps.major < 2) {
    SoDebugError::post("sogl_caps_validate_context",
                       "GLES2 profile requires OpenGL ES 2.0+, got %d.%d (%s)",
                       g_caps.major, g_caps.minor,
                       (const char *)glGetString(GL_VERSION));
    return FALSE;
  }
  return TRUE;
#elif defined(COIN_GLES3)
  if (g_caps.major < 3) {
    SoDebugError::post("sogl_caps_validate_context",
                       "GLES3 profile requires OpenGL ES 3.0+, got %d.%d (%s)",
                       g_caps.major, g_caps.minor,
                       (const char *)glGetString(GL_VERSION));
    return FALSE;
  }
  return TRUE;
#elif defined(COIN_GL3_CORE)
  if (g_caps.major < 3 || (g_caps.major == 3 && g_caps.minor < 2)) {
    SoDebugError::post("sogl_caps_validate_context",
                       "GL3 profile requires OpenGL 3.2 Core+, got %d.%d (%s)",
                       g_caps.major, g_caps.minor,
                       (const char *)glGetString(GL_VERSION));
    return FALSE;
  }
#  ifndef GL_CONTEXT_PROFILE_MASK
#  define GL_CONTEXT_PROFILE_MASK 0x9126
#  endif
#  ifndef GL_CONTEXT_CORE_PROFILE_BIT
#  define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#  endif
  {
    GLint mask = 0;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &mask);
    /* Some drivers return 0 for profile mask; accept if version ok. */
    if (mask != 0 && (mask & GL_CONTEXT_CORE_PROFILE_BIT) == 0) {
      SoDebugError::postWarning("sogl_caps_validate_context",
                                "Context does not report CORE profile (mask=0x%x); "
                                "continuing because version is %d.%d",
                                (int)mask, g_caps.major, g_caps.minor);
    }
    /* Drain INVALID_ENUM from drivers that reject PROFILE_MASK. */
    while (glGetError() != GL_NO_ERROR) { }
  }
  return TRUE;
#else
  return FALSE;
#endif
}

#endif /* COIN_GL_MODERN */
