/**
 * File:   coin3d_coin.h
 * Author: AWTK Develop Team
 * Brief:  Coin3D backend (C API wrapper)
 *
 * Copyright (c) 2026 - 2026 Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 */

/**
 * History:
 * ================================================================
 * 2026-08-17 Li XianJing <xianjimli@hotmail.com> created
 * 2026-08-18 Li XianJing <xianjimli@hotmail.com> node find/move/rotate/resize
 * 2026-08-19 Li XianJing <xianjimli@hotmail.com> File node include other .iv
 * 2026-08-20 Li XianJing <xianjimli@hotmail.com> get_background_rgb for opaque FBO clear
 *
 */

#ifndef TK_COIN3D_COIN_H
#define TK_COIN3D_COIN_H

#include "base/widget.h"

BEGIN_C_DECLS

/**
 * @class coin3d_coin_t
 * Coin 场景与渲染状态（不透明）。
 */
typedef struct _coin3d_coin_t coin3d_coin_t;

/**
 * @method coin3d_coin_create
 * 创建 Coin 后端。
 * @return {coin3d_coin_t*} 后端对象。
 */
coin3d_coin_t* coin3d_coin_create(void);

/**
 * @method coin3d_coin_destroy
 * 销毁 Coin 后端。
 * @param {coin3d_coin_t*} coin 后端。
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_coin_destroy(coin3d_coin_t* coin);

/**
 * @method coin3d_coin_ensure_ready
 * 确保 SoDB / SceneManager 已初始化，并装载内置或指定场景。
 * @param {coin3d_coin_t*} coin 后端。
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_coin_ensure_ready(coin3d_coin_t* coin);

/**
 * @method coin3d_coin_load_model
 * 从本地路径或 AWTK 资源加载 .iv 场景；`File` 可引用同目录其它文件。失败则回退内置立方体。
 * @param {coin3d_coin_t*} coin 后端。
 * @param {const char*} model 资源名或 `.iv` 路径（如 cube.iv / cube_iv）。
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_coin_load_model(coin3d_coin_t* coin, const char* model);

/**
 * @method coin3d_coin_set_background
 * 设置背景色字符串（支持 color_parse 格式）。
 * @param {coin3d_coin_t*} coin 后端。
 * @param {const char*} background 颜色字符串。
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_coin_set_background(coin3d_coin_t* coin, const char* background);

/**
 * @method coin3d_coin_get_background_rgb
 * 读取场景背景色（0–1）。
 * @param {coin3d_coin_t*} coin 后端。
 * @param {float*} r 输出红色。
 * @param {float*} g 输出绿色。
 * @param {float*} b 输出蓝色。
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_coin_get_background_rgb(coin3d_coin_t* coin, float* r, float* g, float* b);

/**
 * @method coin3d_coin_set_viewport
 * 按控件尺寸设置 SceneManager 视口。
 * @param {coin3d_coin_t*} coin 后端。
 * @param {widget_t*} widget 控件。
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_coin_set_viewport(coin3d_coin_t* coin, widget_t* widget);

/**
 * @method coin3d_coin_render
 * 渲染当前场景（调用方负责 GL 状态保存/恢复）。
 * @param {coin3d_coin_t*} coin 后端。
 * @return {ret_t} 返回 RET_OK 表示成功。
 */
ret_t coin3d_coin_render(coin3d_coin_t* coin);

/**
 * @method coin3d_coin_process_sensors
 * 处理 Coin timer/delay 队列。
 * @param {coin3d_coin_t*} coin 后端。
 * @return {bool_t} 若需要重绘返回 TRUE。
 */
bool_t coin3d_coin_process_sensors(coin3d_coin_t* coin);

/**
 * @method coin3d_coin_handle_pointer
 * Examine 交互：处理指针/滚轮事件。
 * @param {coin3d_coin_t*} coin 后端。
 * @param {widget_t*} widget 控件。
 * @param {event_t*} e 事件。
 * @return {ret_t} 返回 RET_OK 表示已处理。
 */
ret_t coin3d_coin_handle_pointer(coin3d_coin_t* coin, widget_t* widget, event_t* e);

/**
 * @method coin3d_coin_find_node
 * 按 DEF 名称查找场景节点。
 * @param {coin3d_coin_t*} coin 后端。
 * @param {const char*} name 节点名。
 * @return {void*} 节点句柄，未找到返回 NULL。
 */
void* coin3d_coin_find_node(coin3d_coin_t* coin, const char* name);

/**
 * @method coin3d_coin_node_move
 * 设置节点平移。
 */
ret_t coin3d_coin_node_move(coin3d_coin_t* coin, const char* name, float x, float y, float z);

/**
 * @method coin3d_coin_node_rotate
 * 设置节点旋转（度，XYZ 欧拉角）。
 */
ret_t coin3d_coin_node_rotate(coin3d_coin_t* coin, const char* name, float x, float y, float z);

/**
 * @method coin3d_coin_node_resize
 * 设置节点缩放。
 */
ret_t coin3d_coin_node_resize(coin3d_coin_t* coin, const char* name, float x, float y, float z);

/**
 * @method coin3d_coin_node_get_translation
 * 读取节点平移。
 */
ret_t coin3d_coin_node_get_translation(coin3d_coin_t* coin, const char* name, float* x, float* y,
                                      float* z);

/**
 * @method coin3d_coin_node_get_rotation
 * 读取节点旋转（度，XYZ 欧拉角）。
 */
ret_t coin3d_coin_node_get_rotation(coin3d_coin_t* coin, const char* name, float* x, float* y,
                                   float* z);

/**
 * @method coin3d_coin_node_get_scale
 * 读取节点缩放。
 */
ret_t coin3d_coin_node_get_scale(coin3d_coin_t* coin, const char* name, float* x, float* y, float* z);

/**
 * @method coin3d_coin_count_indexed_face_sets
 * 统计当前场景 `SoIndexedFaceSet` 数量（单元测试用）。
 */
uint32_t coin3d_coin_count_indexed_face_sets(coin3d_coin_t* coin);

END_C_DECLS

#endif /*TK_COIN3D_COIN_H*/
