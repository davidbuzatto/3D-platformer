#include <stdlib.h>
#include <math.h>

#include "raylib/raylib.h"

#include "MapPiece.h"
#include "MoveAnchor.h"
#include "MoveAnchorPlane.h"

void updateMoveAnchor( MoveAnchor *ma, Vector3 mapPiecePos, Vector3 moveAnchorOffset ) {

    ma->pos.x = mapPiecePos.x + moveAnchorOffset.x;
    ma->pos.y = mapPiecePos.y + moveAnchorOffset.y;
    ma->pos.z = mapPiecePos.z + moveAnchorOffset.z;

    ma->xymp.pos = (Vector3) { 
        ma->pos.x, 
        ma->pos.y + MOVE_PLANE_BIG_SIZE / 2 + MOVE_PLANE_SMALL_SIZE / 2, 
        ma->pos.z - ( MOVE_PLANE_BIG_SIZE / 2 + MOVE_PLANE_SMALL_SIZE / 2 )
    };

    ma->xzmp.pos = (Vector3) { 
        ma->pos.x, 
        ma->pos.y, 
        ma->pos.z
    };

    ma->yzmp.pos = (Vector3) { 
        ma->pos.x + MOVE_PLANE_BIG_SIZE / 2 + MOVE_PLANE_SMALL_SIZE / 2, 
        ma->pos.y + MOVE_PLANE_BIG_SIZE / 2 + MOVE_PLANE_SMALL_SIZE / 2,
        ma->pos.z
    };

}

void drawMoveAnchor( MoveAnchor *ma ) {

    const float alpha = 0.5f;

    if ( ma->xymp.selected ) {
        DrawCubeV( ma->xymp.pos, ma->xymp.dim, ma->xymp.color );
        DrawCubeWiresV( ma->xymp.pos, ma->xymp.dim, BLACK );
    } else {
        DrawCubeV( ma->xymp.pos, ma->xymp.dim, Fade( ma->xymp.color, alpha ) );
        DrawCubeWiresV( ma->xymp.pos, ma->xymp.dim, Fade( BLACK, alpha ) );
    }

    if ( ma->xzmp.selected ) {
        DrawCubeV( ma->xzmp.pos, ma->xzmp.dim, ma->xzmp.color );
        DrawCubeWiresV( ma->xzmp.pos, ma->xzmp.dim, BLACK );
    } else {
        DrawCubeV( ma->xzmp.pos, ma->xzmp.dim, Fade( ma->xzmp.color, alpha ) );
        DrawCubeWiresV( ma->xzmp.pos, ma->xzmp.dim, Fade( BLACK, alpha ) );
    }

    if ( ma->yzmp.selected ) {
        DrawCubeV( ma->yzmp.pos, ma->yzmp.dim, ma->yzmp.color );
        DrawCubeWiresV( ma->yzmp.pos, ma->yzmp.dim, BLACK );
    } else {
        DrawCubeV( ma->yzmp.pos, ma->yzmp.dim, Fade( ma->yzmp.color, alpha ) );
        DrawCubeWiresV( ma->yzmp.pos, ma->yzmp.dim, Fade( BLACK, alpha ) );
    }

}

MoveAnchorCollisionType checkCollisionMouseMoveAnchor( MoveAnchor *ma, Camera3D *camera ) {

    Ray ray = GetScreenToWorldRay( GetMousePosition(), *camera );

    RayCollision xyCol = GetRayCollisionBox( 
        ray,
        (BoundingBox) {
            .min = { 
                ma->xymp.pos.x - ma->movePlaneBigSize / 2,
                ma->xymp.pos.y - ma->movePlaneBigSize / 2,
                ma->xymp.pos.z - ma->movePlaneSmallSize / 2,
            },
            .max = {
                ma->xymp.pos.x + ma->movePlaneBigSize / 2,
                ma->xymp.pos.y + ma->movePlaneBigSize / 2,
                ma->xymp.pos.z + ma->movePlaneSmallSize / 2,
            },
        }
    );

    RayCollision xzCol = GetRayCollisionBox( 
        ray,
        (BoundingBox) {
            .min = { 
                ma->xzmp.pos.x - ma->movePlaneBigSize / 2,
                ma->xzmp.pos.y - ma->movePlaneSmallSize / 2,
                ma->xzmp.pos.z - ma->movePlaneBigSize / 2,
            },
            .max = {
                ma->xzmp.pos.x + ma->movePlaneBigSize / 2,
                ma->xzmp.pos.y + ma->movePlaneSmallSize / 2,
                ma->xzmp.pos.z + ma->movePlaneBigSize / 2,
            },
        }
    );

    RayCollision yzCol = GetRayCollisionBox( 
        ray,
        (BoundingBox) {
            .min = { 
                ma->yzmp.pos.x - ma->movePlaneSmallSize / 2,
                ma->yzmp.pos.y - ma->movePlaneBigSize / 2,
                ma->yzmp.pos.z - ma->movePlaneBigSize / 2,
            },
            .max = {
                ma->yzmp.pos.x + ma->movePlaneSmallSize / 2,
                ma->yzmp.pos.y + ma->movePlaneBigSize / 2,
                ma->yzmp.pos.z + ma->movePlaneBigSize / 2,
            },
        }
    );

    float minDist = INFINITY;
    MoveAnchorCollisionType collType = MOVE_ANCHOR_COLLISION_TYPE_NONE;

    float xyDist = INFINITY;
    float xzDist = INFINITY;
    float yzDist = INFINITY;

    if ( xyCol.hit ) xyDist = xyCol.distance;
    if ( xzCol.hit ) xzDist = xzCol.distance;
    if ( yzCol.hit ) yzDist = yzCol.distance;

    if ( xyDist < minDist ) { 
        minDist = xyDist;
        collType = MOVE_ANCHOR_COLLISION_TYPE_XY;
    }

    if ( xzDist < minDist ) {
        minDist = xzDist;
        collType = MOVE_ANCHOR_COLLISION_TYPE_XZ;
    }

    if ( yzDist < minDist ) {
        minDist = yzDist;
        collType = MOVE_ANCHOR_COLLISION_TYPE_YZ;
    }

    return collType;

}