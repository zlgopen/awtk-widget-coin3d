#ifndef COIN_SOGLSHADERSOURCE_H
#define COIN_SOGLSHADERSOURCE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

enum SoGLShaderDialect {
  SOGL_SHADER_DIALECT_GL3 = 0,
  SOGL_SHADER_DIALECT_GLES3,
  SOGL_SHADER_DIALECT_GLES2
};

SoGLShaderDialect sogl_shader_compile_dialect(void);
const char * sogl_shader_preamble_for(SoGLShaderDialect dialect);
const char * sogl_shader_default_vert_for(SoGLShaderDialect dialect);
const char * sogl_shader_default_frag_for(SoGLShaderDialect dialect);
const char * sogl_shader_overlay_vert_for(SoGLShaderDialect dialect);
const char * sogl_shader_overlay_frag_for(SoGLShaderDialect dialect);
const char * sogl_shader_overlay_line_vert_for(SoGLShaderDialect dialect);
const char * sogl_shader_overlay_line_frag_for(SoGLShaderDialect dialect);

#endif /* !COIN_SOGLSHADERSOURCE_H */
