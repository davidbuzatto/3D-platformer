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

#define GAMEPAD_ID          0
#define GAMEPAD_DEADZONE    0.2f
#define GROUND_PROBE_RADIUS 0.3f

// horizontal collision box, centered on pos: a thin slice from
// PLAYER_STEP_HEIGHT to PLAYER_HEIGHT above the feet, so low curbs/steps
// (like the low grass blocks) don't get treated as walls
#define PLAYER_RADIUS      0.3f
#define PLAYER_STEP_HEIGHT 0.4f
#define PLAYER_HEIGHT      1.8f

#define JUMP_BUFFER_TIME 0.15f // remember a jump press for this long
#define COYOTE_TIME      0.15f // still allow jumping this long after leaving the ground

static void input( Player* p, Camera3D *camera );
static void update( Player *p, MapPiece *mapPieces, int mapPiecesCount, float delta );
static void draw( Player *p );

static float getGroundY( Vector3 pos, MapPiece *mapPieces, int mapPiecesCount );
static bool collidesWithMapPieces( Vector3 pos, MapPiece *mapPieces, int mapPiecesCount );
static bool isSlopedMapPieceModelType( MapPieceModelType type );

static const float moveSpeed = 6.0f; // units per second
static const float jumpSpeed = 8.5f; // initial upward velocity

// naive gravity: no ground collision yet, so the player just keeps
// falling forever
static const float gravity = -20.0f;

void initPlayer( Player *p, Vector3 pos ) {

    p->pos = pos;
    p->vel = (Vector3) { 0 };
    p->facingYaw = 0.0f;
    p->grounded = false;
    p->jumpBufferTimer = 0.0f;
    p->coyoteTimer = 0.0f;

    p->model = rm->mapPieceModelAtlas[MODEL_TYPE_CHARACTER_OOBI];
    p->baseBB = GetModelBoundingBox( p->model );

    p->input = input;
    p->update = update;
    p->draw = draw;

}

static void input( Player* p, Camera3D *camera ) {

    // just remembers that the player asked to jump -- update() decides
    // whether that request is actually honored this frame (see the jump
    // buffer / coyote time logic there)
    if ( IsGamepadButtonPressed( GAMEPAD_ID, GAMEPAD_BUTTON_RIGHT_FACE_DOWN ) ) {
        p->jumpBufferTimer = JUMP_BUFFER_TIME;
    }

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

    // face the direction of movement
    p->facingYaw = RAD2DEG * atan2f( -dir.x, -dir.z ) + 180.0f;

    p->vel.x = dir.x * moveSpeed;
    p->vel.z = dir.z * moveSpeed;

}

static void update( Player *p, MapPiece *mapPieces, int mapPiecesCount, float delta ) {

    // jump buffer: count down a remembered press instead of requiring it
    // to land on the exact frame grounded happens to be true
    if ( p->jumpBufferTimer > 0.0f ) {
        p->jumpBufferTimer -= delta;
    }

    // coyote time: keep jumping allowed for a short grace window after
    // leaving solid ground -- without it, walking on a ramp/slope can
    // flicker grounded false for a frame here and there (the ground probe
    // snaps each frame, and gravity nudges the player off the surface in
    // between), silently eating jump presses
    if ( p->grounded ) {
        p->coyoteTimer = COYOTE_TIME;
    } else if ( p->coyoteTimer > 0.0f ) {
        p->coyoteTimer -= delta;
    }

    if ( p->jumpBufferTimer > 0.0f && p->coyoteTimer > 0.0f ) {
        p->vel.y = jumpSpeed;
        p->grounded = false;
        p->jumpBufferTimer = 0.0f;
        p->coyoteTimer = 0.0f;
    }

    // horizontal movement, resolved one axis at a time -- so sliding along
    // a wall works instead of getting stuck when approaching it at an angle
    float newX = p->pos.x + p->vel.x * delta;
    if ( !collidesWithMapPieces( (Vector3) { newX, p->pos.y, p->pos.z }, mapPieces, mapPiecesCount ) ) {
        p->pos.x = newX;
    }

    float newZ = p->pos.z + p->vel.z * delta;
    if ( !collidesWithMapPieces( (Vector3) { p->pos.x, p->pos.y, newZ }, mapPieces, mapPiecesCount ) ) {
        p->pos.z = newZ;
    }

    float oldY = p->pos.y;

    // Euler integration: v += a*dt, then pos += v*dt
    p->vel.y += gravity * delta;
    p->pos.y += p->vel.y * delta;

    // probe from slightly above where the player was before this frame's
    // movement -- guarantees the ray starts above any surface it's resting
    // on, instead of starting already inside it (which is what was
    // breaking the landing check)
    Vector3 probeOrigin = p->pos;
    probeOrigin.y = oldY + 0.25f;

    float groundY = getGroundY( probeOrigin, mapPieces, mapPiecesCount );
    p->grounded = ( !isinf( groundY ) && p->pos.y <= groundY );

    // snap to ground
    if ( p->grounded ) {
        p->pos.y = groundY;
        p->vel.y = 0.0f;
    }

}

