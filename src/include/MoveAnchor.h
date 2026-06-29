#pragma once

#include "raylib/raylib.h"
#include "MoveAnchorPlane.h"

typedef enum MoveAnchorCollisionType {
    MOVE_ANCHOR_COLLISION_TYPE_NONE,
    MOVE_ANCHOR_COLLISION_TYPE_XY,
    MOVE_ANCHOR_COLLISION_TYPE_XZ,
    MOVE_ANCHOR_COLLISION_TYPE_YZ
} MoveAnchorCollisionType;

typedef struct {

    Vector3 pos;

    float movePlaneBigSize;
    float movePlaneSmallSize;

    MoveAnchorPlane xymp;
    MoveAnchorPlane xzmp;
    MoveAnchorPlane yzmp;

} MoveAnchor;