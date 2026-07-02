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
#include "Gizmo.h"
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
    GIZMO_MODE_TRANSLATE,
    GIZMO_MODE_ROTATE,
    GIZMO_MODE_SCALE,
} EditMode;

static EditMode gizmoMode = GIZMO_MODE_TRANSLATE;

/**
 * @brief Creates a dinamically allocated GameWorld struct instance.
 */
GameWorld *createGameWorld( void ) {

    GameWorld *gw = (GameWorld*) malloc( sizeof( GameWorld ) );

    int rows = 7;
    int cols = 5;
    /*int rows = 1;
    int cols = 1;*/
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

            mp->gizmo.xAxis.selected = false;
            mp->gizmo.zAxis.selected = false;
            mp->gizmo.yAxis.selected = false;

            switch ( checkCollisionMouseGizmo( &mp->gizmo, &gw->camera ) ) {
                case GIZMO_AXIS_COLLISION_TYPE_NONE:
                    select = false;
                    break;
                case GIZMO_AXIS_COLLISION_TYPE_XY:
                    mp->gizmo.xAxis.selected = true;
                    select = true;
                    break;
                case GIZMO_AXIS_COLLISION_TYPE_XZ:
                    mp->gizmo.zAxis.selected = true;
                    select = true;
                    break;
                case GIZMO_AXIS_COLLISION_TYPE_YZ:
                    mp->gizmo.yAxis.selected = true;
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
            selectedMapPiece->gizmo.xAxis.selected = false;
            selectedMapPiece->gizmo.zAxis.selected = false;
            selectedMapPiece->gizmo.yAxis.selected = false;
        }
        selectedMapPiece = NULL;
    }

    if ( selectedMapPiece != NULL ) {

        MapPiece *mp = selectedMapPiece;

        const float translateAmount = 0.1f;
        const float rotateAmount = 1.0f;
        const float scaleAmount = 0.05f;

        float xAmount = 0.0f;
        float yAmount = 0.0f;
        float zAmount = 0.0f;

        //int ud = ( IsKeyPressed( KEY_UP ) ? 1 : 0 ) + ( IsKeyPressed( KEY_DOWN ) ?  -1 : 0 );
        int ud = yMousePos - GetMouseY();
        yMousePos = GetMouseY();

        if ( gizmoMode == GIZMO_MODE_TRANSLATE ) {

            if ( mp->gizmo.xAxis.selected ) {
                xAmount = translateAmount * ud;
            } else if ( mp->gizmo.yAxis.selected ) {
                yAmount = translateAmount * ud;
            } else if ( mp->gizmo.zAxis.selected ) {
                zAmount = translateAmount * ud;
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

        } else if ( gizmoMode == GIZMO_MODE_ROTATE ) {

            if ( mp->gizmo.xAxis.selected ) {
                xAmount = rotateAmount * ud;
            } else if ( mp->gizmo.yAxis.selected ) {
                yAmount = rotateAmount * ud;
            } else if ( mp->gizmo.zAxis.selected ) {
                zAmount = rotateAmount * ud;
            }

            mp->rot.x += xAmount;
            mp->rot.y += yAmount;
            mp->rot.z += zAmount;

        } else if ( gizmoMode == GIZMO_MODE_SCALE ) {

            if ( mp->gizmo.xAxis.selected ) {
                xAmount = scaleAmount * ud;
            } else if ( mp->gizmo.yAxis.selected ) {
                yAmount = scaleAmount * ud;
            } else if ( mp->gizmo.zAxis.selected ) {
                zAmount = scaleAmount * ud;
            }

            mp->sca.x += xAmount;
            mp->sca.y += yAmount;
            mp->sca.z += zAmount;

        }

        mp->update( mp, &gw->camera, delta );

    }

    if ( IsKeyPressed( KEY_F1 ) ) drawDebugInfo = !drawDebugInfo;
    if ( IsKeyPressed( KEY_ONE ) ) gizmoMode = GIZMO_MODE_TRANSLATE;
    if ( IsKeyPressed( KEY_TWO ) ) gizmoMode = GIZMO_MODE_ROTATE;
    if ( IsKeyPressed( KEY_THREE ) ) gizmoMode = GIZMO_MODE_SCALE;

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
        const int marginTop = 10;
        int marginLeft = 10;

        DrawRectangleRounded(
            (Rectangle) { pos.x, pos.y, 300, 100 },
            0.2f,
            10,
            Fade( WHITE, 0.5f )
        );

        const char *px = TextFormat( "x: %.2f", mp->pos.x );
        const char *py = TextFormat( "y: %.2f", mp->pos.y );
        const char *pz = TextFormat( "z: %.2f", mp->pos.z );

        DrawTextEx( rm->baseFont, "Position:", (Vector2) { pos.x + marginLeft, pos.y + marginTop      }, 20, 0.0f, BLACK );
        DrawTextEx( rm->baseFont, px,          (Vector2) { pos.x + marginLeft, pos.y + marginTop + 20 }, 20, 0.0f, MAROON );
        DrawTextEx( rm->baseFont, py,          (Vector2) { pos.x + marginLeft, pos.y + marginTop + 40 }, 20, 0.0f, DARKGREEN );
        DrawTextEx( rm->baseFont, pz,          (Vector2) { pos.x + marginLeft, pos.y + marginTop + 60 }, 20, 0.0f, DARKBLUE );


        const char *ax = TextFormat( "x: %.2f", mp->rot.x );
        const char *ay = TextFormat( "y: %.2f", mp->rot.y );
        const char *az = TextFormat( "z: %.2f", mp->rot.z );

        marginLeft += 100;
        DrawTextEx( rm->baseFont, "Rotation:", (Vector2) { pos.x + marginLeft, pos.y + marginTop      }, 20, 0.0f, BLACK );
        DrawTextEx( rm->baseFont, ax,          (Vector2) { pos.x + marginLeft, pos.y + marginTop + 20 }, 20, 0.0f, MAROON );
        DrawTextEx( rm->baseFont, ay,          (Vector2) { pos.x + marginLeft, pos.y + marginTop + 40 }, 20, 0.0f, DARKGREEN );
        DrawTextEx( rm->baseFont, az,          (Vector2) { pos.x + marginLeft, pos.y + marginTop + 60 }, 20, 0.0f, DARKBLUE );


        const char *sx = TextFormat( "x: %.2f", mp->sca.x );
        const char *sy = TextFormat( "y: %.2f", mp->sca.y );
        const char *sz = TextFormat( "z: %.2f", mp->sca.z );

        marginLeft += 100;
        DrawTextEx( rm->baseFont, "Scale:",    (Vector2) { pos.x + marginLeft, pos.y + marginTop      }, 20, 0.0f, BLACK );
        DrawTextEx( rm->baseFont, sx,          (Vector2) { pos.x + marginLeft, pos.y + marginTop + 20 }, 20, 0.0f, MAROON );
        DrawTextEx( rm->baseFont, sy,          (Vector2) { pos.x + marginLeft, pos.y + marginTop + 40 }, 20, 0.0f, DARKGREEN );
        DrawTextEx( rm->baseFont, sz,          (Vector2) { pos.x + marginLeft, pos.y + marginTop + 60 }, 20, 0.0f, DARKBLUE );
        
    }

}

static void drawOptionsHud( void ) {

    switch ( gizmoMode ) {
        case GIZMO_MODE_TRANSLATE: DrawTextEx( rm->baseFont, "Gizmo Mode: TRANSLATE", (Vector2) { 10, 10 }, 20, 0.0f, BLACK ); break;
        case GIZMO_MODE_ROTATE:    DrawTextEx( rm->baseFont, "Gizmo Mode: ROTATE",    (Vector2) { 10, 10 }, 20, 0.0f, BLACK ); break;
        case GIZMO_MODE_SCALE:     DrawTextEx( rm->baseFont, "Gizmo Mode: SCALE",     (Vector2) { 10, 10 }, 20, 0.0f, BLACK ); break;
    }

}