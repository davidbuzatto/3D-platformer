#pragma once

#include <stdbool.h>

#include "raylib/raylib.h"

typedef enum GizmoAxisCollisionType {
    GIZMO_AXIS_COLLISION_TYPE_NONE,
    GIZMO_AXIS_COLLISION_TYPE_X,
    GIZMO_AXIS_COLLISION_TYPE_Y,
    GIZMO_AXIS_COLLISION_TYPE_Z,
    GIZMO_AXIS_COLLISION_TYPE_CENTER,
} GizmoAxisCollisionType;

typedef struct {
    Vector3 pos;
    float radius;
    Color color;
    bool selected;
} GizmoAxis;