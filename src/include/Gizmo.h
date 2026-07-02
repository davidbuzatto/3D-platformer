#pragma once

#include "raylib/raylib.h"

#include "MapPiece.h"
#include "GizmoAxis.h"

typedef struct {

    Vector3 pos;

    GizmoAxis xAxis;
    GizmoAxis yAxis;
    GizmoAxis zAxis;

} Gizmo;

void updateGizmo( Gizmo *ma, Vector3 mapPiecePos, Vector3 gizmoOffset );
void drawGizmo( Gizmo *ma );
GizmoAxisCollisionType checkCollisionMouseGizmo( Gizmo *ma, Camera3D *camera );