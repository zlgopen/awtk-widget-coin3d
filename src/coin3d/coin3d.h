/**
 * File:   coin3d.h
 * Author: AWTK Develop Team
 * Brief:  awtk widget coin3d
 *
 * Copyright (c) 2026 - 2026 Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2026-08-16 Li XianJing <xianjimli@hotmail.com> created
 * 2026-08-17 Li XianJing <xianjimli@hotmail.com> integrate Coin3D
 * 2026-08-18 Li XianJing <xianjimli@hotmail.com> camera pan/rotate/zoom props
 * 2026-08-18 Li XianJing <xianjimli@hotmail.com> node find/move/rotate/resize
 * 2026-08-19 Li XianJing <xianjimli@hotmail.com> set_model viewAll without camera props
 *
 */

#ifndef TK_COIN3D_H
#define TK_COIN3D_H

#include "base/widget.h"
#include "coin3d_gl.h"
#include "coin3d_coin.h"
#include "coin3d_gizmo.h"

BEGIN_C_DECLS

/**
 * 场景资源名属性。
 */
#define COIN3D_PROP_MODEL "model"

/**
 * 背景色属性。
 */
#define COIN3D_PROP_BACKGROUND "background"

/**
 * 是否显示视角导航 Gizmo。
 */
#define COIN3D_PROP_GIZMO "gizmo"

/**
 * 观察点（look-at），格式 "x,y,z"。
 */
#define COIN3D_PROP_TRANSLATION "translation"

/**
 * 绕观察点的轨道角（度），格式 "pitch,yaw"。
 */
#define COIN3D_PROP_ROTATION "rotation"

/**
 * 相机到观察点的距离。
 */
#define COIN3D_PROP_SCALE "scale"

#define COIN3D_PROP_STR_MAX 64

/**
 * @class coin3d_t
 * @parent widget_t
 * @annotation ["scriptable","design","widget"]
 * awtk widget coin3d
 * 在xml中使用"coin3d"标签创建控件。如：
 *
 * ```xml
 * <!-- ui -->
 * <coin3d x="c" y="10" w="90%" h="-100" model="cube.iv" background="#1e2430"
 *   gizmo="true" translation="0,0,0" rotation="20,30" scale="8"/>
 * ```
 *
 * 可用通过style来设置控件的显示风格，如字体的大小和颜色等等。如：
 *
 * ```xml
 * <!-- style -->
 * <coin3d>
 *   <style name="default" font_size="32">
 *     <normal text_color="black" />
 *   </style>
 * </coin3d>
 * ```
 */
typedef struct _coin3d_t {
  widget_t widget;

  /**
   * @property {char*} model
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 场景资源名。
   */
  char* model;

  /**
   * @property {char*} background
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 背景色。
   */
  char* background;

  /**
   * @property {bool_t} gizmo
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 是否显示视角导航 Gizmo。
   */
  bool_t gizmo;

  /**
   * @property {char*} translation
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 观察点，格式 "x,y,z"。
   */
  char translation[COIN3D_PROP_STR_MAX];

  /**
   * @property {char*} rotation
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 轨道角（度），格式 "pitch,yaw"。
   */
  char rotation[COIN3D_PROP_STR_MAX];

  /**
   * @property {float} scale
   * @annotation ["set_prop","get_prop","readable","persitent","design","scriptable"]
   * 相机到观察点的距离。
   */
  float scale;

  /**
   * 视角导航 Gizmo 状态。
   */
  coin3d_gizmo_t gizmo_state;

  /**
   * OpenGL 绘制状态。
   */
  coin3d_gl_t gl;

  /**
   * Coin 后端。
   */
  coin3d_coin_t* coin;

  /**
   * sensor 定时器 id。
   */
  uint32_t timer_id;
} coin3d_t;

/**
 * @method coin3d_create
 * @annotation ["constructor", "scriptable"]
 * 创建coin3d对象
 * @param {widget_t*} parent 父控件
 * @param {xy_t} x x坐标
 * @param {xy_t} y y坐标
 * @param {wh_t} w 宽度
 * @param {wh_t} h 高度
 *
 * @return {widget_t*} coin3d对象。
 */
