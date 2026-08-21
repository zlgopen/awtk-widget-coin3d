#ifndef COIN_SOGL_MODERN_BRIDGE_H
#define COIN_SOGL_MODERN_BRIDGE_H

/* Helpers to push Coin CPU element state into the modern GL device. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef COIN_GL_MODERN

#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewingMatrixElement.h>
#include <Inventor/elements/SoProjectionMatrixElement.h>
#include <Inventor/misc/SoState.h>
#include "rendering/glmodern/SoGLDevice.h"

static inline void
sogl_modern_sync_mvp(SoState * state)
{
  if (!state || !sogl_device_is_active()) return;
  /* Coin row-vector convention: p_eye = p_obj * Model * Viewing.
     Uploaded with GL_FALSE so GLSL column-vector (MV * p) matches. */
  SbMatrix modelview = SoModelMatrixElement::get(state);
  modelview.multRight(SoViewingMatrixElement::get(state));
  sogl_device_set_modelview(modelview);
  sogl_device_set_projection(SoProjectionMatrixElement::get(state));
}

#else
static inline void sogl_modern_sync_mvp(SoState *) {}
#endif

#endif /* !COIN_SOGL_MODERN_BRIDGE_H */
