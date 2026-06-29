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

//static float cameraAngle    = 135.0f;
static float cameraAngle    = 90.0f;
static float cameraDistance = 5.0f;
static float cameraSpeed      = 10.0f;

/**
 * @brief Creates a dinamically allocated GameWorld struct instance.
 */
GameWorld *createGameWorld( void ) {

    GameWorld *gw = (GameWorld*) malloc( sizeof( GameWorld ) );

    int rows = 5;
    int cols = 5;
    gw->entityCount = rows * cols;

    float entitySpacing = 1.0f;
    float entitySize = 1.0f;
    float startPosX = - ( ( cols * entitySize + ( cols - 1 ) * entitySpacing ) / 2.0f - entitySize / 2.0f );
    float startPosZ = - ( ( rows * entitySize + ( rows - 1 ) * entitySpacing ) / 2.0f - entitySize / 2.0f );
    
    gw->entities = (Entity*) malloc( sizeof( Entity ) * gw->entityCount );

    for ( int i = 0; i < rows; i++ ) {
        for ( int j = 0; j < cols; j++ ) {
            int p = i * cols + j;
            initEntity( 
                &gw->entities[p], 
                (Vector3) { 
                    startPosX + j * (entitySize + entitySpacing), 
                    0, 
                    startPosZ + i * (entitySize + entitySpacing)
                }
            );
        }
    }

    gw->camera = (Camera3D) {
        .fovy = 60.0f,
        .position = (Vector3) { 0.0f, 5.0f, cameraDistance },
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
    if ( gw != NULL ) {
        if ( gw->entities != NULL ) {
            free( gw->entities );
        }
        free( gw );
    }
}

/**
 * @brief Reads user input and updates the state of the game.
 */
void updateGameWorld( GameWorld *gw, float delta ) {

    updateCamera( gw );

    for ( int i = 0; i < gw->entityCount; i++ ) {
        gw->entities[i].update( &gw->entities[i], &gw->camera, delta );
    }

    float m = -GetMouseWheelMove();
    cameraDistance += cameraSpeed * m * delta;

}

/**
 * @brief Draws the state of the game.
 */
void drawGameWorld( GameWorld *gw ) {

    BeginDrawing();
    ClearBackground( WHITE );
    BeginMode3D( gw->camera );

    for ( int i = 0; i < gw->entityCount; i++ ) {
        gw->entities[i].draw( &gw->entities[i] );
    }

    DrawGrid( 100, 1 );

    EndMode3D();
    EndDrawing();

}

static void updateCamera( GameWorld *gw ) {

    Camera3D *c = &gw->camera;

    if ( IsKeyDown( KEY_DELETE ) ) {
        cameraAngle += 1;
    }

    if ( IsKeyDown( KEY_PAGE_DOWN ) ) {
        cameraAngle += -1;
    }

    c->position = (Vector3) {
        .x = cameraDistance * cosf( DEG2RAD * cameraAngle ),
        .y = 5.0f,
        .z = cameraDistance * sinf( DEG2RAD * cameraAngle )
    };

    //c->target = e->pos;

}