#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "rendering/glmodern/SoGLTessellate.h"

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void
sogl_clamp_sphere_tess(int * stacks, int * slices)
{
  if (*stacks < 3) {
    *stacks = 3;
  }
  if (*slices < 4) {
    *slices = 4;
  }
  if (*slices > 128) {
    *slices = 128;
  }
}

static void
sogl_emit_pnuv(float * verts, int * out,
               float x, float y, float z,
               float nx, float ny, float nz,
               float u, float v)
{
  int o = *out;
  verts[o++] = x;
  verts[o++] = y;
  verts[o++] = z;
  verts[o++] = nx;
  verts[o++] = ny;
  verts[o++] = nz;
  verts[o++] = u;
  verts[o++] = v;
  *out = o;
}

int
sogl_tessellate_sphere_vertex_count(int stacks, int slices)
{
  sogl_clamp_sphere_tess(&stacks, &slices);
  return 6 * stacks * slices;
}

int
sogl_tessellate_sphere(float radius, int stacks, int slices,
                       float * verts, int max_floats)
{
  int nverts = 0;
  int need = 0;
  int out = 0;
  int i = 0;
  int j = 0;

  sogl_clamp_sphere_tess(&stacks, &slices);
  nverts = 6 * stacks * slices;
  need = nverts * SOGL_PNUV_FLOATS;
  if (verts == NULL || max_floats < need) {
    return 0;
  }

  if (radius < 0.0f) {
    radius = -radius;
  }

  for (i = 0; i < stacks; ++i) {
    const float phi0 = static_cast<float>(M_PI) * static_cast<float>(i) / static_cast<float>(stacks);
    const float phi1 = static_cast<float>(M_PI) * static_cast<float>(i + 1) / static_cast<float>(stacks);
    const float y0 = std::cos(phi0);
    const float r0 = std::sin(phi0);
    const float y1 = std::cos(phi1);
    const float r1 = std::sin(phi1);
    const float v0 = 1.0f - static_cast<float>(i) / static_cast<float>(stacks);
    const float v1 = 1.0f - static_cast<float>(i + 1) / static_cast<float>(stacks);

    for (j = 0; j < slices; ++j) {
      const float th0 = 2.0f * static_cast<float>(M_PI) * static_cast<float>(j) / static_cast<float>(slices);
      const float th1 = 2.0f * static_cast<float>(M_PI) * static_cast<float>(j + 1) / static_cast<float>(slices);
      const float x00 = r0 * std::sin(th0);
      const float z00 = r0 * std::cos(th0);
      const float x01 = r0 * std::sin(th1);
      const float z01 = r0 * std::cos(th1);
      const float x10 = r1 * std::sin(th0);
      const float z10 = r1 * std::cos(th0);
      const float x11 = r1 * std::sin(th1);
      const float z11 = r1 * std::cos(th1);
      const float u0 = static_cast<float>(j) / static_cast<float>(slices);
      const float u1 = static_cast<float>(j + 1) / static_cast<float>(slices);

      sogl_emit_pnuv(verts, &out, x00 * radius, y0 * radius, z00 * radius, x00, y0, z00, u0, v0);
      sogl_emit_pnuv(verts, &out, x10 * radius, y1 * radius, z10 * radius, x10, y1, z10, u0, v1);
      sogl_emit_pnuv(verts, &out, x11 * radius, y1 * radius, z11 * radius, x11, y1, z11, u1, v1);

      sogl_emit_pnuv(verts, &out, x00 * radius, y0 * radius, z00 * radius, x00, y0, z00, u0, v0);
      sogl_emit_pnuv(verts, &out, x11 * radius, y1 * radius, z11 * radius, x11, y1, z11, u1, v1);
      sogl_emit_pnuv(verts, &out, x01 * radius, y0 * radius, z01 * radius, x01, y0, z01, u1, v0);
    }
  }

  return nverts;
}
