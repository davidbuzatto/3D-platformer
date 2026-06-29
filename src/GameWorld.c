/**
 * @file GameWorld.c
 * @author Prof. Dr. David Buzatto
 * @brief GameWorld implementation.
 * 
 * @copyright Copyright (c) 2026
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "raylib/raylib.h"
//#include "raylib/raymath.h"
//#define RAYGUI_IMPLEMENTATION    // to use raygui, comment these three lines.
//#include "raylib/raygui.h"       // other compilation units must only include
//#undef RAYGUI_IMPLEMENTATION     // raygui.h

#include "Entity.h"
#include "GameWorld.h"
#include "ResourceManager.h"

static void updateCamera( GameWorld *gw );

static float cameraAngle    = 135.0f;
static float cameraDistance = 5.0f;

/**
 * @brief Creates a dinamically allocated GameWorld struct instance.
 */
GameWorld *createGameWorld( void ) {

    GameWorld *gw = (GameWorld*) malloc( sizeof( GameWorld ) );

    initEntity( &gw->entity );

    gw->camera = (Camera3D) {
        .fovy = 60.0f,
        .position = (Vector3) { 0, 2, cameraDistance },
        .projection = CAMERA_PERSPECTIVE,
        .target = (Vector3) { 0 },
        .up = (Vector3) { 0, 1, 0 }
    };

    return gw;

}

/**
 * @brief Destroys a GameWindow object and its dependecies.
 */
void destroyGameWorld( GameWorld *gw ) {
    free( gw );
}

/**
 * @brief Reads user input and updates the state of the game.
 */
void updateGameWorld( GameWorld *gw, float delta ) {

    gw->entity.update( &gw->entity, delta );

    updateCamera( gw );

}

/**
 * @brief Draws the state of the game.
 */
void drawGameWorld( GameWorld *gw ) {

    BeginDrawing();
    ClearBackground( WHITE );

    BeginMode3D( gw->camera );

    gw->entity.draw( &gw->entity );
    DrawGrid( 100, 1 );

    EndMode3D();

    EndDrawing();

}

static void updateCamera( GameWorld *gw ) {

    Camera3D *c = &gw->camera;
    Entity *e = &gw->entity;

    if ( IsKeyDown( KEY_DELETE ) ) {
        cameraAngle += 1;
    }

    if ( IsKeyDown( KEY_PAGE_DOWN ) ) {
        cameraAngle += -1;
    }

    c->position = (Vector3) {
        .x = e->pos.x + cameraDistance * cosf( DEG2RAD * cameraAngle ),
        .y = 2.0f,
        .z = e->pos.z + cameraDistance * sinf( DEG2RAD * cameraAngle )
    };

    c->target = e->pos;

}