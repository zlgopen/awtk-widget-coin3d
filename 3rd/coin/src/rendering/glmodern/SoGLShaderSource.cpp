#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "rendering/glmodern/SoGLShaderSource.h"

SoGLShaderDialect
sogl_shader_compile_dialect(void)
{
#if defined(COIN_GLES2)
  return SOGL_SHADER_DIALECT_GLES2;
#elif defined(COIN_GLES3)
  return SOGL_SHADER_DIALECT_GLES3;
#else
  return SOGL_SHADER_DIALECT_GL3;
#endif
}

static const char * PREAMBLE_GL3 = "#version 150 core\n";
static const char * PREAMBLE_GLES3 =
  "#version 300 es\nprecision highp float;\nprecision highp int;\n";
static const char * PREAMBLE_GLES2 =
  "#version 100\nprecision mediump float;\nprecision mediump int;\n";

static const char * VERT_CORE =
  "in vec3 a_position;\n"
  "in vec3 a_normal;\n"
  "in vec4 a_color;\n"
  "in vec2 a_texcoord0;\n"
  "uniform mat4 u_modelView;\n"
  "uniform mat4 u_projection;\n"
  "uniform mat4 u_normalMatrix;\n"
  "uniform int u_numClipPlanes;\n"
  "uniform vec4 u_clipPlanes[6];\n"
  "out vec3 v_eyePos;\n"
  "out vec3 v_normal;\n"
  "out vec4 v_color;\n"
  "out vec2 v_texcoord0;\n"
  "out float v_clipDist[6];\n"
  "void main(void) {\n"
  "  vec4 eye = u_modelView * vec4(a_position, 1.0);\n"
  "  v_eyePos = eye.xyz;\n"
  "  v_normal = normalize((u_normalMatrix * vec4(a_normal, 0.0)).xyz);\n"
  "  v_color = a_color;\n"
  "  v_texcoord0 = a_texcoord0;\n"
  "  for (int i = 0; i < 6; ++i) {\n"
  "    v_clipDist[i] = (i < u_numClipPlanes) ? dot(u_clipPlanes[i], vec4(a_position, 1.0)) : 1.0;\n"
  "  }\n"
  "  gl_Position = u_projection * eye;\n"
  "}\n";

static const char * VERT_ES2 =
  "attribute vec3 a_position;\n"
  "attribute vec3 a_normal;\n"
  "attribute vec4 a_color;\n"
  "attribute vec2 a_texcoord0;\n"
  "uniform mat4 u_modelView;\n"
  "uniform mat4 u_projection;\n"
  "uniform mat4 u_normalMatrix;\n"
  "uniform int u_numClipPlanes;\n"
  "uniform vec4 u_clipPlanes[6];\n"
  "varying vec3 v_eyePos;\n"
  "varying vec3 v_normal;\n"
  "varying vec4 v_color;\n"
  "varying vec2 v_texcoord0;\n"
  "varying float v_clipDist[6];\n"
  "void main(void) {\n"
  "  vec4 eye = u_modelView * vec4(a_position, 1.0);\n"
  "  v_eyePos = eye.xyz;\n"
  "  v_normal = normalize((u_normalMatrix * vec4(a_normal, 0.0)).xyz);\n"
  "  v_color = a_color;\n"
  "  v_texcoord0 = a_texcoord0;\n"
  "  for (int i = 0; i < 6; ++i) {\n"
  "    v_clipDist[i] = (i < u_numClipPlanes) ? dot(u_clipPlanes[i], vec4(a_position, 1.0)) : 1.0;\n"
  "  }\n"
  "  gl_Position = u_projection * eye;\n"
  "}\n";

