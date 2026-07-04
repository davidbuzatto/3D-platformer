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
//#define RAYGUI_IMPLEMENTATION    // to use raygui, comment these three lines.
//#include "raylib/raygui.h"       // other compilation units must only include
//#undef RAYGUI_IMPLEMENTATION     // raygui.h

#include "GameWorld.h"
#include "Gizmo.h"
#include "Macros.h"
#include "MapPiece.h"
#include "ResourceManager.h"

#define MAP_FILE_PATH "resources/maps/testMap.txt"

typedef enum {
    EDITOR_MODE_SELECT_MAP_PIECE,
    EDITOR_MODE_ADD_MAP_PIECE,
    EDITOR_MODE_SELECT_MAP_PIECE_MODEL_TYPE,
    EDITOR_MODE_TRANSLATE_MAP_PIECE,
    EDITOR_MODE_ROTATE_MAP_PIECE,
    EDITOR_MODE_SCALE_MAP_PIECE,
} EditorMode;

typedef enum {
    MAP_START_MODE_EMPTY,
    MAP_START_MODE_FILLED,
    MAP_START_MODE_LOAD_TEST_MAP
} MapStartMode;

static void updateCamera( Camera *camera, float delta );
static void drawEditorHud( void );

static void deselectSelectedMapPiece( void );
static MapPiece *getMapPieceFromRay( GameWorld *gw );
static RayCollision getAddRayCollisionFromRay( GameWorld *gw );
static bool getNearestMapPieceHit( GameWorld *gw, Ray ray, MapPiece **outMp, RayCollision *outRc );
static void addMapPiece( GameWorld *gw );
static void removeMapPiece( MapPiece *mp, GameWorld *gw );

static bool selectGizmoAxisFromSelectedMapPiece( MapPiece *mp, Camera *camera );
static void performGizmoOperation( MapPiece *mp, Camera *camera );
static GizmoOperationMode toGizmoOperationMode( EditorMode mode );
static float closestPointOnAxisToRay( Vector3 lineOrigin, Vector3 axisDir, Ray ray );

static void saveMap( const char *filePath, GameWorld *gw );
static void loadMap( const char *filePath, GameWorld *gw );

static bool drawDebugInfo = true;

// camera control
static float cameraYaw               = 90.0f;   // XZ plane angle (degrees)
static float cameraPitch             = 20.0f;   // pitch angle (degrees)
static float cameraDistance          = 8.0f;    // current (smoothed) distance to the target
static float cameraTargetDistance    = 8.0f;    // desired distance, set instantly by the wheel
static float cameraOrbitSpeed        = 0.2f;    // degrees per pixel (mouse movement)
static float cameraZoomSpeed         = 1.0f;    // units per wheel notch
static float cameraZoomSmoothing     = 10.0f;   // how fast distance chases the target (per second)
static float cameraPanSpeed          = 5.0f;    // units per second
static const float cameraPitchMin    = -85.0f;
static const float cameraPitchMax    = 85.0f;
static const float cameraDistanceMin = 1.5f;

// gizmo operation
static bool performingGizmoOperation = false;
static Vector3 gizmoDragStartPos = { 0 };
static Vector3 gizmoDragStartRot = { 0 };
static Vector3 gizmoDragStartSca = { 0 };
static Vector3 gizmoDragStartPlaneHit = { 0 };
static float gizmoDragStartT = 0.0f;
static float gizmoDragAccum = 0.0f;

// selected map piece to perform operations
static MapPiece *selectedMapPiece = NULL;

// variables for map piece model selection
static MapPieceModelType selectedMapPieceModelType = MODEL_TYPE_BLOCK_GRASS;
static Rectangle mouseHoverMapPieceModelRect = { 0, 0, 0, 0 };
static int hoveredMapPieceModelType = -1;  // -1 while the mouse isn't over any tile
static int targetWSelect;
static int targetHSelect;
static int piecesPerLineSelect;
static int pieceSizeSelect;
static int startXSelect;
static int startYSelect;
static int marginSelect;
static int spacingSelect;

