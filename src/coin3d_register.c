/**
 * File:   coin3d.c
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
 *
 */

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "coin3d_register.h"
#include "base/widget_factory.h"
#include "coin3d/coin3d.h"

ret_t coin3d_register(void) {
  return widget_factory_register(widget_factory(), WIDGET_TYPE_COIN3D, coin3d_create);
}

const char* coin3d_supported_render_mode(void) {
  return "OpenGL|AGGE-BGR565|AGGE-BGRA8888|AGGE-MONO";
}
