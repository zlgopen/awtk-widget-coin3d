/* Fixed-function APIs removed in OpenGL Core / ES.
   Provided as no-ops / stubs so legacy Coin call sites compile.
   Real drawing must go through SoGLDevice / VAO. */

#ifndef COIN_SOGL_FF_STUBS_H
#define COIN_SOGL_FF_STUBS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Linux <GL/gl.h> already declares fixed-function entry points as extern.
   Redeclaring them static inline is a C++ linkage error. Core / ES / Apple
   gl3.h do not define these tokens; detect the compat header first. */
#if defined(GL_LIGHTING) || defined(GL_MODELVIEW)
#define COIN_SYSTEM_GL_HAS_FF_DECLS 1
#endif

/* glDrawBuffer / glReadBuffer / glGetTexLevelParameter* remain in desktop
   Core (Apple OpenGL/gl3.h, GL 3.x). Those headers omit GL_LIGHTING, so the
   FF-stub block still applies — but these four must not become static.
   GLES3/gl3.h declares glReadBuffer as extern; GLES2 does not. Desktop-only
   glDrawBuffer / glGetTexLevelParameter* stay stubs on ES. */
#if defined(GL_VERSION_3_0) && !defined(GL_ES_VERSION_2_0) && !defined(GL_ES_VERSION_3_0)
#define COIN_SYSTEM_GL_HAS_CORE_BUFFER_DECLS 1
#define COIN_SYSTEM_GL_HAS_READ_BUFFER_DECL 1
#endif
#if defined(GL_ES_VERSION_3_0)
#define COIN_SYSTEM_GL_HAS_READ_BUFFER_DECL 1
#endif

