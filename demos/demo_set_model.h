/**
 * File:   demo_set_model.h
 * Author: AWTK Develop Team
 * Brief:  load model while keeping the current examine orbit
 *
 * Copyright (c) 2026 - 2026 Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 */

#ifndef DEMO_SET_MODEL_H
#define DEMO_SET_MODEL_H

#include <string.h>
#include "coin3d/coin3d.h"

BEGIN_C_DECLS

/**
 * 装入模型（viewAll）后恢复当前轨道角，避免 CLI / 下拉 / screenshot 回到正视。
 * 距离仍由 viewAll 决定，以便不同尺寸的模型都能入画。
 */
static inline ret_t demo_set_model_keep_orbit(widget_t* coin3d, const char* model) {
  value_t v;
  char rotation[COIN3D_PROP_STR_MAX];

  memset(&v, 0, sizeof(v));
  memset(rotation, 0, sizeof(rotation));
  return_value_if_fail(coin3d != NULL && model != NULL && model[0] != '\0', RET_BAD_PARAMS);

  if (widget_get_prop(coin3d, COIN3D_PROP_ROTATION, &v) == RET_OK && value_str(&v) != NULL) {
    tk_strncpy(rotation, value_str(&v), sizeof(rotation) - 1);
  }

  return_value_if_fail(coin3d_set_model(coin3d, model) == RET_OK, RET_FAIL);

  if (rotation[0] != '\0') {
    return coin3d_set_rotation(coin3d, rotation);
  }

  return RET_OK;
}

END_C_DECLS

#endif /*DEMO_SET_MODEL_H*/
