/**
 * File:   coin3d_gizmo.h
 * Author: AWTK Develop Team
 * Brief:  View orientation gizmo for coin3d
 *
 * Copyright (c) 2026 - 2026 Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 */

/**
 * History:
 * ================================================================
 * 2026-08-18 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_COIN3D_GIZMO_H
#define TK_COIN3D_GIZMO_H

#include "base/widget.h"

BEGIN_C_DECLS

struct _coin3d_coin_t;
typedef struct _coin3d_coin_t coin3d_coin_t;

/**
 * Gizmo 默认边长（像素）。
 */
#define COIN3D_GIZMO_SIZE 96

/**
 * Gizmo 与控件边缘的边距（像素）。
 */
#define COIN3D_GIZMO_MARGIN 12

/**
 * @enum coin3d_gizmo_pick_t
 * @prefix COIN3D_GIZMO_PICK_
 * Gizmo 命中结果。
 */
typedef enum _coin3d_gizmo_pick_t {
  COIN3D_GIZMO_PICK_NONE = 0,
  COIN3D_GIZMO_PICK_DISK,
  COIN3D_GIZMO_PICK_POS_X,
  COIN3D_GIZMO_PICK_NEG_X,
  COIN3D_GIZMO_PICK_POS_Y,
  COIN3D_GIZMO_PICK_NEG_Y,
  COIN3D_GIZMO_PICK_POS_Z,
  COIN3D_GIZMO_PICK_NEG_Z
} coin3d_gizmo_pick_t;

/**
 * @class coin3d_gizmo_t
 * 视角导航 Gizmo 状态。
 */
typedef struct _coin3d_gizmo_t {
  bool_t visible;
  bool_t dragging;
  bool_t captured;
  float last_x;
  float last_y;
  coin3d_gizmo_pick_t hover;
} coin3d_gizmo_t;

/**
 * @method coin3d_gizmo_init
 * 初始化 Gizmo，默认不可见。
 * @param {coin3d_gizmo_t*} gizmo Gizmo。
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_gizmo_init(coin3d_gizmo_t* gizmo);

/**
 * @method coin3d_gizmo_calc_rect
 * 计算右上角 Gizmo 矩形（控件局部坐标）。
 * @param {wh_t} widget_w 控件宽。
 * @param {wh_t} widget_h 控件高。
 * @param {rect_t*} r 输出矩形。
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_gizmo_calc_rect(wh_t widget_w, wh_t widget_h, rect_t* r);

/**
 * @method coin3d_gizmo_pick_at
 * 根据已投影的轴端点做命中测试。
 * @param {const rect_t*} r Gizmo 矩形。
 * @param {const float*} points_x 6 个轴端点 X。
 * @param {const float*} points_y 6 个轴端点 Y。
 * @param {const float*} depth 6 个轴深度（>0 朝向相机）。
 * @param {float} x 局部 X。
 * @param {float} y 局部 Y。
 * @return {coin3d_gizmo_pick_t} 命中结果。
 */
coin3d_gizmo_pick_t coin3d_gizmo_pick_at(const rect_t* r, const float* points_x,
                                         const float* points_y, const float* depth, float x,
                                         float y);

/**
 * @method coin3d_gizmo_draw
 * 在控件上绘制 Gizmo。
 * @param {coin3d_gizmo_t*} gizmo Gizmo。
 * @param {widget_t*} widget 宿主控件。
 * @param {canvas_t*} c 画布。
 * @param {coin3d_coin_t*} coin Coin 后端。
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_gizmo_draw(coin3d_gizmo_t* gizmo, widget_t* widget, canvas_t* c, coin3d_coin_t* coin);

/**
 * @method coin3d_gizmo_on_event
 * 处理指针事件；已处理返回 TRUE。
 * @param {coin3d_gizmo_t*} gizmo Gizmo。
 * @param {widget_t*} widget 宿主控件。
 * @param {event_t*} e 事件。
 * @param {coin3d_coin_t*} coin Coin 后端。
 * @return {bool_t} 已处理返回 TRUE。
 */
bool_t coin3d_gizmo_on_event(coin3d_gizmo_t* gizmo, widget_t* widget, event_t* e,
                             coin3d_coin_t* coin);

END_C_DECLS

#endif /*TK_COIN3D_GIZMO_H*/
