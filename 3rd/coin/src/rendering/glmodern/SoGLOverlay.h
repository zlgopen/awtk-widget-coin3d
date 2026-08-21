#ifndef COIN_SOGL_OVERLAY_H
#define COIN_SOGL_OVERLAY_H

#include <Inventor/C/basic.h>

class SoState;

/* Draw an RGBA image in framebuffer pixel coordinates. winz is NDC depth. */
void sogl_overlay_draw_rgba(SoState * state,
                            float winx, float winy, float winz,
                            int width, int height,
                            const unsigned char * rgba,
                            SbBool origin_bottom_left);

/* Draw 2D pixel-space line strip/loop (selection overlays). */
void sogl_overlay_draw_lines(SoState * state,
                             const float * xy_pairs,
                             int num_verts,
                             SbBool loop,
                             float r, float g, float b,
                             float linewidth);

#endif /* COIN_SOGL_OVERLAY_H */
