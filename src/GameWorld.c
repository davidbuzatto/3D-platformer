/**
 * @file GameWorld.c
 * @author Prof. Dr. David Buzatto
 * @brief GameWorld implementation.
 * 
 * @copyright Copyright (c) 2026
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "raylib/raylib.h"
//#include "raylib/raymath.h"
//#define RAYGUI_IMPLEMENTATION    // to use raygui, comment these three lines.
//#include "raylib/raygui.h"       // other compilation units must only include
//#undef RAYGUI_IMPLEMENTATION     // raygui.h

#include "MapPiece.h"
#include "GameWorld.h"
#include "MoveAnchor.h"
#include "ResourceManager.h"

static void updateCamera( GameWorld *gw, float delta );
static void drawSelectedMapPieceDebugData( MapPiece *e );

static bool drawDebugInfo = true;

static float cameraAngle      = 90.0f;
static float cameraDistance   = 5.0f;
static float cameraSpeed      = 10.0f;
static float cameraAngleSpeed = 60.0f;

static MapPiece *selectedMapPiece = NULL;

/**
 * @brief Creates a dinamically allocated GameWorld struct instance.
 */
GameWorld *createGameWorld( void ) {

    GameWorld *gw = (GameWorld*) malloc( sizeof( GameWorld ) );

    int rows = 7;
    int cols = 5;
    gw->mapPieceCount = rows * cols;
    gw->mapPieces = (MapPiece*) malloc( sizeof( MapPiece ) * gw->mapPieceCount );

    float mapPieceSpacing = 0.0f;

    Model models[7] = {
        rm->blockGrassLargeTallModel,
        rm->blockGrassLargeModel,
        rm->blockGrassLongModel,
        rm->blockGrassCurveModel,
        rm->blockGrassModel,
        rm->blockGrassCornerModel,
        rm->blockGrassEdgeModel,
    };

    float startPosZ = 0;

    for ( int i = 0; i < rows; i++ ) {

        Model baseModel = models[i];
        BoundingBox bb = GetModelBoundingBox( baseModel );
        float mapPieceSizeZ = bb.max.z - bb.min.z;

        startPosZ -= mapPieceSizeZ + mapPieceSpacing;

    }

    for ( int i = 0; i < rows; i++ ) {

        Model baseModel = models[i];
        BoundingBox bb = GetModelBoundingBox( baseModel );

        float mapPieceSizeX = bb.max.x - bb.min.x;
        float mapPieceSizeZ = bb.max.z - bb.min.z;

        float startPosX = - ( ( cols * mapPieceSizeX + ( cols - 1 ) * mapPieceSpacing ) / 2.0f - mapPieceSizeX / 2.0f );

        for ( int j = 0; j < cols; j++ ) {

            int p = i * cols + j;

            initMapPiece( 
                &gw->mapPieces[p], 
                (Vector3) { 
                    startPosX + j * (mapPieceSizeX + mapPieceSpacing), 
                    0, 
                    startPosZ + i * (mapPieceSizeZ + mapPieceSpacing)
                },
                baseModel
            );

        }
        
        startPosZ += mapPieceSizeZ;

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
        if ( gw->mapPieces != NULL ) {
            free( gw->mapPieces );
        }
        free( gw );
    }
}

/**
 * @brief Reads user input and updates the state of the game.
 */
