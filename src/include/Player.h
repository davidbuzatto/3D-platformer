#pragma once

#include "raylib/raylib.h"

typedef struct Player Player;

struct Player {

    Vector3 pos;
    Vector3 vel;

    Model model;
    BoundingBox baseBB;

    void (*input)( Player* p, Camera3D *camera );
    void (*update)( Player* p, float delta );
    void (*draw)( Player* p );

};

void initPlayer( Player *p, Vector3 pos );