// editor state
static EditorMode editorMode = EDITOR_MODE_SELECT_MAP_PIECE;
static EditorMode gizmoMode = EDITOR_MODE_TRANSLATE_MAP_PIECE;

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
        loadMap( MAP_FILE_PATH, gw );
    }

    gw->camera = (Camera3D) {
        .fovy = 60.0f,
        .position = (Vector3) { 0.0f, 5.0f, cameraDistance },
        .projection = CAMERA_PERSPECTIVE,
        .target = (Vector3) { 0 },
        .up = (Vector3) { 0, 1, 0 }
    };


    // variables for map piece model selection
    targetWSelect = rm->mapPieceModelAtlasPreviewTexture.width / 2;
    targetHSelect = rm->mapPieceModelAtlasPreviewTexture.height / 2;
    piecesPerLineSelect = 15;
    pieceSizeSelect = 32;
    startYSelect = 55;
    marginSelect = 5;
    spacingSelect = 3;

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
    updateCamera( camera, delta );

    for ( int i = 0; i < gw->mapPiecesCount; i++ ) {
        gw->mapPieces[i].update( &gw->mapPieces[i] );
    }

    if ( editorMode == EDITOR_MODE_SELECT_MAP_PIECE ) {

        if ( IsMouseButtonPressed( MOUSE_BUTTON_LEFT ) ) {

            // priority 1: gizmo operation for selected map piece
            if ( selectedMapPiece != NULL ) {
                if ( selectGizmoAxisFromSelectedMapPiece( selectedMapPiece, camera ) ) {
                    performingGizmoOperation = true;
                }
            }

            // priority 2: select a map piece
            if ( !performingGizmoOperation ) {

                MapPiece *mp = getMapPieceFromRay( gw );

                if ( mp != NULL ) {
                    deselectSelectedMapPiece();
                    selectedMapPiece = mp;
                    selectedMapPiece->selected = true;
                }

            }

        }

        if ( IsMouseButtonReleased( MOUSE_BUTTON_LEFT ) ) {
            if ( selectedMapPiece != NULL ) {
                selectedMapPiece->gizmo.xAxis.selected = false;
                selectedMapPiece->gizmo.zAxis.selected = false;
                selectedMapPiece->gizmo.yAxis.selected = false;
                selectedMapPiece->gizmo.center.selected = false;
                selectedMapPiece->gizmo.xyPlane.selected = false;
                selectedMapPiece->gizmo.xzPlane.selected = false;
                selectedMapPiece->gizmo.yzPlane.selected = false;
            }
            performingGizmoOperation = false;
        }

        if ( selectedMapPiece != NULL && performingGizmoOperation ) {
            performGizmoOperation( selectedMapPiece, camera );
        }

        if ( IsKeyPressed( KEY_R ) && selectedMapPiece != NULL ) {
            selectedMapPiece->rot = (Vector3) { 0 };
            selectedMapPiece->sca = (Vector3) { 1.0f, 1.0f, 1.0f };
        }

        if ( IsKeyPressed( KEY_DELETE ) && selectedMapPiece != NULL ) {
            removeMapPiece( selectedMapPiece, gw );
        }

        if ( IsKeyPressed( KEY_ESCAPE ) ) {
            deselectSelectedMapPiece();
        }

        if ( IsKeyPressed( KEY_ONE ) )   gizmoMode = EDITOR_MODE_TRANSLATE_MAP_PIECE;
        if ( IsKeyPressed( KEY_TWO ) )   gizmoMode = EDITOR_MODE_ROTATE_MAP_PIECE;
        if ( IsKeyPressed( KEY_THREE ) ) gizmoMode = EDITOR_MODE_SCALE_MAP_PIECE;
        
    } else if ( editorMode == EDITOR_MODE_ADD_MAP_PIECE ) {

        if ( IsMouseButtonPressed( MOUSE_BUTTON_LEFT ) ) {
            addMapPiece( gw );
        }

    } else if ( editorMode == EDITOR_MODE_SELECT_MAP_PIECE_MODEL_TYPE ) {

        int mouseX = GetMouseX();
        int mouseY = GetMouseY();
        startXSelect = GetScreenWidth() / 2 - targetWSelect / 2;

        hoveredMapPieceModelType = -1;

        if ( mouseX >= startXSelect + marginSelect && mouseX <= startXSelect + targetWSelect - marginSelect &&
             mouseY >= startYSelect + marginSelect && mouseY <= startYSelect + targetHSelect - marginSelect ) {

            int offsetX = mouseX - ( startXSelect + marginSelect );
            int offsetY = mouseY - ( startYSelect + marginSelect );

            int col = offsetX / ( pieceSizeSelect + spacingSelect );
            int row = offsetY / ( pieceSizeSelect + spacingSelect );

            mouseHoverMapPieceModelRect.x = startXSelect + marginSelect + col * ( pieceSizeSelect + spacingSelect );
            mouseHoverMapPieceModelRect.y = startYSelect + marginSelect + row * ( pieceSizeSelect + spacingSelect );
            mouseHoverMapPieceModelRect.width = mouseHoverMapPieceModelRect.height = pieceSizeSelect;

            int modelPos = row * piecesPerLineSelect + col;
            if ( modelPos < rm->mapPieceModelAtlasCount ) {
                hoveredMapPieceModelType = modelPos;
                if ( IsMouseButtonPressed( MOUSE_BUTTON_LEFT ) ) {
                    selectedMapPieceModelType = modelPos;
                }
            }

        }

    }

    if ( IsKeyPressed( KEY_F1 ) ) {
        editorMode = EDITOR_MODE_SELECT_MAP_PIECE;
        deselectSelectedMapPiece();
    }

    if ( IsKeyPressed( KEY_F2 ) ) {
        editorMode = EDITOR_MODE_ADD_MAP_PIECE;
        deselectSelectedMapPiece();
    }

    if ( IsKeyPressed( KEY_F3 ) ) {
        editorMode = EDITOR_MODE_SELECT_MAP_PIECE_MODEL_TYPE;
        deselectSelectedMapPiece();
    }

    if ( IsKeyPressed( KEY_F6 ) ) drawDebugInfo = !drawDebugInfo;

    if ( IsKeyDown( KEY_LEFT_CONTROL ) && IsKeyPressed( KEY_S ) ) {
        saveMap( MAP_FILE_PATH, gw );
    }

    if ( IsKeyDown( KEY_LEFT_CONTROL ) && IsKeyPressed( KEY_O ) ) {
        loadMap( MAP_FILE_PATH, gw );
    }

}