void updateGameWorld( GameWorld *gw, float delta ) {

    updateCamera( gw, delta );

    for ( int i = 0; i < gw->mapPieceCount; i++ ) {
        gw->mapPieces[i].update( &gw->mapPieces[i], &gw->camera, delta );
    }

    if ( selectedMapPiece == NULL ) {

        for ( int i = 0; i < gw->mapPieceCount; i++ ) {

            MapPiece *e = &gw->mapPieces[i];
            bool select = false;

            e->moveAnchor.xymp.selected = false;
            e->moveAnchor.xzmp.selected = false;
            e->moveAnchor.yzmp.selected = false;

            switch ( checkCollisionMouseMoveAnchor( &e->moveAnchor, &gw->camera ) ) {
                case MOVE_ANCHOR_COLLISION_TYPE_NONE:
                    select = false;
                    break;
                case MOVE_ANCHOR_COLLISION_TYPE_XY:
                    e->moveAnchor.xymp.selected = true;
                    select = true;
                    break;
                case MOVE_ANCHOR_COLLISION_TYPE_XZ:
                    e->moveAnchor.xzmp.selected = true;
                    select = true;
                    break;
                case MOVE_ANCHOR_COLLISION_TYPE_YZ:
                    e->moveAnchor.yzmp.selected = true;
                    select = true;
                    break;
            }

            if ( select && IsMouseButtonPressed( MOUSE_BUTTON_LEFT ) ) {
                selectedMapPiece = e;
                break;
            }

        }

    }

    if ( IsMouseButtonReleased( MOUSE_BUTTON_LEFT ) ) {
        if ( selectedMapPiece != NULL ) {
            selectedMapPiece->moveAnchor.xymp.selected = false;
            selectedMapPiece->moveAnchor.xzmp.selected = false;
            selectedMapPiece->moveAnchor.yzmp.selected = false;
        }
        selectedMapPiece = NULL;
    }

    if ( selectedMapPiece != NULL ) {

        MapPiece *e = selectedMapPiece;

        const float moveAmount = 0.1f;
        float xAmount = 0.0f;
        float yAmount = 0.0f;
        float zAmount = 0.0f;

        if ( e->moveAnchor.xymp.selected ) {
            int h = ( IsKeyPressed( KEY_LEFT ) ? -1 : 0 ) + ( IsKeyPressed( KEY_RIGHT ) ?  1 : 0 );
            int v = ( IsKeyPressed( KEY_UP )   ?  1 : 0 ) + ( IsKeyPressed( KEY_DOWN )  ? -1 : 0 );
            xAmount = moveAmount * h;
            yAmount = moveAmount * v;
            //e->vel.x = e->baseSpeed * h;
            //e->vel.y = e->baseSpeed * v;
        } else if ( e->moveAnchor.xzmp.selected ) {
            int h = ( IsKeyPressed( KEY_LEFT ) ? -1 : 0 ) + ( IsKeyPressed( KEY_RIGHT ) ? 1 : 0 );
            int f = ( IsKeyPressed( KEY_UP )   ? -1 : 0 ) + ( IsKeyPressed( KEY_DOWN )  ? 1 : 0 );
            xAmount = moveAmount * h;
            zAmount = moveAmount * f;
            //e->vel.x = e->baseSpeed * h;
            //e->vel.z = e->baseSpeed * f;
        } else if ( e->moveAnchor.yzmp.selected ) {
            int v = ( IsKeyPressed( KEY_UP )   ?  1 : 0 ) + ( IsKeyPressed( KEY_DOWN )  ? -1 : 0 );
            int f = ( IsKeyPressed( KEY_LEFT ) ? -1 : 0 ) + ( IsKeyPressed( KEY_RIGHT ) ? 1 : 0 );
            yAmount = moveAmount * v;
            zAmount = moveAmount * f;
            //e->vel.y = e->baseSpeed * v;
            //e->vel.z = e->baseSpeed * f;
        }

        e->pos.x += xAmount;
        e->pos.y += yAmount;
        e->pos.z += zAmount;

        e->bb.min.x += xAmount;
        e->bb.max.x += xAmount;
        e->bb.min.y += yAmount;
        e->bb.max.y += yAmount;
        e->bb.min.z += zAmount;
        e->bb.max.z += zAmount;

        e->update( e, &gw->camera, delta );

    }

    if ( IsKeyPressed( KEY_F1 ) ) {
        drawDebugInfo = !drawDebugInfo;
    }

}

/**
 * @brief Draws the state of the game.
 */
void drawGameWorld( GameWorld *gw ) {

    BeginDrawing();
    ClearBackground( WHITE );

    BeginMode3D( gw->camera );
    for ( int i = 0; i < gw->mapPieceCount; i++ ) {
        gw->mapPieces[i].draw( &gw->mapPieces[i], drawDebugInfo );
    }
    DrawGrid( 100, 1 );
    EndMode3D();

    drawSelectedMapPieceDebugData( selectedMapPiece );

    EndDrawing();

}

static void updateCamera( GameWorld *gw, float delta ) {

    Camera3D *c = &gw->camera;

    if ( selectedMapPiece == NULL ) {

        if ( IsKeyDown( KEY_UP ) ) {
            c->position.y += cameraSpeed * delta;
        }

        if ( IsKeyDown( KEY_DOWN ) ) {
            c->position.y -= cameraSpeed * delta;
        }

        if ( IsKeyDown( KEY_LEFT ) ) {
            cameraAngle += cameraAngleSpeed * delta;
        }

        if ( IsKeyDown( KEY_RIGHT ) ) {
            cameraAngle -= cameraAngleSpeed * delta;
        }

    }

    float m = -GetMouseWheelMove();
    cameraDistance += cameraSpeed * m * delta;

    c->position.x = cameraDistance * cosf( DEG2RAD * cameraAngle );
    c->position.z = cameraDistance * sinf( DEG2RAD * cameraAngle );

}

static void drawSelectedMapPieceDebugData( MapPiece *e ) {

    if ( e != NULL ) {

        Vector2 pos = GetMousePosition();
        const int marginLeft = 10;
        const int marginTop = 10;

        const char *tx = TextFormat( "x: %.2f", e->pos.x );
        const char *ty = TextFormat( "y: %.2f", e->pos.y );
        const char *tz = TextFormat( "z: %.2f", e->pos.z );

        DrawRectangleRounded(
            (Rectangle) { pos.x, pos.y, 100, 80 },
            0.5f,
            10,
            Fade( WHITE, 0.5f )
        );

        DrawText( tx, pos.x + marginLeft, pos.y + marginTop, 20, BLACK );
        DrawText( ty, pos.x + marginLeft, pos.y + marginTop + 20, 20, BLACK );
        DrawText( tz, pos.x + marginLeft, pos.y + marginTop + 40, 20, BLACK );

        
    }

}