# Coin 修改说明

本仓库在 [`3rd/coin`](../3rd/coin) 相对上游 [Coin](https://github.com/coin3d/coin) 做了下列改动，以便和 AWTK 的 NanoVG 共用同一套 GL 上下文，并降低嵌入式构建依赖。

更细的 OpenGL 配置见 [COIN_GL_PROFILE.md](../3rd/coin/docs/COIN_GL_PROFILE.md)；STL 彩色补丁见 [stl-coin-patch.md](stl-coin-patch.md)。

## 1. 支持 OpenGL 3

上游默认走 **COMPAT**（固定管线 / compatibility profile）。本仓库增加 `COIN_GL_PROFILE=GL3`：桌面 OpenGL Core 3.2+，用 **shader + VAO + VBO** 替代 `glBegin`、display list 等固定管线路径。

要点：

* 编译宏：`COIN_GL_MODERN`、`COIN_GL3_CORE`
* 现代后端在 `3rd/coin/src/rendering/glmodern/`（`SoGLDevice`、`SoGLShader`、`SoGLVAO` 等）
* 3D 形状：`generatePrimitives` → PVCache → VAO
* Overlay（`SoText2` / `SoImage` / `SoMarkerSet` 等）走 textured / line VAO，不再用 `glBitmap` / `glDrawPixels`
* 与 AWTK 配对：Coin `GL3` ↔ `NANOVG_GL3`；输出目录 `3rd/coin/build-gl3`
* Ubuntu CI 用 `tools/glparity` 对比 COMPAT 与 GL3 的 RGB RMSE

桌面 Linux / macOS 用系统 Core 头；Windows 无可用 Core 头时走 AWTK glad（`-DAWTK_GLAD_DIR`）。

## 2. 支持 OpenGL ES 2.0

`COIN_GL_PROFILE=GLES2` 已对接并正常工作，面向树莓派等默认 `NANOVG_BACKEND=GLES2` 的平台（查看器路径：光照、2D 纹理、overlay）。

要点：

* 编译宏：`COIN_GL_MODERN`、`COIN_GLES2`
* 着色器为 **GLSL ES 1.00**（`attribute` / `varying` / `texture2D`）
* VBO 必用；VAO 可选（`OES_vertex_array_object`）
* 与 AWTK 配对：Coin `GLES2` ↔ `NANOVG_GLES2`；输出目录 `3rd/coin/build-gles2`
* 树莓派用系统 `GLES2/gl2.h`，**不**混用 AWTK glad；缺的桌面 API 由 `sogl_es_types.h` 与 `cc_gl_*` 包装补齐（如 `glDepthRangef`、`glPolygonMode` stub）
* Windows 无 GLES 头时才用 AWTK glad
* ES 2.0 本身没有几何着色器 / 3D 纹理 / 阴影贴图等桌面能力，这些路径不保证

`scons` 读到 AWTK `NANOVG_BACKEND=GLES2` 会自动编此 profile；也可 `COIN_GL_PROFILE=GLES2 ./3rd/build_coin.sh`。

## 3. 支持 OpenGL ES 3.0

`COIN_GL_PROFILE=GLES3` 与 GL3 共用同一套现代后端（`SoGLDevice`），面向嵌入式 / 移动上的 OpenGL ES 3.0。

要点：

* 编译宏：`COIN_GL_MODERN`、`COIN_GLES3`
* 与 AWTK 配对：Coin `GLES3` ↔ `NANOVG_GLES3`；输出目录 `3rd/coin/build-gles3`
* Linux / 树莓派优先 `GLES3/gl3.h`，否则回退 `GLES2/gl2.h` + `sogl_es_types.h`
* Windows 同样走 AWTK glad；macOS 无 GLES SDK 时用桌面 Core 头做编译验证

`scons` 按 `NANOVG_BACKEND=GLES3`（或 `WITH_NANOVG_GLES3` / `WITH_GPU_GLES3` 等）自动选择。

## 4. 完善 STL 支持

上游 Coin 已有 `SoSTLFileKit`。本仓库补齐彩色扩展，并在控件层打通资源打包与 `.iv` 内嵌引用。

Coin 侧（`3rd/coin/src/foreignfiles/SoSTLFileKit.cpp`）：

* ASCII 与二进制几何均可读
* 二进制彩色：Materialise Magics（头 `COLOR=RRGGBBAA` + 按面属性字）与 VisCAM / SolidView（按面 RGB）
* 有按面颜色时用 `SoMaterialBinding::PER_FACE`，在 `organizeModel()` 写入 `SoMaterial::diffuseColor`

控件侧（`src/coin3d/coin3d_coin.cpp`）：

* 按 `.stl` 后缀（或 `_stl` 资源别名）走 `SoSTLFileKit`
* 打包资源先物化到临时文件再导入
* `.iv` 里 `File { name "xxx.stl" }` 先转为临时 `.iv` 再 `pushFile()`；含 STL 的 `.iv` 先读入内存再解析，保证嵌套 include 稳定

示例：`pyramid.stl`、`pyramid_binary.stl`、`color_pyramid.stl`、`viscam_pyramid.stl`；混排见 `include_stl.iv`。STL 没有 `DEF` 场景层级，按名移动节点对纯 STL 基本无效。

## 5. 去掉 Boost 依赖

上游部分路径依赖 Boost。本仓库按 C++17 标准库替换，**库代码与 CMake 不再要求安装 Boost**（参考上游 [coin3d/coin#596](https://github.com/coin3d/coin/pull/596)，未整棵合并 v4.0.8）。

| 原 Boost | 替换 |
| --- | --- |
| `boost::scoped_ptr` / `scoped_array` | `std::unique_ptr` / `std::unique_ptr<T[]>` |
| `boost::intrusive_ptr<SoBase*>` | `SoRefPtr<T>`（`include/Inventor/misc/SoRefPtr.h`） |
| `boost::intrusive_ptr<cc_xml_doc>` | `std::unique_ptr` + 自定义 deleter |
| `boost::shared_array<char>` | `std::shared_ptr<char>` |
| `boost::lexical_cast` | `std::to_string` |
| `BOOST_STATIC_ASSERT` | `static_assert` |
| `BOOST_WORKAROUND` | `COIN_WORKAROUND`（`src/coindefs.h`） |
| `find_package(Boost REQUIRED)` | 删除 |

结果：`src/` / `include/` 不再 `#include <boost/...>`；`libCoin` 不链接 `libboost_*`。`testsuite/` 的 Boost.Test 未改，本工程构建时 `COIN_BUILD_TESTS=OFF`，不影响。
