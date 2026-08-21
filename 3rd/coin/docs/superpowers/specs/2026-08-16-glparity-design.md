# GL Modern / COMPAT Parity Harness Design

**Date:** 2026-08-16  
**Status:** Approved for implementation planning  
**Scope:** Prove Coin `COIN_GL_PROFILE=GL3` renders the same *functional* scenes as `COMPAT`. GLES3 smoke is deferred to P1 (not a phase-1 CI gate).

## Problem

The modern OpenGL backend (`COIN_GL_MODERN` / GL3 / GLES3) replaces fixed-function paths with shaders, VAOs, and CPU tessellation. CI currently only verifies that profiles **compile**. Regressions such as black screens, empty display-list caches, and wrong MVP composition still slip through.

We need a repeatable harness that compares **COMPAT vs GL3** on fixed Inventor scenes and fails CI when rendering is empty or grossly wrong.

## Goals

1. **P0 smoke:** For each listed `.iv`, GL3 produces a non-empty framebuffer (non-background pixels above a floor).
2. **P0 parity:** Same scene rendered with COMPAT and GL3 stays within an RMSE threshold (behaviorally equivalent, not pixel-identical).
3. **P1 GLES3 smoke (not a CI gate in phase 1):** Same renderer can be built for GLES3 and report non-black frames locally / in a follow-up job. Phase-1 Ubuntu CI does **not** require GLES3 parity or GLES3 smoke.
4. Local developers can run the suite without a GUI focus grab (hidden SDL window) and inspect `diff` artifacts on failure.

## Non-goals (YAGNI)

- Pixel-perfect match to fixed-function OpenGL.
- Checking in large golden PNG baselines as the primary oracle (COMPAT build of the same commit is the oracle).
- Exhaustive coverage of all `models/**/*.iv` in phase 1.
- MSAA / HiDPI in the harness (they skew diffs).
- Automating macOS CI GPU capture in phase 1 (Ubuntu CI is the gate; macOS remains local).

## Architecture

```
models/*.iv ──► glparity-render (COMPAT) ──► out/compat/<scene>/frame0000.rgba + stats.json
            └──► glparity-render (GL3)    ──► out/gl3/<scene>/frame0000.rgba + stats.json
                                                      │
                                                      ▼
                                            compare_rgba.py ──► pass/fail + optional diff.ppm
```

### `glparity-render` (C++)

- Location: `tools/glparity/glparity_render.cpp` (+ `tools/glparity/CMakeLists.txt`).
- Built when `COIN_BUILD_EXAMPLES=ON` (phase 1: no separate `COIN_BUILD_GLPARITY` option).
- Uses SDL3 hidden OpenGL window (same pattern as `examples/sdl3`).
- Selects GL context from Coin profile macros (`COIN_GL3_CORE` / `COIN_GLES3` / COMPAT).
- Loads one `.iv` via `SoDB::readAll` / `SoInput`.
- Fixed viewport (default **400×300**), no MSAA, no `HIGH_PIXEL_DENSITY`.
- **Lighting:** Always prepend a default `SoDirectionalLight` (and, when the scene has no camera, a `SoPerspectiveCamera` + `viewAll` with widened near/far) under a harness root `SoSeparator`. Do **not** edit checked-in `.iv` files for lights. This matches `examples/sdl3` and keeps unlit models like `dead_simple/cube.iv` renderable under both profiles.
- Background clear color is fixed to **RGB (0.12, 0.14, 0.18)** (same as sdl3 example). A pixel counts as non-background if any channel differs from the quantized clear color by more than **8** (on 0–255), i.e. `|Δ| > 8` on R, G, or B.
- Camera: if scene already has a camera, use it (still prepend light); else insert `SoPerspectiveCamera` and `viewAll`, then widen near/far so rotated AABBs stay in frustum.
- Renders N frames (default 1; phase 1: static frame 0 only).
- Writes:
  - `frameXXXX.rgba`: raw RGBA8 tightly packed, **OpenGL `glReadPixels` bottom-row-first** order. README and `compare_rgba.py` must use the same convention.
  - `stats.json`: `{ "width", "height", "non_bg_pixels", "non_bg_ratio", "center_rgba", "gl_version", "profile", "bg_rgb" }`.