widget_t* coin3d_create(widget_t* parent, xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * @method coin3d_cast
 * 转换为coin3d对象(供脚本语言使用)。
 * @annotation ["cast", "scriptable"]
 * @param {widget_t*} widget coin3d对象。
 *
 * @return {widget_t*} coin3d对象。
 */
widget_t* coin3d_cast(widget_t* widget);

/**
 * @method coin3d_set_model
 * 设置场景资源名。装入后会 viewAll，不保留此前的 translation / rotation / scale。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {const char*} model 资源名。
 *
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_set_model(widget_t* widget, const char* model);

/**
 * @method coin3d_set_background
 * 设置背景色。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {const char*} background 颜色字符串。
 *
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_set_background(widget_t* widget, const char* background);

/**
 * @method coin3d_set_gizmo
 * 设置是否显示视角导航 Gizmo。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {bool_t} gizmo 是否显示。
 *
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_set_gizmo(widget_t* widget, bool_t gizmo);

/**
 * @method coin3d_set_translation
 * 设置观察点，格式 "x,y,z"。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {const char*} translation 观察点字符串。
 *
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_set_translation(widget_t* widget, const char* translation);

/**
 * @method coin3d_set_rotation
 * 设置轨道角（度），格式 "pitch,yaw"。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {const char*} rotation 轨道角字符串。
 *
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_set_rotation(widget_t* widget, const char* rotation);

/**
 * @method coin3d_set_scale
 * 设置相机到观察点的距离。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {float} scale 距离。
 *
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_set_scale(widget_t* widget, float scale);

/**
 * @method coin3d_pan
 * 沿相机视平面平移观察点（世界长度：dx 向右，dy 向上）。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {float} dx 向右偏移。
 * @param {float} dy 向上偏移。
 *
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_pan(widget_t* widget, float dx, float dy);

/**
 * @method coin3d_rotate
 * 绕观察点旋转（度：dx 为 pitch，dy 为 yaw）。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {float} dx pitch 增量（度）。
 * @param {float} dy yaw 增量（度）。
 *
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_rotate(widget_t* widget, float dx, float dy);

/**
 * @method coin3d_zoom
 * 增减相机到观察点的距离，小于下限时钳制。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {float} delta 距离增量。
 *
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_zoom(widget_t* widget, float delta);

/**
 * @method coin3d_find_node
 * 按 DEF 名称查找场景节点。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {const char*} name 节点名（对应 .iv 中的 DEF）。
 *
 * @return {void*} 节点句柄，未找到返回 NULL。
 */
void* coin3d_find_node(widget_t* widget, const char* name);

/**
 * @method coin3d_node_move
 * 设置节点平移。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {const char*} name 节点名。
 * @param {float} x X 平移。
 * @param {float} y Y 平移。
 * @param {float} z Z 平移。
 *
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_node_move(widget_t* widget, const char* name, float x, float y, float z);

/**
 * @method coin3d_node_rotate
 * 设置节点旋转（度，XYZ 欧拉角）。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {const char*} name 节点名。
 * @param {float} x 绕 X 轴角度（度）。
 * @param {float} y 绕 Y 轴角度（度）。
 * @param {float} z 绕 Z 轴角度（度）。
 *
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_node_rotate(widget_t* widget, const char* name, float x, float y, float z);

/**
 * @method coin3d_node_resize
 * 设置节点缩放。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {const char*} name 节点名。
 * @param {float} x X 缩放。
 * @param {float} y Y 缩放。
 * @param {float} z Z 缩放。
 *
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_node_resize(widget_t* widget, const char* name, float x, float y, float z);

/**
 * @method coin3d_node_get_translation
 * 读取节点平移。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {const char*} name 节点名。
 * @param {float*} x 返回 X。
 * @param {float*} y 返回 Y。
 * @param {float*} z 返回 Z。
 *
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_node_get_translation(widget_t* widget, const char* name, float* x, float* y, float* z);

/**
 * @method coin3d_node_get_rotation
 * 读取节点旋转（度，XYZ 欧拉角）。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {const char*} name 节点名。
 * @param {float*} x 返回绕 X 轴角度（度）。
 * @param {float*} y 返回绕 Y 轴角度（度）。
 * @param {float*} z 返回绕 Z 轴角度（度）。
 *
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_node_get_rotation(widget_t* widget, const char* name, float* x, float* y, float* z);

/**
 * @method coin3d_node_get_scale
 * 读取节点缩放。
 * @annotation ["scriptable"]
 * @param {widget_t*} widget 控件对象。
 * @param {const char*} name 节点名。
 * @param {float*} x 返回 X 缩放。
 * @param {float*} y 返回 Y 缩放。
 * @param {float*} z 返回 Z 缩放。
 *
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_node_get_scale(widget_t* widget, const char* name, float* x, float* y, float* z);

#define WIDGET_TYPE_COIN3D "coin3d"

#define COIN3D(widget) ((coin3d_t*)(coin3d_cast(WIDGET(widget))))

/*public for subclass and runtime type check*/
TK_EXTERN_VTABLE(coin3d);

END_C_DECLS

#endif /*TK_COIN3D_H*/
