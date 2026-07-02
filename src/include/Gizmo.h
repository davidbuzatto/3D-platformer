#pragma once

#include "raylib/raylib.h"

#include "MapPiece.h"
#include "GizmoAxis.h"

#define GIZMO_PLANE_BIG_SIZE 0.3f
#define GIZMO_PLANE_SMALL_SIZE 0.05f

typedef struct {

    Vector3 pos;

    float gizmoPlaneBigSize;
    float gizmoPlaneSmallSize;

    GizmoAxis xAxis;
    GizmoAxis yAxis;
    GizmoAxis zAxis;

} Gizmo;

void updateGizmo( Gizmo *ma, Vector3 mapPiecePos, Vector3 gizmoOffset );
void drawGizmo( Gizmo *ma );
GizmoAxisCollisionType checkCollisionMouseGizmo( Gizmo *ma, Camera3D *camera );