#ifndef GL_QUADS
#define GL_QUADS 0x0007
#endif
#ifndef GL_POLYGON
#define GL_POLYGON 0x0009
#endif
#ifndef GL_QUAD_STRIP
#define GL_QUAD_STRIP 0x0008
#endif
#ifndef GL_LIGHTING
#define GL_LIGHTING 0x0B50
#endif
#ifndef GL_LIGHT0
#define GL_LIGHT0 0x4000
#endif
#ifndef GL_AMBIENT
#define GL_AMBIENT 0x1200
#endif
#ifndef GL_DIFFUSE
#define GL_DIFFUSE 0x1201
#endif
#ifndef GL_SPECULAR
#define GL_SPECULAR 0x1202
#endif
#ifndef GL_EMISSION
#define GL_EMISSION 0x1600
#endif
#ifndef GL_SHININESS
#define GL_SHININESS 0x1601
#endif
#ifndef GL_POSITION
#define GL_POSITION 0x1203
#endif
#ifndef GL_SPOT_DIRECTION
#define GL_SPOT_DIRECTION 0x1204
#endif
#ifndef GL_SPOT_EXPONENT
#define GL_SPOT_EXPONENT 0x1205
#endif
#ifndef GL_SPOT_CUTOFF
#define GL_SPOT_CUTOFF 0x1206
#endif
#ifndef GL_CONSTANT_ATTENUATION
#define GL_CONSTANT_ATTENUATION 0x1207
#endif
#ifndef GL_LINEAR_ATTENUATION
#define GL_LINEAR_ATTENUATION 0x1208
#endif
#ifndef GL_QUADRATIC_ATTENUATION
#define GL_QUADRATIC_ATTENUATION 0x1209
#endif
#ifndef GL_MODELVIEW
#define GL_MODELVIEW 0x1700
#endif
#ifndef GL_PROJECTION
#define GL_PROJECTION 0x1701
#endif
#ifndef GL_COLOR_MATERIAL
#define GL_COLOR_MATERIAL 0x0B57
#endif
#ifndef GL_NORMALIZE
#define GL_NORMALIZE 0x0BA1
#endif
#ifndef GL_FLAT
#define GL_FLAT 0x1D00
#endif
#ifndef GL_SMOOTH
#define GL_SMOOTH 0x1D01
#endif
#ifndef GL_LIGHT_MODEL_TWO_SIDE
#define GL_LIGHT_MODEL_TWO_SIDE 0x0B52
#endif
#ifndef GL_ALPHA_TEST
#define GL_ALPHA_TEST 0x0BC0
#endif
#ifndef GL_POLYGON_STIPPLE
#define GL_POLYGON_STIPPLE 0x0B25
#endif
#ifndef GL_VERTEX_ARRAY
#define GL_VERTEX_ARRAY 0x8074
#endif
#ifndef GL_NORMAL_ARRAY
#define GL_NORMAL_ARRAY 0x8075
#endif
#ifndef GL_COLOR_ARRAY
#define GL_COLOR_ARRAY 0x8076
#endif
#ifndef GL_TEXTURE_COORD_ARRAY
#define GL_TEXTURE_COORD_ARRAY 0x8078
#endif
#ifndef GL_COMPILE
#define GL_COMPILE 0x1300
#endif
#ifndef GL_COMPILE_AND_EXECUTE
#define GL_COMPILE_AND_EXECUTE 0x1301
#endif
#ifndef GL_ALL_ATTRIB_BITS
#define GL_ALL_ATTRIB_BITS 0xFFFFFFFF
#endif
#ifndef GL_CURRENT_BIT
#define GL_CURRENT_BIT 0x00000001
#endif
#ifndef GL_ENABLE_BIT
#define GL_ENABLE_BIT 0x00002000
#endif
#ifndef GL_FOG
#define GL_FOG 0x0B60
#endif
#ifndef GL_TEXTURE_ENV
#define GL_TEXTURE_ENV 0x2300
#endif
#ifndef GL_TEXTURE_ENV_MODE
#define GL_TEXTURE_ENV_MODE 0x2200
#endif
#ifndef GL_MODULATE
#define GL_MODULATE 0x2100
#endif
#ifndef GL_DECAL
#define GL_DECAL 0x2101
#endif
#ifndef GL_BLEND
#define GL_BLEND 0x0BE2
#endif
#ifndef GL_CLAMP
#define GL_CLAMP 0x2900
#endif
#ifndef GL_STACK_OVERFLOW
#define GL_STACK_OVERFLOW 0x0503
#endif
#ifndef GL_POINT_SMOOTH
#define GL_POINT_SMOOTH 0x0B10
#endif
#ifndef GL_LINE_SMOOTH
#define GL_LINE_SMOOTH 0x0B20
#endif
#ifndef GL_FRONT_AND_BACK
#define GL_FRONT_AND_BACK 0x0408
#endif
#ifndef GL_TEXTURE_ENV_COLOR
#define GL_TEXTURE_ENV_COLOR 0x2201
#endif
#ifndef GL_ALPHA_SCALE
#define GL_ALPHA_SCALE 0x0D1C
#endif
#ifndef GL_COMBINE
#define GL_COMBINE 0x8570
#endif
#ifndef GL_COMBINE_RGB
#define GL_COMBINE_RGB 0x8571
#endif
#ifndef GL_COMBINE_ALPHA
#define GL_COMBINE_ALPHA 0x8572
#endif
#ifndef GL_SOURCE0_RGB
#define GL_SOURCE0_RGB 0x8580
#endif
#ifndef GL_SOURCE1_RGB
#define GL_SOURCE1_RGB 0x8581
#endif
#ifndef GL_SOURCE2_RGB
#define GL_SOURCE2_RGB 0x8582
#endif
#ifndef GL_SOURCE0_ALPHA
#define GL_SOURCE0_ALPHA 0x8588
#endif
#ifndef GL_SOURCE1_ALPHA
#define GL_SOURCE1_ALPHA 0x8589
#endif
#ifndef GL_SOURCE2_ALPHA
#define GL_SOURCE2_ALPHA 0x858A
#endif
#ifndef GL_OPERAND0_RGB
#define GL_OPERAND0_RGB 0x8590
#endif
#ifndef GL_OPERAND1_RGB
#define GL_OPERAND1_RGB 0x8591
#endif
#ifndef GL_OPERAND2_RGB
#define GL_OPERAND2_RGB 0x8592
#endif
#ifndef GL_OPERAND0_ALPHA
#define GL_OPERAND0_ALPHA 0x8598
#endif
#ifndef GL_OPERAND1_ALPHA
#define GL_OPERAND1_ALPHA 0x8599
#endif
#ifndef GL_OPERAND2_ALPHA
#define GL_OPERAND2_ALPHA 0x859A
#endif
#ifndef GL_RGB_SCALE
#define GL_RGB_SCALE 0x8573
#endif
#ifndef GL_ADD_SIGNED
#define GL_ADD_SIGNED 0x8574
#endif
#ifndef GL_INTERPOLATE
#define GL_INTERPOLATE 0x8575
#endif
#ifndef GL_SUBTRACT
#define GL_SUBTRACT 0x84E7
#endif
#ifndef GL_CONSTANT
#define GL_CONSTANT 0x8576
#endif
#ifndef GL_PRIMARY_COLOR
#define GL_PRIMARY_COLOR 0x8577
#endif
#ifndef GL_PREVIOUS
#define GL_PREVIOUS 0x8578
#endif
#ifndef GL_DOT3_RGB
#define GL_DOT3_RGB 0x86AE
#endif
#ifndef GL_DOT3_RGBA
#define GL_DOT3_RGBA 0x86AF
#endif
#ifndef GL_TEXTURE_GEN_S
#define GL_TEXTURE_GEN_S 0x0C60
#endif
#ifndef GL_TEXTURE_GEN_T
#define GL_TEXTURE_GEN_T 0x0C61
#endif
#ifndef GL_TEXTURE_GEN_R
#define GL_TEXTURE_GEN_R 0x0C62
#endif
#ifndef GL_TEXTURE_GEN_Q
#define GL_TEXTURE_GEN_Q 0x0C63
#endif
#ifndef GL_TEXTURE_GEN_MODE
#define GL_TEXTURE_GEN_MODE 0x2500
#endif
#ifndef GL_OBJECT_LINEAR
#define GL_OBJECT_LINEAR 0x2401
#endif
#ifndef GL_EYE_LINEAR
#define GL_EYE_LINEAR 0x2400
#endif
#ifndef GL_SPHERE_MAP
#define GL_SPHERE_MAP 0x2402
#endif
#ifndef GL_OBJECT_PLANE
#define GL_OBJECT_PLANE 0x2501
#endif
#ifndef GL_EYE_PLANE
#define GL_EYE_PLANE 0x2502
#endif
#ifndef GL_S
#define GL_S 0x2000
#endif
#ifndef GL_T
#define GL_T 0x2001
#endif
#ifndef GL_R
#define GL_R 0x2002
#endif
#ifndef GL_Q
#define GL_Q 0x2003
#endif
#ifndef GL_CLIP_PLANE0
#define GL_CLIP_PLANE0 0x3000
#endif
#ifndef GL_FOG_MODE
#define GL_FOG_MODE 0x0B65
#endif
#ifndef GL_FOG_DENSITY
#define GL_FOG_DENSITY 0x0B62
#endif
#ifndef GL_FOG_COLOR
#define GL_FOG_COLOR 0x0B66
#endif
#ifndef GL_EXP
#define GL_EXP 0x0800
#endif
#ifndef GL_EXP2
#define GL_EXP2 0x0801
#endif
#ifndef GL_CLIENT_ALL_ATTRIB_BITS
#define GL_CLIENT_ALL_ATTRIB_BITS 0xFFFFFFFF
#endif
#ifndef GL_LIST_BIT
#define GL_LIST_BIT 0x00020000
#endif
#ifndef GL_TEXTURE_BIT
#define GL_TEXTURE_BIT 0x00040000
#endif
#ifndef GL_TRANSFORM_BIT
#define GL_TRANSFORM_BIT 0x00001000
#endif
#ifndef GL_MAX_CLIP_PLANES
#define GL_MAX_CLIP_PLANES 0x0D32
#endif
#ifndef GL_ACCUM_RED_BITS
#define GL_ACCUM_RED_BITS 0x0D58
#endif
#ifndef GL_ACCUM_GREEN_BITS
#define GL_ACCUM_GREEN_BITS 0x0D59
#endif
#ifndef GL_ACCUM_BLUE_BITS
#define GL_ACCUM_BLUE_BITS 0x0D5A
#endif
#ifndef GL_ACCUM_ALPHA_BITS
#define GL_ACCUM_ALPHA_BITS 0x0D5B
#endif
#ifndef GL_ACCUM
#define GL_ACCUM 0x0100
#endif
#ifndef GL_LOAD
#define GL_LOAD 0x0101
#endif
#ifndef GL_RETURN
#define GL_RETURN 0x0102
#endif
#ifndef GL_MULT
#define GL_MULT 0x0103
#endif
#ifndef GL_ADD
#define GL_ADD 0x0104
#endif
#ifndef GL_DEPTH_BITS
#define GL_DEPTH_BITS 0x0D56
#endif
#ifndef GL_ALPHA_BITS
#define GL_ALPHA_BITS 0x0D55
#endif
#ifndef GL_STENCIL_BITS
#define GL_STENCIL_BITS 0x0D57
#endif
#ifndef GL_RED_BITS
#define GL_RED_BITS 0x0D52
#endif
#ifndef GL_GREEN_BITS
#define GL_GREEN_BITS 0x0D53
#endif
#ifndef GL_BLUE_BITS
#define GL_BLUE_BITS 0x0D54
#endif
#ifndef GL_FEEDBACK
#define GL_FEEDBACK 0x1C01
#endif
#ifndef GL_SELECT
#define GL_SELECT 0x1C02
#endif
#ifndef GL_RENDER
#define GL_RENDER 0x1C00
#endif
#ifndef GL_3D_COLOR
#define GL_3D_COLOR 0x0602
#endif
#ifndef GL_PASS_THROUGH_TOKEN
#define GL_PASS_THROUGH_TOKEN 0x0700
#endif
#ifndef GL_POLYGON_TOKEN
#define GL_POLYGON_TOKEN 0x0703
#endif
#ifndef GL_BITMAP_TOKEN
#define GL_BITMAP_TOKEN 0x0704
#endif
#ifndef GL_LINE_TOKEN
#define GL_LINE_TOKEN 0x0702
#endif
#ifndef GL_LINE_RESET_TOKEN
#define GL_LINE_RESET_TOKEN 0x0707
#endif
#ifndef GL_POINT_TOKEN
#define GL_POINT_TOKEN 0x0701
#endif
#ifndef GL_DRAW_PIXEL_TOKEN
#define GL_DRAW_PIXEL_TOKEN 0x0705
#endif
#ifndef GL_COPY_PIXEL_TOKEN
#define GL_COPY_PIXEL_TOKEN 0x0706
#endif
#ifndef GL_LOGIC_OP
#define GL_LOGIC_OP 0x0BF1
#endif
#ifndef GL_INDEX_LOGIC_OP
#define GL_INDEX_LOGIC_OP 0x0BF1
#endif
#ifndef GL_COLOR_LOGIC_OP
#define GL_COLOR_LOGIC_OP 0x0BF2
#endif
#ifndef GL_FEEDBACK_BUFFER_POINTER
#define GL_FEEDBACK_BUFFER_POINTER 0x0DF0
#endif
#ifndef GL_FEEDBACK_BUFFER_SIZE
#define GL_FEEDBACK_BUFFER_SIZE 0x0DF1
#endif
#ifndef GL_FEEDBACK_BUFFER_TYPE
#define GL_FEEDBACK_BUFFER_TYPE 0x0DF2
#endif
#ifndef GL_SELECTION_BUFFER_POINTER
#define GL_SELECTION_BUFFER_POINTER 0x0DF3
#endif
#ifndef GL_SELECTION_BUFFER_SIZE
#define GL_SELECTION_BUFFER_SIZE 0x0DF4
#endif
#ifndef GL_TEXTURE_LUMINANCE_SIZE
#define GL_TEXTURE_LUMINANCE_SIZE 0x8060
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
#ifndef GL_LUMINANCE
#define GL_LUMINANCE 0x1909
#endif
#ifndef GL_LUMINANCE_ALPHA
#define GL_LUMINANCE_ALPHA 0x190A
#endif
#ifndef GL_INTENSITY
#define GL_INTENSITY 0x8049
#endif
#ifndef GL_PROXY_TEXTURE_1D
#define GL_PROXY_TEXTURE_1D 0x8063
#endif
#ifndef GL_PROXY_TEXTURE_2D
#define GL_PROXY_TEXTURE_2D 0x8064
#endif
#ifndef GL_TEXTURE_PRIORITY
#define GL_TEXTURE_PRIORITY 0x8066
#endif
#ifndef GL_TEXTURE_RESIDENT
#define GL_TEXTURE_RESIDENT 0x8067
#endif
#ifndef GL_TEXTURE_BINDING_1D
#define GL_TEXTURE_BINDING_1D 0x8068
#endif
#ifndef GL_TEXTURE_BINDING_2D
#define GL_TEXTURE_BINDING_2D 0x8069
#endif
#ifndef GL_CLIENT_PIXEL_STORE_BIT
#define GL_CLIENT_PIXEL_STORE_BIT 0x00000001
#endif
#ifndef GL_CLIENT_VERTEX_ARRAY_BIT
#define GL_CLIENT_VERTEX_ARRAY_BIT 0x00000002
#endif
#ifndef GL_EDGE_FLAG_ARRAY
#define GL_EDGE_FLAG_ARRAY 0x8079
#endif
#ifndef GL_INDEX_ARRAY
#define GL_INDEX_ARRAY 0x8077
#endif
#ifndef GL_V2F
#define GL_V2F 0x2A20
#endif
#ifndef GL_V3F
#define GL_V3F 0x2A21
#endif
#ifndef GL_C4UB_V2F
#define GL_C4UB_V2F 0x2A22
#endif
#ifndef GL_C4UB_V3F
#define GL_C4UB_V3F 0x2A23
#endif
#ifndef GL_C3F_V3F
#define GL_C3F_V3F 0x2A24
#endif
#ifndef GL_N3F_V3F
#define GL_N3F_V3F 0x2A25
#endif
#ifndef GL_C4F_N3F_V3F
#define GL_C4F_N3F_V3F 0x2A26
#endif
#ifndef GL_T2F_V3F
#define GL_T2F_V3F 0x2A27
#endif
#ifndef GL_T4F_V4F
#define GL_T4F_V4F 0x2A28
#endif
#ifndef GL_T2F_C4UB_V3F
#define GL_T2F_C4UB_V3F 0x2A29
#endif
#ifndef GL_T2F_C3F_V3F
#define GL_T2F_C3F_V3F 0x2A2A
#endif
#ifndef GL_T2F_N3F_V3F
#define GL_T2F_N3F_V3F 0x2A2B
#endif
#ifndef GL_T2F_C4F_N3F_V3F
#define GL_T2F_C4F_N3F_V3F 0x2A2C
#endif
#ifndef GL_T4F_C4F_N3F_V4F
#define GL_T4F_C4F_N3F_V4F 0x2A2D
#endif


