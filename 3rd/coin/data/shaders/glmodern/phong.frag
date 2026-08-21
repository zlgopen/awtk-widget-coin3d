/* Shared Phong fragment body */

in vec3 v_eyePos;
in vec3 v_normal;
in vec4 v_color;
in vec2 v_texcoord0;
in float v_clipDist[6];

uniform vec4 u_diffuse;
uniform vec4 u_ambient;
uniform vec4 u_specular;
uniform vec4 u_emissive;
uniform float u_shininess;
uniform int u_lightModel;
uniform int u_numLights;
uniform int u_useTexture;
uniform int u_texEnvMode;
uniform sampler2D u_tex0;
uniform vec4 u_fogColor;
uniform float u_fogDensity;
uniform int u_fogType;
uniform int u_numClipPlanes;

uniform int u_lightType[8];
uniform vec4 u_lightPosition[8];
uniform vec3 u_lightDirection[8];
uniform vec4 u_lightDiffuse[8];
uniform vec4 u_lightSpecular[8];
uniform vec4 u_lightAmbient[8];
uniform float u_lightSpotCutoff[8];
uniform float u_lightSpotExponent[8];
uniform vec3 u_lightAttenuation[8];

out vec4 fragColor;

vec4 applyTexEnv(vec4 lit, vec4 tex)
{
  if (u_texEnvMode == 1) return tex; /* REPLACE */
  if (u_texEnvMode == 2) { /* DECAL */
    return vec4(mix(lit.rgb, tex.rgb, tex.a), lit.a);
  }
  if (u_texEnvMode == 3) { /* BLEND */
    return vec4(mix(lit.rgb, u_ambient.rgb, tex.rgb), lit.a * tex.a);
  }
  if (u_texEnvMode == 4) return lit + tex; /* ADD */
  return lit * tex; /* MODULATE */
}

void main(void)
{
  for (int i = 0; i < 6; ++i) {
    if (i < u_numClipPlanes && v_clipDist[i] < 0.0) discard;
  }

  vec4 base = u_diffuse * v_color;
  vec3 N = normalize(v_normal);
  vec3 V = normalize(-v_eyePos);
  vec3 rgb = u_emissive.rgb;

  if (u_lightModel == 0) {
    rgb = base.rgb;
  } else {
    rgb += u_ambient.rgb * base.rgb;
    for (int i = 0; i < 8; ++i) {
      if (i >= u_numLights) break;
      int t = u_lightType[i];
      if (t == 0) continue;

      vec3 L;
      float atten = 1.0;
      if (t == 1) { /* directional: position.xyz is direction toward light */
        L = normalize(-u_lightDirection[i]);
      } else {
        vec3 toLight = u_lightPosition[i].xyz - v_eyePos;
        float dist = length(toLight);
        L = toLight / max(dist, 1e-6);
        atten = 1.0 / (u_lightAttenuation[i].x
                       + u_lightAttenuation[i].y * dist
                       + u_lightAttenuation[i].z * dist * dist);
        if (t == 3) {
          float cosAngle = dot(-L, normalize(u_lightDirection[i]));
          float cut = cos(radians(u_lightSpotCutoff[i]));
          if (cosAngle < cut) atten = 0.0;
          else atten *= pow(max(cosAngle, 0.0), u_lightSpotExponent[i]);
        }
      }

      float ndotl = max(dot(N, L), 0.0);
      rgb += atten * u_lightAmbient[i].rgb * base.rgb;
      rgb += atten * ndotl * u_lightDiffuse[i].rgb * base.rgb;
      if (ndotl > 0.0) {
        vec3 H = normalize(L + V);
        float ndoth = max(dot(N, H), 0.0);
        rgb += atten * pow(ndoth, max(u_shininess * 128.0, 1.0))
               * u_lightSpecular[i].rgb * u_specular.rgb;
      }
    }
  }

  vec4 lit = vec4(rgb, base.a * u_diffuse.a);
  if (u_useTexture != 0) {
    lit = applyTexEnv(lit, texture(u_tex0, v_texcoord0));
  }

  if (u_fogType != 0) {
    float d = length(v_eyePos);
    float f = 1.0;
    if (u_fogType == 1) f = exp(-u_fogDensity * d);
    else if (u_fogType == 2) f = exp(-u_fogDensity * u_fogDensity * d * d);
    else f = clamp((50.0 - d) / 40.0, 0.0, 1.0);
    lit.rgb = mix(u_fogColor.rgb, lit.rgb, f);
  }

  fragColor = lit;
}
