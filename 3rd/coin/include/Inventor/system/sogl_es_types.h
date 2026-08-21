/* GLES headers omit desktop GL types and a few still-used enums / entry
   points. Add tokens with #ifndef only. Do not redeclare glDepthRange /
   glPolygonMode / glPointSize: GLX may already have declared them extern
   without defining GL_VERSION_1_0. */

#ifndef COIN_SOGL_ES_TYPES_H
#define COIN_SOGL_ES_TYPES_H

#ifndef GL_DOUBLE
typedef double GLdouble;
#define GL_DOUBLE 0x140A
#endif /* ! GL_DOUBLE */

#ifndef GL_POINT
#define GL_POINT 0x1B00
#endif
#ifndef GL_LINE
#define GL_LINE 0x1B01
#endif
#ifndef GL_FILL
#define GL_FILL 0x1B02
#endif

#ifndef GL_LINE_WIDTH_RANGE
#ifdef GL_ALIASED_LINE_WIDTH_RANGE
#define GL_LINE_WIDTH_RANGE GL_ALIASED_LINE_WIDTH_RANGE
#else
#define GL_LINE_WIDTH_RANGE 0x0B22
#endif
#endif
#ifndef GL_POINT_SIZE_RANGE
#ifdef GL_ALIASED_POINT_SIZE_RANGE
#define GL_POINT_SIZE_RANGE GL_ALIASED_POINT_SIZE_RANGE
#else
#define GL_POINT_SIZE_RANGE 0x0B12
#endif
#endif
#ifndef GL_LINE_WIDTH_GRANULARITY
#define GL_LINE_WIDTH_GRANULARITY 0x0B23
#endif
#ifndef GL_POINT_SIZE_GRANULARITY
#define GL_POINT_SIZE_GRANULARITY 0x0B13
#endif

#ifndef GL_UNPACK_SWAP_BYTES
#define GL_UNPACK_SWAP_BYTES 0x0CF0
#endif
#ifndef GL_UNPACK_LSB_FIRST
#define GL_UNPACK_LSB_FIRST 0x0CF1
#endif
#ifndef GL_UNPACK_ROW_LENGTH
#define GL_UNPACK_ROW_LENGTH 0x0CF2
#endif
#ifndef GL_UNPACK_SKIP_ROWS
#define GL_UNPACK_SKIP_ROWS 0x0CF3
#endif
#ifndef GL_UNPACK_SKIP_PIXELS
#define GL_UNPACK_SKIP_PIXELS 0x0CF4
#endif
#ifndef GL_PACK_SWAP_BYTES
#define GL_PACK_SWAP_BYTES 0x0D00
#endif
#ifndef GL_PACK_LSB_FIRST
#define GL_PACK_LSB_FIRST 0x0D01
#endif
#ifndef GL_PACK_ROW_LENGTH
#define GL_PACK_ROW_LENGTH 0x0D02
#endif
#ifndef GL_PACK_SKIP_ROWS
#define GL_PACK_SKIP_ROWS 0x0D03
#endif
#ifndef GL_PACK_SKIP_PIXELS
#define GL_PACK_SKIP_PIXELS 0x0D04
#endif

#ifndef GL_RED
#define GL_RED 0x1903
#endif
#ifndef GL_GREEN
#define GL_GREEN 0x1904
#endif
#ifndef GL_BLUE
#define GL_BLUE 0x1905
#endif
#ifndef GL_STENCIL_INDEX
#define GL_STENCIL_INDEX 0x1901
#endif

#ifndef GL_TEXTURE_WIDTH
#define GL_TEXTURE_WIDTH 0x1000
#endif
#ifndef GL_TEXTURE_HEIGHT
#define GL_TEXTURE_HEIGHT 0x1001
#endif
#ifndef GL_TEXTURE_INTERNAL_FORMAT
#define GL_TEXTURE_INTERNAL_FORMAT 0x1003
#endif

#ifndef GL_FRONT_LEFT
#define GL_FRONT_LEFT 0x0400
#endif
#ifndef GL_FRONT_RIGHT
#define GL_FRONT_RIGHT 0x0401
#endif
#ifndef GL_BACK_LEFT
#define GL_BACK_LEFT 0x0402
#endif
#ifndef GL_BACK_RIGHT
#define GL_BACK_RIGHT 0x0403
#endif

#ifndef GL_POLYGON_OFFSET_POINT
#define GL_POLYGON_OFFSET_POINT 0x2A01
#endif
#ifndef GL_POLYGON_OFFSET_LINE
#define GL_POLYGON_OFFSET_LINE 0x2A02
#endif

#ifndef GL_VERTEX_ARRAY_BINDING
#define GL_VERTEX_ARRAY_BINDING 0x85B5
#endif
#ifndef GL_CONTEXT_PROFILE_MASK
#define GL_CONTEXT_PROFILE_MASK 0x9126
#endif
#ifndef GL_CONTEXT_CORE_PROFILE_BIT
#define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#endif

#if defined(GL_ES_VERSION_2_0) || defined(GL_VERSION_1_0) || defined(__gl_h_) || defined(__gl2_h_) || defined(__gl3_h_)
#ifdef __cplusplus
extern "C" {
#endif

static inline void
cc_gl_depth_range(double n, double f)
{
#if defined(GL_ES_VERSION_2_0)
  glDepthRangef((float)n, (float)f);
#else
  glDepthRange(n, f);
#endif
}

static inline void
cc_gl_polygon_mode(GLenum face, GLenum mode)
{
#if defined(GL_ES_VERSION_2_0)
  (void)face;
  (void)mode;
#else
  glPolygonMode(face, mode);
#endif
}

static inline void
cc_gl_point_size(float size)
{
#if defined(GL_ES_VERSION_2_0)
  (void)size;
#else
  glPointSize(size);
#endif
}

#ifdef __cplusplus
}
#endif
#endif /* GL types available */

#endif /* ! COIN_SOGL_ES_TYPES_H */