#ifndef GL_MAX_LIGHTS
#define GL_MAX_LIGHTS 0x0D31
#endif
#ifndef GL_LUMINANCE8
#define GL_LUMINANCE8 0x8040
#endif
#ifndef GL_LUMINANCE8_ALPHA8
#define GL_LUMINANCE8_ALPHA8 0x8045
#endif
#ifndef GL_STACK_UNDERFLOW
#define GL_STACK_UNDERFLOW 0x0504
#endif
#ifndef GL_LIGHT_MODEL_AMBIENT
#define GL_LIGHT_MODEL_AMBIENT 0x0B53
#endif
#ifndef GL_FOG_START
#define GL_FOG_START 0x0B63
#endif
#ifndef GL_FOG_END
#define GL_FOG_END 0x0B64
#endif
#ifndef GL_MAP1_VERTEX_3
#define GL_MAP1_VERTEX_3 0x0D97
#endif
#ifndef GL_MAP1_VERTEX_4
#define GL_MAP1_VERTEX_4 0x0D98
#endif
#ifndef GL_MAP1_COLOR_4
#define GL_MAP1_COLOR_4 0x0D90
#endif
#ifndef GL_MAP1_NORMAL
#define GL_MAP1_NORMAL 0x0D92
#endif
#ifndef GL_MAP1_TEXTURE_COORD_1
#define GL_MAP1_TEXTURE_COORD_1 0x0D93
#endif
#ifndef GL_MAP1_TEXTURE_COORD_2
#define GL_MAP1_TEXTURE_COORD_2 0x0D94
#endif
#ifndef GL_MAP1_TEXTURE_COORD_3
#define GL_MAP1_TEXTURE_COORD_3 0x0D95
#endif
#ifndef GL_MAP1_TEXTURE_COORD_4
#define GL_MAP1_TEXTURE_COORD_4 0x0D96
#endif
#ifndef GL_MAP2_VERTEX_3
#define GL_MAP2_VERTEX_3 0x0DB7
#endif
#ifndef GL_MAP2_VERTEX_4
#define GL_MAP2_VERTEX_4 0x0DB8
#endif
#ifndef GL_MAP2_COLOR_4
#define GL_MAP2_COLOR_4 0x0DB0
#endif
#ifndef GL_MAP2_NORMAL
#define GL_MAP2_NORMAL 0x0DB2
#endif
#ifndef GL_MAP2_TEXTURE_COORD_1
#define GL_MAP2_TEXTURE_COORD_1 0x0DB3
#endif
#ifndef GL_MAP2_TEXTURE_COORD_2
#define GL_MAP2_TEXTURE_COORD_2 0x0DB4
#endif
#ifndef GL_MAP2_TEXTURE_COORD_3
#define GL_MAP2_TEXTURE_COORD_3 0x0DB5
#endif
#ifndef GL_MAP2_TEXTURE_COORD_4
#define GL_MAP2_TEXTURE_COORD_4 0x0DB6
#endif
#ifndef GL_AUTO_NORMAL
#define GL_AUTO_NORMAL 0x0D80
#endif
#ifndef GL_MAP1_GRID_DOMAIN
#define GL_MAP1_GRID_DOMAIN 0x0DD0
#endif
#ifndef GL_MAP1_GRID_SEGMENTS
#define GL_MAP1_GRID_SEGMENTS 0x0DD1
#endif
#ifndef GL_MAP2_GRID_DOMAIN
#define GL_MAP2_GRID_DOMAIN 0x0DD2
#endif
#ifndef GL_MAP2_GRID_SEGMENTS
#define GL_MAP2_GRID_SEGMENTS 0x0DD3
#endif
#ifndef GL_COEFF
#define GL_COEFF 0x0A00
#endif
#ifndef GL_ORDER
#define GL_ORDER 0x0A01
#endif
#ifndef GL_DOMAIN
#define GL_DOMAIN 0x0A02
#endif


