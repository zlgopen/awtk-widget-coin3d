/**
 * File:   demo_screenshot.h
 * Author: AWTK Develop Team
 * Brief:  capture the window via widget_take_snapshot (GPU FBO with depth)
 *
 * Copyright (c) 2026 - 2026 Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 */

#ifndef DEMO_SCREENSHOT_H
#define DEMO_SCREENSHOT_H

#include "awtk.h"

BEGIN_C_DECLS

/**
 * 截取当前窗口。GPU 路径走 widget_take_snapshot，FBO 带深度，Coin 深度测试有效。
 */
static inline bitmap_t* demo_capture_displayed_window(widget_t* widget) {
  return_value_if_fail(widget != NULL, NULL);
  return widget_take_snapshot(widget);
}

END_C_DECLS

#endif /*DEMO_SCREENSHOT_H*/
