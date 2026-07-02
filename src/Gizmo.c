#include <stdlib.h>
#include <math.h>

#include "raylib/raylib.h"

#include "MapPiece.h"
#include "Gizmo.h"
#include "GizmoAxis.h"

static const float spc = 0.2f;

void updateGizmo( Gizmo *ma, Vector3 mapPiecePos, Vector3 gizmoOffset ) {

    ma->pos.x = mapPiecePos.x + gizmoOffset.x;
    ma->pos.y = mapPiecePos.y + gizmoOffset.y;
    ma->pos.z = mapPiecePos.z + gizmoOffset.z;

    ma->xAxis.pos = (Vector3) { 
        ma->pos.x - spc, 
        ma->pos.y, 
        ma->pos.z - spc
    };

    ma->yAxis.pos = (Vector3) { 
        ma->pos.x + spc, 
        ma->pos.y + spc * 2.0f,
        ma->pos.z - spc
    };

    ma->zAxis.pos = (Vector3) { 
        ma->pos.x + spc,
        ma->pos.y, 
        ma->pos.z + spc
    };

}

void drawGizmo( Gizmo *ma ) {

    const float alpha = 0.7f;
    Vector3 cornerPos = { ma->yAxis.pos.x, ma->xAxis.pos.y, ma->xAxis.pos.z };

    DrawSphere( cornerPos, ma->xAxis.radius, DARKGRAY );
    DrawLine3D( cornerPos, ma->xAxis.pos, DARKGRAY );
    DrawLine3D( cornerPos, ma->yAxis.pos, DARKGRAY );
    DrawLine3D( cornerPos, ma->zAxis.pos, DARKGRAY );

    if ( ma->xAxis.selected ) {
        DrawSphere( ma->xAxis.pos, ma->xAxis.radius, ma->xAxis.color );
    } else {
        DrawSphere( ma->xAxis.pos, ma->xAxis.radius, Fade( ma->xAxis.color, alpha ) );
    }

    if ( ma->yAxis.selected ) {
        DrawSphere( ma->yAxis.pos, ma->yAxis.radius, ma->yAxis.color );
    } else {
        DrawSphere( ma->yAxis.pos, ma->yAxis.radius, Fade( ma->yAxis.color, alpha ) );
    }

    if ( ma->zAxis.selected ) {
        DrawSphere( ma->zAxis.pos, ma->zAxis.radius, ma->zAxis.color );
    } else {
        DrawSphere( ma->zAxis.pos, ma->zAxis.radius, Fade( ma->zAxis.color, alpha ) );
    }

}

GizmoAxisCollisionType checkCollisionMouseGizmo( Gizmo *ma, Camera3D *camera ) {

    Ray ray = GetScreenToWorldRay( GetMousePosition(), *camera );

    RayCollision xAxisCol = GetRayCollisionSphere( 
        ray, ma->xAxis.pos, ma->xAxis.radius
    );

    RayCollision yAxisCol = GetRayCollisionSphere( 
        ray, ma->yAxis.pos, ma->yAxis.radius
    );

    RayCollision zAxisCol = GetRayCollisionSphere( 
        ray, ma->zAxis.pos, ma->zAxis.radius
    );

    float minDist = INFINITY;
    GizmoAxisCollisionType collType = GIZMO_AXIS_COLLISION_TYPE_NONE;

    float xyDist = INFINITY;
    float xzDist = INFINITY;
    float yzDist = INFINITY;

    if ( xAxisCol.hit ) xyDist = xAxisCol.distance;
    if ( zAxisCol.hit ) xzDist = zAxisCol.distance;
    if ( yAxisCol.hit ) yzDist = yAxisCol.distance;

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