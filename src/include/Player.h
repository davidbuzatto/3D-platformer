#pragma once

#include "raylib/raylib.h"

typedef struct Player Player;

struct Player {

    Vector3 pos;
    Vector3 vel;

    float facingYaw;
    bool grounded;
    float jumpBufferTimer;  // remembers a jump press for a short window,
                             // in case grounded isn't true yet this frame
    float coyoteTimer;      // keeps jump allowed for a short window after
                             // leaving the ground (e.g. walking off a ramp)

    Model model;
    BoundingBox baseBB;

    void (*input)( Player* p, Camera3D *camera );
    void (*update)( Player* p, MapPiece *mapPieces, int mapPiecesCount, float delta );
    void (*draw)( Player* p );

};

void initPlayer( Player *p, Vector3 pos );