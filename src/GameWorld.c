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

typedef enum {
    EDITOR_MODE_SELECT_MAP_PIECE,
    EDITOR_MODE_ADD_MAP_PIECE,
    EDITOR_MODE_SELECT_MAP_PIECE_MODEL_TYPE,
    EDITOR_MODE_TRANSLATE_MAP_PIECE,
    EDITOR_MODE_ROTATE_MAP_PIECE,
    EDITOR_MODE_SCALE_MAP_PIECE,
} EditorMode;

typedef struct {
    MapPiece *mapPiece;
    RayCollision rc;
} MapPieceDistance;

static void updateCamera( Camera *camera, float delta );
static void drawEditorHud( void );

static void deselectSelectedMapPiece( void );
static MapPiece *getMapPieceFromRay( GameWorld *gw );
static RayCollision getAddRayCollisionFromRay( GameWorld *gw );
static bool getNearestMapPieceHit( GameWorld *gw, Ray ray, MapPiece **outMp, RayCollision *outRc );
static void addMapPiece( GameWorld *gw );
static void removeMapPiece( MapPiece *mp, GameWorld *gw );
static bool selectGizmoAxisFromSelectedMapPiece( MapPiece *mp, Camera *camera );
static void performGizmoOperation( MapPiece *mp, Camera *camera, float delta );

static bool drawDebugInfo = true;

// camera control
static float cameraYaw               = -90.0f;  // XZ plane angle (degrees)
static float cameraPitch             = 20.0f;   // pitch angle (degrees)
static float cameraDistance          = 8.0f;    // distance to the target
static float cameraOrbitSpeed        = 0.2f;    // degrees per pixel (mouse movement)
static float cameraZoomSpeed         = 10.0f;   // units per second
static float cameraPanSpeed          = 5.0f;    // units per second
static const float cameraPitchMin    = -85.0f;
static const float cameraPitchMax    = 85.0f;
static const float cameraDistanceMin = 1.5f;

// gizmo operation
static bool performingGizmoOperation = false;
static int yForGizmoOperation = 0;

// selected map piece to perform operations
static MapPiece *selectedMapPiece = NULL;

// variables for EDITOR_MODE_SELECT_MAP_PIECE_MODEL_TYPE
static MapPieceModelType selectedMapPieceModel = MODEL_TYPE_BLOCK_GRASS;
static Rectangle mouseHoverMapPieceModelRect = { 0, 0, 0, 0 };
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

/**
 * @brief Creates a dinamically allocated GameWorld struct instance.
 */
