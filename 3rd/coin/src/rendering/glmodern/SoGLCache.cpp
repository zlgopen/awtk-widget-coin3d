/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
\**************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "rendering/glmodern/SoGLCache.h"

#ifdef COIN_GL_MODERN

#include <cstdlib>
#include <cstring>
#include "rendering/glmodern/SoGLVAO.h"

void
sogl_cache_init(SoGLModernCache * c, int contextid)
{
  memset(c, 0, sizeof(*c));
  c->contextid = contextid;
}

void
sogl_cache_destroy(SoGLModernCache * c)
{
  free(c->draws);
  memset(c, 0, sizeof(*c));
}

void
sogl_cache_open(SoGLModernCache * c)
{
  c->numDraws = 0;
  c->open = TRUE;
}

void
sogl_cache_close(SoGLModernCache * c)
{
  c->open = FALSE;
}

void
sogl_cache_add_draw(SoGLModernCache * c, const SoGLModernCacheDraw * draw)
{
  if (!c->open) return;
  if (c->numDraws >= c->capacity) {
    int ncap = c->capacity ? c->capacity * 2 : 8;
    SoGLModernCacheDraw * nd =
      (SoGLModernCacheDraw *)realloc(c->draws, sizeof(SoGLModernCacheDraw) * ncap);
    if (!nd) return;
    c->draws = nd;
    c->capacity = ncap;
  }
  c->draws[c->numDraws++] = *draw;
}

void
sogl_cache_call(const SoGLModernCache * c)
{
  for (int i = 0; i < c->numDraws; ++i) {
    const SoGLModernCacheDraw * d = &c->draws[i];
    sogl_vao_bind_id(d->vao);
    if (d->indexed)
      glDrawElements(d->mode, d->count, d->indexType, NULL);
    else
      glDrawArrays(d->mode, 0, d->count);
  }
  sogl_vao_unbind();
}

#endif /* COIN_GL_MODERN */
