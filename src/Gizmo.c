#include <stdlib.h>
#include <math.h>

#include "raylib/raylib.h"
#include "raylib/raymath.h"

#include "MapPiece.h"
#include "Gizmo.h"
#include "GizmoAxis.h"

static const float spc = 0.2f;
static const float planeThickness = 0.02f;  // half-thickness of the flat plane handles

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

    // plane handles sit halfway between the corner and the pair of axes
    // that define each plane
    const float halfSpc = spc * 0.5f;
    Vector3 corner = ma->center.pos;

    ma->xyPlane.pos = (Vector3) { corner.x - halfSpc, corner.y + halfSpc, corner.z };
    ma->xzPlane.pos = (Vector3) { corner.x - halfSpc, corner.y, corner.z + halfSpc };
    ma->yzPlane.pos = (Vector3) { corner.x, corner.y + halfSpc, corner.z + halfSpc };

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

    // plane handles are drawn as flat, thin boxes -- one dimension squashed
    // down to planeThickness (the axis missing from that plane) -- so they
    // read as little squares instead of spheres
    Vector3 xySize = { ma->xyPlane.radius * 2.0f, ma->xyPlane.radius * 2.0f, planeThickness * 2.0f };
    Vector3 xzSize = { ma->xzPlane.radius * 2.0f, planeThickness * 2.0f, ma->xzPlane.radius * 2.0f };
    Vector3 yzSize = { planeThickness * 2.0f, ma->yzPlane.radius * 2.0f, ma->yzPlane.radius * 2.0f };

    if ( ma->xyPlane.selected ) {
        Vector3 size = Vector3Scale( xySize, expandFactor );
        DrawCubeV( ma->xyPlane.pos, size, ma->xyPlane.color );
        DrawCubeWiresV( ma->xyPlane.pos, size, BLACK );
    } else {
        DrawCubeV( ma->xyPlane.pos, xySize, Fade( ma->xyPlane.color, alpha ) );
        DrawCubeWiresV( ma->xyPlane.pos, xySize, Fade( BLACK, alpha ) );
    }

    if ( ma->xzPlane.selected ) {
        Vector3 size = Vector3Scale( xzSize, expandFactor );
        DrawCubeV( ma->xzPlane.pos, size, ma->xzPlane.color );
        DrawCubeWiresV( ma->xzPlane.pos, size, BLACK );
    } else {
        DrawCubeV( ma->xzPlane.pos, xzSize, Fade( ma->xzPlane.color, alpha ) );
        DrawCubeWiresV( ma->xzPlane.pos, xzSize, Fade( BLACK, alpha ) );
    }

    if ( ma->yzPlane.selected ) {
        Vector3 size = Vector3Scale( yzSize, expandFactor );
        DrawCubeV( ma->yzPlane.pos, size, ma->yzPlane.color );
        DrawCubeWiresV( ma->yzPlane.pos, size, BLACK );
    } else {
        DrawCubeV( ma->yzPlane.pos, yzSize, Fade( ma->yzPlane.color, alpha ) );
        DrawCubeWiresV( ma->yzPlane.pos, yzSize, Fade( BLACK, alpha ) );
    }

}

GizmoAxisCollisionType checkCollisionMouseGizmo( Gizmo *ma, Camera3D *camera ) {

    Ray ray = GetScreenToWorldRay( GetMousePosition(), *camera );

    RayCollision xAxisCol  = GetRayCollisionSphere( ray, ma->xAxis.pos, ma->xAxis.radius );
    RayCollision yAxisCol  = GetRayCollisionSphere( ray, ma->yAxis.pos, ma->yAxis.radius );
    RayCollision zAxisCol  = GetRayCollisionSphere( ray, ma->zAxis.pos, ma->zAxis.radius );
    RayCollision centerCol = GetRayCollisionSphere( ray, ma->center.pos, ma->center.radius );

    // plane handles are flat boxes now, so hit-test them as boxes -- matches
    // the thin/squashed shape drawn in drawGizmo, instead of a sphere that
    // would stick out past the visible square
    BoundingBox xyBB = {
        .min = { ma->xyPlane.pos.x - ma->xyPlane.radius, ma->xyPlane.pos.y - ma->xyPlane.radius, ma->xyPlane.pos.z - planeThickness },
        .max = { ma->xyPlane.pos.x + ma->xyPlane.radius, ma->xyPlane.pos.y + ma->xyPlane.radius, ma->xyPlane.pos.z + planeThickness }
    };
    BoundingBox xzBB = {
        .min = { ma->xzPlane.pos.x - ma->xzPlane.radius, ma->xzPlane.pos.y - planeThickness, ma->xzPlane.pos.z - ma->xzPlane.radius },
        .max = { ma->xzPlane.pos.x + ma->xzPlane.radius, ma->xzPlane.pos.y + planeThickness, ma->xzPlane.pos.z + ma->xzPlane.radius }
    };
    BoundingBox yzBB = {
        .min = { ma->yzPlane.pos.x - planeThickness, ma->yzPlane.pos.y - ma->yzPlane.radius, ma->yzPlane.pos.z - ma->yzPlane.radius },
        .max = { ma->yzPlane.pos.x + planeThickness, ma->yzPlane.pos.y + ma->yzPlane.radius, ma->yzPlane.pos.z + ma->yzPlane.radius }
    };

    RayCollision xyPlaneCol = GetRayCollisionBox( ray, xyBB );
    RayCollision xzPlaneCol = GetRayCollisionBox( ray, xzBB );
    RayCollision yzPlaneCol = GetRayCollisionBox( ray, yzBB );

    float minDist = INFINITY;
    GizmoAxisCollisionType collType = GIZMO_AXIS_COLLISION_TYPE_NONE;

    float xDist      = xAxisCol.hit   ? xAxisCol.distance   : INFINITY;
    float yDist      = yAxisCol.hit   ? yAxisCol.distance   : INFINITY;
    float zDist      = zAxisCol.hit   ? zAxisCol.distance   : INFINITY;
    float centerDist = centerCol.hit  ? centerCol.distance  : INFINITY;
    float xyDist     = xyPlaneCol.hit ? xyPlaneCol.distance : INFINITY;
    float xzDist     = xzPlaneCol.hit ? xzPlaneCol.distance : INFINITY;
    float yzDist     = yzPlaneCol.hit ? yzPlaneCol.distance : INFINITY;

    if ( xDist < minDist )      { minDist = xDist;      collType = GIZMO_AXIS_COLLISION_TYPE_X; }
    if ( yDist < minDist )      { minDist = yDist;      collType = GIZMO_AXIS_COLLISION_TYPE_Y; }
    if ( zDist < minDist )      { minDist = zDist;      collType = GIZMO_AXIS_COLLISION_TYPE_Z; }
    if ( centerDist < minDist ) { minDist = centerDist; collType = GIZMO_AXIS_COLLISION_TYPE_CENTER; }
    if ( xyDist < minDist )     { minDist = xyDist;     collType = GIZMO_AXIS_COLLISION_TYPE_XY; }
    if ( xzDist < minDist )     { minDist = xzDist;     collType = GIZMO_AXIS_COLLISION_TYPE_XZ; }
    if ( yzDist < minDist )     { minDist = yzDist;     collType = GIZMO_AXIS_COLLISION_TYPE_YZ; }

    return collType;

}