GameWorld *createGameWorld( void ) {

    SetExitKey( KEY_NULL );

    GameWorld *gw = (GameWorld*) malloc( sizeof( GameWorld ) );

    /*int rows = 7;
    int cols = 5;*/
    int rows = 0;
    int cols = 0;
    gw->maxMapPieces = 1000;
    gw->mapPiecesCount = 0;
    gw->mapPieces = (MapPiece*) malloc( sizeof( MapPiece ) * gw->maxMapPieces );

    float mapPieceSpacing = 0.2f;

    Model models[7] = {
        rm->mapPieceModelAtlas[MODEL_TYPE_BLOCK_GRASS_LARGE_TALL],
        rm->mapPieceModelAtlas[MODEL_TYPE_BLOCK_GRASS_LARGE],
        rm->mapPieceModelAtlas[MODEL_TYPE_BLOCK_GRASS_LONG],
        rm->mapPieceModelAtlas[MODEL_TYPE_BLOCK_GRASS_CURVE],
        rm->mapPieceModelAtlas[MODEL_TYPE_BLOCK_GRASS],
        rm->mapPieceModelAtlas[MODEL_TYPE_BLOCK_GRASS_CORNER],
        rm->mapPieceModelAtlas[MODEL_TYPE_BLOCK_GRASS_EDGE]
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

            gw->mapPiecesCount++;

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


    // variables for EDITOR_MODE_SELECT_MAP_PIECE_MODEL_TYPE
    targetWSelect = rm->mapPieceModelAtlasPreviewTexture.width / 2;
    targetHSelect = rm->mapPieceModelAtlasPreviewTexture.height / 2;
    piecesPerLineSelect = 15;
    pieceSizeSelect = 32;
    startXSelect = GetScreenWidth() / 2 - targetWSelect / 2;
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
        gw->mapPieces[i].update( &gw->mapPieces[i], camera, delta );
    }

    if ( editorMode == EDITOR_MODE_SELECT_MAP_PIECE ) {

        if ( IsMouseButtonPressed( MOUSE_BUTTON_LEFT ) ) {

            // priority 1: gizmo operation for selected map piece
            if ( selectedMapPiece != NULL ) {
                if ( selectGizmoAxisFromSelectedMapPiece( selectedMapPiece, camera ) ) {
                    yForGizmoOperation = GetMouseY();
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
            }
            performingGizmoOperation = false;
        }

        if ( selectedMapPiece != NULL && performingGizmoOperation ) {
            performGizmoOperation( selectedMapPiece, camera, delta );
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

        if ( mouseX >= startXSelect + marginSelect && mouseX <= startXSelect + targetWSelect - marginSelect &&
             mouseY >= startYSelect + marginSelect && mouseY <= startYSelect + targetHSelect - marginSelect ) {
            
            int offsetX = mouseX - ( startXSelect + marginSelect );
            int offsetY = mouseY - ( startYSelect + marginSelect );

            int col = offsetX / ( pieceSizeSelect + spacingSelect );
            int row = offsetY / ( pieceSizeSelect + spacingSelect );

            mouseHoverMapPieceModelRect.x = startXSelect + marginSelect + col * ( pieceSizeSelect + spacingSelect );
            mouseHoverMapPieceModelRect.y = startYSelect + marginSelect + row * ( pieceSizeSelect + spacingSelect );
            mouseHoverMapPieceModelRect.width = mouseHoverMapPieceModelRect.height = pieceSizeSelect;

            if ( IsMouseButtonPressed( MOUSE_BUTTON_LEFT ) ) {
                int modelPos = row * piecesPerLineSelect + col;
                if ( modelPos < rm->mapPieceModelAtlasCount ) {
                    selectedMapPieceModel = row * piecesPerLineSelect + col;
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

}

/**
 * @brief Draws the state of the game.
 */
void drawGameWorld( GameWorld *gw ) {

    BeginDrawing();
    ClearBackground( WHITE );

    BeginMode3D( gw->camera );
    for ( int i = 0; i < gw->mapPiecesCount; i++ ) {
        gw->mapPieces[i].draw( &gw->mapPieces[i] );
    }
    DrawGrid( 100, 1 );
    EndMode3D();

    drawEditorHud();

    EndDrawing();

}

static void updateCamera( Camera *camera, float delta ) {
    
    if ( IsMouseButtonDown( MOUSE_BUTTON_RIGHT ) ) {
        Vector2 mouseDelta = GetMouseDelta();
        cameraYaw   -= mouseDelta.x * cameraOrbitSpeed;
        cameraPitch -= mouseDelta.y * cameraOrbitSpeed;
        cameraPitch = Clamp( cameraPitch, cameraPitchMin, cameraPitchMax );
    }

    cameraDistance -= GetMouseWheelMove() * cameraZoomSpeed * delta;
    if ( cameraDistance < cameraDistanceMin ) {
        cameraDistance = cameraDistanceMin;
    }

    float yawRad   = DEG2RAD * cameraYaw;
    float pitchRad = DEG2RAD * cameraPitch;

    // ground-plane forward/right, derived from yaw only (pitch ignored so
    // moving "forward" never changes height) -- moves the target, which
    // stands in for the future player position
    Vector3 forward = { -cosf( yawRad ), 0.0f, -sinf( yawRad ) };
    Vector3 right   = { -sinf( yawRad ), 0.0f,  cosf( yawRad ) };
    float panAmount = cameraPanSpeed * delta;

    if ( IsKeyDown( KEY_W ) ) {
        camera->target.x += forward.x * panAmount;
        camera->target.z += forward.z * panAmount;
    }
    if ( IsKeyDown( KEY_S ) ) {
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

        int pieceModelPos = (int) selectedMapPieceModel;
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

    }

}

static void deselectSelectedMapPiece( void ) {
    if ( selectedMapPiece != NULL ) {
        selectedMapPiece->gizmo.xAxis.selected = false;
        selectedMapPiece->gizmo.zAxis.selected = false;
        selectedMapPiece->gizmo.yAxis.selected = false;
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
            rm->mapPieceModelAtlas[selectedMapPieceModel]
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

static bool selectGizmoAxisFromSelectedMapPiece( MapPiece *mp, Camera *camera ) {

    mp->gizmo.xAxis.selected = false;
    mp->gizmo.zAxis.selected = false;
    mp->gizmo.yAxis.selected = false;

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
    }

    return true;

}

static void performGizmoOperation( MapPiece *mp, Camera *camera, float delta ) {

    const float translateAmount = 0.1f;
    const float rotateAmount = 1.0f;
    const float scaleAmount = 0.05f;

    float xAmount = 0.0f;
    float yAmount = 0.0f;
    float zAmount = 0.0f;

    int ud = yForGizmoOperation - GetMouseY();
    yForGizmoOperation = GetMouseY();

    if ( gizmoMode == EDITOR_MODE_TRANSLATE_MAP_PIECE ) {

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

    } else if ( gizmoMode == EDITOR_MODE_ROTATE_MAP_PIECE ) {

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

    } else if ( gizmoMode == EDITOR_MODE_SCALE_MAP_PIECE ) {

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

    mp->update( mp, camera, delta );

}