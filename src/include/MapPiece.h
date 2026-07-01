#pragma once

#include <stdbool.h>

#include "raylib/raylib.h"

#include "MoveAnchor.h"
#include "MoveAnchorPlane.h"

typedef struct MapPiece MapPiece;

struct MapPiece {

    Vector3 pos;
    Vector3 vel;
    float baseSpeed;

    Color color;
    Model model;
    BoundingBox bb;

    MoveAnchor moveAnchor;
    Vector3 moveAnchorOffset;

    void (*update)( MapPiece *e, Camera3D *camera, float delta );
    void (*draw)( MapPiece *e, bool drawDebugInfo );

};

void initMapPiece( MapPiece *e, Vector3 pos, Model model );