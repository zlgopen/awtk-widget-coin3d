# 最新动态

2026/8/21
  * Coin GLES3：系统头已声明的 glReadBuffer 不再包 static stub，修复 Ubuntu 编译失败。
  * Coin GL3：Linux 桌面补 VBO/glGetStringi 原型，修复 Ubuntu 22 编译失败。
  * Coin GLES2 不再引用未声明的 glGetStringi，修复树莓派编译失败。
  * Coin 现代 GL 跳过 glEnable(GL_TEXTURE_2D)/glTexEnv 与 glGetString(GL_EXTENSIONS)，消除 textured_cube 的 GL_INVALID_ENUM。
  * Coin 现代 GL 不再查询 GL_RGBA_MODE，避免误入 color-index 导致材质变灰。
  * Coin GL3：桌面 Core 已声明的 glDrawBuffer 等不再包 static stub，修复 macOS 编译失败。
  * Coin GLES2：SoVBO 补上 config.h；glue 在指针为空时回退到已链接的 glGenBuffers。
  * Coin GLES2 修正版本串解析并补 VBO 入口，修复树莓派 SoVBO 断言崩溃。
  * Coin GLES 一次补齐常用桌面枚举（pixel store / draw buffer / texture query）及 glDrawBuffer 等 stub。
  * Coin GLES 桌面 API 改为 cc_gl_* 包装，避免与 GLX 的 extern 声明冲突。
  * Coin GLES shim 在桌面 GL/gl.h（经 GLX）已声明时不再包 static 函数。
  * 明确 Coin GLES：树莓派用系统 ES 头，Windows 用 glad，shim 避开重定义。
  * Coin GLES 桌面 API 包装避开 glad 的 glXxx 宏，修复 Windows GLES2 重定义。
  * Coin GLES stub glPointSize（ES2 无此入口，点大小需走 gl_PointSize）。
  * Coin GLES 将 GL_LINE_WIDTH_RANGE / GL_POINT_SIZE_RANGE 映射到 ALIASED 查询。
  * Coin GLES 将 glDepthRange 转到 glDepthRangef，并 stub glPolygonMode。
  * Coin GLES 头补 GL_RED/GREEN/BLUE，修复树莓派编译 SoGLRenderAction 失败。
  * Coin GLES 头补 GLdouble，修复树莓派编译 Inventor/C/glue/gl.h 失败。
  * scons 按 AWTK 的 NANOVG_BACKEND 自动选择 Coin GL3 / GLES3 / GLES2。
  * Coin GLES2 查看器后端落地，对接树莓派默认 NANOVG_GLES2。

2026/8/20
  * 修正 parkbench.iv 椅背顶梁右侧三角带绕序与法线，消除朝后的缺口。
  * screenshot 改用 widget_take_snapshot（依赖 AWTK GPU FBO 深度缓冲）。
  * 绘制时用不透明背景清屏，避免 Coin 默认 alpha=0 导致离屏截图背景发黑。
  * CLI / 下拉换模型只保留轨道角，距离由 viewAll 框住全景；文档插图按此重导。
  * viewAll 后拉开相机距离，避免立方体等小模型铺满视口；文档插图按此重导。