/**
 * @brief Draws the state of the game.
 */
void drawGameWorld( GameWorld *gw ) {

    BeginDrawing();
    ClearBackground( RAYWHITE );

    GizmoOperationMode currentGizmoOperationMode = toGizmoOperationMode( gizmoMode );

    BeginMode3D( gw->camera );
    for ( int i = 0; i < gw->mapPiecesCount; i++ ) {
        gw->mapPieces[i].draw( &gw->mapPieces[i], currentGizmoOperationMode );
    }
    if ( drawDebugInfo ) {
        DrawGrid( 100, 1 );
    }
    EndMode3D();

    drawEditorHud();

    EndDrawing();

}

static void updateCamera( Camera *camera, float delta ) {
    
    if ( IsMouseButtonDown( MOUSE_BUTTON_RIGHT ) ) {
        Vector2 mouseDelta = GetMouseDelta();
        cameraYaw   += mouseDelta.x * cameraOrbitSpeed;
        cameraPitch -= mouseDelta.y * cameraOrbitSpeed;
        cameraPitch = Clamp( cameraPitch, cameraPitchMin, cameraPitchMax );
    }

    cameraTargetDistance -= GetMouseWheelMove() * cameraZoomSpeed;
    if ( cameraTargetDistance < cameraDistanceMin ) {
        cameraTargetDistance = cameraDistanceMin;
    }

    // chases the target distance instead of jumping straight to it --
    // smooth, but doesn't change how big a wheel notch actually is
    cameraDistance += ( cameraTargetDistance - cameraDistance ) * cameraZoomSmoothing * delta;

    float yawRad   = DEG2RAD * cameraYaw;
    float pitchRad = DEG2RAD * cameraPitch;

    // ground-plane forward/right, derived from yaw only (pitch ignored so
    // moving "forward" never changes height) -- moves the target, which
    // stands in for the future player position
    Vector3 forward = { -cosf( yawRad ), 0.0f, -sinf( yawRad ) };
    Vector3 right   = {  sinf( yawRad ), 0.0f, -cosf( yawRad ) };
    float panAmount = cameraPanSpeed * delta;

    if ( IsKeyDown( KEY_W ) ) {
        camera->target.x += forward.x * panAmount;
        camera->target.z += forward.z * panAmount;
    }
    if ( !IsKeyDown( KEY_LEFT_CONTROL ) && IsKeyDown( KEY_S ) ) {
        camera->target.x -= forward.x * panAmount;
        camera->target.z -= forward.z * panAmount;
    }
    if ( IsKeyDown( KEY_D ) ) {
        camera->target.x += right.x * panAmount;
        camera->target.z += right.z * panAmount;
    }
    if ( IsKeyDown( KEY_A ) ) {
        camera->target.x -= right.x * panAmount;
        camera->target.z -= right.z * panAmount;
    }

    camera->position.x = camera->target.x + cameraDistance * cosf( pitchRad ) * cosf( yawRad );
    camera->position.y = camera->target.y + cameraDistance * sinf( pitchRad );
    camera->position.z = camera->target.z + cameraDistance * cosf( pitchRad ) * sinf( yawRad );

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
        default:
            break;
    }

    if ( selectedMapPiece != NULL ) {

        MapPiece *mp = selectedMapPiece;

        const int mpPropMarginTop = 10;
        int mpPropMarginLeft = 10;
        Vector2 mpPropPos = { 10, GetScreenHeight() - 120 - mpPropMarginTop };

        DrawRectangleRounded(
            (Rectangle) { mpPropPos.x, mpPropPos.y, 300, 120 },
            0.2f,
            10,
            Fade( WHITE, 0.7f )
        );

        DrawRectangleRoundedLinesEx(
            (Rectangle) { mpPropPos.x, mpPropPos.y, 300, 120 },
            0.2f,
            10,
            2.0f,
            BLACK
        );

        switch ( gizmoMode ) {
            case EDITOR_MODE_TRANSLATE_MAP_PIECE: 
                DrawTextEx( 
                    rm->baseFont, 
                    "Gizmo Mode: TRANSLATE", 
                    (Vector2) { mpPropPos.x + mpPropMarginLeft, mpPropPos.y + mpPropMarginTop }, 
                    20, 0.0f, BLACK
                );
                break;
            case EDITOR_MODE_ROTATE_MAP_PIECE:
                DrawTextEx( 
                    rm->baseFont, 
                    "Gizmo Mode: ROTATE",
                    (Vector2) { mpPropPos.x + mpPropMarginLeft, mpPropPos.y + mpPropMarginTop },
                    20, 0.0f, BLACK
                );
                break;
            case EDITOR_MODE_SCALE_MAP_PIECE:
                DrawTextEx( 
                    rm->baseFont,
                    "Gizmo Mode: SCALE",
                    (Vector2) { mpPropPos.x + mpPropMarginLeft, mpPropPos.y + mpPropMarginTop },
                    20, 0.0f,
                    BLACK
                );
                break;
            default:
                break;
        }

        const char *px = TextFormat( "x: %.2f", mp->pos.x );
        const char *py = TextFormat( "y: %.2f", mp->pos.y );
        const char *pz = TextFormat( "z: %.2f", mp->pos.z );

        DrawTextEx( rm->baseFont, "Position:", (Vector2) { mpPropPos.x + mpPropMarginLeft, mpPropPos.y + mpPropMarginTop + 20 }, 20, 0.0f, BLACK );
        DrawTextEx( rm->baseFont, px,          (Vector2) { mpPropPos.x + mpPropMarginLeft, mpPropPos.y + mpPropMarginTop + 40 }, 20, 0.0f, MAROON );
        DrawTextEx( rm->baseFont, py,          (Vector2) { mpPropPos.x + mpPropMarginLeft, mpPropPos.y + mpPropMarginTop + 60 }, 20, 0.0f, DARKGREEN );
        DrawTextEx( rm->baseFont, pz,          (Vector2) { mpPropPos.x + mpPropMarginLeft, mpPropPos.y + mpPropMarginTop + 80 }, 20, 0.0f, DARKBLUE );


        const char *ax = TextFormat( "x: %.2fº", mp->rot.x );
        const char *ay = TextFormat( "y: %.2fº", mp->rot.y );
        const char *az = TextFormat( "z: %.2fº", mp->rot.z );

        mpPropMarginLeft += 100;
        DrawTextEx( rm->baseFont, "Rotation:", (Vector2) { mpPropPos.x + mpPropMarginLeft, mpPropPos.y + mpPropMarginTop + 20 }, 20, 0.0f, BLACK );
        DrawTextEx( rm->baseFont, ax,          (Vector2) { mpPropPos.x + mpPropMarginLeft, mpPropPos.y + mpPropMarginTop + 40 }, 20, 0.0f, MAROON );
        DrawTextEx( rm->baseFont, ay,          (Vector2) { mpPropPos.x + mpPropMarginLeft, mpPropPos.y + mpPropMarginTop + 60 }, 20, 0.0f, DARKGREEN );
        DrawTextEx( rm->baseFont, az,          (Vector2) { mpPropPos.x + mpPropMarginLeft, mpPropPos.y + mpPropMarginTop + 80 }, 20, 0.0f, DARKBLUE );


        const char *sx = TextFormat( "x: %.2f", mp->sca.x );
        const char *sy = TextFormat( "y: %.2f", mp->sca.y );
        const char *sz = TextFormat( "z: %.2f", mp->sca.z );

        mpPropMarginLeft += 100;
        DrawTextEx( rm->baseFont, "Scale:",    (Vector2) { mpPropPos.x + mpPropMarginLeft, mpPropPos.y + mpPropMarginTop + 20 }, 20, 0.0f, BLACK );
        DrawTextEx( rm->baseFont, sx,          (Vector2) { mpPropPos.x + mpPropMarginLeft, mpPropPos.y + mpPropMarginTop + 40 }, 20, 0.0f, MAROON );
        DrawTextEx( rm->baseFont, sy,          (Vector2) { mpPropPos.x + mpPropMarginLeft, mpPropPos.y + mpPropMarginTop + 60 }, 20, 0.0f, DARKGREEN );
        DrawTextEx( rm->baseFont, sz,          (Vector2) { mpPropPos.x + mpPropMarginLeft, mpPropPos.y + mpPropMarginTop + 80 }, 20, 0.0f, DARKBLUE );

    }

    if ( editorMode == EDITOR_MODE_SELECT_MAP_PIECE_MODEL_TYPE ) {

        DrawTexturePro( 
            rm->mapPieceModelAtlasPreviewTexture,
            (Rectangle) { 0, 0, rm->mapPieceModelAtlasPreviewTexture.width, rm->mapPieceModelAtlasPreviewTexture.height },
            (Rectangle) { GetScreenWidth() / 2 - targetWSelect / 2, startYSelect, targetWSelect, targetHSelect },
            (Vector2) { 0 },
            0,
            WHITE
        );

        int pieceModelPos = (int) selectedMapPieceModelType;
        int row = pieceModelPos / piecesPerLineSelect;
        int col = pieceModelPos % piecesPerLineSelect;

        Rectangle selectedMapPieceModelRect = { 
            startXSelect + marginSelect + ( pieceSizeSelect + spacingSelect ) * col, 
            startYSelect + marginSelect + ( pieceSizeSelect + spacingSelect ) * row, 
            pieceSizeSelect, 
            pieceSizeSelect
        };
        DrawRectangleLinesEx( selectedMapPieceModelRect, 2.0f, BLUE );
        DrawRectangleLinesEx( mouseHoverMapPieceModelRect, 2.0f, WHITE );

        if ( hoveredMapPieceModelType >= 0 ) {

            const char *name = getMapPieceModelTypeName( (MapPieceModelType) hoveredMapPieceModelType );
            const int fontSize = 20;
            const int tooltipMargin = 8;
            const int tooltipOffset = 16;

            Vector2 mousePos = GetMousePosition();
            Vector2 textSize = MeasureTextEx( rm->baseFont, name, fontSize, 0.0f );

            Rectangle tooltipRect = {
                mousePos.x + tooltipOffset,
                mousePos.y + tooltipOffset,
                textSize.x + tooltipMargin * 2,
                textSize.y + tooltipMargin * 2
            };

            DrawRectangleRounded( tooltipRect, 0.2f, 10, Fade( WHITE, 0.7f ) );
            DrawRectangleRoundedLinesEx( tooltipRect, 0.2f, 10, 2.0f, BLACK );
            DrawTextEx(
                rm->baseFont, name,
                (Vector2) { tooltipRect.x + tooltipMargin, tooltipRect.y + tooltipMargin },
                fontSize, 0.0f, BLACK
            );

        }

    }

}