static const char * FRAG_CORE =
  "in vec3 v_eyePos;\n"
  "in vec3 v_normal;\n"
  "in vec4 v_color;\n"
  "in vec2 v_texcoord0;\n"
  "in float v_clipDist[6];\n"
  "uniform vec4 u_diffuse;\n"
  "uniform vec4 u_ambient;\n"
  "uniform vec4 u_specular;\n"
  "uniform vec4 u_emissive;\n"
  "uniform float u_shininess;\n"
  "uniform int u_lightModel;\n"
  "uniform int u_numLights;\n"
  "uniform int u_useTexture;\n"
  "uniform int u_texEnvMode;\n"
  "uniform sampler2D u_tex0;\n"
  "uniform vec4 u_fogColor;\n"
  "uniform float u_fogDensity;\n"
  "uniform int u_fogType;\n"
  "uniform int u_numClipPlanes;\n"
  "uniform int u_lightType[8];\n"
  "uniform vec4 u_lightPosition[8];\n"
  "uniform vec3 u_lightDirection[8];\n"
  "uniform vec4 u_lightDiffuse[8];\n"
  "uniform vec4 u_lightSpecular[8];\n"
  "uniform vec4 u_lightAmbient[8];\n"
  "uniform float u_lightSpotCutoff[8];\n"
  "uniform float u_lightSpotExponent[8];\n"
  "uniform vec3 u_lightAttenuation[8];\n"
  "out vec4 fragColor;\n"
  "vec4 applyTexEnv(vec4 lit, vec4 tex) {\n"
  "  if (u_texEnvMode == 1) return tex;\n"
  "  if (u_texEnvMode == 2) return vec4(mix(lit.rgb, tex.rgb, tex.a), lit.a);\n"
  "  if (u_texEnvMode == 3) return vec4(mix(lit.rgb, u_ambient.rgb, tex.rgb), lit.a * tex.a);\n"
  "  if (u_texEnvMode == 4) return lit + tex;\n"
  "  return lit * tex;\n"
  "}\n"
  "void main(void) {\n"
  "  for (int i = 0; i < 6; ++i) {\n"
  "    if (i < u_numClipPlanes && v_clipDist[i] < 0.0) discard;\n"
  "  }\n"
  "  vec4 base = u_diffuse * v_color;\n"
  "  vec3 N = normalize(v_normal);\n"
  "  vec3 V = normalize(-v_eyePos);\n"
  "  vec3 rgb = u_emissive.rgb;\n"
  "  if (u_lightModel == 0) { rgb = base.rgb; }\n"
  "  else {\n"
  "    rgb += u_ambient.rgb * base.rgb;\n"
  "    for (int i = 0; i < 8; ++i) {\n"
  "      if (i >= u_numLights) break;\n"
  "      int t = u_lightType[i];\n"
  "      if (t == 0) continue;\n"
  "      vec3 L; float atten = 1.0;\n"
  "      if (t == 1) { L = normalize(u_lightDirection[i]); }\n"
  "      else {\n"
  "        vec3 toLight = u_lightPosition[i].xyz - v_eyePos;\n"
  "        float dist = length(toLight);\n"
  "        L = toLight / max(dist, 1e-6);\n"
  "        atten = 1.0 / (u_lightAttenuation[i].x + u_lightAttenuation[i].y * dist + u_lightAttenuation[i].z * dist * dist);\n"
  "        if (t == 3) {\n"
  "          float cosAngle = dot(-L, normalize(u_lightDirection[i]));\n"
  "          float cut = cos(radians(u_lightSpotCutoff[i]));\n"
  "          if (cosAngle < cut) atten = 0.0;\n"
  "          else atten *= pow(max(cosAngle, 0.0), u_lightSpotExponent[i]);\n"
  "        }\n"
  "      }\n"
  "      float ndotl = max(dot(N, L), 0.0);\n"
  "      rgb += atten * u_lightAmbient[i].rgb * base.rgb;\n"
  "      rgb += atten * ndotl * u_lightDiffuse[i].rgb * base.rgb;\n"
  "      if (ndotl > 0.0) {\n"
  "        vec3 H = normalize(L + V);\n"
  "        float ndoth = max(dot(N, H), 0.0);\n"
  "        rgb += atten * pow(ndoth, max(u_shininess * 128.0, 1.0)) * u_lightSpecular[i].rgb * u_specular.rgb;\n"
  "      }\n"
  "    }\n"
  "  }\n"
  "  vec4 lit = vec4(rgb, base.a * u_diffuse.a);\n"
  "  if (u_useTexture != 0) lit = applyTexEnv(lit, texture(u_tex0, v_texcoord0));\n"
  "  if (u_fogType != 0) {\n"
  "    float d = length(v_eyePos); float f = 1.0;\n"
  "    if (u_fogType == 1) f = exp(-u_fogDensity * d);\n"
  "    else if (u_fogType == 2) f = exp(-u_fogDensity * u_fogDensity * d * d);\n"
  "    else f = clamp((50.0 - d) / 40.0, 0.0, 1.0);\n"
  "    lit.rgb = mix(u_fogColor.rgb, lit.rgb, f);\n"
  "  }\n"
  "  fragColor = lit;\n"
  "}\n";

