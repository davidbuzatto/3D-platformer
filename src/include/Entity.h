#pragma once

#include "raylib/raylib.h"
#include "MoveAnchor.h"
#include "MoveAnchorPlane.h"

typedef struct Entity Entity;

struct Entity {

    Vector3 pos;
    Vector3 dim;
    Vector3 vel;
    float baseSpeed;

    Color color;

    MoveAnchor moveAnchor;
    Vector3 moveAnchorOffset;

    void (*update)( Entity *e, float delta );
    void (*draw)( Entity *e );

};

void initEntity( Entity *e );