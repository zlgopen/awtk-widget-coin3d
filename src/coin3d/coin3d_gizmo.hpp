/**
 * File:   coin3d_gizmo.hpp
 * Author: AWTK Develop Team
 * Brief:  View orientation gizmo helpers
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

#ifndef TK_COIN3D_GIZMO_HPP
#define TK_COIN3D_GIZMO_HPP

#include "coin3d_gizmo.h"

class SoCamera;
class Coin3dView;
struct _coin3d_coin_t;

SoCamera* coin3d_coin_get_camera(coin3d_coin_t* coin);
Coin3dView* coin3d_coin_get_view(coin3d_coin_t* coin);

ret_t coin3d_gizmo_project_axes(SoCamera* camera, const rect_t* r, float* points_x, float* points_y,
                                float* depth);
ret_t coin3d_gizmo_snap_camera(SoCamera* camera, coin3d_gizmo_pick_t pick);

#endif /*TK_COIN3D_GIZMO_HPP*/
