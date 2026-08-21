# COMPAT ↔ GL3 rendering parity

This harness renders the same deterministic scene manifest with Coin's
`COMPAT` and `GL3` profiles, rejects empty frames, and compares the captured
RGB pixels. The Ubuntu/Xvfb job is the authoritative full-suite environment.

## Requirements

- CMake and a C++ compiler
- SDL3
- glfw3 (`COIN_BUILD_EXAMPLES=ON` also configures the required
  `examples/glfw` target)
- Python 3
- An OpenGL display; use Xvfb for headless Linux runs

## Build both renderers

Configure separate build trees so each executable is compiled against the
intended Coin OpenGL profile:

```sh
cmake -S . -B build-glparity-compat \
  -DCOIN_GL_PROFILE=COMPAT -DCOIN_BUILD_EXAMPLES=ON -DCOIN_BUILD_TESTS=OFF
cmake --build build-glparity-compat --target glparity-render -j4

cmake -S . -B build-glparity-gl3 \
  -DCOIN_GL_PROFILE=GL3 -DCOIN_BUILD_EXAMPLES=ON -DCOIN_BUILD_TESTS=OFF
cmake --build build-glparity-gl3 --target glparity-render -j4
```

## Run the P0 suite

From the repository root:

```sh
tools/glparity/run_parity.sh \
  --compat "$PWD/build-glparity-compat/bin/glparity-render" \
  --gl3 "$PWD/build-glparity-gl3/bin/glparity-render" \
  --scenes tools/glparity/scenes_p0.txt \
  --output /tmp/glparity-suite
```

The default frame size is 400x300. Override it with `--width` and `--height`.
On a headless Linux machine, prefix the command with `xvfb-run -a`.
The runner attempts every non-comment manifest entry and exits nonzero if any
render or comparison fails.

## Artifacts

For a manifest scene named `cube.iv`, the output is:

```text
<output>/
├── compat/cube/
│   ├── frame0000.rgba
│   └── stats.json
├── gl3/cube/
│   ├── frame0000.rgba
│   └── stats.json
└── diff/cube/
    └── diff.ppm
```

`frame0000.rgba` contains tightly packed RGBA8 pixels: four bytes per pixel,
with no header or row padding. Rows remain in the bottom-row-first order
returned by `glReadPixels`. `diff.ppm` is row-flipped into normal top-row-first
viewer order.

## Metrics and failures

The comparator reports non-background coverage for each profile, RGB RMSE as a
percentage of the full 8-bit channel range, and the maximum absolute RGB
channel delta. Alpha does not contribute to coverage or RMSE. Both frames must
meet the 0.01 coverage floor, which makes empty or nearly empty output a real
failure.

RMSE limits live in `thresholds.json`. Most scenes use
`default_rmse_percent`; entries under `scenes` override that value by basename
without `.iv`. Do not raise a threshold without reviewing the rendered frames
and recording why the difference is acceptable.

Open `diff/<scene>/diff.ppm` with any PPM-capable image viewer when a comparison
fails. Bright regions identify where COMPAT and GL3 differ; inspect the two
`stats.json` files alongside it to distinguish missing geometry from localized
shading or rasterization differences.

`text2.iv` requests Arial. Coin may fall back to another installed font,
especially on Linux, so this scene has a reviewed, higher RMSE limit for font
fallback differences.

`texture2.iv` uses a 2x2 inline texture on a cube with low texture quality.
COMPAT and modern OpenGL can select different sampling and filtering behavior,
so this scene has a reviewed 25% RMSE limit. The suite does not claim
pixel-perfect texture parity.

GLES3 rendering smoke coverage is deliberately deferred to P1; this suite only
asserts COMPAT ↔ GL3 parity.
