/**
 * @file Player.c
 * @author Prof. Dr. David Buzatto
 * @brief Player implementation.
 *
 * @copyright Copyright (c) 2026
 */
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "raylib/raylib.h"
#include "raylib/raymath.h"

#include "Macros.h"
#include "MapPiece.h"
#include "Player.h"
#include "ResourceManager.h"

#define GAMEPAD_ID       0
#define GAMEPAD_DEADZONE 0.2f

static void input( Player* p, Camera3D *camera );
static void update( Player *p, MapPiece *mapPieces, int mapPiecesCount, float delta );
static void draw( Player *p );

static float getGroundY( Vector3 pos, MapPiece *mapPieces, int mapPiecesCount );

static const float moveSpeed = 6.0f; // units per second

// naive gravity: no ground collision yet, so the player just keeps
// falling forever
static const float gravity = -20.0f;

void initPlayer( Player *p, Vector3 pos ) {

    p->pos = pos;
    p->vel = (Vector3) { 0 };
    
    p->model = rm->mapPieceModelAtlas[MODEL_TYPE_CHARACTER_OOBI];
    p->baseBB = GetModelBoundingBox( p->model );

    p->input = input;
    p->update = update;
    p->draw = draw;

}

static void input( Player* p, Camera3D *camera ) {

    float moveX = 0.0f;
    float moveZ = 0.0f;

    if ( IsGamepadAvailable( GAMEPAD_ID ) ) {
        moveX = GetGamepadAxisMovement( GAMEPAD_ID, GAMEPAD_AXIS_LEFT_X );
        moveZ = GetGamepadAxisMovement( GAMEPAD_ID, GAMEPAD_AXIS_LEFT_Y );
        // dead zone: ignore tiny stick drift when the stick is "resting"
        if ( fabsf( moveX ) < GAMEPAD_DEADZONE ) moveX = 0.0f;
        if ( fabsf( moveZ ) < GAMEPAD_DEADZONE ) moveZ = 0.0f;
    }

    if ( moveX == 0.0f && moveZ == 0.0f ) {
        p->vel.x = 0.0f;
        p->vel.z = 0.0f;
        return;
    }

    // camera forward, flattened onto the XZ plane (ignore pitch) --
    // movement direction follows where the camera looks, not world axes
    Vector3 forward = Vector3Subtract( camera->target, camera->position );
    forward.y = 0.0f;
    forward = Vector3Normalize( forward );

    // right = forward x up: same convention already used in EditorCamera.c
    Vector3 right = Vector3CrossProduct( forward, (Vector3) { 0.0f, 1.0f, 0.0f } );

    // stick Y comes back negative when pushed "up" (forward), so it's inverted here
    Vector3 dir = Vector3Add(
        Vector3Scale( right, moveX ),
        Vector3Scale( forward, -moveZ )
    );

    // clamp diagonal movement so it isn't faster than straight movement
    if ( Vector3Length( dir ) > 1.0f ) {
        dir = Vector3Normalize( dir );
    }

    p->vel.x = dir.x * moveSpeed;
    p->vel.z = dir.z * moveSpeed;

}

static void update( Player *p, MapPiece *mapPieces, int mapPiecesCount, float delta ) {

    float oldY = p->pos.y;

    // Euler integration: v += a*dt, then pos += v*dt
    p->vel.y += gravity * delta;
    p->pos = Vector3Add( p->pos, Vector3Scale( p->vel, delta ) );

    // probe from slightly above where the player was before this frame's
    // movement -- guarantees the ray starts above any surface it's resting
    // on, instead of starting already inside it (which is what was
    // breaking the landing check)
    Vector3 probeOrigin = p->pos;
    probeOrigin.y = oldY + 0.05f;

    float groundY = getGroundY( probeOrigin, mapPieces, mapPiecesCount );

    // snap to ground
    if ( !isinf( groundY ) && p->pos.y <= groundY ) {
        p->pos.y = groundY;
        p->vel.y = 0.0f;
    }

}

static void draw( Player *p ) {
    DrawModel( p->model, p->pos, 1.0f, WHITE );
}

static float getGroundY( Vector3 pos, MapPiece *mapPieces, int mapPiecesCount ) {

    Ray ray = {
        .position = pos,
        .direction = { 0.0f, -1.0f, 0.0 }
    };

    float highestY = -INFINITY;

    for ( int i = 0; i < mapPiecesCount; i++ ) {

        MapPiece *mp = &mapPieces[i];
        Matrix transform = getMapPieceWorldTransform( mp );

        for ( int m = 0; m < mp->model.meshCount; m++ ) {
            RayCollision hit = GetRayCollisionMesh( ray, mp->model.meshes[m], transform );
            if ( hit.hit && hit.point.y > highestY ) {
                highestY = hit.point.y;
            }
        }

    }

    return highestY;

}