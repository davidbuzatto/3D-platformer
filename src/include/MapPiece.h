#pragma once

#include <stdbool.h>

#include "raylib/raylib.h"

#include "Gizmo.h"
#include "GizmoAxis.h"

typedef struct MapPiece MapPiece;

struct MapPiece {

    Vector3 pos;
    Vector3 rot;
    Vector3 sca;

    Color color;
    Model model;
    BoundingBox bb;

    Gizmo gizmo;
    Vector3 gizmoOffset;

    void (*update)( MapPiece *mp, Camera3D *camera, float delta );
    void (*draw)( MapPiece *mp, bool drawDebugInfo );

};

void initMapPiece( MapPiece *mp, Vector3 pos, Model model );