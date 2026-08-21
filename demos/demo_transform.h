/**
 * File:   demo_transform.h
 * Author: AWTK Develop Team
 * Brief:  demo pan/rotate/zoom button deltas
 *
 * Copyright (c) 2026 - 2026 Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 */

#ifndef DEMO_TRANSFORM_H
#define DEMO_TRANSFORM_H

#include "tkc/types_def.h"
#include "tkc/utils.h"

BEGIN_C_DECLS

/**
 * 按钮按物体在屏幕上的移动方向。
 * coin3d_pan 平移的是观察点：观察点右移时物体在画面上左移，故符号与观察点相反。
 */
static inline ret_t demo_pan_delta(const char* name, float step, float* dx, float* dy) {
  return_value_if_fail(name != NULL && dx != NULL && dy != NULL, RET_BAD_PARAMS);

  *dx = 0.0f;
  *dy = 0.0f;
  if (tk_str_eq(name, "pan_left")) {
    *dx = step;
    return RET_OK;
  }
  if (tk_str_eq(name, "pan_right")) {
    *dx = -step;
    return RET_OK;
  }
  if (tk_str_eq(name, "pan_up")) {
    *dy = -step;
    return RET_OK;
  }
  if (tk_str_eq(name, "pan_down")) {
    *dy = step;
    return RET_OK;
  }

  return RET_NOT_FOUND;
}

END_C_DECLS

#endif /*DEMO_TRANSFORM_H*/