static void deselectSelectedMapPiece( void ) {
    if ( selectedMapPiece != NULL ) {
        selectedMapPiece->gizmo.xAxis.selected = false;
        selectedMapPiece->gizmo.zAxis.selected = false;
        selectedMapPiece->gizmo.yAxis.selected = false;
        selectedMapPiece->gizmo.center.selected = false;
        selectedMapPiece->gizmo.xyPlane.selected = false;
        selectedMapPiece->gizmo.xzPlane.selected = false;
        selectedMapPiece->gizmo.yzPlane.selected = false;
        selectedMapPiece->selected = false;
        selectedMapPiece = NULL;
        performingGizmoOperation = false;
    }
}

static MapPiece *getMapPieceFromRay( GameWorld *gw ) {
    Ray ray = GetScreenToWorldRay( GetMousePosition(), gw->camera );
    MapPiece *mp = NULL;
    getNearestMapPieceHit( gw, ray, &mp, NULL );
    return mp;
}

static RayCollision getAddRayCollisionFromRay( GameWorld *gw ) {

    Ray ray = GetScreenToWorldRay( GetMousePosition(), gw->camera );
    RayCollision rc;

    if ( getNearestMapPieceHit( gw, ray, NULL, &rc ) ) {
        return rc;
    }

    // does not hit any piece, falls back to the grid (y = 0)
    return GetRayCollisionBox( 
        ray, 
        (BoundingBox) {
            .min = { -10000.0f, 0.0f, -10000.0f },
            .max = {  10000.0f, 0.0f,  10000.0f }
        }
    );

}