#ifndef GL_BITMAP
#define GL_BITMAP 0x1A00
#endif
#ifndef GL_CURRENT_COLOR
#define GL_CURRENT_COLOR 0x0B00
#endif
#ifndef GL_FOG_BIT
#define GL_FOG_BIT 0x00000080
#endif
#ifndef GL_LIGHTING_BIT
#define GL_LIGHTING_BIT 0x00000040
#endif
#ifndef GL_LINE_BIT
#define GL_LINE_BIT 0x00000004
#endif
#ifndef GL_LINE_STIPPLE
#define GL_LINE_STIPPLE 0x0B24
#endif
#ifndef GL_RGBA_MODE
#define GL_RGBA_MODE 0x0C31
#endif
#ifndef GL_ZOOM_X
#define GL_ZOOM_X 0x0D16
#endif
#ifndef GL_ZOOM_Y
#define GL_ZOOM_Y 0x0D17
#endif
#ifndef GL_POLYGON_BIT
#define GL_POLYGON_BIT 0x00000008
#endif
#ifndef GL_POINT_BIT
#define GL_POINT_BIT 0x00000002
#endif
#ifndef GL_SCISSOR_BIT
#define GL_SCISSOR_BIT 0x00080000
#endif
#ifndef GL_HINT_BIT
#define GL_HINT_BIT 0x00008000
#endif
#ifndef GL_EVAL_BIT
#define GL_EVAL_BIT 0x00010000
#endif
#ifndef GL_PIXEL_MODE_BIT
#define GL_PIXEL_MODE_BIT 0x00000020
#endif
#ifndef GL_COLOR_BUFFER_BIT
#define GL_COLOR_BUFFER_BIT 0x00004000
#endif

