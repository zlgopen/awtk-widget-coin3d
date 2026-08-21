/* Shared Phong body; prepended with preamble.gl3.glsl or preamble.gles3.glsl.
 * Attribute locations are bound with glBindAttribLocation (GLSL 150 has no
 * layout(location=) for attributes; that arrives in GLSL 330). */

in vec3 a_position;
in vec3 a_normal;
in vec4 a_color;
in vec2 a_texcoord0;

uniform mat4 u_modelView;
uniform mat4 u_projection;
uniform mat4 u_normalMatrix;
uniform int u_numClipPlanes;
uniform vec4 u_clipPlanes[6];

out vec3 v_eyePos;
out vec3 v_normal;
out vec4 v_color;
out vec2 v_texcoord0;
out float v_clipDist[6];

void main(void)
{
  vec4 eye = u_modelView * vec4(a_position, 1.0);
  v_eyePos = eye.xyz;
  v_normal = normalize((u_normalMatrix * vec4(a_normal, 0.0)).xyz);
  v_color = a_color;
  v_texcoord0 = a_texcoord0;
  for (int i = 0; i < 6; ++i) {
    v_clipDist[i] = (i < u_numClipPlanes)
      ? dot(u_clipPlanes[i], vec4(a_position, 1.0))
      : 1.0;
  }
  gl_Position = u_projection * eye;
}