static bool getNearestMapPieceHit( GameWorld *gw, Ray ray, MapPiece **outMp, RayCollision *outRc ) {

    MapPiece *nearestMp = NULL;
    RayCollision nearestRc = { 0 };
    float nearestDistance = INFINITY;

    for ( int i = 0; i < gw->mapPiecesCount; i++ ) {
        MapPiece *mp = &gw->mapPieces[i];
        RayCollision rc = GetRayCollisionBox( ray, mp->bb );
        if ( rc.hit && rc.distance < nearestDistance ) {
            nearestDistance = rc.distance;
            nearestMp = mp;
            nearestRc = rc;
        }
    }

    if ( outMp != NULL ) *outMp = nearestMp;
    if ( outRc != NULL ) *outRc = nearestRc;

    return nearestMp != NULL;

}

static void addMapPiece( GameWorld *gw ) {

    RayCollision rc = getAddRayCollisionFromRay( gw );

    if ( rc.hit && gw->mapPiecesCount < gw->maxMapPieces ) {
        initMapPiece( 
            &gw->mapPieces[gw->mapPiecesCount],
            rc.point,
            selectedMapPieceModelType
        );
        gw->mapPiecesCount++;
    }

}

static void removeMapPiece( MapPiece *mp, GameWorld *gw ) {

    deselectSelectedMapPiece();
    int delPos = -1;

    for ( int i = 0; i < gw->mapPiecesCount; i++ ) {
        MapPiece *cMp = &gw->mapPieces[i];
        if ( cMp == mp ) {
            delPos = i;
            break;
        }
    }

    if ( delPos != -1 ) {
        for ( int i = delPos + 1; i < gw->mapPiecesCount; i++ ) {
            gw->mapPieces[i-1]  = gw->mapPieces[i];
        }
        gw->mapPiecesCount--;
    }

}

