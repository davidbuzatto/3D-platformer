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
static void drawSelectedMapPieceDebugData( MapPiece *mp );
static void drawOptionsHud( void );

static bool drawDebugInfo = true;

static float cameraAngle      = 90.0f;
static float cameraDistance   = 5.0f;
static float cameraSpeed      = 10.0f;
static float cameraAngleSpeed = 60.0f;

static MapPiece *selectedMapPiece = NULL;

typedef enum {
    EDIT_MODE_TRANSLATE,
    EDIT_MODE_ROTATE,
    EDIT_MODE_SCALE,
} EditMode;

static EditMode editMode = EDIT_MODE_TRANSLATE;

/**
 * @brief Creates a dinamically allocated GameWorld struct instance.
 */
GameWorld *createGameWorld( void ) {

    GameWorld *gw = (GameWorld*) malloc( sizeof( GameWorld ) );

    int rows = 7;
    int cols = 5;
    gw->mapPieceCount = rows * cols;
    gw->mapPieces = (MapPiece*) malloc( sizeof( MapPiece ) * gw->mapPieceCount );

    float mapPieceSpacing = 0.2f;

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

        startPosZ -= mapPieceSizeZ;

        if ( i < rows - 1 ) {
            startPosZ -= mapPieceSpacing;
        }

    }

    startPosZ /= 2.0f;

    for ( int i = 0; i < rows; i++ ) {

        Model baseModel = models[i];
        BoundingBox bb = GetModelBoundingBox( baseModel );

        float mapPieceSizeX = bb.max.x - bb.min.x;
        float mapPieceSizeZ = bb.max.z - bb.min.z;

        float startPosX = - ( ( cols * mapPieceSizeX + ( cols - 1 ) * mapPieceSpacing ) / 2.0f - mapPieceSizeX / 2.0f );
        float rowCenter = startPosZ + mapPieceSizeZ / 2.0f;

        for ( int j = 0; j < cols; j++ ) {

            int p = i * cols + j;

            initMapPiece( 
                &gw->mapPieces[p], 
                (Vector3) { 
                    startPosX + j * (mapPieceSizeX + mapPieceSpacing), 
                    0,
                    rowCenter
                },
                baseModel
            );

        }
        
        startPosZ += mapPieceSizeZ + mapPieceSpacing;

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

    static int yMousePos = 0;

    updateCamera( gw, delta );

    for ( int i = 0; i < gw->mapPieceCount; i++ ) {
        gw->mapPieces[i].update( &gw->mapPieces[i], &gw->camera, delta );
    }

    if ( selectedMapPiece == NULL ) {

        for ( int i = 0; i < gw->mapPieceCount; i++ ) {

            MapPiece *mp = &gw->mapPieces[i];
            bool select = false;

            mp->moveAnchor.xmp.selected = false;
            mp->moveAnchor.zmp.selected = false;
            mp->moveAnchor.ymp.selected = false;

            switch ( checkCollisionMouseMoveAnchor( &mp->moveAnchor, &gw->camera ) ) {
                case MOVE_ANCHOR_COLLISION_TYPE_NONE:
                    select = false;
                    break;
                case MOVE_ANCHOR_COLLISION_TYPE_XY:
                    mp->moveAnchor.xmp.selected = true;
                    select = true;
                    break;
                case MOVE_ANCHOR_COLLISION_TYPE_XZ:
                    mp->moveAnchor.zmp.selected = true;
                    select = true;
                    break;
                case MOVE_ANCHOR_COLLISION_TYPE_YZ:
                    mp->moveAnchor.ymp.selected = true;
                    select = true;
                    break;
            }

            if ( select && IsMouseButtonPressed( MOUSE_BUTTON_LEFT ) ) {
                selectedMapPiece = mp;
                yMousePos = GetMouseY();
                break;
            }

        }

    }

    if ( IsMouseButtonReleased( MOUSE_BUTTON_LEFT ) ) {
        if ( selectedMapPiece != NULL ) {
            selectedMapPiece->moveAnchor.xmp.selected = false;
            selectedMapPiece->moveAnchor.zmp.selected = false;
            selectedMapPiece->moveAnchor.ymp.selected = false;
        }
        selectedMapPiece = NULL;
    }

    if ( selectedMapPiece != NULL ) {

        MapPiece *mp = selectedMapPiece;

        const float moveAmount = 0.1f;
        const float rotateAmount = 10.0f;
        const float scaleAmount = 0.1f;

        float xAmount = 0.0f;
        float yAmount = 0.0f;
        float zAmount = 0.0f;

        //int ud = ( IsKeyPressed( KEY_UP ) ? 1 : 0 ) + ( IsKeyPressed( KEY_DOWN ) ?  -1 : 0 );
        int ud = yMousePos - GetMouseY();
        yMousePos = GetMouseY();

        if ( editMode == EDIT_MODE_TRANSLATE ) {

            if ( mp->moveAnchor.xmp.selected ) {
                xAmount = moveAmount * ud;
            } else if ( mp->moveAnchor.ymp.selected ) {
                yAmount = moveAmount * ud;
            } else if ( mp->moveAnchor.zmp.selected ) {
                zAmount = moveAmount * ud;
            }

            mp->pos.x += xAmount;
            mp->pos.y += yAmount;
            mp->pos.z += zAmount;

            mp->bb.min.x += xAmount;
            mp->bb.max.x += xAmount;
            mp->bb.min.y += yAmount;
            mp->bb.max.y += yAmount;
            mp->bb.min.z += zAmount;
            mp->bb.max.z += zAmount;

        } else if ( editMode == EDIT_MODE_ROTATE ) {

            if ( mp->moveAnchor.xmp.selected ) {
                xAmount = rotateAmount * ud;
            } else if ( mp->moveAnchor.ymp.selected ) {
                yAmount = rotateAmount * ud;
            } else if ( mp->moveAnchor.zmp.selected ) {
                zAmount = rotateAmount * ud;
            }

            mp->rot.x += xAmount;
            mp->rot.y += yAmount;
            mp->rot.z += zAmount;

        } else if ( editMode == EDIT_MODE_SCALE ) {

            if ( mp->moveAnchor.xmp.selected ) {
                xAmount = scaleAmount * ud;
            } else if ( mp->moveAnchor.ymp.selected ) {
                yAmount = scaleAmount * ud;
            } else if ( mp->moveAnchor.zmp.selected ) {
                zAmount = scaleAmount * ud;
            }

            mp->sca.x += xAmount;
            mp->sca.y += yAmount;
            mp->sca.z += zAmount;

        }

        mp->update( mp, &gw->camera, delta );

    }

    if ( IsKeyPressed( KEY_F1 ) ) drawDebugInfo = !drawDebugInfo;
    if ( IsKeyPressed( KEY_ONE ) ) editMode = EDIT_MODE_TRANSLATE;
    if ( IsKeyPressed( KEY_TWO ) ) editMode = EDIT_MODE_ROTATE;
    if ( IsKeyPressed( KEY_THREE ) ) editMode = EDIT_MODE_SCALE;

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
    drawOptionsHud();

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

static void drawSelectedMapPieceDebugData( MapPiece *mp ) {

    if ( mp != NULL ) {

        Vector2 pos = GetMousePosition();
        const int marginLeft = 10;
        const int marginTop = 10;

        const char *tx = TextFormat( "x: %.2f", mp->pos.x );
        const char *ty = TextFormat( "y: %.2f", mp->pos.y );
        const char *tz = TextFormat( "z: %.2f", mp->pos.z );

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

static void drawOptionsHud( void ) {

    switch ( editMode ) {
        case EDIT_MODE_TRANSLATE: DrawText( "Mode: TRANSLATE", 10, 10, 20, BLACK ); break;
        case EDIT_MODE_ROTATE:    DrawText( "Mode: ROTATE", 10, 10, 20, BLACK ); break;
        case EDIT_MODE_SCALE:     DrawText( "Mode: SCALE", 10, 10, 20, BLACK ); break;
    }

}