#ifndef GL_MAP_COLOR
#define GL_MAP_COLOR 0x0D10
#endif
#ifndef GL_MAP_STENCIL
#define GL_MAP_STENCIL 0x0D11
#endif
#ifndef GL_INDEX_SHIFT
#define GL_INDEX_SHIFT 0x0D12
#endif
#ifndef GL_INDEX_OFFSET
#define GL_INDEX_OFFSET 0x0D13
#endif
#ifndef GL_RED_SCALE
#define GL_RED_SCALE 0x0D14
#endif
#ifndef GL_RED_BIAS
#define GL_RED_BIAS 0x0D15
#endif
#ifndef GL_GREEN_SCALE
#define GL_GREEN_SCALE 0x0D18
#endif
#ifndef GL_GREEN_BIAS
#define GL_GREEN_BIAS 0x0D19
#endif
#ifndef GL_BLUE_SCALE
#define GL_BLUE_SCALE 0x0D1A
#endif
#ifndef GL_BLUE_BIAS
#define GL_BLUE_BIAS 0x0D1B
#endif
#ifndef GL_ALPHA_BIAS
#define GL_ALPHA_BIAS 0x0D1D
#endif
#ifndef GL_DEPTH_SCALE
#define GL_DEPTH_SCALE 0x0D1E
#endif
#ifndef GL_DEPTH_BIAS
#define GL_DEPTH_BIAS 0x0D1F
#endif
#ifndef GL_PIXEL_MAP_I_TO_I
#define GL_PIXEL_MAP_I_TO_I 0x0C70
#endif
#ifndef GL_PIXEL_MAP_S_TO_S
#define GL_PIXEL_MAP_S_TO_S 0x0C71
#endif
#ifndef GL_PIXEL_MAP_I_TO_R
#define GL_PIXEL_MAP_I_TO_R 0x0C72
#endif
#ifndef GL_PIXEL_MAP_I_TO_G
#define GL_PIXEL_MAP_I_TO_G 0x0C73
#endif
#ifndef GL_PIXEL_MAP_I_TO_B
#define GL_PIXEL_MAP_I_TO_B 0x0C74
#endif
#ifndef GL_PIXEL_MAP_I_TO_A
#define GL_PIXEL_MAP_I_TO_A 0x0C75
#endif
#ifndef GL_PIXEL_MAP_R_TO_R
#define GL_PIXEL_MAP_R_TO_R 0x0C76
#endif
#ifndef GL_PIXEL_MAP_G_TO_G
#define GL_PIXEL_MAP_G_TO_G 0x0C77
#endif
#ifndef GL_PIXEL_MAP_B_TO_B
#define GL_PIXEL_MAP_B_TO_B 0x0C78
#endif
#ifndef GL_PIXEL_MAP_A_TO_A
#define GL_PIXEL_MAP_A_TO_A 0x0C79
#endif
#ifndef GL_PIXEL_MAP_I_TO_I_SIZE
#define GL_PIXEL_MAP_I_TO_I_SIZE 0x0CB0
#endif
#ifndef GL_PIXEL_MAP_S_TO_S_SIZE
#define GL_PIXEL_MAP_S_TO_S_SIZE 0x0CB1
#endif
#ifndef GL_PIXEL_MAP_I_TO_R_SIZE
#define GL_PIXEL_MAP_I_TO_R_SIZE 0x0CB2
#endif
#ifndef GL_PIXEL_MAP_I_TO_G_SIZE
#define GL_PIXEL_MAP_I_TO_G_SIZE 0x0CB3
#endif
#ifndef GL_PIXEL_MAP_I_TO_B_SIZE
#define GL_PIXEL_MAP_I_TO_B_SIZE 0x0CB4
#endif
#ifndef GL_PIXEL_MAP_I_TO_A_SIZE
#define GL_PIXEL_MAP_I_TO_A_SIZE 0x0CB5
#endif
#if !defined(COIN_SYSTEM_GL_HAS_FF_DECLS) || defined(COIN_USE_GLAD)
/* Drop glad compatibility macros so Core no-op stubs apply. Do not use
   function-like #define glXxx — Coin glue calls w->glXxx members. */