static GizmoOperationMode toGizmoOperationMode( EditorMode mode ) {

    switch ( mode ) {
        case EDITOR_MODE_ROTATE_MAP_PIECE: return GIZMO_OPERATION_ROTATE;
        case EDITOR_MODE_SCALE_MAP_PIECE:  return GIZMO_OPERATION_SCALE;
        default:                           return GIZMO_OPERATION_TRANSLATE;
    }

}

static bool selectGizmoAxisFromSelectedMapPiece( MapPiece *mp, Camera *camera ) {

    mp->gizmo.xAxis.selected = false;
    mp->gizmo.zAxis.selected = false;
    mp->gizmo.yAxis.selected = false;
    mp->gizmo.center.selected = false;
    mp->gizmo.xyPlane.selected = false;
    mp->gizmo.xzPlane.selected = false;
    mp->gizmo.yzPlane.selected = false;

    switch ( checkCollisionMouseGizmo( &mp->gizmo, camera ) ) {
        case GIZMO_AXIS_COLLISION_TYPE_NONE:
            return false;
            break;
        case GIZMO_AXIS_COLLISION_TYPE_X:
            mp->gizmo.xAxis.selected = true;
            break;
        case GIZMO_AXIS_COLLISION_TYPE_Y:
            mp->gizmo.yAxis.selected = true;
            break;
        case GIZMO_AXIS_COLLISION_TYPE_Z:
            mp->gizmo.zAxis.selected = true;
            break;
        case GIZMO_AXIS_COLLISION_TYPE_CENTER:
            // only meaningful in scale mode -- otherwise, act as if nothing
            // was hit, so the click falls through to selecting another piece
            if ( gizmoMode != EDITOR_MODE_SCALE_MAP_PIECE ) {
                return false;
            }
            mp->gizmo.center.selected = true;
            break;
        case GIZMO_AXIS_COLLISION_TYPE_XY:
            // planes only do something in translate/scale, not rotate
            if ( gizmoMode == EDITOR_MODE_ROTATE_MAP_PIECE ) {
                return false;
            }
            mp->gizmo.xyPlane.selected = true;
            break;
        case GIZMO_AXIS_COLLISION_TYPE_XZ:
            if ( gizmoMode == EDITOR_MODE_ROTATE_MAP_PIECE ) {
                return false;
            }
            mp->gizmo.xzPlane.selected = true;
            break;
        case GIZMO_AXIS_COLLISION_TYPE_YZ:
            if ( gizmoMode == EDITOR_MODE_ROTATE_MAP_PIECE ) {
                return false;
            }
            mp->gizmo.yzPlane.selected = true;
            break;
    }

    return true;

}

