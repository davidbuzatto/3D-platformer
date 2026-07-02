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

    ma->xmp.pos = (Vector3) { 
        ma->pos.x, 
        ma->pos.y + MOVE_PLANE_BIG_SIZE / 2 + MOVE_PLANE_SMALL_SIZE / 2, 
        ma->pos.z - ( MOVE_PLANE_BIG_SIZE / 2 + MOVE_PLANE_SMALL_SIZE / 2 )
    };

    ma->zmp.pos = (Vector3) { 
        ma->pos.x, 
        ma->pos.y, 
        ma->pos.z
    };

    ma->ymp.pos = (Vector3) { 
        ma->pos.x + MOVE_PLANE_BIG_SIZE / 2 + MOVE_PLANE_SMALL_SIZE / 2, 
        ma->pos.y + MOVE_PLANE_BIG_SIZE / 2 + MOVE_PLANE_SMALL_SIZE / 2,
        ma->pos.z
    };

}

void drawMoveAnchor( MoveAnchor *ma ) {

    const float alpha = 0.5f;

    if ( ma->xmp.selected ) {
        DrawCubeV( ma->xmp.pos, ma->xmp.dim, ma->xmp.color );
        DrawCubeWiresV( ma->xmp.pos, ma->xmp.dim, BLACK );
    } else {
        DrawCubeV( ma->xmp.pos, ma->xmp.dim, Fade( ma->xmp.color, alpha ) );
        DrawCubeWiresV( ma->xmp.pos, ma->xmp.dim, Fade( BLACK, alpha ) );
    }

    if ( ma->zmp.selected ) {
        DrawCubeV( ma->zmp.pos, ma->zmp.dim, ma->zmp.color );
        DrawCubeWiresV( ma->zmp.pos, ma->zmp.dim, BLACK );
    } else {
        DrawCubeV( ma->zmp.pos, ma->zmp.dim, Fade( ma->zmp.color, alpha ) );
        DrawCubeWiresV( ma->zmp.pos, ma->zmp.dim, Fade( BLACK, alpha ) );
    }

    if ( ma->ymp.selected ) {
        DrawCubeV( ma->ymp.pos, ma->ymp.dim, ma->ymp.color );
        DrawCubeWiresV( ma->ymp.pos, ma->ymp.dim, BLACK );
    } else {
        DrawCubeV( ma->ymp.pos, ma->ymp.dim, Fade( ma->ymp.color, alpha ) );
        DrawCubeWiresV( ma->ymp.pos, ma->ymp.dim, Fade( BLACK, alpha ) );
    }

}

MoveAnchorCollisionType checkCollisionMouseMoveAnchor( MoveAnchor *ma, Camera3D *camera ) {

    Ray ray = GetScreenToWorldRay( GetMousePosition(), *camera );

    RayCollision xyCol = GetRayCollisionBox( 
        ray,
        (BoundingBox) {
            .min = { 
                ma->xmp.pos.x - ma->movePlaneBigSize / 2,
                ma->xmp.pos.y - ma->movePlaneBigSize / 2,
                ma->xmp.pos.z - ma->movePlaneSmallSize / 2,
            },
            .max = {
                ma->xmp.pos.x + ma->movePlaneBigSize / 2,
                ma->xmp.pos.y + ma->movePlaneBigSize / 2,
                ma->xmp.pos.z + ma->movePlaneSmallSize / 2,
            },
        }
    );

    RayCollision xzCol = GetRayCollisionBox( 
        ray,
        (BoundingBox) {
            .min = { 
                ma->zmp.pos.x - ma->movePlaneBigSize / 2,
                ma->zmp.pos.y - ma->movePlaneSmallSize / 2,
                ma->zmp.pos.z - ma->movePlaneBigSize / 2,
            },
            .max = {
                ma->zmp.pos.x + ma->movePlaneBigSize / 2,
                ma->zmp.pos.y + ma->movePlaneSmallSize / 2,
                ma->zmp.pos.z + ma->movePlaneBigSize / 2,
            },
        }
    );

    RayCollision yzCol = GetRayCollisionBox( 
        ray,
        (BoundingBox) {
            .min = { 
                ma->ymp.pos.x - ma->movePlaneSmallSize / 2,
                ma->ymp.pos.y - ma->movePlaneBigSize / 2,
                ma->ymp.pos.z - ma->movePlaneBigSize / 2,
            },
            .max = {
                ma->ymp.pos.x + ma->movePlaneSmallSize / 2,
                ma->ymp.pos.y + ma->movePlaneBigSize / 2,
                ma->ymp.pos.z + ma->movePlaneBigSize / 2,
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