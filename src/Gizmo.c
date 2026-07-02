#include <stdlib.h>
#include <math.h>

#include "raylib/raylib.h"

#include "MapPiece.h"
#include "Gizmo.h"
#include "GizmoAxis.h"

void updateGizmo( Gizmo *ma, Vector3 mapPiecePos, Vector3 gizmoOffset ) {

    ma->pos.x = mapPiecePos.x + gizmoOffset.x;
    ma->pos.y = mapPiecePos.y + gizmoOffset.y;
    ma->pos.z = mapPiecePos.z + gizmoOffset.z;

    ma->xAxis.pos = (Vector3) { 
        ma->pos.x, 
        ma->pos.y + GIZMO_PLANE_BIG_SIZE / 2 + GIZMO_PLANE_SMALL_SIZE / 2, 
        ma->pos.z - ( GIZMO_PLANE_BIG_SIZE / 2 + GIZMO_PLANE_SMALL_SIZE / 2 )
    };

    ma->zAxis.pos = (Vector3) { 
        ma->pos.x, 
        ma->pos.y, 
        ma->pos.z
    };

    ma->yAxis.pos = (Vector3) { 
        ma->pos.x + GIZMO_PLANE_BIG_SIZE / 2 + GIZMO_PLANE_SMALL_SIZE / 2, 
        ma->pos.y + GIZMO_PLANE_BIG_SIZE / 2 + GIZMO_PLANE_SMALL_SIZE / 2,
        ma->pos.z
    };

}

void drawGizmo( Gizmo *ma ) {

    const float alpha = 0.5f;

    if ( ma->xAxis.selected ) {
        DrawCubeV( ma->xAxis.pos, ma->xAxis.dim, ma->xAxis.color );
        DrawCubeWiresV( ma->xAxis.pos, ma->xAxis.dim, BLACK );
    } else {
        DrawCubeV( ma->xAxis.pos, ma->xAxis.dim, Fade( ma->xAxis.color, alpha ) );
        DrawCubeWiresV( ma->xAxis.pos, ma->xAxis.dim, Fade( BLACK, alpha ) );
    }

    if ( ma->zAxis.selected ) {
        DrawCubeV( ma->zAxis.pos, ma->zAxis.dim, ma->zAxis.color );
        DrawCubeWiresV( ma->zAxis.pos, ma->zAxis.dim, BLACK );
    } else {
        DrawCubeV( ma->zAxis.pos, ma->zAxis.dim, Fade( ma->zAxis.color, alpha ) );
        DrawCubeWiresV( ma->zAxis.pos, ma->zAxis.dim, Fade( BLACK, alpha ) );
    }

    if ( ma->yAxis.selected ) {
        DrawCubeV( ma->yAxis.pos, ma->yAxis.dim, ma->yAxis.color );
        DrawCubeWiresV( ma->yAxis.pos, ma->yAxis.dim, BLACK );
    } else {
        DrawCubeV( ma->yAxis.pos, ma->yAxis.dim, Fade( ma->yAxis.color, alpha ) );
        DrawCubeWiresV( ma->yAxis.pos, ma->yAxis.dim, Fade( BLACK, alpha ) );
    }

}

GizmoAxisCollisionType checkCollisionMouseGizmo( Gizmo *ma, Camera3D *camera ) {

    Ray ray = GetScreenToWorldRay( GetMousePosition(), *camera );

    RayCollision xyCol = GetRayCollisionBox( 
        ray,
        (BoundingBox) {
            .min = { 
                ma->xAxis.pos.x - ma->gizmoPlaneBigSize / 2,
                ma->xAxis.pos.y - ma->gizmoPlaneBigSize / 2,
                ma->xAxis.pos.z - ma->gizmoPlaneSmallSize / 2,
            },
            .max = {
                ma->xAxis.pos.x + ma->gizmoPlaneBigSize / 2,
                ma->xAxis.pos.y + ma->gizmoPlaneBigSize / 2,
                ma->xAxis.pos.z + ma->gizmoPlaneSmallSize / 2,
            },
        }
    );

    RayCollision xzCol = GetRayCollisionBox( 
        ray,
        (BoundingBox) {
            .min = { 
                ma->zAxis.pos.x - ma->gizmoPlaneBigSize / 2,
                ma->zAxis.pos.y - ma->gizmoPlaneSmallSize / 2,
                ma->zAxis.pos.z - ma->gizmoPlaneBigSize / 2,
            },
            .max = {
                ma->zAxis.pos.x + ma->gizmoPlaneBigSize / 2,
                ma->zAxis.pos.y + ma->gizmoPlaneSmallSize / 2,
                ma->zAxis.pos.z + ma->gizmoPlaneBigSize / 2,
            },
        }
    );

    RayCollision yzCol = GetRayCollisionBox( 
        ray,
        (BoundingBox) {
            .min = { 
                ma->yAxis.pos.x - ma->gizmoPlaneSmallSize / 2,
                ma->yAxis.pos.y - ma->gizmoPlaneBigSize / 2,
                ma->yAxis.pos.z - ma->gizmoPlaneBigSize / 2,
            },
            .max = {
                ma->yAxis.pos.x + ma->gizmoPlaneSmallSize / 2,
                ma->yAxis.pos.y + ma->gizmoPlaneBigSize / 2,
                ma->yAxis.pos.z + ma->gizmoPlaneBigSize / 2,
            },
        }
    );

    float minDist = INFINITY;
    GizmoAxisCollisionType collType = GIZMO_AXIS_COLLISION_TYPE_NONE;

    float xyDist = INFINITY;
    float xzDist = INFINITY;
    float yzDist = INFINITY;

    if ( xyCol.hit ) xyDist = xyCol.distance;
    if ( xzCol.hit ) xzDist = xzCol.distance;
    if ( yzCol.hit ) yzDist = yzCol.distance;

    if ( xyDist < minDist ) { 
        minDist = xyDist;
        collType = GIZMO_AXIS_COLLISION_TYPE_XY;
    }

    if ( xzDist < minDist ) {
        minDist = xzDist;
        collType = GIZMO_AXIS_COLLISION_TYPE_XZ;
    }

    if ( yzDist < minDist ) {
        minDist = yzDist;
        collType = GIZMO_AXIS_COLLISION_TYPE_YZ;
    }

    return collType;

}