#undef glBegin
#undef glEnd
#undef glVertex2f
#undef glVertex2s
#undef glVertex2i
#undef glClearIndex
#undef glIndexi
#undef glIndexf
#undef glLineStipple
#undef glPixelZoom
#undef glPixelMapfv
#undef glPixelMapuiv
#undef glPixelMapusv
#undef glGetPixelMapfv
#undef glPixelTransferf
#undef glPixelTransferi
#undef glMap1f
#undef glMap2f
#undef glMapGrid1f
#undef glMapGrid2f
#undef glEvalMesh1
#undef glEvalMesh2
#undef glEvalCoord1f
#undef glEvalCoord2f
#undef glEvalPoint1
#undef glEvalPoint2
#undef glVertex2fv
#undef glVertex3f
#undef glVertex3fv
#undef glVertex4f
#undef glVertex4fv
#undef glNormal3f
#undef glNormal3fv
#undef glColor3f
#undef glColor3fv
#undef glColor3ub
#undef glColor3ubv
#undef glColor4f
#undef glColor4fv
#undef glColor4ub
#undef glColor4ubv
#undef glTexCoord2f
#undef glTexCoord2fv
#undef glTexCoord3f
#undef glTexCoord3fv
#undef glTexCoord4f
#undef glTexCoord4fv
#undef glMaterialf
#undef glMaterialfv
#undef glLightf
#undef glLightfv
#undef glLightModelf
#undef glLightModelfv
#undef glLightModeli
#undef glColorMaterial
#undef glShadeModel
#undef glAlphaFunc
#undef glPolygonStipple
#undef glMatrixMode
#undef glLoadIdentity
#undef glLoadMatrixf
#undef glLoadMatrixd
#undef glMultMatrixf
#undef glMultMatrixd
#undef glPushMatrix
#undef glPopMatrix
#undef glTranslatef
#undef glTranslated
#undef glRotatef
#undef glRotated
#undef glScalef
#undef glScaled
#undef glOrtho
#undef glFrustum
#undef glPushAttrib
#undef glPopAttrib
#undef glPushClientAttrib
#undef glPopClientAttrib
#undef glEnableClientState
#undef glDisableClientState
#undef glVertexPointer
#undef glNormalPointer
#undef glColorPointer
#undef glTexCoordPointer
#undef glEdgeFlagPointer
#undef glIndexPointer
#undef glInterleavedArrays
#undef glArrayElement
#undef glGenLists
#undef glNewList
#undef glEndList
#undef glCallList
#undef glCallLists
#undef glDeleteLists
#undef glListBase
#undef glTexEnvf
#undef glTexEnvi
#undef glTexEnvfv
#undef glTexGenf
#undef glTexGeni
#undef glTexGenfv
#undef glTexGendv
#undef glFogi
#undef glFogf
#undef glFogfv
#undef glClipPlane
#undef glRasterPos2f
#undef glRasterPos2i
#undef glRasterPos3f
#undef glBitmap
#undef glDrawPixels
#undef glCopyPixels
#undef glRectf
#undef glRecti
#undef glGetClipPlane
#undef glGetLightfv
#undef glGetMaterialfv
#undef glGetTexEnviv
#undef glGetTexEnvfv
#undef glAccum
#undef glClearAccum
#undef glPassThrough
#undef glSelectBuffer
#undef glFeedbackBuffer
#undef glRenderMode
#undef glInitNames
#undef glLoadName
#undef glPushName
#undef glPopName
#undef glPrioritizeTextures
#undef glAreTexturesResident
#if !defined(COIN_SYSTEM_GL_HAS_CORE_BUFFER_DECLS)
#undef glDrawBuffer
#undef glGetTexLevelParameteriv
#undef glGetTexLevelParameterfv
#endif
#if !defined(COIN_SYSTEM_GL_HAS_READ_BUFFER_DECL)
#undef glReadBuffer
#endif

