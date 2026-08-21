#ifndef COIN_SOGLDEVICE_H
#define COIN_SOGLDEVICE_H

/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
\**************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Inventor/C/basic.h>
#include <Inventor/SbMatrix.h>
#include <Inventor/SbColor.h>

#include "rendering/glmodern/SoGLCaps.h"
#include "rendering/glmodern/SoGLUniforms.h"

class SoState;

#ifdef COIN_GL_MODERN

/* Load GL entry points (glad on Windows). Safe to call multiple times. */
SbBool sogl_gl_loader_init(void);

/* Bind and validate current GL context for the modern backend.
   Returns FALSE on wrong profile or missing context. */
SbBool sogl_device_bind(int contextid);

/* Ensure default program is bound and frame uniforms are uploaded. */
void sogl_device_begin_frame(SoState * state);
void sogl_device_end_frame(void);

void sogl_device_set_modelview(const SbMatrix & mv);
void sogl_device_set_projection(const SbMatrix & p);
void sogl_device_set_material(const SbColor & diffuse, float alpha,
                             const SbColor & ambient,
                             const SbColor & specular,
                             const SbColor & emissive,
                             float shininess);
void sogl_device_set_light_model(int phong);
void sogl_device_clear_lights(void);
void sogl_device_add_light(const SoGLLightData * light);
void sogl_device_set_texture(SbBool enabled, int texEnvMode, unsigned int textureId);
void sogl_device_set_fog(int type, const SbColor & color, float density);
void sogl_device_set_clip_plane(int index, const float plane[4]);
void sogl_device_set_num_clip_planes(int n);

void sogl_device_apply_state(void);
void sogl_device_mark_dirty(void);

SbBool sogl_device_is_active(void);
int sogl_device_context_id(void);

/* Disable display-list auto cache for modern builds. */
void sogl_device_disable_auto_cache(SoState * state);

#else /* !COIN_GL_MODERN */

static inline SbBool sogl_gl_loader_init(void) { return TRUE; }
static inline SbBool sogl_device_bind(int) { return FALSE; }
static inline SbBool sogl_device_is_active(void) { return FALSE; }

#endif /* COIN_GL_MODERN */

#endif /* !COIN_SOGLDEVICE_H */