- Exit non-zero on context failure, read failure, or zero non-background pixels (optional flag `--allow-empty` for debugging only; CI never sets it).

### `compare_rgba.py` (Python 3, stdlib only)

- Location: `tools/glparity/compare_rgba.py`.
- Inputs: two `.rgba` paths, width, height, thresholds.
- Metrics: RMSE (0–100% of full scale), max absolute channel delta, non-bg counts for both.
- Fail if either side `non_bg_ratio < floor` (default 1%).
- Fail if RMSE% > threshold (default 12%; per-scene override via manifest).
- Optional `--write-diff` writes `diff.ppm` (stdlib only; no Pillow).

### `run_parity.sh`

- Location: `tools/glparity/run_parity.sh`.
- Args: paths to COMPAT and GL3 `glparity-render` binaries, scene list file, output root.
- For each scene: render both → compare → aggregate exit code.
- Scene list: `tools/glparity/scenes_p0.txt` (one relative `.iv` path per line, optional `#` comments).

### Scene list (P0)

Prefer small, deterministic scenes under `models/`:

| Scene | Path |
| --- | --- |
| cube | `models/dead_simple/cube.iv` |
| rotated_cube | `models/dead_simple/rotated_cube.iv` |
| sphere | `models/dead_simple/sphere.iv` |
| cone | `models/dead_simple/cone.iv` |
| material | `models/dead_simple/material.iv` |
| texture2 | `models/dead_simple/texture2.iv` |
| text2 | `models/dead_simple/text2.iv` |
| indexedfaceset | `models/dead_simple/indexedfaceset.iv` |
| directionallight | `models/oiv_compliance/directionallight.iv` |
| clipplane | `models/oiv_compliance/clipplane.iv` |

Per-scene RMSE overrides (if needed) live in `tools/glparity/thresholds.json` keyed by scene basename; default 12%, `text2` default 20%.

### CI

Extend `.github/workflows/continuous-integration-workflow.yml` (Ubuntu):

1. Configure/build **COMPAT** with `-DCOIN_BUILD_EXAMPLES=ON -DCOIN_GL_PROFILE=COMPAT` (glparity target lives under examples/tools wiring; no separate `COIN_BUILD_GLPARITY` in phase 1).
2. Configure/build **GL3** with the same examples flag.
3. Install SDL3 if not already present for examples.
4. Run `run_parity.sh` under `xvfb-run` if required, with both binaries and `scenes_p0.txt`.

Do **not** gate phase-1 CI on GLES3. GLES3 smoke remains P1 / local.

Artifact upload on failure: `out/` tree including `diff.ppm` and `stats.json`.

## Acceptance criteria

- Local: `run_parity.sh` exits 0 on macOS/Linux with working GL for all P0 scenes.
- CI Ubuntu: COMPAT↔GL3 parity job exits 0 for all P0 scenes.
- Intentional break (e.g. force identity projection in GL3) makes the suite fail.
- Docs: short section in `docs/COIN_GL_PROFILE.md` linking to `tools/glparity/README.md`.

## Risks and mitigations

| Risk | Mitigation |
| --- | --- |
| COMPAT vs GL3 lighting differs slightly | RMSE budget 12%; raise per-scene only when reviewed |
| Headless CI has no display | Use SDL hidden window + xvfb-run if needed |
| Font / Text2 rasterization differs | Higher RMSE or non-bg-only check for `text2` |
| Scenes without lights | Harness always prepends a default directional light |

## Implementation order

1. Scaffold `tools/glparity` + CMake target `glparity-render`.
2. Implement render + stats + empty-frame fail.
3. Implement `compare_rgba.py` + unit-ish self-check on two synthetic buffers.
4. `scenes_p0.txt` + `run_parity.sh`.
5. Wire CI job; document in `COIN_GL_PROFILE.md`.

## Open decisions (resolved)

- **Oracle:** live COMPAT render of same commit, not checked-in goldens.
- **GLES3:** P1 only; not required for phase-1 CI.
- **Compare dependency:** Python stdlib only (PPM diffs).