static const char * FRAG_ES2 =
  "varying vec3 v_eyePos;\n"
  "varying vec3 v_normal;\n"
  "varying vec4 v_color;\n"
  "varying vec2 v_texcoord0;\n"
  "varying float v_clipDist[6];\n"
  "uniform vec4 u_diffuse;\n"
  "uniform vec4 u_ambient;\n"
  "uniform vec4 u_specular;\n"
  "uniform vec4 u_emissive;\n"
  "uniform float u_shininess;\n"
  "uniform int u_lightModel;\n"
  "uniform int u_numLights;\n"
  "uniform int u_useTexture;\n"
  "uniform int u_texEnvMode;\n"
  "uniform sampler2D u_tex0;\n"
  "uniform vec4 u_fogColor;\n"
  "uniform float u_fogDensity;\n"
  "uniform int u_fogType;\n"
  "uniform int u_numClipPlanes;\n"
  "uniform int u_lightType[8];\n"
  "uniform vec4 u_lightPosition[8];\n"
  "uniform vec3 u_lightDirection[8];\n"
  "uniform vec4 u_lightDiffuse[8];\n"
  "uniform vec4 u_lightSpecular[8];\n"
  "uniform vec4 u_lightAmbient[8];\n"
  "uniform float u_lightSpotCutoff[8];\n"
  "uniform float u_lightSpotExponent[8];\n"
  "uniform vec3 u_lightAttenuation[8];\n"
  "vec4 applyTexEnv(vec4 lit, vec4 tex) {\n"
  "  if (u_texEnvMode == 1) return tex;\n"
  "  if (u_texEnvMode == 2) return vec4(mix(lit.rgb, tex.rgb, tex.a), lit.a);\n"
  "  if (u_texEnvMode == 3) return vec4(mix(lit.rgb, u_ambient.rgb, tex.rgb), lit.a * tex.a);\n"
  "  if (u_texEnvMode == 4) return lit + tex;\n"
  "  return lit * tex;\n"
  "}\n"
  "void main(void) {\n"
  "  for (int i = 0; i < 6; ++i) {\n"
  "    if (i < u_numClipPlanes && v_clipDist[i] < 0.0) discard;\n"
  "  }\n"
  "  vec4 base = u_diffuse * v_color;\n"
  "  vec3 N = normalize(v_normal);\n"
  "  vec3 V = normalize(-v_eyePos);\n"
  "  vec3 rgb = u_emissive.rgb;\n"
  "  if (u_lightModel == 0) { rgb = base.rgb; }\n"
  "  else {\n"
  "    rgb += u_ambient.rgb * base.rgb;\n"
  "    for (int i = 0; i < 8; ++i) {\n"
  "      if (i < u_numLights) {\n"
  "        int t = u_lightType[i];\n"
  "        if (t != 0) {\n"
  "          vec3 L; float atten = 1.0;\n"
  "          if (t == 1) { L = normalize(u_lightDirection[i]); }\n"
  "          else {\n"
  "            vec3 toLight = u_lightPosition[i].xyz - v_eyePos;\n"
  "            float dist = length(toLight);\n"
  "            L = toLight / max(dist, 1e-6);\n"
  "            atten = 1.0 / (u_lightAttenuation[i].x + u_lightAttenuation[i].y * dist + u_lightAttenuation[i].z * dist * dist);\n"
  "            if (t == 3) {\n"
  "              float cosAngle = dot(-L, normalize(u_lightDirection[i]));\n"
  "              float cut = cos(radians(u_lightSpotCutoff[i]));\n"
  "              if (cosAngle < cut) atten = 0.0;\n"
  "              else atten *= pow(max(cosAngle, 0.0), u_lightSpotExponent[i]);\n"
  "            }\n"
  "          }\n"
  "          float ndotl = max(dot(N, L), 0.0);\n"
  "          rgb += atten * u_lightAmbient[i].rgb * base.rgb;\n"
  "          rgb += atten * ndotl * u_lightDiffuse[i].rgb * base.rgb;\n"
  "          if (ndotl > 0.0) {\n"
  "            vec3 H = normalize(L + V);\n"
  "            float ndoth = max(dot(N, H), 0.0);\n"
  "            rgb += atten * pow(ndoth, max(u_shininess * 128.0, 1.0)) * u_lightSpecular[i].rgb * u_specular.rgb;\n"
  "          }\n"
  "        }\n"
  "      }\n"
  "    }\n"
  "  }\n"
  "  vec4 lit = vec4(rgb, base.a * u_diffuse.a);\n"
  "  if (u_useTexture != 0) lit = applyTexEnv(lit, texture2D(u_tex0, v_texcoord0));\n"
  "  if (u_fogType != 0) {\n"
  "    float d = length(v_eyePos); float f = 1.0;\n"
  "    if (u_fogType == 1) f = exp(-u_fogDensity * d);\n"
  "    else if (u_fogType == 2) f = exp(-u_fogDensity * u_fogDensity * d * d);\n"
  "    else f = clamp((50.0 - d) / 40.0, 0.0, 1.0);\n"
  "    lit.rgb = mix(u_fogColor.rgb, lit.rgb, f);\n"
  "  }\n"
  "  gl_FragColor = lit;\n"
  "}\n";