static inline void glBegin(GLenum) {}
static inline void glEnd(void) {}
static inline void glVertex2f(float, float) {}

static inline void glVertex2s(short, short) {}
static inline void glVertex2i(int, int) {}
static inline void glClearIndex(float) {}
static inline void glIndexi(int) {}
static inline void glIndexf(float) {}
static inline void glLineStipple(int, unsigned short) {}
static inline void glPixelZoom(float, float) {}
static inline void glPixelMapfv(GLenum, int, const float *) {}
static inline void glPixelMapuiv(GLenum, int, const unsigned int *) {}
static inline void glPixelMapusv(GLenum, int, const unsigned short *) {}
static inline void glGetPixelMapfv(GLenum, float *) {}

static inline void glPixelTransferf(GLenum, float) {}
static inline void glPixelTransferi(GLenum, int) {}
static inline void glMap1f(GLenum, float, float, int, int, const float *) {}
static inline void glMap2f(GLenum, float, float, int, int, float, float, int, int, const float *) {}
static inline void glMapGrid1f(int, float, float) {}
static inline void glMapGrid2f(int, float, float, int, float, float) {}
static inline void glEvalMesh1(GLenum, int, int) {}
static inline void glEvalMesh2(GLenum, int, int, int, int) {}
static inline void glEvalCoord1f(float) {}
static inline void glEvalCoord2f(float, float) {}
static inline void glEvalPoint1(int) {}
static inline void glEvalPoint2(int, int) {}