static void draw( Player *p ) {
    DrawModelEx( 
        p->model, 
        p->pos, 
        (Vector3) { 0.0f, 1.0f, 0.0f }, 
        p->facingYaw, 
        (Vector3) { 1.0f, 1.0f, 1.0f },
        WHITE
    );
}

static float getGroundY( Vector3 pos, MapPiece *mapPieces, int mapPiecesCount ) {

    // sample a small cluster under the player instead of a single ray --
    // some meshes (like slatted wood bridges) have real gaps between
    // planks, and a lone center ray can slip through one even while the
    // character is solidly standing on the piece as a whole
    Vector3 offsets[5] = {
        {  0.0f,                 0.0f, 0.0f },
        {  GROUND_PROBE_RADIUS,  0.0f, 0.0f },
        { -GROUND_PROBE_RADIUS,  0.0f, 0.0f },
        {  0.0f,                 0.0f,  GROUND_PROBE_RADIUS },
        {  0.0f,                 0.0f, -GROUND_PROBE_RADIUS },
    };

    float highestY = -INFINITY;

    for ( int s = 0; s < 5; s++ ) {

        Ray ray = {
            .position = Vector3Add( pos, offsets[s] ),
            .direction = { 0.0f, -1.0f, 0.0f }
        };

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

    }

    return highestY;

}

static bool collidesWithMapPieces( Vector3 pos, MapPiece *mapPieces, int mapPiecesCount ) {

    // a thin vertical slice of the player, from PLAYER_STEP_HEIGHT to
    // PLAYER_HEIGHT above the feet -- low curbs/steps stay below this
    // slice and don't block horizontal movement, only genuine walls do.
    // uses mp->bb (world AABB), so rotated pieces are only approximated --
    // fine for now since walls here are mostly axis-aligned
    BoundingBox playerBox = {
        { pos.x - PLAYER_RADIUS, pos.y + PLAYER_STEP_HEIGHT, pos.z - PLAYER_RADIUS },
        { pos.x + PLAYER_RADIUS, pos.y + PLAYER_HEIGHT,      pos.z + PLAYER_RADIUS }
    };

    for ( int i = 0; i < mapPiecesCount; i++ ) {

        // a sloped piece's AABB spans its full rise (an AABB can't follow
        // the diagonal), so it would wrongly block the player from even
        // stepping onto the low end -- skip these here and let the ground
        // probe (which tests the real mesh) handle them entirely
        if ( isSlopedMapPieceModelType( mapPieces[i].modelType ) ) {
            continue;
        }

        if ( CheckCollisionBoxes( playerBox, mapPieces[i].bb ) ) {
            return true;
        }

    }

    return false;

}

static bool isSlopedMapPieceModelType( MapPieceModelType type ) {
    switch ( type ) {
        case MODEL_TYPE_BLOCK_GRASS_CURVE:
        case MODEL_TYPE_BLOCK_GRASS_CURVE_HALF:
        case MODEL_TYPE_BLOCK_GRASS_CURVE_LOW:
        case MODEL_TYPE_BLOCK_GRASS_LARGE_SLOPE:
        case MODEL_TYPE_BLOCK_GRASS_LARGE_SLOPE_NARROW:
        case MODEL_TYPE_BLOCK_GRASS_LARGE_SLOPE_STEEP:
        case MODEL_TYPE_BLOCK_GRASS_LARGE_SLOPE_STEEP_NARROW:
        case MODEL_TYPE_BLOCK_GRASS_OVERHANG_LARGE_SLOPE:
        case MODEL_TYPE_BLOCK_GRASS_OVERHANG_LARGE_SLOPE_NARROW:
        case MODEL_TYPE_BLOCK_GRASS_OVERHANG_LARGE_SLOPE_STEEP:
        case MODEL_TYPE_BLOCK_GRASS_OVERHANG_LARGE_SLOPE_STEEP_NARROW:
        case MODEL_TYPE_BLOCK_SNOW_CURVE:
        case MODEL_TYPE_BLOCK_SNOW_CURVE_HALF:
        case MODEL_TYPE_BLOCK_SNOW_CURVE_LOW:
        case MODEL_TYPE_BLOCK_SNOW_LARGE_SLOPE:
        case MODEL_TYPE_BLOCK_SNOW_LARGE_SLOPE_NARROW:
        case MODEL_TYPE_BLOCK_SNOW_LARGE_SLOPE_STEEP:
        case MODEL_TYPE_BLOCK_SNOW_LARGE_SLOPE_STEEP_NARROW:
        case MODEL_TYPE_BLOCK_SNOW_OVERHANG_LARGE_SLOPE:
        case MODEL_TYPE_BLOCK_SNOW_OVERHANG_LARGE_SLOPE_NARROW:
        case MODEL_TYPE_BLOCK_SNOW_OVERHANG_LARGE_SLOPE_STEEP:
        case MODEL_TYPE_BLOCK_SNOW_OVERHANG_LARGE_SLOPE_STEEP_NARROW:
        case MODEL_TYPE_PLATFORM_RAMP:
            return true;
        default:
            return false;
    }
}