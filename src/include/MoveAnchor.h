#pragma once

#include "raylib/raylib.h"
#include "MoveAnchorPlane.h"

typedef struct {

    Vector3 pos;
    
    float movePlaneBigSize;
    float movePlaneSmallSize;

    MoveAnchorPlane xymp;
    MoveAnchorPlane xzmp;
    MoveAnchorPlane yzmp;

} MoveAnchor;