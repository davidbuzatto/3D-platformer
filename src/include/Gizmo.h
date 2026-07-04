#pragma once

#include "raylib/raylib.h"

#include "MapPiece.h"
#include "GizmoAxis.h"

typedef enum GizmoOperationMode {
    GIZMO_OPERATION_TRANSLATE,
    GIZMO_OPERATION_ROTATE,
    GIZMO_OPERATION_SCALE,
} GizmoOperationMode;

typedef struct {

    Vector3 pos;

    GizmoAxis xAxis;
    GizmoAxis yAxis;
    GizmoAxis zAxis;
    GizmoAxis center;   // uniform scale handle

    GizmoAxis xyPlane;
    GizmoAxis xzPlane;
    GizmoAxis yzPlane;

} Gizmo;

void updateGizmo( Gizmo *ma, Vector3 mapPiecePos, Vector3 gizmoOffset );
void drawGizmo( Gizmo *ma, GizmoOperationMode mode );
GizmoAxisCollisionType checkCollisionMouseGizmo( Gizmo *ma, Camera3D *camera );