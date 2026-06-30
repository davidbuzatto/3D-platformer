#pragma once

#include "raylib/raylib.h"

#include "Entity.h"
#include "MoveAnchorPlane.h"

#define MOVE_PLANE_BIG_SIZE 0.3f
#define MOVE_PLANE_SMALL_SIZE 0.05f

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

void updateMoveAnchor( MoveAnchor *ma, Vector3 entityPos, Vector3 moveAnchorOffset );
void drawMoveAnchor( MoveAnchor *ma );
MoveAnchorCollisionType checkCollisionMouseMoveAnchor( MoveAnchor *ma, Camera3D *camera );