static void performGizmoOperation( MapPiece *mp, Camera *camera ) {

    const float rotateAmount = 1.0f;
    const float scaleAmount = 0.05f;

    const float translateSnap = 0.05f;
    const float rotateSnap    = 5.0f;
    const float scaleSnap     = 0.05f;
    bool snap = IsKeyDown( KEY_LEFT_SHIFT );

    // first frame of this drag: remember where every transform started, so
    // later frames measure the total change since the click -- both for the
    // "grab offset" fix (translation) and to make snapping possible (snapping
    // a whole-frame's tiny delta would just always round down to zero)
    if ( IsMouseButtonPressed( MOUSE_BUTTON_LEFT ) ) {
        gizmoDragStartPos = mp->pos;
        gizmoDragStartRot = mp->rot;
        gizmoDragStartSca = mp->sca;
        gizmoDragAccum = 0.0f;
    }

    if ( mp->gizmo.center.selected ) {

        // uniform scale: no single axis to project the mouse onto, so we
        // just use the raw vertical movement -- up grows, down shrinks
        gizmoDragAccum += -GetMouseDelta().y;
        float amount = scaleAmount * gizmoDragAccum;
        if ( snap ) amount = roundf( amount / scaleSnap ) * scaleSnap;

        mp->sca = Vector3Add( gizmoDragStartSca, (Vector3) { amount, amount, amount } );
        mp->update( mp );

        return;

    }

    // plane handles: translate or scale two axes at once
    Vector3 planeNormal = { 0 };
    Vector3 planeAxisA  = { 0 };
    Vector3 planeAxisB  = { 0 };

    if ( mp->gizmo.xyPlane.selected ) {
        planeNormal = (Vector3) { 0.0f, 0.0f, 1.0f };
        planeAxisA  = (Vector3) { 1.0f, 0.0f, 0.0f };
        planeAxisB  = (Vector3) { 0.0f, 1.0f, 0.0f };
    } else if ( mp->gizmo.xzPlane.selected ) {
        planeNormal = (Vector3) { 0.0f, 1.0f, 0.0f };
        planeAxisA  = (Vector3) { 1.0f, 0.0f, 0.0f };
        planeAxisB  = (Vector3) { 0.0f, 0.0f, 1.0f };
    } else if ( mp->gizmo.yzPlane.selected ) {
        planeNormal = (Vector3) { 1.0f, 0.0f, 0.0f };
        planeAxisA  = (Vector3) { 0.0f, 1.0f, 0.0f };
        planeAxisB  = (Vector3) { 0.0f, 0.0f, 1.0f };
    }

    if ( planeNormal.x != 0.0f || planeNormal.y != 0.0f || planeNormal.z != 0.0f ) {

        if ( gizmoMode == EDITOR_MODE_TRANSLATE_MAP_PIECE ) {

            // intersects the mouse ray with the plane through the pivot,
            // perpendicular to the axis left out of the pair -- simpler than
            // the closest-point-between-lines math the single axis needs,
            // since a plane only leaves one unknown (t) to solve for
            Ray mouseRay = GetScreenToWorldRay( GetMousePosition(), *camera );
            float denom = Vector3DotProduct( mouseRay.direction, planeNormal );

            if ( fabsf( denom ) > 0.0001f ) {

                float t = Vector3DotProduct( Vector3Subtract( gizmoDragStartPos, mouseRay.position ), planeNormal ) / denom;
                Vector3 hitPoint = Vector3Add( mouseRay.position, Vector3Scale( mouseRay.direction, t ) );

                if ( IsMouseButtonPressed( MOUSE_BUTTON_LEFT ) ) {
                    gizmoDragStartPlaneHit = hitPoint;
                }

                // both hitPoint and gizmoDragStartPlaneHit lie on the same
                // plane, so this difference is automatically perpendicular
                // to planeNormal -- no third-axis drift to guard against
                Vector3 offset = Vector3Subtract( hitPoint, gizmoDragStartPlaneHit );

                if ( snap ) {
                    offset.x = roundf( offset.x / translateSnap ) * translateSnap;
                    offset.y = roundf( offset.y / translateSnap ) * translateSnap;
                    offset.z = roundf( offset.z / translateSnap ) * translateSnap;
                }

                mp->pos = Vector3Add( gizmoDragStartPos, offset );

            }

        } else if ( gizmoMode == EDITOR_MODE_SCALE_MAP_PIECE ) {

            // no natural "point under the cursor" for scale -- same as the
            // uniform scale handle, just use vertical mouse movement, applied
            // to both in-plane axes at once
            gizmoDragAccum += -GetMouseDelta().y;
            float amount = scaleAmount * gizmoDragAccum;
            if ( snap ) amount = roundf( amount / scaleSnap ) * scaleSnap;

            Vector3 scaleOffset = Vector3Scale( Vector3Add( planeAxisA, planeAxisB ), amount );
            mp->sca = Vector3Add( gizmoDragStartSca, scaleOffset );

        }

        mp->update( mp );
        return;

    }

    Vector3 axisDir = { 0 };

    if ( mp->gizmo.xAxis.selected ) {
        axisDir = (Vector3) { 1.0f, 0.0f, 0.0f };
    } else if ( mp->gizmo.yAxis.selected ) {
        axisDir = (Vector3) { 0.0f, 1.0f, 0.0f };
    } else if ( mp->gizmo.zAxis.selected ) {
        axisDir = (Vector3) { 0.0f, 0.0f, 1.0f };
    } else {
        return;
    }

    if ( gizmoMode == EDITOR_MODE_TRANSLATE_MAP_PIECE ) {

        Ray mouseRay = GetScreenToWorldRay( GetMousePosition(), *camera );

        if ( IsMouseButtonPressed( MOUSE_BUTTON_LEFT ) ) {
            gizmoDragStartT = closestPointOnAxisToRay( gizmoDragStartPos, axisDir, mouseRay );
        }

        float currentT = closestPointOnAxisToRay( gizmoDragStartPos, axisDir, mouseRay );
        float totalOffset = currentT - gizmoDragStartT;
        if ( snap ) totalOffset = roundf( totalOffset / translateSnap ) * translateSnap;

        mp->pos = Vector3Add( gizmoDragStartPos, Vector3Scale( axisDir, totalOffset ) );

    } else {

        // rotate/scale: no natural "point under the cursor" here, so we keep
        // projecting the axis to the screen and accumulating how much the
        // mouse moved along that projected direction since the drag started
        Vector2 originScreen  = GetWorldToScreen( mp->gizmo.pos, *camera );
        Vector2 axisScreen    = GetWorldToScreen( Vector3Add( mp->gizmo.pos, axisDir ), *camera );
        Vector2 screenAxisDir = Vector2Normalize( Vector2Subtract( axisScreen, originScreen ) );
        gizmoDragAccum += Vector2DotProduct( GetMouseDelta(), screenAxisDir );

        if ( gizmoMode == EDITOR_MODE_ROTATE_MAP_PIECE ) {

            float amount = rotateAmount * gizmoDragAccum;
            if ( snap ) amount = roundf( amount / rotateSnap ) * rotateSnap;
            mp->rot = Vector3Add( gizmoDragStartRot, Vector3Scale( axisDir, amount ) );

        } else if ( gizmoMode == EDITOR_MODE_SCALE_MAP_PIECE ) {

            float amount = scaleAmount * gizmoDragAccum;
            if ( snap ) amount = roundf( amount / scaleSnap ) * scaleSnap;
            mp->sca = Vector3Add( gizmoDragStartSca, Vector3Scale( axisDir, amount ) );

        }

    }

    mp->update( mp );

}

