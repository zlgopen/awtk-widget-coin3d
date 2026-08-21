#ifndef COIN_SOGLTESSELLATE_H
#define COIN_SOGLTESSELLATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Interleaved pos3 + nrm3 + uv2 floats per vertex. */
#define SOGL_PNUV_FLOATS 8

int sogl_tessellate_sphere_vertex_count(int stacks, int slices);
int sogl_tessellate_sphere(float radius, int stacks, int slices,
                           float * verts, int max_floats);

#ifdef __cplusplus
}
#endif

#endif /* !COIN_SOGLTESSELLATE_H */
