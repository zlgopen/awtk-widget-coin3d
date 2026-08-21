# Coin OpenGL Profiles

Configure Coin's OpenGL backend with `COIN_GL_PROFILE`:

| Profile | Purpose | Build macro |
| --- | --- | --- |
| `COMPAT` | Legacy fixed-function OpenGL backend (default) | none |
| `GL3` | Desktop OpenGL Core backend | `COIN_GL_MODERN`, `COIN_GL3_CORE` |
| `GLES3` | OpenGL ES 3 backend | `COIN_GL_MODERN`, `COIN_GLES3` |
| `GLES2` | Reserved OpenGL ES 2 backend | `COIN_GL_MODERN`, `COIN_GLES2` |

For example, configure a desktop Core build with:

```sh
cmake -S . -B build-gl3 -DCOIN_GL_PROFILE=GL3
cmake --build build-gl3
```

`GLES2` implements the modern viewer subset (shaders + VBO, optional
`OES_vertex_array_object`). It is the profile AWTK uses by default on
Raspberry Pi. Shadows, 3D textures, and geometry shaders are not guaranteed.

## Phase status

- The GL3 Core backend is built in CI and remains the actively validated modern
  profile.
- GLES3 uses the shared modern backend (`SoGLDevice`) where platform GLES
  headers are available. Ubuntu CI builds GLES3; on macOS, configure falls back
  to desktop Core headers for compile-only stub verification when GLES SDK
  headers are missing.
- GLES2 uses GLSL ES 1.00 (`attribute`/`varying`/`texture2D`) and treats VAO
  as optional (`OES_vertex_array_object`). Ubuntu CI still compiles GLES2;
  Raspberry Pi is the intended runtime. ES headers omit `GLdouble`,
  `GL_RED` / `GL_GREEN` / `GL_BLUE`, `glDepthRange`, `glPolygonMode`,
  and `GL_LINE_WIDTH_RANGE` / `GL_POINT_SIZE_RANGE`; `sogl_es_types.h`
  maps depth range through `cc_gl_depth_range` (`glDepthRangef` on ES),
  and no-ops polygon mode / point size via `cc_gl_polygon_mode` /
  `cc_gl_point_size` so GLX's extern prototypes are never redeclared.
- Legacy display-list auto-caching stays disabled in modern profiles. Set
  `COIN_GL_MODERN_AUTO_CACHE=1` only to exercise the limited modern-cache
  scaffolding; it does not provide full legacy display-list recording.
- Overlay nodes (`SoText2` / `SoImage` / `SoMarkerSet` / selection lasso) use
  textured/line VAO helpers instead of `glBitmap` / `glDrawPixels`.
- 3D shapes under modern builds prefer `generatePrimitives` → PVCache → VAO.

## AWTK integration

`awtk-widget-coin3d` `scons` maps AWTK's current GPU backend to `COIN_GL_PROFILE`
(`scripts/coin_gl_profile.py`):

- AWTK `NANOVG_BACKEND=GL3` / `WITH_NANOVG_GL3` / `NVGP_GL3` → Coin `GL3`
- AWTK `NANOVG_BACKEND=GLES3` / `WITH_NANOVG_GLES3` / `WITH_GPU_GLES3` /
  `NVGP_GLES3` / `NANOVG_GLES3` → Coin `GLES3`
- AWTK `NANOVG_BACKEND=GLES2` / `WITH_NANOVG_GLES2` / `WITH_GPU_GLES2` /
  `NVGP_GLES2` / `NANOVG_GLES2` → Coin `GLES2` (Raspberry Pi default)
- AWTK `GL2`, `AGGE`, and Cairo are rejected

### GL headers (do not switch Raspberry Pi to glad)

| Platform | Coin includes | Why |
| --- | --- | --- |
| Linux / Raspberry Pi (GLES SDK present) | `GLES2/gl2.h` or `GLES3/gl3.h` plus `sogl_es_types.h` | Real ES entry points; missing desktop APIs are remapped or stubbed |
| Windows GLES2 / GLES3 / GL3 | AWTK `glad.h` | No system GLES headers; glad already `#define`s `glXxx` |
| macOS GLES stub | Desktop Core headers | Compile-only when GLES SDK is absent |

AWTK itself still loads GL via glad (`gladLoadGL()`). Coin must not include glad on Pi: mixing `GLES2/gl2.h` and `glad.h` is a glad `#error`, and glad's desktop pointers are NULL under a GLES2 driver. `sogl_es_types.h` skips wrappers when glad already defined those names, so Windows GLES2 and Pi GLES2 can share the same shim header.

Use matching NanoVG renderers when integrating Coin with AWTK:

- Coin `GL3` ↔ `NANOVG_GL3`
- Coin `GLES3` ↔ `NANOVG_GLES3`
- Coin `GLES2` (stub) ↔ `NANOVG_GLES2`

Render the modern Coin scene into an FBO, then composite that texture through
AWTK's `custom_draw_model`. The host application owns the FBO lifecycle and
must ensure that Coin and AWTK use compatible OpenGL contexts and profiles.

## GL parity regression

The Ubuntu CI parity job renders `tools/glparity/scenes_p0.txt` with both
`COMPAT` and `GL3`. It rejects empty frames and compares RGB RMSE against
per-scene limits. It intentionally does not provide pixel-perfect parity or
GLES3 rendering coverage; GLES3 smoke is planned separately.

See the [COMPAT ↔ GL3 rendering parity guide](../tools/glparity/README.md) for
local build, execution, threshold, and artifact details.
