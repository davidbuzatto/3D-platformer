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
#include "raylib/raymath.h"

#include "Macros.h"

#ifdef USE_GUI
#define RAYGUI_IMPLEMENTATION    // to use raygui, comment the USE_GUI define in Macros.h
#include "raylib/raygui.h"       // other compilation units must only include
#undef RAYGUI_IMPLEMENTATION     // raygui.h
#endif

#include "EditorCamera.h"
#include "GameplayCamera.h"
#include "GameWorld.h"
#include "Gizmo.h"
#include "MapPiece.h"
#include "MapPieceEditor.h"
#include "MapPieceModelPicker.h"
#include "MapSerializer.h"
#include "ResourceManager.h"

#define MAP_FILE_PATH "resources/maps/collisionTestMap.txt"
#define CENTER_LOADED_MAP false

typedef enum {
    EDITOR_MODE_SELECT_MAP_PIECE,
    EDITOR_MODE_ADD_MAP_PIECE,
    EDITOR_MODE_SELECT_MAP_PIECE_MODEL_TYPE,
} EditorMode;

typedef enum {
    MAP_START_MODE_EMPTY,
    MAP_START_MODE_FILLED,
    MAP_START_MODE_LOAD_TEST_MAP
} MapStartMode;

static void drawEditorHud( void );

static bool drawDebugInfo = true;
static bool playMode = true;
static Vector3 playerStartPos = { 1.89f, 1.0f, 4.24f };

// editor state
static EditorMode editorMode = EDITOR_MODE_SELECT_MAP_PIECE;

// map start mode
static MapStartMode mapStartMode = MAP_START_MODE_LOAD_TEST_MAP;

/**
 * @brief Creates a dinamically allocated GameWorld struct instance.
 */
GameWorld *createGameWorld( void ) {

    SetExitKey( KEY_NULL );

    GameWorld *gw = (GameWorld*) malloc( sizeof( GameWorld ) );

    gw->maxMapPieces = 1000;
    gw->mapPiecesCount = 0;
    gw->mapPieces = (MapPiece*) malloc( sizeof( MapPiece ) * gw->maxMapPieces );

    if ( mapStartMode == MAP_START_MODE_FILLED ) {

        int rows = 7;
        int cols = 5;
        float mapPieceSpacing = 0.2f;

        MapPieceModelType modelTypes[7] = {
            MODEL_TYPE_BLOCK_GRASS_LARGE_TALL,
            MODEL_TYPE_BLOCK_GRASS_LARGE,
            MODEL_TYPE_BLOCK_GRASS_LONG,
            MODEL_TYPE_BLOCK_GRASS_CURVE,
            MODEL_TYPE_BLOCK_GRASS,
            MODEL_TYPE_BLOCK_GRASS_CORNER,
            MODEL_TYPE_BLOCK_GRASS_EDGE
        };

        float startPosZ = 0;

        for ( int i = 0; i < rows; i++ ) {

            BoundingBox bb = GetModelBoundingBox( rm->mapPieceModelAtlas[modelTypes[i]] );
            float mapPieceSizeZ = bb.max.z - bb.min.z;

            startPosZ -= mapPieceSizeZ;

            if ( i < rows - 1 ) {
                startPosZ -= mapPieceSpacing;
            }

        }

        startPosZ /= 2.0f;

        for ( int i = 0; i < rows; i++ ) {

            BoundingBox bb = GetModelBoundingBox( rm->mapPieceModelAtlas[modelTypes[i]] );

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
                    modelTypes[i]
                );

                gw->mapPiecesCount++;

            }

            startPosZ += mapPieceSizeZ + mapPieceSpacing;

        }

    } else if ( mapStartMode == MAP_START_MODE_LOAD_TEST_MAP ) {
        loadMap( MAP_FILE_PATH, gw, CENTER_LOADED_MAP );
    }

    initPlayer( &gw->player, playerStartPos );

    gw->camera = (Camera3D) {
        .fovy = 60.0f,
        .position = (Vector3) { 0.0f, 5.0f, 8.0f },
        .projection = CAMERA_PERSPECTIVE,
        .target = (Vector3) { 0 },
        .up = (Vector3) { 0, 1, 0 }
    };

    initMapPieceModelPicker();

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

    Camera *camera = &gw->camera;

    if ( IsKeyPressed( KEY_F10 ) ) {
        playMode = !playMode;
    }

    if ( IsKeyPressed( KEY_F11 ) ) {
        gw->player.pos = playerStartPos;
    }

    if ( playMode ) {
        gw->player.input( &gw->player, camera );
        gw->player.update( &gw->player, gw->mapPieces, gw->mapPiecesCount, delta );
        updateGameplayCamera( camera, gw->player.pos, delta );
        return;
    }

    updateEditorCamera( camera, delta );

    for ( int i = 0; i < gw->mapPiecesCount; i++ ) {
        gw->mapPieces[i].update( &gw->mapPieces[i] );
    }

    if ( editorMode == EDITOR_MODE_SELECT_MAP_PIECE ) {

        updateMapPieceSelectionAndGizmo( gw, camera );

        MapPiece *mp = getSelectedMapPiece();

        if ( IsKeyPressed( KEY_R ) && mp != NULL ) {
            mp->rot = (Vector3) { 0 };
            mp->sca = (Vector3) { 1.0f, 1.0f, 1.0f };
        }

        if ( IsKeyPressed( KEY_DELETE ) && mp != NULL ) {
            removeMapPiece( mp, gw );
        }

        if ( IsKeyPressed( KEY_ESCAPE ) ) {
            deselectSelectedMapPiece();
        }

        if ( IsKeyPressed( KEY_F1 ) ) setGizmoMode( GIZMO_MODE_TRANSLATE );
        if ( IsKeyPressed( KEY_F2 ) ) setGizmoMode( GIZMO_MODE_ROTATE );
        if ( IsKeyPressed( KEY_F3 ) ) setGizmoMode( GIZMO_MODE_SCALE );

        // duplicate the selected piece flush against itself, one bounding-box
        // size over in the given world-axis direction
        if ( IsKeyDown( KEY_LEFT_SHIFT ) ) {
            if ( IsKeyPressed( KEY_LEFT ) ) duplicateSelectedMapPiece( gw, DUPLICATE_OPERATION_X, -1.0f );
            if ( IsKeyPressed( KEY_RIGHT ) ) duplicateSelectedMapPiece( gw, DUPLICATE_OPERATION_X, 1.0f );
            if ( IsKeyPressed( KEY_UP ) ) duplicateSelectedMapPiece( gw, DUPLICATE_OPERATION_Z, -1.0f );
            if ( IsKeyPressed( KEY_DOWN ) ) duplicateSelectedMapPiece( gw, DUPLICATE_OPERATION_Z, 1.0f );
            if ( IsKeyPressed( KEY_PAGE_UP ) ) duplicateSelectedMapPiece( gw, DUPLICATE_OPERATION_Y, 1.0f );
            if ( IsKeyPressed( KEY_PAGE_DOWN ) ) duplicateSelectedMapPiece( gw, DUPLICATE_OPERATION_Y, -1.0f );
        }

    } else if ( editorMode == EDITOR_MODE_ADD_MAP_PIECE ) {

        if ( IsMouseButtonPressed( MOUSE_BUTTON_LEFT ) ) {
            addMapPiece( gw, getSelectedMapPieceModelType() );
        }

    } else if ( editorMode == EDITOR_MODE_SELECT_MAP_PIECE_MODEL_TYPE ) {
        updateMapPieceModelPicker();
    }

    if ( IsKeyPressed( KEY_F5 ) ) {
        editorMode = EDITOR_MODE_SELECT_MAP_PIECE;
        deselectSelectedMapPiece();
    }

    if ( IsKeyPressed( KEY_F6 ) ) {
        editorMode = EDITOR_MODE_ADD_MAP_PIECE;
        deselectSelectedMapPiece();
    }

    if ( IsKeyPressed( KEY_F7 ) ) {
        editorMode = EDITOR_MODE_SELECT_MAP_PIECE_MODEL_TYPE;
        deselectSelectedMapPiece();
    }

    if ( IsKeyPressed( KEY_F9 ) ) drawDebugInfo = !drawDebugInfo;

    if ( IsKeyDown( KEY_LEFT_CONTROL ) && IsKeyPressed( KEY_S ) ) {
        saveMap( MAP_FILE_PATH, gw );
    }

    if ( IsKeyDown( KEY_LEFT_CONTROL ) && IsKeyPressed( KEY_O ) ) {
        deselectSelectedMapPiece();
        loadMap( MAP_FILE_PATH, gw, CENTER_LOADED_MAP );
    }

}

