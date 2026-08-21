# awtk-widget-coin3d

AWTK 的 Coin3D（Open Inventor）场景控件：在控件内加载 `.iv` / `.stl` 模型，用 Examine 方式旋转、平移、缩放相机。

![](docs/images/iv-ground-grid.png)

## Coin 库

本控件依赖 [Coin](https://github.com/coin3d/coin)（Coin3D 的核心库）。Coin 实现 Open Inventor 2.1 API：用场景图（scene graph）保留模式描述三维模型，从 `.iv` / VRML 装入，再经 OpenGL 绘制。科学计算与工程可视化里常用它做三维查看与仿真。

源码在 [`3rd/coin`](3rd/coin)。上游默认走 **COMPAT** 后端（固定管线 / compatibility profile）。我们在此基础上加了现代 OpenGL 配置 `COIN_GL_PROFILE`。**GL3 / GLES3 / GLES2 均已对接并正常工作**：

| 配置 | 用途 |
| --- | --- |
| `COMPAT` | 上游默认，固定管线 |
| `GL3` | 桌面 OpenGL Core 3.2+（着色器、VAO） |
| `GLES3` | OpenGL ES 3.0，面向嵌入式 / 移动 |
| `GLES2` | OpenGL ES 2.0（树莓派默认；着色器 + VBO，VAO 可选） |

现代后端用 shader + VAO（GLES2 上 VAO 为 `OES_vertex_array_object`）替代 `glBegin` / display list 等固定管线路径，以便和 AWTK 的 NanoVG 共用同一套 GL 上下文：

| Coin | AWTK NanoVG |
| --- | --- |
| `GL3` | `NANOVG_GL3` |
| `GLES3` | `NANOVG_GLES3` |
| `GLES2` | `NANOVG_GLES2` |

`scons` 会读 AWTK 的 `NANOVG_BACKEND`（以及 `WITH_NANOVG_GL3` / `WITH_NANOVG_GLES3` / `WITH_NANOVG_GLES2` 等编译宏），自动选 Coin profile，并编到对应目录：

| AWTK | Coin | 输出目录 |
| --- | --- | --- |
| `GL3`（默认） | `GL3` | `3rd/coin/build-gl3` |
| `GLES3` | `GLES3` | `3rd/coin/build-gles3` |
| `GLES2`（树莓派默认） | `GLES2` | `3rd/coin/build-gles2` |
| `GL2` / `AGGE` | 不支持 | 构建时报错 |

树莓派上 AWTK 默认 `NANOVG_BACKEND=GLES2`，`scons` 会自动编 Coin GLES2。Coin 在树莓派用系统 `GLES2/gl2.h`（缺的桌面 API 由 `sogl_es_types.h` 补），不改走 glad；Windows 无 GLES 头时才用 AWTK glad。也可手动覆盖：

```
COIN_GL_PROFILE=GLES2 ./3rd/build_coin.sh
```

更细的配置、限制与 AWTK 对接说明见 [3rd/coin/docs/COIN_GL_PROFILE.md](3rd/coin/docs/COIN_GL_PROFILE.md)。

## 准备

1. 获取 awtk 并编译

```
git clone https://github.com/zlgopen/awtk.git
cd awtk; scons; cd -
```

## 运行

1. 生成示例代码的资源

```
python scripts/update_res.py all
```
> 也可以使用 Designer 打开项目，之后点击 “打包” 按钮进行生成；
> 如果资源发生修改，则需要重新生成资源。

如果 PIL 没有安装，执行上述脚本可能会出现如下错误：
```cmd
Traceback (most recent call last):
...
ModuleNotFoundError: No module named 'PIL'
```
请用 pip 安装：
```cmd
pip install Pillow
```

2. 编译

* 编译PC版本

```
scons
```

* 编译LINUX FB版本

```
scons LINUX_FB=true
```

> 完整编译选项请参考[编译选项](https://github.com/zlgopen/awtk-widget-generator/blob/master/docs/build_options.md)

3. 运行

```
./bin/demo
./bin/demo robot.iv
./bin/demo pyramid.stl
./bin/demo design/default/data/solar.iv
./bin/demo --screenshot docs/images/ui.png materials.iv
```

命令行参数中第一个 `.iv` 或 `.stl` 会作为启动模型：可以是已打包的资源名，也可以是本地文件路径。不指定时默认 `rotating_cube.iv`，仍可用界面上的 combo-box 切换。`--screenshot <png>` 会在首帧绘制后把窗口存成 PNG 并退出，用来导出文档插图。

## XML

```xml
<coin3d name="coin3d" x="c" y="10" w="90%" h="-100"
  model="cube.iv" background="#1e2430" gizmo="true"
  translation="0,0,0" rotation="20,30" scale="8"/>
```

`model` 可以是资源名（如 `cube.iv`、`pyramid.stl`）或本地文件路径。换模型会 `viewAll` 框住整场景；`translation` / `rotation` / `scale` 须在装模型之后设置（XML 里把这些属性写在 `model` 后面即可）。`.iv` 语法、`DEF` 命名与示例见 [Open Inventor（.iv）文件格式](docs/iv-format.md)。

## 支持的模型格式

| 格式 | 状态 | 说明 |
| --- | --- | --- |
| `.iv`（Open Inventor） | 正式支持 | 场景图、动画、`DEF` 节点、材质与贴图 |
| `.stl`（Stereolithography） | 正式支持 | ASCII 与二进制几何；二进制可选 Materialise / VisCAM 按面彩色 |
| `.obj` / `.dae` | 未支持 | 后续评估，见 [formats-future.md](docs/formats-future.md) |

### STL 限制

* **无场景层级**：STL 只有三角网格，没有 `DEF` 节点；`coin3d_find_node` / `node_move` 等按名操作对 STL 基本无效。
* **无标准材质/贴图**：普通 STL 只有几何；彩色依赖非标准二进制扩展。
* **彩色扩展**：仅二进制 STL；支持 Materialise Magics（头 `COLOR=RRGGBBAA` + 按面属性字）与 VisCAM / SolidView（按面属性字）。无颜色信息时使用默认灰色材质。Coin 侧补丁说明见 [stl-coin-patch.md](docs/stl-coin-patch.md)。
* **加载方式**：须通过带 `.stl` 后缀的文件名识别（本地路径或打包资源）；打包资源会先物化到临时文件再交给 Coin 导入。
* **`.iv` 内引用**：`File { name "xxx.stl" }` 会在 `pushFile()` 时转为临时 `.iv` 再嵌入；含 STL 的 `.iv` 会先读入内存后解析，以保证嵌套 include 稳定。

示例：`design/default/data/pyramid.stl`（ASCII）、`pyramid_binary.stl`、`color_pyramid.stl`、`viscam_pyramid.stl`；混排示例见 `include_stl.iv`。

## 属性

| 属性 | 说明 |
| --- | --- |
| `model` | 场景资源名或 `.iv` / `.stl` 文件路径 |
| `background` | 背景色（`color_parse` 支持的格式，如 `#1e2430`） |
| `gizmo` | 是否显示右上角视角导航 Gizmo |
| `translation` | 观察点 look-at，格式 `"x,y,z"` |
| `rotation` | 绕观察点的轨道角（度），格式 `"pitch,yaw"` |
| `scale` | 相机到观察点的距离 |

`get_prop` 返回当前相机状态（鼠标或按钮改过之后也能读到）。

对应 setter：`coin3d_set_model` / `coin3d_set_background` / `coin3d_set_gizmo` / `coin3d_set_translation` / `coin3d_set_rotation` / `coin3d_set_scale`。

## 增量函数

平移、旋转、缩放作用在**相机**上（与鼠标 Examine 同一套），不动物体。

| 函数 | 含义 |
| --- | --- |
| `coin3d_pan(w, dx, dy)` | 沿视平面平移观察点。`dx` 向右、`dy` 向上，单位为世界长度 |
| `coin3d_rotate(w, dx, dy)` | 绕观察点旋转。`dx` 为 pitch、`dy` 为 yaw，单位为度 |
| `coin3d_zoom(w, delta)` | 增减相机到观察点的距离；小于下限时钳制 |

```c
coin3d_pan(coin3d, 0.2f, 0.0f);
coin3d_rotate(coin3d, 15.0f, 0.0f);
coin3d_zoom(coin3d, -0.5f);
```

## 节点操作

按 `.iv` 里的 `DEF` 名称查找并变换物体（不改相机）。没有 `SoTransform` 时会自动插入一个。

| 函数 | 含义 |
| --- | --- |
| `coin3d_find_node(w, name)` | 查找节点，未找到返回 `NULL` |
| `coin3d_node_move(w, name, x, y, z)` | 设置平移 |
| `coin3d_node_rotate(w, name, x, y, z)` | 设置旋转（度，XYZ 欧拉角） |
| `coin3d_node_resize(w, name, sx, sy, sz)` | 设置缩放 |
| `coin3d_node_get_translation` / `get_rotation` / `get_scale` | 读取当前变换 |

```c
if (coin3d_find_node(coin3d, "box") != NULL) {
  coin3d_node_move(coin3d, "box", 1.0f, 0.0f, 0.0f);
  coin3d_node_rotate(coin3d, "box", 0.0f, 45.0f, 0.0f);
  coin3d_node_resize(coin3d, "box", 2.0f, 1.0f, 1.0f);
}
```

示例模型：`named_nodes.iv`（`DEF box` / `DEF ball` / `DEF mover`）。`include_files.iv` 用 `File` 引用 `cube.iv` 等其它 `.iv`；`include_stl.iv` 用 `File` 引用 `pyramid.stl`、`color_pyramid.stl` 等 `.stl`（加载时会先转为临时 `.iv` 再嵌入场景）。`ground_grid.iv` 用 `IndexedLineSet` 画 XZ 地面网格。

## 交互

* 左键拖拽：绕观察点旋转
* 中键拖拽（AWTK 将中键映射为 `TK_KEY_WHEEL`）：平移
* 右键拖拽 / 滚轮：缩放
* Gizmo（`gizmo="true"`）：右上角坐标轴，点轴对齐视角，拖圆环旋转

## Demo

底部第一行：模型下拉框、Close。第二行按钮调用增量函数：

* **PanL / PanR / PanU / PanD**：物体在屏幕上朝该方向移动
* **RotX± / RotY±**：绕观察点旋转（pitch / yaw，每次 15°）
* **Zoom+ / Zoom-**：拉近 / 拉远（每次距离 0.5）

修改 `design/default/ui/main.xml` 后需重新 `python scripts/update_res.py all`。

## 测试

```
python3 scripts/test_coin_gl_profile.py
scons
./bin/runTest
```

## 文档

* [Coin 修改说明](docs/coin-changes.md)
* [Open Inventor（.iv）文件格式](docs/iv-format.md)
* [Coin OpenGL 配置（GL3 / GLES3 / GLES2）](3rd/coin/docs/COIN_GL_PROFILE.md)
* [资源生成脚本](scripts/README.md)
* [完善自定义控件](https://github.com/zlgopen/awtk-widget-generator/blob/master/docs/improve_generated_widget.md)