static inline void glVertex2fv(const float *) {}
static inline void glVertex3f(float, float, float) {}
static inline void glVertex3fv(const float *) {}
static inline void glVertex4f(float, float, float, float) {}
static inline void glVertex4fv(const float *) {}
static inline void glNormal3f(float, float, float) {}
static inline void glNormal3fv(const float *) {}
static inline void glColor3f(float, float, float) {}
static inline void glColor3fv(const float *) {}
static inline void glColor3ub(unsigned char, unsigned char, unsigned char) {}
static inline void glColor3ubv(const unsigned char *) {}
static inline void glColor4f(float, float, float, float) {}
static inline void glColor4fv(const float *) {}
static inline void glColor4ub(unsigned char, unsigned char, unsigned char, unsigned char) {}
static inline void glColor4ubv(const unsigned char *) {}
static inline void glTexCoord2f(float, float) {}
static inline void glTexCoord2fv(const float *) {}
static inline void glTexCoord3f(float, float, float) {}
static inline void glTexCoord3fv(const float *) {}
static inline void glTexCoord4f(float, float, float, float) {}
static inline void glTexCoord4fv(const float *) {}
static inline void glMaterialf(GLenum, GLenum, float) {}
static inline void glMaterialfv(GLenum, GLenum, const float *) {}
static inline void glLightf(GLenum, GLenum, float) {}
static inline void glLightfv(GLenum, GLenum, const float *) {}
static inline void glLightModelf(GLenum, float) {}
static inline void glLightModelfv(GLenum, const float *) {}
static inline void glLightModeli(GLenum, int) {}
static inline void glColorMaterial(GLenum, GLenum) {}
static inline void glShadeModel(GLenum) {}
static inline void glAlphaFunc(GLenum, float) {}
static inline void glPolygonStipple(const unsigned char *) {}
static inline void glMatrixMode(GLenum) {}
static inline void glLoadIdentity(void) {}
static inline void glLoadMatrixf(const float *) {}
static inline void glLoadMatrixd(const double *) {}
static inline void glMultMatrixf(const float *) {}
static inline void glMultMatrixd(const double *) {}
static inline void glPushMatrix(void) {}
static inline void glPopMatrix(void) {}
static inline void glTranslatef(float, float, float) {}
static inline void glTranslated(double, double, double) {}
static inline void glRotatef(float, float, float, float) {}
static inline void glRotated(double, double, double, double) {}
static inline void glScalef(float, float, float) {}
static inline void glScaled(double, double, double) {}
static inline void glOrtho(double, double, double, double, double, double) {}
static inline void glFrustum(double, double, double, double, double, double) {}
static inline void glPushAttrib(unsigned int) {}
static inline void glPopAttrib(void) {}
static inline void glPushClientAttrib(unsigned int) {}
static inline void glPopClientAttrib(void) {}
static inline void glEnableClientState(GLenum) {}
static inline void glDisableClientState(GLenum) {}
static inline void glVertexPointer(int, GLenum, int, const void *) {}
static inline void glNormalPointer(GLenum, int, const void *) {}
static inline void glColorPointer(int, GLenum, int, const void *) {}
static inline void glTexCoordPointer(int, GLenum, int, const void *) {}
static inline void glEdgeFlagPointer(int, const void *) {}
static inline void glIndexPointer(GLenum, int, const void *) {}
static inline void glInterleavedArrays(GLenum, int, const void *) {}
static inline void glArrayElement(int) {}
static inline unsigned int glGenLists(int) { return 0; }
static inline void glNewList(unsigned int, GLenum) {}
static inline void glEndList(void) {}
static inline void glCallList(unsigned int) {}
static inline void glCallLists(int, GLenum, const void *) {}
static inline void glDeleteLists(unsigned int, int) {}
static inline void glListBase(unsigned int) {}
static inline void glTexEnvf(GLenum, GLenum, float) {}
static inline void glTexEnvi(GLenum, GLenum, int) {}
static inline void glTexEnvfv(GLenum, GLenum, const float *) {}
static inline void glTexGenf(GLenum, GLenum, float) {}
static inline void glTexGeni(GLenum, GLenum, int) {}
static inline void glTexGenfv(GLenum, GLenum, const float *) {}
static inline void glTexGendv(GLenum, GLenum, const double *) {}
static inline void glFogi(GLenum, int) {}
static inline void glFogf(GLenum, float) {}
static inline void glFogfv(GLenum, const float *) {}
static inline void glClipPlane(GLenum, const double *) {}
static inline void glRasterPos2f(float, float) {}
static inline void glRasterPos2i(int, int) {}
static inline void glRasterPos3f(float, float, float) {}
static inline void glBitmap(int, int, float, float, float, float, const unsigned char *) {}
static inline void glDrawPixels(int, int, GLenum, GLenum, const void *) {}
static inline void glCopyPixels(int, int, int, int, GLenum) {}
static inline void glRectf(float, float, float, float) {}
static inline void glRecti(int, int, int, int) {}
static inline void glGetClipPlane(GLenum, double *) {}
static inline void glGetLightfv(GLenum, GLenum, float *) {}
static inline void glGetMaterialfv(GLenum, GLenum, float *) {}
static inline void glGetTexEnviv(GLenum, GLenum, int *) {}
static inline void glGetTexEnvfv(GLenum, GLenum, float *) {}
static inline void glAccum(GLenum, float) {}
static inline void glClearAccum(float, float, float, float) {}
static inline void glPassThrough(float) {}
static inline void glSelectBuffer(int, unsigned int *) {}
static inline void glFeedbackBuffer(int, GLenum, float *) {}
static inline int glRenderMode(GLenum) { return 0; }
static inline void glInitNames(void) {}
static inline void glLoadName(unsigned int) {}
static inline void glPushName(unsigned int) {}
static inline void glPopName(void) {}
static inline void glPrioritizeTextures(int, const unsigned int *, const float *) {}
static inline unsigned char glAreTexturesResident(int, const unsigned int *, unsigned char *) { return 0; }
#if !defined(COIN_SYSTEM_GL_HAS_CORE_BUFFER_DECLS)
static inline void glDrawBuffer(GLenum) {}
static inline void glGetTexLevelParameteriv(GLenum, int, GLenum, int * params)
{
  if (params) { *params = 0; }
}
static inline void glGetTexLevelParameterfv(GLenum, int, GLenum, float * params)
{
  if (params) { *params = 0.0f; }
}
#endif /* !COIN_SYSTEM_GL_HAS_CORE_BUFFER_DECLS */
#if !defined(COIN_SYSTEM_GL_HAS_READ_BUFFER_DECL)
static inline void glReadBuffer(GLenum) {}
#endif /* !COIN_SYSTEM_GL_HAS_READ_BUFFER_DECL */

#endif /* !COIN_SYSTEM_GL_HAS_FF_DECLS || COIN_USE_GLAD */

#ifdef __cplusplus
}
#endif

#endif /* !COIN_SOGL_FF_STUBS_H */
