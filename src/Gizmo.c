#include <stdlib.h>
#include <math.h>

#include "raylib/raylib.h"
#include "raylib/raymath.h"

#include "MapPiece.h"
#include "Gizmo.h"
#include "GizmoAxis.h"

static const float spc = 0.2f;

void updateGizmo( Gizmo *ma, Vector3 mapPiecePos, Vector3 gizmoOffset ) {

    ma->pos = Vector3Add( mapPiecePos, gizmoOffset );

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

    ma->center.pos = (Vector3) { ma->yAxis.pos.x, ma->xAxis.pos.y, ma->xAxis.pos.z };

}

void drawGizmo( Gizmo *ma ) {

    const float expandFactor = 1.2f;
    const float alpha = 0.7f;

    DrawLine3D( ma->center.pos, ma->xAxis.pos, DARKGRAY );
    DrawLine3D( ma->center.pos, ma->yAxis.pos, DARKGRAY );
    DrawLine3D( ma->center.pos, ma->zAxis.pos, DARKGRAY );

    if ( ma->center.selected ) {
        DrawSphere( ma->center.pos, ma->center.radius * expandFactor, ma->center.color );
    } else {
        DrawSphere( ma->center.pos, ma->center.radius, Fade( ma->center.color, alpha ) );
    }

    if ( ma->xAxis.selected ) {
        DrawSphere( ma->xAxis.pos, ma->xAxis.radius * expandFactor, ma->xAxis.color );
    } else {
        DrawSphere( ma->xAxis.pos, ma->xAxis.radius, Fade( ma->xAxis.color, alpha ) );
    }

    if ( ma->yAxis.selected ) {
        DrawSphere( ma->yAxis.pos, ma->yAxis.radius * expandFactor, ma->yAxis.color );
    } else {
        DrawSphere( ma->yAxis.pos, ma->yAxis.radius, Fade( ma->yAxis.color, alpha ) );
    }

    if ( ma->zAxis.selected ) {
        DrawSphere( ma->zAxis.pos, ma->zAxis.radius * expandFactor, ma->zAxis.color );
    } else {
        DrawSphere( ma->zAxis.pos, ma->zAxis.radius, Fade( ma->zAxis.color, alpha ) );
    }

}

GizmoAxisCollisionType checkCollisionMouseGizmo( Gizmo *ma, Camera3D *camera ) {

    Ray ray = GetScreenToWorldRay( GetMousePosition(), *camera );

    RayCollision xAxisCol  = GetRayCollisionSphere( ray, ma->xAxis.pos, ma->xAxis.radius );
    RayCollision yAxisCol  = GetRayCollisionSphere( ray, ma->yAxis.pos, ma->yAxis.radius );
    RayCollision zAxisCol  = GetRayCollisionSphere( ray, ma->zAxis.pos, ma->zAxis.radius );
    RayCollision centerCol = GetRayCollisionSphere( ray, ma->center.pos, ma->center.radius );

    float minDist = INFINITY;
    GizmoAxisCollisionType collType = GIZMO_AXIS_COLLISION_TYPE_NONE;

    float xDist      = xAxisCol.hit  ? xAxisCol.distance  : INFINITY;
    float yDist      = yAxisCol.hit  ? yAxisCol.distance  : INFINITY;
    float zDist      = zAxisCol.hit  ? zAxisCol.distance  : INFINITY;
    float centerDist = centerCol.hit ? centerCol.distance : INFINITY;

    if ( xDist < minDist )      { minDist = xDist;      collType = GIZMO_AXIS_COLLISION_TYPE_X; }
    if ( yDist < minDist )      { minDist = yDist;      collType = GIZMO_AXIS_COLLISION_TYPE_Y; }
    if ( zDist < minDist )      { minDist = zDist;      collType = GIZMO_AXIS_COLLISION_TYPE_Z; }
    if ( centerDist < minDist ) { minDist = centerDist; collType = GIZMO_AXIS_COLLISION_TYPE_CENTER; }

    return collType;

}