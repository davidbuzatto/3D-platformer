#include <stdlib.h>
#include <math.h>

#include "raylib/raylib.h"
#include "raylib/raymath.h"

#include "MapPiece.h"
#include "Gizmo.h"
#include "GizmoAxis.h"

static const float spc = 0.2f;
static const float planeThickness = 0.02f;  // half-thickness of the flat plane handles

// translate-mode cone: length as a multiple of the handle radius (2.0 = the
// rotate sphere's diameter, so the cone fits exactly inside it once shifted
// towards the center -- see drawAxisHandle) and how wide its base is
// relative to the handle radius
static const float translateConeLengthFactor = 2.0f;
static const float translateConeRadiusFactor  = 0.8f;

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

// draws one axis handle with a shape that matches the current operation:
// sphere for rotate, cube for scale, arrowhead cone (pointing away from
// origin) for translate
static void drawAxisHandle( Vector3 origin, GizmoAxis *axis, GizmoOperationMode mode, float expandFactor, float alpha ) {

    float radius = axis->selected ? axis->radius * expandFactor : axis->radius;
    Color color  = axis->selected ? axis->color : Fade( axis->color, alpha );
    Color dark   = ColorBrightness( axis->color, -0.4f );
    Color wire   = axis->selected ? dark : Fade( dark, alpha );

    switch ( mode ) {

        case GIZMO_OPERATION_ROTATE:
            // wires drawn slightly larger so they don't get swallowed by the
            // solid sphere's surface
            DrawSphere( axis->pos, radius, color );
            DrawSphereWires( axis->pos, radius * 1.06f, 6, 6, wire );
            break;

        case GIZMO_OPERATION_SCALE: {
            Vector3 size = { radius * 2.0f, radius * 2.0f, radius * 2.0f };
            DrawCubeV( axis->pos, size, color );
            DrawCubeWiresV( axis->pos, size, wire );
            break;
        }

        case GIZMO_OPERATION_TRANSLATE: {
            // the whole cone is shifted towards the center by its own
            // collision radius (shift), which lands the hit-test sphere
            // exactly on axis->pos -- same anchor rotate/scale use
            float coneLength = axis->radius * translateConeLengthFactor;
            float coneRadius = radius * translateConeRadiusFactor;
            float shift = coneLength * 0.5f;
            Vector3 dir = Vector3Normalize( Vector3Subtract( axis->pos, origin ) );
            Vector3 coneBase = Vector3Subtract( axis->pos, Vector3Scale( dir, shift ) );
            Vector3 coneTip  = Vector3Add( coneBase, Vector3Scale( dir, coneLength ) );
            DrawCylinderEx( coneBase, coneTip, coneRadius, 0.0f, 8, color );
            DrawCylinderWiresEx( coneBase, coneTip, coneRadius, 0.0f, 8, wire );

            break;
        }

    }

}

void drawGizmo( Gizmo *ma, GizmoOperationMode mode ) {

    const float expandFactor = 1.2f;
    const float alpha = 0.7f;

    DrawLine3D( ma->center.pos, ma->xAxis.pos, DARKGRAY );
    DrawLine3D( ma->center.pos, ma->yAxis.pos, DARKGRAY );
    DrawLine3D( ma->center.pos, ma->zAxis.pos, DARKGRAY );

    Color centerDark = ColorBrightness( ma->center.color, -0.4f );

    if ( ma->center.selected ) {
        DrawSphere( ma->center.pos, ma->center.radius * expandFactor, ma->center.color );
        DrawSphereWires( ma->center.pos, ma->center.radius * expandFactor * 1.06f, 6, 6, centerDark );
    } else {
        DrawSphere( ma->center.pos, ma->center.radius, Fade( ma->center.color, alpha ) );
        DrawSphereWires( ma->center.pos, ma->center.radius * 1.06f, 6, 6, Fade( centerDark, alpha ) );
    }

    drawAxisHandle( ma->center.pos, &ma->xAxis, mode, expandFactor, alpha );
    drawAxisHandle( ma->center.pos, &ma->yAxis, mode, expandFactor, alpha );
    drawAxisHandle( ma->center.pos, &ma->zAxis, mode, expandFactor, alpha );

    // plane handles are drawn as flat, thin boxes -- one dimension squashed
    // down to planeThickness (the axis missing from that plane) -- so they
    // read as little squares instead of spheres
    Vector3 xySize = { ma->xyPlane.radius * 2.0f, ma->xyPlane.radius * 2.0f, planeThickness * 2.0f };
    Vector3 xzSize = { ma->xzPlane.radius * 2.0f, planeThickness * 2.0f, ma->xzPlane.radius * 2.0f };
    Vector3 yzSize = { planeThickness * 2.0f, ma->yzPlane.radius * 2.0f, ma->yzPlane.radius * 2.0f };

    Color xyDark = ColorBrightness( ma->xyPlane.color, -0.4f );
    Color xzDark = ColorBrightness( ma->xzPlane.color, -0.4f );
    Color yzDark = ColorBrightness( ma->yzPlane.color, -0.4f );

    if ( ma->xyPlane.selected ) {
        Vector3 size = Vector3Scale( xySize, expandFactor );
        DrawCubeV( ma->xyPlane.pos, size, ma->xyPlane.color );
        DrawCubeWiresV( ma->xyPlane.pos, size, xyDark );
    } else {
        DrawCubeV( ma->xyPlane.pos, xySize, Fade( ma->xyPlane.color, alpha ) );
        DrawCubeWiresV( ma->xyPlane.pos, xySize, Fade( xyDark, alpha ) );
    }

    if ( ma->xzPlane.selected ) {
        Vector3 size = Vector3Scale( xzSize, expandFactor );
        DrawCubeV( ma->xzPlane.pos, size, ma->xzPlane.color );
        DrawCubeWiresV( ma->xzPlane.pos, size, xzDark );
    } else {
        DrawCubeV( ma->xzPlane.pos, xzSize, Fade( ma->xzPlane.color, alpha ) );
        DrawCubeWiresV( ma->xzPlane.pos, xzSize, Fade( xzDark, alpha ) );
    }

    if ( ma->yzPlane.selected ) {
        Vector3 size = Vector3Scale( yzSize, expandFactor );
        DrawCubeV( ma->yzPlane.pos, size, ma->yzPlane.color );
        DrawCubeWiresV( ma->yzPlane.pos, size, yzDark );
    } else {
        DrawCubeV( ma->yzPlane.pos, yzSize, Fade( ma->yzPlane.color, alpha ) );
        DrawCubeWiresV( ma->yzPlane.pos, yzSize, Fade( yzDark, alpha ) );
    }

}

GizmoAxisCollisionType checkCollisionMouseGizmo( Gizmo *ma, Camera3D *camera ) {

    Ray ray = GetScreenToWorldRay( GetMousePosition(), *camera );

    // the translate-mode cone is exactly as tall as the rotate sphere's
    // diameter and shifted towards the center by its own radius (see
    // drawAxisHandle), so it fits entirely inside that same sphere -- no
    // special case needed here, the plain handle radius already covers it
    float xHitRadius = ma->xAxis.radius;
    float yHitRadius = ma->yAxis.radius;
    float zHitRadius = ma->zAxis.radius;

    RayCollision xAxisCol  = GetRayCollisionSphere( ray, ma->xAxis.pos, xHitRadius );
    RayCollision yAxisCol  = GetRayCollisionSphere( ray, ma->yAxis.pos, yHitRadius );
    RayCollision zAxisCol  = GetRayCollisionSphere( ray, ma->zAxis.pos, zHitRadius );
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