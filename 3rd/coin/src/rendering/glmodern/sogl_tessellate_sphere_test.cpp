#include "rendering/glmodern/SoGLTessellate.h"

#include <cmath>
#include <cstdio>
#include <vector>

int
main(void)
{
  const float radius = 2.0f;
  const int stacks = 8;
  const int slices = 8;
  const int nverts = sogl_tessellate_sphere_vertex_count(stacks, slices);
  if (nverts < 3 || (nverts % 3) != 0) {
    std::fprintf(stderr, "expected triangle vertices, got %d\n", nverts);
    return 1;
  }

  std::vector<float> verts(static_cast<size_t>(nverts) * 8u, 0.0f);
  const int written = sogl_tessellate_sphere(radius, stacks, slices,
                                            verts.data(), static_cast<int>(verts.size()));
  if (written != nverts) {
    std::fprintf(stderr, "wrote %d vertices, expected %d\n", written, nverts);
    return 1;
  }

  for (int i = 0; i < written; ++i) {
    const float x = verts[static_cast<size_t>(i) * 8u];
    const float y = verts[static_cast<size_t>(i) * 8u + 1u];
    const float z = verts[static_cast<size_t>(i) * 8u + 2u];
    const float len = std::sqrt(x * x + y * y + z * z);
    if (std::fabs(len - radius) > 0.02f) {
      std::fprintf(stderr, "vertex %d not on sphere: length %f\n", i, len);
      return 1;
    }
  }

  return 0;
}