// finds the point along the axis line (through lineOrigin, direction axisDir)
// that comes closest to the mouse ray -- perpendicularity to both lines is
// the condition that pins down the closest pair of points on two skew lines
static float closestPointOnAxisToRay( Vector3 lineOrigin, Vector3 axisDir, Ray ray ) {

    Vector3 w0 = Vector3Subtract( lineOrigin, ray.position );

    float b = Vector3DotProduct( axisDir, ray.direction );  // both are unit vectors
    float d = Vector3DotProduct( axisDir, w0 );
    float e = Vector3DotProduct( ray.direction, w0 );

    float denom = 1.0f - b * b;

    if ( fabsf( denom ) < 0.0001f ) {
        // axis pointing almost straight at/away from the camera -- moving
        // along it wouldn't be visible on screen either, so there's no
        // reliable point to solve for; just skip this frame
        return 0.0f;
    }

    return ( b * e - d ) / denom;

}

static void saveMap( const char *filePath, GameWorld *gw ) {

    int bufferSize = gw->mapPiecesCount * 128 + 1;   // upper bound per line
    char *buffer = (char*) malloc( bufferSize );
    int offset = 0;

    for ( int i = 0; i < gw->mapPiecesCount; i++ ) {

        MapPiece *mp = &gw->mapPieces[i];

        offset += snprintf(
            buffer + offset,
            bufferSize - offset,
            "%d %.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f\n",
            (int) mp->modelType,
            mp->pos.x, mp->pos.y, mp->pos.z, 
            mp->rot.x, mp->rot.y, mp->rot.z, 
            mp->sca.x, mp->sca.y, mp->sca.z
        );

    }

    SaveFileText( filePath, buffer );
    free( buffer );

}

static void loadMap( const char *filePath, GameWorld *gw ) {

    char *fileText = LoadFileText( filePath );
    if ( fileText == NULL ) {
        return;
    }

    deselectSelectedMapPiece();
    gw->mapPiecesCount = 0;

    const char *cursor = fileText;
    int modelType;
    Vector3 pos;
    Vector3 rot;
    Vector3 sca;
    int charsRead = 0;

    while ( gw->mapPiecesCount < gw->maxMapPieces && 
            sscanf( cursor, "%d %f %f %f %f %f %f %f %f %f%n",
                    &modelType,
                    &pos.x, &pos.y, &pos.z,
                    &rot.x, &rot.y, &rot.z,
                    &sca.x, &sca.y, &sca.z, &charsRead ) == 10 ) {
        
        if ( modelType >= 0 && modelType < rm->mapPieceModelAtlasCount ) {
            MapPiece *mp = &gw->mapPieces[gw->mapPiecesCount];
            initMapPiece( mp, pos, (MapPieceModelType) modelType );
            mp->rot = rot;
            mp->sca = sca;
            gw->mapPiecesCount++;
        }

        cursor += charsRead;

    }

    UnloadFileText( fileText );

}