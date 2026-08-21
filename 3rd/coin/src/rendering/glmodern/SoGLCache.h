#ifndef COIN_SOGLCACHE_MODERN_H
#define COIN_SOGLCACHE_MODERN_H

/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
\**************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Inventor/C/basic.h>
#include <Inventor/system/gl.h>

/* Modern replacement for OpenGL display lists: recorded VAO draw calls. */

struct SoGLModernCacheDraw {
  GLuint vao;
  GLuint mode;
  int count;
  SbBool indexed;
  GLenum indexType;
};

struct SoGLModernCache {
  SoGLModernCacheDraw * draws;
  int numDraws;
  int capacity;
  int contextid;
  SbBool open;
};

#ifdef COIN_GL_MODERN
void sogl_cache_init(SoGLModernCache * c, int contextid);
void sogl_cache_destroy(SoGLModernCache * c);
void sogl_cache_open(SoGLModernCache * c);
void sogl_cache_close(SoGLModernCache * c);
void sogl_cache_add_draw(SoGLModernCache * c, const SoGLModernCacheDraw * draw);
void sogl_cache_call(const SoGLModernCache * c);
#else
#endif

#endif /* !COIN_SOGLCACHE_MODERN_H */