/**
 * @brief Draws the state of the game.
 */
void drawGameWorld( GameWorld *gw ) {

    BeginDrawing();
    ClearBackground( RAYWHITE );

    GizmoOperationMode currentGizmoOperationMode = toGizmoOperationMode( getGizmoMode() );

    BeginMode3D( gw->camera );
    //DrawModel( rm->seaModel, (Vector3) { 0.0f, 0.15f, 0.0f }, 1.0f, WHITE );
    for ( int i = 0; i < gw->mapPiecesCount; i++ ) {
        gw->mapPieces[i].draw( &gw->mapPieces[i], currentGizmoOperationMode );
    }
    gw->player.draw( &gw->player );
    if ( drawDebugInfo ) {
        DrawGrid( 100, 1 );
    }
    EndMode3D();

    drawEditorHud();

    EndDrawing();

}

static void drawEditorHud( void ) {

    const int marginTop = 10;
    const int marginLeft = 10;
    Vector2 pos = { 10, 10 };

    DrawRectangleRounded(
        (Rectangle) { pos.x, pos.y, GetScreenWidth() - marginLeft * 2, 40 },
        0.2f,
        10,
        Fade( WHITE, 0.7f )
    );

    DrawRectangleRoundedLinesEx(
        (Rectangle) { pos.x, pos.y, GetScreenWidth() - marginLeft * 2, 40 },
        0.2f,
        10,
        2.0f,
        BLACK
    );

    switch ( editorMode ) {
        case EDITOR_MODE_SELECT_MAP_PIECE:
            DrawTextEx(
                rm->baseFont,
                "Editor Mode: SELECT",
                (Vector2) { pos.x + marginLeft, pos.y + marginTop },
                20, 0.0f, BLACK
            );
            break;
        case EDITOR_MODE_ADD_MAP_PIECE:
            DrawTextEx(
                rm->baseFont,
                "Editor Mode: ADD",
                (Vector2) { pos.x + marginLeft, pos.y + marginTop },
                20, 0.0f, BLACK
            );
            break;
        case EDITOR_MODE_SELECT_MAP_PIECE_MODEL_TYPE:
            DrawTextEx(
                rm->baseFont,
                "Editor Mode: SELECT MAP PIECE MODEL TYPE",
                (Vector2) { pos.x + marginLeft, pos.y + marginTop },
                20, 0.0f, BLACK
            );
            break;
    }

    drawMapPiecePropertiesPanel();

    if ( editorMode == EDITOR_MODE_SELECT_MAP_PIECE_MODEL_TYPE ) {
        drawMapPieceModelPicker();
    }

}
