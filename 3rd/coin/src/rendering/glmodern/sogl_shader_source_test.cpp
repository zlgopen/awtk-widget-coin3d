#include "rendering/glmodern/SoGLShaderSource.h"

#include <cstdio>
#include <cstring>

static int
require_has(const char * label, const char * src, const char * token)
{
  if (!src || !std::strstr(src, token)) {
    std::fprintf(stderr, "%s missing '%s'\n", label, token);
    return 1;
  }
  return 0;
}

static int
require_absent(const char * label, const char * src, const char * token)
{
  if (src && std::strstr(src, token)) {
    std::fprintf(stderr, "%s must not contain '%s'\n", label, token);
    return 1;
  }
  return 0;
}

int
main(void)
{
  int rc = 0;
  const char * pre = sogl_shader_preamble_for(SOGL_SHADER_DIALECT_GLES2);
  const char * vs = sogl_shader_default_vert_for(SOGL_SHADER_DIALECT_GLES2);
  const char * fs = sogl_shader_default_frag_for(SOGL_SHADER_DIALECT_GLES2);
  rc |= require_has("gles2 preamble", pre, "#version 100");
  rc |= require_has("gles2 vert", vs, "attribute vec3 a_position");
  rc |= require_has("gles2 vert", vs, "varying vec3 v_eyePos");
  rc |= require_absent("gles2 vert", vs, "in vec3 a_position");
  rc |= require_has("gles2 frag", fs, "texture2D");
  rc |= require_has("gles2 frag", fs, "gl_FragColor");
  rc |= require_absent("gles2 frag", fs, "texture(");
  rc |= require_absent("gles2 frag", fs, "out vec4 fragColor");

  const char * ovs = sogl_shader_overlay_vert_for(SOGL_SHADER_DIALECT_GLES2);
  const char * ofs = sogl_shader_overlay_frag_for(SOGL_SHADER_DIALECT_GLES2);
  rc |= require_has("overlay vert", ovs, "attribute");
  rc |= require_has("overlay frag", ofs, "texture2D");

  const char * gl3vs = sogl_shader_default_vert_for(SOGL_SHADER_DIALECT_GL3);
  rc |= require_has("gl3 vert", gl3vs, "in vec3 a_position");
  rc |= require_has("gl3 preamble", sogl_shader_preamble_for(SOGL_SHADER_DIALECT_GL3),
                    "#version 150");
  rc |= require_has("gles3 preamble", sogl_shader_preamble_for(SOGL_SHADER_DIALECT_GLES3),
                    "#version 300 es");
  return rc;
}
