# Remove Coin Boost Dependency Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 让 `3rd/coin` 的 CMake / 库代码在不安装 Boost 的情况下完成 GL3 构建，并继续支撑 `awtk-widget-coin3d` 控件与 demo。

**Architecture:** 不整棵合并上游 v4.0.8。按 [coin3d/coin#596](https://github.com/coin3d/coin/pull/596) 的替换规则，把 Boost header-only 用法改成 C++11/17 标准库，并新增 `SoRefPtr` 承接 `SoBase` 引用计数。本地 `opengl3` 分支上只手工合并会冲突的文件（`CMakeLists.txt`、`SoGLRenderAction.cpp`）。Autotools / AppVeyor / 上游 CI 去 Boost 清理不在本计划范围内。

**Tech Stack:** Coin 4.0.7 + 本地 GL3，CMake，C++17（控件侧已用 `-std=gnu++17`），C++11 智能指针 / `static_assert` / `std::to_string`。

---

## 范围

**做：**

- Coin 库源码与公开头里的 Boost 引用
- CMake `find_package(Boost REQUIRED)` 与 `coin-config.cmake.in` 的 `find_dependency(Boost)`
- 无 Boost 重新配置并编译 `libCoin`
- 控件 `scons` + demo 冒烟

**不做：**

- 合并上游 v4.0.8 全部分支
- Autotools（`configure.ac` / `Makefile.am` / `include/Makefile.inc`）
- AppVeyor / 上游文档工作流里的 Boost 安装步骤
- 改写 `testsuite/` 的 Boost.Test（本仓库 `build_coin.sh` 已 `COIN_BUILD_TESTS=OFF`）。若以后打开测试，需另开任务移植上游 `testsuite/CoinTest.h`

**成功标准：**

1. `cmake -DCMAKE_DISABLE_FIND_PACKAGE_Boost=ON` 能配置成功
2. `src/`、`include/` 中库代码不再 `#include <boost/...>`（`COIN_TEST_SUITE` 块也一并清掉，避免以后误开测试踩坑）
3. `libCoin.dylib` 仍不链接任何 `libboost_*`
4. `awtk-widget-coin3d` 的 `scons` 与 `./bin/demo` 能跑通默认模型

---

## 文件结构

| 文件 | 职责 |
| --- | --- |
| `include/Inventor/misc/SoRefPtr.h` | 新增。`SoBase::ref/unref` 的 RAII 指针，替代 `boost::intrusive_ptr<So*>` |
| `include/Inventor/misc/SoBase.h` | 删除 `intrusive_ptr_add_ref/release` 辅助函数 |
| `include/Inventor/SbByteBuffer.h` | `boost::shared_array<char>` → `std::shared_ptr<char>` |
| `include/Inventor/SbByteBufferP.icc` | 同上，`.unique()` → `use_count() == 1` |
| `src/coindefs.h` | 去掉 `boost/detail/workaround.hpp`，本地实现 `COIN_WORKAROUND` |
| `CMakeLists.txt` | 删除 `find_package(Boost REQUIRED)`；保留本地 GL3 选项 |
| `src/coin-config.cmake.in` | 删除静态库消费者的 `find_dependency(Boost)` |
| `src/actions/SoGLRenderAction.cpp` | **冲突文件**：只改 Boost 指针，不动 GL3 渲染逻辑 |
| `src/scxml/*.cpp`、`src/soscxml/*.cpp`、`src/navigation/*.cpp`、`src/profiler/*.cpp` 等 | 机械替换 `scoped_ptr/array`、`intrusive_ptr` |
| `src/base/{heap,SbRotation,SbVec3f,SbVec3s,SbVec4f,SbByteBuffer}.cpp` | 去掉测试块里的 `lexical_cast` |
| `src/io/SoInput.cpp`、`src/io/SoOutput.cpp` | Cygwin 分支 `BOOST_STATIC_ASSERT` → `static_assert` |
| `src/rendering/SoOffscreenRenderer.cpp` | `BOOST_CURRENT_FUNCTION` → `COIN_STUB_FUNC` 或 `__func__` |
| `INSTALL` | 删除 “Boost 必装” 段落 |
| `3rd/build_coin.sh`、仓库根 `SConstruct` | 无需改逻辑；验证时清缓存重配 |

参考实现（只抄替换，不抄无关提交）：

- `SoRefPtr`：<https://raw.githubusercontent.com/coin3d/coin/master/include/Inventor/misc/SoRefPtr.h>
- `SbByteBuffer*`：上游 master 同名文件
- `coindefs.h` 的 `COIN_WORKAROUND`：上游 master
- `cc_xml_doc` 自定义 deleter：上游 `src/scxml/ScXMLDocument.cpp`

---

## 统一替换规则

实现时按这张表机械替换，不要发明新封装。

| 原用法 | 替换 | 注意 |
| --- | --- | --- |
| `#include <boost/scoped_ptr.hpp>` + `boost::scoped_ptr<T>` | `#include <memory>` + `std::unique_ptr<T>` | API 基本兼容：`get/reset/operator->` |
| `boost::scoped_array<T>` / `scoped_array<T>(new T[n])` | `std::unique_ptr<T[]>` / `unique_ptr<T[]>(new T[n])` | 用 `[]` 特化，不要写成 `unique_ptr<T>` |
| `boost::intrusive_ptr<SoXxx>`（`SoXxx` 继承 `SoBase`） | `#include <Inventor/misc/SoRefPtr.h>` + `SoRefPtr<SoXxx>` | `SoRefPtr` 构造会 `ref()`，析构 `unref()`。单参构造是 `explicit`：`p = raw` 必须改成 `p.reset(raw)` 或 `SoRefPtr<T> p(raw)` |
| `boost::intrusive_ptr<cc_xml_doc>` | `std::unique_ptr<cc_xml_doc, cc_xml_doc_deleter>` | `cc_xml_doc` 没有 `ref/unref`，**不能**用 `SoRefPtr` |
| `boost::shared_array<char>` | `std::shared_ptr<char>` + `std::default_delete<char[]>()` | 见 Task 3 |
| `buffer.unique()` | `buffer.use_count() == 1` | `shared_ptr` 没有 `.unique()` |
| `boost::lexical_cast<std::string>(x)` | `std::to_string(x)` | 仅测试块；同时删 `#include <boost/lexical_cast.hpp>` |
| `BOOST_STATIC_ASSERT(expr)` | `static_assert(expr, "msg")` | 仅 `__CYGWIN__` 分支 |
| `BOOST_WORKAROUND(def, test)` | `COIN_WORKAROUND(def, test)`，宏改为 `((def) != 0 && ((def) test))` | 先改 `coindefs.h`，再改调用点 |
| `BOOST_CURRENT_FUNCTION` | `COIN_STUB_FUNC`（已在 `coindefs.h`） | 该文件已 include `coindefs.h` |
| `SoBase.h` 里 `intrusive_ptr_add_ref/release` | 删除 | **等所有 `intrusive_ptr` 都改完再删**（Task 6），否则中途全库编译会失败 |

`std::unique_ptr` 成员的默认构造是空指针，与 `scoped_ptr` 一致。赋值用 `reset(new T)` 或 `= std::unique_ptr<T>(new T)`，不要依赖 Boost 的隐式行为。

---

## 冲突文件（必须手工合并）

本地 `v4.0.7..opengl3` 与 PR #596 重叠：

| 文件 | 本地改了什么 | 去 Boost 怎么合 |
| --- | --- | --- |
| `CMakeLists.txt` | `COIN_GL_PROFILE`、`tools/glparity` | 只删 Boost 段，保留 GL3 / glparity |
| `src/actions/SoGLRenderAction.cpp` | GL3 渲染路径 | 只改 include 与 4 个智能指针成员 |
| `.github/workflows/continuous-integration-workflow.yml` | glparity CI | **不要**套用上游去 Boost 的 workflow 大改 |
| `src/rendering/SoVertexArrayIndexer.cpp` | GL3 indexer | 只把 `BOOST_WORKAROUND` 换成 `COIN_WORKAROUND` |

禁止：`git merge` / `git cherry-pick` 整个 #596 到 `opengl3`。按文件移植。

---

### Task 1: 建立验收门禁（先证明现在失败）

**Files:**
- 不改源码
- 工作目录：`3rd/coin`

- [ ] **Step 1: 记录当前 Boost 依赖仍然是硬性的**

在 `3rd/coin` 用干净构建目录（不要复用已有 `build-gl3`）：

```bash
rm -rf /tmp/coin-noboost-probe
cmake -S . -B /tmp/coin-noboost-probe \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=17 \
  -DCOIN_GL_PROFILE=GL3 \
  -DCOIN_BUILD_SHARED_LIBS=ON \
  -DCOIN_BUILD_TESTS=OFF \
  -DCOIN_BUILD_DOCUMENTATION=OFF \
  -DCOIN_BUILD_EXAMPLES=OFF \
  -DCOIN_HAVE_JAVASCRIPT=OFF \
  -DHAVE_SOUND=OFF \
  -DHAVE_3DS_IMPORT_CAPABILITIES=OFF \
  -DUSE_EXTERNAL_EXPAT=OFF \
  -DUSE_EXCEPTIONS=OFF \
  -DUSE_SUPERGLU=OFF \
  -DCOIN_BUILD_SINGLE_LIB=ON \
  -DCMAKE_DISABLE_FIND_PACKAGE_Boost=ON
```

Expected: configure **失败**，报找不到 Boost（`Could not find a package configuration file provided by "Boost"` 或同类错误）。

- [ ] **Step 2: 记录当前源码命中数**

```bash
rg -n --glob '*.{h,hpp,cpp,c,cc,icc}' '#include <boost/' src include
```

Expected: 至少包括 `SbByteBuffer.h`、`coindefs.h`、`SoGLRenderAction.cpp`、以及 scxml / navigation / profiler 等约 40 个文件。把输出留着，Task 9 用来对比。

---

### Task 2: 加入 `SoRefPtr` 并去掉 `BOOST_WORKAROUND`

**Files:**
- Create: `include/Inventor/misc/SoRefPtr.h`
- Modify: `src/coindefs.h:50`、`src/coindefs.h:183-188`

`SoBase.h` 的 `intrusive_ptr_add_ref/release` **不要在本任务删除**。钩子必须留到 Task 6，否则 Task 4/5 全库编译会在 profiler/navigation 等未改文件上失败，且失败指引会把人带偏到 `SoGLRenderAction.cpp`。

- [ ] **Step 1: 新增 `SoRefPtr.h`**

完整内容按上游 master 抄入（保留 Coin 版权头风格，与邻近头文件一致）：

```cpp
#ifndef COIN_SOREFPTR_H
#define COIN_SOREFPTR_H

#include <algorithm>

template <typename T>
class SoRefPtr {
public:
  SoRefPtr(void) noexcept : ptr(NULL) { }

  explicit SoRefPtr(T * p) : ptr(p)
  {
    if (this->ptr) this->ptr->ref();
  }

  SoRefPtr(const SoRefPtr & other) : ptr(other.ptr)
  {
    if (this->ptr) this->ptr->ref();
  }

  SoRefPtr(SoRefPtr && other) noexcept : ptr(other.ptr)
  {
    other.ptr = NULL;
  }

  ~SoRefPtr(void)
  {
    if (this->ptr) this->ptr->unref();
  }

  SoRefPtr & operator=(SoRefPtr other) noexcept
  {
    this->swap(other);
    return *this;
  }

  void reset(T * p = NULL)
  {
    SoRefPtr tmp(p);
    this->swap(tmp);
  }

  T * get(void) const noexcept { return this->ptr; }
  T & operator*(void) const { return *this->ptr; }
  T * operator->(void) const noexcept { return this->ptr; }
  explicit operator bool(void) const noexcept { return this->ptr != NULL; }

  void swap(SoRefPtr & other) noexcept
  {
    using std::swap;
    swap(this->ptr, other.ptr);
  }

private:
  T * ptr;
};

#endif // !COIN_SOREFPTR_H
```

CMake 通过 `include/` 目录安装公开头，放到 `include/Inventor/misc/` 即可，不必改 CMake 文件列表。

- [ ] **Step 2: 改 `coindefs.h`**

删除：

```cpp
#include <boost/detail/workaround.hpp> /* For BOOST_WORKAROUND */
```

把

```cpp
#define COIN_WORKAROUND(def, test) BOOST_WORKAROUND(def,test)

#if BOOST_WORKAROUND(_MSC_VER, <= COIN_MSVC_6_0_VERSION)
```

改成：

```cpp
#define COIN_WORKAROUND(def, test) ((def) != 0 && ((def) test))

#if COIN_WORKAROUND(_MSC_VER, <= COIN_MSVC_6_0_VERSION)
```

- [ ] **Step 3: 替换剩余 `BOOST_WORKAROUND` 调用点**

这些文件只改宏名，不改条件：

- `src/collision/SoIntersectionDetectionAction.cpp`
- `src/rendering/SoVertexArrayIndexer.cpp`（GL3 冲突文件，只改这一处宏）
- `src/rendering/SoRenderManager.cpp`
- `src/rendering/SoGLImage.cpp`
- `src/misc/CoinResources.cpp`
- `src/misc/SoDB.cpp`（若 `#include <boost/detail/workaround.hpp>` 只在 `COIN_TEST_SUITE` 里，整段 include 删掉）

```bash
rg -n 'BOOST_WORKAROUND|#include <boost/detail/workaround.hpp>' src include
```

Expected: 无命中。

---

### Task 3: `SbByteBuffer` 去掉 `shared_array`

**Files:**
- Modify: `include/Inventor/SbByteBuffer.h`
- Modify: `include/Inventor/SbByteBufferP.icc`
- Modify: `src/base/SbByteBuffer.cpp`（只动测试块 include）

默认未定义 `ABI_BREAKING_OPTIMIZE`，公开头走 pimpl；`.icc` 仍会编进库。

- [ ] **Step 1: 改公开头宏**

`SBBYTEBUFFER_PRIVATE_VARIABLES` 中：

```cpp
boost::shared_array<char> buffer;
```

改为：

```cpp
std::shared_ptr<char> buffer;
```

`#ifndef ABI_BREAKING_OPTIMIZE` 分支保持 pimpl，不要在默认路径 `#include <boost/shared_array.hpp>`。在文件顶部增加：

```cpp
#include <memory>
```

`ABI_BREAKING_OPTIMIZE` 打开时才需要 `<memory>` 可见；为简单起见两个分支都 include `<memory>`。

- [ ] **Step 2: 改 `.icc` 构造与 `makeUnique`**

`#include <boost/shared_array.hpp>` → `#include <memory>`。

所有

```cpp
boost::shared_array<char>(new char[size_in])
```

改为：

```cpp
std::shared_ptr<char>(new char[size_in], std::default_delete<char[]>())
```

空缓冲：

```cpp
std::shared_ptr<char>()
```

`makeUnique()`：

```cpp
if (PRIVATE(this)->size_ && !(PRIVATE(this)->buffer.use_count() == 1)) {
  std::shared_ptr<char> tmp_buffer(new char[PRIVATE(this)->size_], std::default_delete<char[]>());
  memcpy(tmp_buffer.get(), PRIVATE(this)->buffer.get(), PRIVATE(this)->size_);
  PRIVATE(this)->buffer = tmp_buffer;
}
```

不要用 `std::shared_ptr<char[]>` 特化去赌编译器对 `use_count` / `get` 的支持差异；与上游 master 保持一致。

- [ ] **Step 3: 清 `SbByteBuffer.cpp` 测试 include**

`#ifdef COIN_TEST_SUITE` 里的 `#include <boost/lexical_cast.hpp>` 和 `boost::lexical_cast` 改成 `std::to_string`。本任务只改这一处测试消息拼接。

---

### Task 4: 手工改 `SoGLRenderAction.cpp`

**Files:**
- Modify: `src/actions/SoGLRenderAction.cpp:70-71`
- Modify: `src/actions/SoGLRenderAction.cpp:570`
- Modify: `src/actions/SoGLRenderAction.cpp:587`
- Modify: `src/actions/SoGLRenderAction.cpp:627-629`

这是唯一既有 GL3 改动、又有 Boost 智能指针的核心渲染文件。禁止用上游整文件覆盖。

- [ ] **Step 1: 只替换 include**

```cpp
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>
```

改为：

```cpp
#include <memory>
```

若文件已有 `<memory>` / `<cstdlib>`，合并，不要重复。

- [ ] **Step 2: 只替换 PImpl 成员类型**

```cpp
boost::scoped_ptr<SoGetBoundingBoxAction> bboxaction;
boost::scoped_array<GLuint> rgbatextureids;
boost::scoped_ptr<SoAlarmSensor> redrawSensor;
boost::scoped_ptr<SoNodeSensor> deleteSensor;
```

改为：

```cpp
std::unique_ptr<SoGetBoundingBoxAction> bboxaction;
std::unique_ptr<GLuint[]> rgbatextureids;
std::unique_ptr<SoAlarmSensor> redrawSensor;
std::unique_ptr<SoNodeSensor> deleteSensor;
```

- [ ] **Step 3: 检查该文件其余 Boost 用法**

```bash
rg -n 'boost::' src/actions/SoGLRenderAction.cpp
```

Expected: 无命中。`reset()` / `get()` / `[]` 调用保持不动。

- [ ] **Step 4: 增量编译该翻译单元（仍允许系统 Boost 存在）**

若还没有 `build-gl3`，先在 `3rd/` 跑 `./build_coin.sh`（或按 Task 9 同参数、**不要**加 `CMAKE_DISABLE_FIND_PACKAGE_Boost`）生成目录，再增量编译。

在已有 `build-gl3` 上：

```bash
cmake --build build-gl3 --target Coin -j"$(sysctl -n hw.ncpu)"
```

Expected: `SoGLRenderAction.cpp` 通过。若失败，只修这个文件，不要顺手改 GL3 逻辑。

---

### Task 5: 机械替换 `scoped_ptr` / `scoped_array`

按目录一批一批改，每批编译一次，避免一次改 30 个文件后无法定位错误。

**Files（库代码，不含 testsuite）：**

- `src/scxml/ScXMLAssignElt.cpp`
- `src/scxml/ScXMLDocument.cpp`（`scoped_*` 部分；`intrusive_ptr<cc_xml_doc>` 留到 Task 6）
- `src/scxml/ScXMLEvaluator.cpp`
- `src/scxml/ScXMLEventTarget.cpp`
- `src/scxml/ScXMLFinalElt.cpp`
- `src/scxml/ScXMLHistoryElt.cpp`
- `src/scxml/ScXMLIfElt.cpp`
- `src/scxml/ScXMLInitialElt.cpp`
- `src/scxml/ScXMLInvokeElt.cpp`
- `src/scxml/ScXMLMinimumEvaluator.cpp`
- `src/scxml/ScXMLParallelElt.cpp`
- `src/scxml/ScXMLScxmlElt.cpp`
- `src/scxml/ScXMLStateElt.cpp`
- `src/scxml/ScXMLStateMachine.cpp`
- `src/scxml/ScXMLTransitionElt.cpp`
- `src/soscxml/ScXMLCoinEvaluator.cpp`
- `src/soscxml/SoScXMLStateMachine.cpp`（`scoped_ptr` 部分；`intrusive_ptr` 留到 Task 6）
- `src/navigation/SoScXMLNavigationTarget.cpp`
- `src/navigation/SoScXMLRotateTarget.cpp`（`scoped_ptr`）
- `src/navigation/SoScXMLDollyTarget.cpp`
- `src/profiler/SoNodeVisualize.cpp`
- `src/profiler/SoProfilerVisualizeKit.cpp`
- `src/profiler/SoProfilingReportGenerator.cpp`
- `src/profiler/SoScrollingGraphKit.cpp`（`scoped_*`）
- `src/engines/SoHeightMapToNormalMap.cpp`
- `src/nodes/SoVertexAttribute.cpp`
- `src/xml/document.cpp`

- [ ] **Step 1: 替换 scxml + soscxml 的 `scoped_*`**

每个文件：`#include <boost/scoped_ptr.hpp>` / `scoped_array.hpp` → `#include <memory>`；类型按统一规则替换。注释里的 `//boost::scoped_ptr<ScXMLDocument> srcref` 改成 `// std::unique_ptr<ScXMLDocument> srcref`。

- [ ] **Step 2: 编译**

```bash
cmake --build build-gl3 --target Coin -j"$(sysctl -n hw.ncpu)"
```

Expected: PASS。失败则留在 scxml/soscxml 修。

- [ ] **Step 3: 替换 navigation + profiler + 其余 `scoped_*`**

同样规则。`SoScrollingGraphKit.cpp` 里 `scoped_array<SoBaseColor *>` 等是指针数组，必须变成 `std::unique_ptr<SoBaseColor *[]>`。

- [ ] **Step 4: 再编译**

```bash
cmake --build build-gl3 --target Coin -j"$(sysctl -n hw.ncpu)"
```

Expected: PASS。

---

### Task 6: `intrusive_ptr` → `SoRefPtr` / 自定义 deleter

**Files:**
- Modify: `src/soscxml/SoScXMLStateMachine.cpp`
- Modify: `src/navigation/SoScXMLRotateTarget.cpp`
- Modify: `src/navigation/SoScXMLSpinTarget.cpp`
- Modify: `src/navigation/SoScXMLZoomTarget.cpp`
- Modify: `src/profiler/SoProfilerTopKit.cpp`
- Modify: `src/profiler/SoScrollingGraphKit.cpp`
- Modify: `src/geo/SoGeoCoordinate.cpp`
- Modify: `src/geo/SoGeoSeparator.cpp`
- Modify: `src/scxml/ScXMLDocument.cpp`
- Modify: `include/Inventor/misc/SoBase.h:135-137`

- [ ] **Step 1: 替换 `SoBase` 派生类上的 `intrusive_ptr`**

例如 `SoProfilerTopKit.cpp`：

```cpp
#include <boost/intrusive_ptr.hpp>
boost::intrusive_ptr<SoCalculator> geometryEngine;
boost::intrusive_ptr<SoProfilerTopEngine> topListEngine;
```

改为：

```cpp
#include <Inventor/misc/SoRefPtr.h>
SoRefPtr<SoCalculator> geometryEngine;
SoRefPtr<SoProfilerTopEngine> topListEngine;
```

`SoScXMLStateMachine.cpp` 的 `scenegraphroot` / `activecamera`、navigation 里的 `SoCamera`、`SoScrollingGraphKit` 的 `SoSeparator`、geo 测试块里的节点，同一规则。

`SoRefPtr` 的 `explicit` 单参构造禁止隐式转换。成员赋值必须改 `reset`：

```cpp
// 错：SoRefPtr<SoCamera> camera = raw;
// 错：camera = raw;
camera.reset(raw);
SoRefPtr<SoCamera> camera(raw);
```

至少检查这些赋值：`topListEngine` / `geometryEngine`、`chart`、`scenegraphroot` / `activecamera`、`cameraclone`、`defaultcamera`。

- [ ] **Step 2: `cc_xml_doc` 不要用 `SoRefPtr`**

在 `ScXMLDocument.cpp` 用上游同款局部 deleter（放在匿名 namespace）：

```cpp
namespace {
struct cc_xml_doc_deleter {
  void operator()(cc_xml_doc * doc) const
  {
    if (doc) cc_xml_doc_delete_x(doc);
  }
};
typedef std::unique_ptr<cc_xml_doc, cc_xml_doc_deleter> cc_xml_doc_ptr;
}

// 删除 intrusive_ptr_add_ref / intrusive_ptr_release
// boost::intrusive_ptr<cc_xml_doc> xmldoc(cc_xml_doc_new());
cc_xml_doc_ptr xmldoc(cc_xml_doc_new());
```

`add_ref` 原本是空操作、`release` 调用 `cc_xml_doc_delete_x`，与 `unique_ptr` 语义一致。

- [ ] **Step 3: 删除 `SoBase.h` 钩子**

所有 `intrusive_ptr` 都改完后，再删：

```cpp
// support for boost::intrusive_ptr<SoBase>
inline void intrusive_ptr_add_ref(SoBase * obj) { obj->ref(); }
inline void intrusive_ptr_release(SoBase * obj) { obj->unref(); }
```

- [ ] **Step 4: 确认无残留**

```bash
rg -n 'intrusive_ptr|boost::' src include --glob '*.{h,hpp,cpp,c,cc,icc}'
```

Expected: 无 `intrusive_ptr`。若还有 `boost::`，应只剩 Task 7 的 lexical_cast / static_assert / current_function。`SbPimplPtr.h` / `SbLazyPimplPtr.h` 注释里的 “boost::pimpl_ptr” 不算残留，不要改。

- [ ] **Step 5: 编译**

```bash
cmake --build build-gl3 --target Coin -j"$(sysctl -n hw.ncpu)"
```

Expected: PASS。

---

### Task 7: 收尾宏与测试块

**Files:**
- Modify: `src/base/heap.cpp`
- Modify: `src/base/SbRotation.cpp`
- Modify: `src/base/SbVec3f.cpp`
- Modify: `src/base/SbVec3s.cpp`
- Modify: `src/base/SbVec4f.cpp`
- Modify: `src/io/SoInput.cpp`（`#ifdef __CYGWIN__` 里 **两处** `BOOST_STATIC_ASSERT`：`long int` 与 `unsigned long int`）
- Modify: `src/io/SoOutput.cpp`（对称 Cygwin 块，同样 **两处**）
- Modify: `src/rendering/SoOffscreenRenderer.cpp:347`、`:1582-1598`

- [ ] **Step 1: `lexical_cast` → `std::to_string`**

`heap.cpp` 顶层 `#include <boost/lexical_cast.hpp>` 移进 `#ifdef COIN_TEST_SUITE` 并改为 `#include <string>`，调用改为 `std::to_string(value->x)`。其余 `Sb*.cpp` 同样只改测试消息。

- [ ] **Step 2: Cygwin `static_assert`**

每个文件删掉 `#include <boost/static_assert.hpp>`，并把该 Cygwin 块里的两处都改掉：

```cpp
static_assert(sizeof(long int) == sizeof(int), "long int size");
static_assert(sizeof(unsigned long int) == sizeof(unsigned int), "unsigned long int size");
```

不需要额外头（C++11）。本机是 macOS，Cygwin 块不会被编到，漏改第二处 Task 9 发现不了，必须靠 grep 确认：

```bash
rg -n 'BOOST_STATIC_ASSERT|#include <boost/static_assert.hpp>' src/io
```

Expected: 无命中。

- [ ] **Step 3: `BOOST_CURRENT_FUNCTION`**

删除 `#include <boost/current_function.hpp>`。三处 `SoDebugError::post(BOOST_CURRENT_FUNCTION, ...)` 改为 `SoDebugError::post(COIN_STUB_FUNC, ...)`。该文件已包含 `coindefs.h`。

- [ ] **Step 4: 全库 Boost include 清零**

硬门禁只查 include（避免 `SbPimplPtr.h` 等注释里的 “boost::pimpl_ptr” 假失败）：

```bash
rg -n --glob '*.{h,hpp,cpp,c,cc,icc}' '#include <boost/' src include
```

Expected: 无命中。`docs/`、`configure`、`testsuite/` 仍可能有 Boost 字样，本任务不要求清 `testsuite/`。

辅助检查（允许注释命中）：

```bash
rg -n --glob '*.{h,hpp,cpp,c,cc,icc}' 'boost::' src include
```

Expected: 若有命中，只能是注释（如 “loosely based on boost::pimpl_ptr”），不能是代码。

---

### Task 8: 从 CMake 去掉 Boost

**Files:**
- Modify: `CMakeLists.txt:209-215`
- Modify: `CMakeLists.txt:559-560`（注释）
- Modify: `src/coin-config.cmake.in:86-89`
- Modify: `INSTALL` 的 Boost 小节

- [ ] **Step 1: 删除根 `CMakeLists.txt` 的 Boost 查找**

删掉整段：

```cmake
find_package(Boost REQUIRED) # almost any Boost will do
# Boost::boost - Target for header only dependencies (Boost include directory)
if(NOT TARGET Boost::boost)
  list(APPEND COIN_TARGET_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
else()
  list(APPEND COIN_TARGET_LINK_LIBRARIES Boost::boost)
endif()
```

不要动其后的 `COIN_BUILD_MAC_X11` / `COIN_GL_PROFILE` / `add_subdirectory(tools/glparity)`。

MSVC 注释从 “for code that uses boost in SbByteBuffer” 改成 “Enable C++ exception handling”，`/EHsc` 保留。

- [ ] **Step 2: 删除 `coin-config.cmake.in` 的 Boost 依赖传播**

删：

```cmake
	set(_Boost_FOUND @Boost_FOUND@)
	if(_Boost_FOUND)
		find_dependency(Boost)
	endif()
```

- [ ] **Step 3: 改 `INSTALL`**

删除 “Boost / Boost C++ libraries are needed...” 整节。不要改 CMake / Git / Doxygen 其它节。

---

### Task 9: 无 Boost 重新配置并编译 Coin

**Files:**
- 构建目录：建议新开 `build-gl3-noboost`，或删掉旧 `build-gl3/CMakeCache.txt` 后重配。旧 cache 里有 `Boost_DIR=/opt/homebrew/lib/cmake/Boost-1.90.0`，不重配会误判。

- [ ] **Step 1: 无 Boost configure（应成功）**

```bash
rm -rf build-gl3-noboost
cmake -S . -B build-gl3-noboost \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=17 \
  -DCOIN_GL_PROFILE=GL3 \
  -DCOIN_BUILD_SHARED_LIBS=ON \
  -DCOIN_BUILD_TESTS=OFF \
  -DCOIN_BUILD_DOCUMENTATION=OFF \
  -DCOIN_BUILD_EXAMPLES=OFF \
  -DCOIN_HAVE_JAVASCRIPT=OFF \
  -DHAVE_SOUND=OFF \
  -DHAVE_3DS_IMPORT_CAPABILITIES=OFF \
  -DUSE_EXTERNAL_EXPAT=OFF \
  -DUSE_EXCEPTIONS=OFF \
  -DUSE_SUPERGLU=OFF \
  -DCOIN_BUILD_SINGLE_LIB=ON \
  -DCMAKE_DISABLE_FIND_PACKAGE_Boost=ON
```

Expected: configure **成功**，日志中无 `Found Boost`。

- [ ] **Step 2: 编译 `Coin`**

```bash
cmake --build build-gl3-noboost --target Coin -j"$(sysctl -n hw.ncpu)"
```

Expected: 成功生成 `build-gl3-noboost/lib/libCoin.dylib`（或 `.so` / `Coin.lib`）。

- [ ] **Step 3: 确认运行时不链 Boost**

```bash
otool -L build-gl3-noboost/lib/libCoin.dylib
```

Expected: 无 `libboost_`。应仍有 OpenGL / CoreFoundation / libc++。

- [ ] **Step 4: 把控件构建指向新库**

本仓库 `SConstruct` / `3rd/build_coin.sh` 默认用 `3rd/coin/build-gl3`。二选一：

1. 用无 Boost 产物替换：`rm -rf build-gl3 && mv build-gl3-noboost build-gl3`
2. 或对已有 `build-gl3` 删 `CMakeCache.txt` 后按 `build_coin.sh` 同样参数加上 `-DCMAKE_DISABLE_FIND_PACKAGE_Boost=ON` 重配再编

推荐 1，避免旧 cache 残留 `Boost_DIR`。

---

### Task 10: 控件编译与 demo 冒烟

**Files:**
- 验证：`/Users/jim/work/awtk-root/awtk-widget-coin3d/SConstruct`
- 验证：`src/coin3d/coin3d_coin.cpp`（不应出现 Boost include）

- [ ] **Step 1: 确认控件源码仍不依赖 Boost**

工作目录必须是 **awtk-widget-coin3d 仓库根**，不要停在 `3rd/coin`（否则 `rg boost src` 会扫到库内 `BOOST_*` 测试宏，误判失败）：

```bash
rg -n 'boost' src SConstruct 3rd/build_coin.sh
```

Expected: 无 Boost 编译依赖。`build_coin.sh` 不必加新选项。

- [ ] **Step 2: 编译控件**

在仓库根（已能找到 awtk）：

```bash
scons
```

Expected: `bin/demo` 与 `libCoin` 拷贝成功。

- [ ] **Step 3: 冒烟**

```bash
./bin/demo
```

或至少加载默认 `rotating_cube.iv`。Expected: 窗口能出来、立方体能转，无动态库加载失败。

若无显示环境，退化为：

```bash
otool -L bin/demo | rg -i 'boost|Coin'
```

Expected: 有 `libCoin`，无 `libboost_`。

---

### Task 11: 文档索引（Coin 侧）

**Files:**
- Modify: `3rd/coin/INSTALL`（Task 8 已做）
- 不改控件 `README.md`，除非准备说明里写了 “需安装 Boost”——当前没有。

- [ ] **Step 1: 在 Coin `NEWS` 或本地变更记录加一句**

若 `opengl3` 分支维护本地 `NEWS` / `RELNOTES`，加一行：`Removed Boost header dependency from the CMake/library build (ported from upstream #596).` 不要改版本号（仍是 4.0.7 本地分支）。

---

## 风险与回退

- **`SoRefPtr` 双 `ref`：** 若某处先 `node->ref()` 再交给 `SoRefPtr(node)`，会多一次 `ref`。对照原 `intrusive_ptr`：它的构造同样 `add_ref`。保持与原来一致即可，不要额外 `ref()`。
- **`cc_xml_doc`：** 误用 `SoRefPtr` 会编不过或泄漏。必须走自定义 deleter。
- **`shared_ptr<char>` + `default_delete<char[]>`：** 必须配对，否则 `delete` / `delete[]` 不匹配。
- **旧 CMake cache：** 验收必须用新 build 目录或删除 cache。
- **打开 `COIN_BUILD_TESTS=ON`：** 本计划完成后仍会因 `testsuite/` 的 Boost.Test 失败。需要时另开任务移植上游 `CoinTest.h`。

回退：`3rd/coin` 是独立 git 仓库，按 commit 或 `git checkout --` 冲突文件即可。不要回退 GL3 无关改动。
