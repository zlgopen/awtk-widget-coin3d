/**
 * File:   coin3d_gl.h
 * Author: AWTK Develop Team
 * Brief:  coin3d OpenGL 绘制与状态保护
 *
 * Copyright (c) 2026 - 2026 Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 */

/**
 * History:
 * ================================================================
 * 2026-08-16 Li XianJing <xianjimli@hotmail.com> created
 * 2026-08-17 Li XianJing <xianjimli@hotmail.com> rewrite for Coin backend
 *
 */

#ifndef TK_COIN3D_GL_H
#define TK_COIN3D_GL_H

#include "base/widget.h"
#include "coin3d_coin.h"

BEGIN_C_DECLS

/**
 * @class coin3d_gl_t
 * coin3d 的 OpenGL 绘制辅助状态。
 */
typedef struct _coin3d_gl_t {
  /**
   * 是否已加载 GL 入口。
   */
  uint8_t ready;
} coin3d_gl_t;

/**
 * @method coin3d_gl_init
 * 初始化 OpenGL 辅助状态。
 * @param {coin3d_gl_t*} gl OpenGL 状态。
 *
 * @return {ret_t} 返回 RET_OK 表示成功，否则表示失败。
 */
ret_t coin3d_gl_init(coin3d_gl_t* gl);

/**
 * @method coin3d_gl_get_widget_gl_rect
 * 计算控件在 OpenGL 坐标系中的矩形（原点在左下）。
 * @param {widget_t*} widget 控件。
 * @param {int32_t*} x 输出 x。
 * @param {int32_t*} y 输出 y。
 * @param {int32_t*} w 输出宽度。
 * @param {int32_t*} h 输出高度。
 *
 * @return {ret_t} 返回 RET_OK 表示成功，否则表示失败。
 */
ret_t coin3d_gl_get_widget_gl_rect(widget_t* widget, int32_t* x, int32_t* y, int32_t* w, int32_t* h);

/**
 * @method coin3d_gl_paint
 * 在控件区域内用 Coin 渲染场景。
 * @param {coin3d_gl_t*} gl OpenGL 状态。
 * @param {coin3d_coin_t*} coin Coin 后端。
 * @param {widget_t*} widget 控件。
 * @param {canvas_t*} c canvas。
 *
 * @return {ret_t} 返回 RET_OK 表示成功，否则表示失败。
 */
ret_t coin3d_gl_paint(coin3d_gl_t* gl, coin3d_coin_t* coin, widget_t* widget, canvas_t* c);

/**
 * @method coin3d_gl_deinit
 * 释放 OpenGL 辅助状态。
 * @param {coin3d_gl_t*} gl OpenGL 状态。
 *
 * @return {ret_t} 返回 RET_OK 表示成功，否则表示失败。
 */
ret_t coin3d_gl_deinit(coin3d_gl_t* gl);

END_C_DECLS

#endif /*TK_COIN3D_GL_H*/