static const char * OVERLAY_VERT_CORE =
  "in vec3 a_position;\n"
  "in vec2 a_texcoord;\n"
  "out vec2 v_texcoord;\n"
  "void main(void) {\n"
  "  gl_Position = vec4(a_position, 1.0);\n"
  "  v_texcoord = a_texcoord;\n"
  "}\n";

static const char * OVERLAY_VERT_ES2 =
  "attribute vec3 a_position;\n"
  "attribute vec2 a_texcoord;\n"
  "varying vec2 v_texcoord;\n"
  "void main(void) {\n"
  "  gl_Position = vec4(a_position, 1.0);\n"
  "  v_texcoord = a_texcoord;\n"
  "}\n";

static const char * OVERLAY_FRAG_CORE =
  "in vec2 v_texcoord;\n"
  "uniform sampler2D u_texture;\n"
  "out vec4 fragColor;\n"
  "void main(void) {\n"
  "  fragColor = texture(u_texture, v_texcoord);\n"
  "}\n";

static const char * OVERLAY_FRAG_ES2 =
  "varying vec2 v_texcoord;\n"
  "uniform sampler2D u_texture;\n"
  "void main(void) {\n"
  "  gl_FragColor = texture2D(u_texture, v_texcoord);\n"
  "}\n";

static const char * OVERLAY_LINE_VERT_CORE =
  "in vec2 a_position;\n"
  "uniform vec2 u_viewport;\n"
  "uniform vec2 u_origin;\n"
  "void main(void) {\n"
  "  vec2 ndc = ((a_position - u_origin) / u_viewport) * 2.0 - 1.0;\n"
  "  gl_Position = vec4(ndc, 0.0, 1.0);\n"
  "}\n";

static const char * OVERLAY_LINE_VERT_ES2 =
  "attribute vec2 a_position;\n"
  "uniform vec2 u_viewport;\n"
  "uniform vec2 u_origin;\n"
  "void main(void) {\n"
  "  vec2 ndc = ((a_position - u_origin) / u_viewport) * 2.0 - 1.0;\n"
  "  gl_Position = vec4(ndc, 0.0, 1.0);\n"
  "}\n";

static const char * OVERLAY_LINE_FRAG_CORE =
  "uniform vec3 u_color;\n"
  "out vec4 fragColor;\n"
  "void main(void) {\n"
  "  fragColor = vec4(u_color, 1.0);\n"
  "}\n";

static const char * OVERLAY_LINE_FRAG_ES2 =
  "uniform vec3 u_color;\n"
  "void main(void) {\n"
  "  gl_FragColor = vec4(u_color, 1.0);\n"
  "}\n";

const char *
sogl_shader_preamble_for(SoGLShaderDialect dialect)
{
  if (dialect == SOGL_SHADER_DIALECT_GLES2) return PREAMBLE_GLES2;
  if (dialect == SOGL_SHADER_DIALECT_GLES3) return PREAMBLE_GLES3;
  return PREAMBLE_GL3;
}

const char *
sogl_shader_default_vert_for(SoGLShaderDialect dialect)
{
  return (dialect == SOGL_SHADER_DIALECT_GLES2) ? VERT_ES2 : VERT_CORE;
}

const char *
sogl_shader_default_frag_for(SoGLShaderDialect dialect)
{
  return (dialect == SOGL_SHADER_DIALECT_GLES2) ? FRAG_ES2 : FRAG_CORE;
}

const char *
sogl_shader_overlay_vert_for(SoGLShaderDialect dialect)
{
  return (dialect == SOGL_SHADER_DIALECT_GLES2) ? OVERLAY_VERT_ES2 : OVERLAY_VERT_CORE;
}

const char *
sogl_shader_overlay_frag_for(SoGLShaderDialect dialect)
{
  return (dialect == SOGL_SHADER_DIALECT_GLES2) ? OVERLAY_FRAG_ES2 : OVERLAY_FRAG_CORE;
}

const char *
sogl_shader_overlay_line_vert_for(SoGLShaderDialect dialect)
{
  return (dialect == SOGL_SHADER_DIALECT_GLES2) ? OVERLAY_LINE_VERT_ES2 : OVERLAY_LINE_VERT_CORE;
}

const char *
sogl_shader_overlay_line_frag_for(SoGLShaderDialect dialect)
{
  return (dialect == SOGL_SHADER_DIALECT_GLES2) ? OVERLAY_LINE_FRAG_ES2 : OVERLAY_LINE_FRAG_CORE;
}
