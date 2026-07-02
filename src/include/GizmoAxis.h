#pragma once

#include <stdbool.h>

#include "raylib/raylib.h"

typedef enum GizmoAxisCollisionType {
    GIZMO_AXIS_COLLISION_TYPE_NONE,
    GIZMO_AXIS_COLLISION_TYPE_XY,
    GIZMO_AXIS_COLLISION_TYPE_XZ,
    GIZMO_AXIS_COLLISION_TYPE_YZ
} GizmoAxisCollisionType;

typedef struct {
    Vector3 pos;
    float radius;
    Color color;
    bool selected;
} GizmoAxis;