#pragma once

#include <stdbool.h>

#include "raylib/raylib.h"

#include "MoveAnchor.h"
#include "MoveAnchorPlane.h"

typedef struct Entity Entity;

struct Entity {

    Vector3 pos;
    Vector3 vel;
    float baseSpeed;

    Color color;
    Model model;
    BoundingBox bb;

    MoveAnchor moveAnchor;
    Vector3 moveAnchorOffset;

    void (*update)( Entity *e, Camera3D *camera, float delta );
    void (*draw)( Entity *e, bool drawDebugInfo );

};

void initEntity( Entity *e, Vector3 pos, Model model );