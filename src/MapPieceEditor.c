/**
 * @file MapPieceEditor.c
 * @author Prof. Dr. David Buzatto
 * @brief MapPieceEditor implementation.
 *
 * @copyright Copyright (c) 2026
 */
#include <stdio.h>
#include <math.h>

#include "raylib/raylib.h"
#include "raylib/raymath.h"

#include "Macros.h"

#ifdef USE_GUI
#include "raylib/raygui.h"   // NOTE: RAYGUI_IMPLEMENTATION is defined once, in GameWorld.c
#endif

#include "MapPieceEditor.h"
#include "GameWorld.h"
#include "MapPiece.h"
#include "Gizmo.h"
#include "ResourceManager.h"

static MapPiece *getMapPieceFromRay( GameWorld *gw );
static RayCollision getAddRayCollisionFromRay( GameWorld *gw );
static bool getNearestMapPieceHit( GameWorld *gw, Ray ray, MapPiece **outMp, RayCollision *outRc );

static bool selectGizmoAxisFromSelectedMapPiece( MapPiece *mp, Camera *camera );
static void performGizmoOperation( MapPiece *mp, Camera *camera );
static float closestPointOnAxisToRay( Vector3 lineOrigin, Vector3 axisDir, Ray ray );

static Rectangle getMapPiecePropertiesPanelRec( void );

// selected map piece to perform operations
static MapPiece *selectedMapPiece = NULL;
static char mpPropTextBuf[9][32] = { 0 };   // text buffers for 9 fields
static int mpPropActiveField = -1;          // the current field being edited (-1 = none)

static GizmoMode gizmoMode = GIZMO_MODE_TRANSLATE;

// gizmo operation
static bool performingGizmoOperation = false;
static Vector3 gizmoDragStartPos = { 0 };
static Vector3 gizmoDragStartRot = { 0 };
static Vector3 gizmoDragStartSca = { 0 };
static Vector3 gizmoDragStartPlaneHit = { 0 };
static float gizmoDragStartT = 0.0f;
static float gizmoDragAccum = 0.0f;

MapPiece *getSelectedMapPiece( void ) {
    return selectedMapPiece;
}

void deselectSelectedMapPiece( void ) {
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

GizmoMode getGizmoMode( void ) {
    return gizmoMode;
}

void setGizmoMode( GizmoMode mode ) {
    gizmoMode = mode;
}

GizmoOperationMode toGizmoOperationMode( GizmoMode mode ) {

    switch ( mode ) {
        case GIZMO_MODE_ROTATE: return GIZMO_OPERATION_ROTATE;
        case GIZMO_MODE_SCALE:  return GIZMO_OPERATION_SCALE;
        default:                return GIZMO_OPERATION_TRANSLATE;
    }

}

void updateMapPieceSelectionAndGizmo( GameWorld *gw, Camera *camera ) {

    #ifdef USE_GUI
        bool mouseOverProperties = selectedMapPiece != NULL &&
            CheckCollisionPointRec( GetMousePosition(), getMapPiecePropertiesPanelRec() );
    #else
        bool mouseOverProperties = false;
    #endif

    if ( IsMouseButtonPressed( MOUSE_BUTTON_LEFT ) && !mouseOverProperties ) {

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

}

void addMapPiece( GameWorld *gw, MapPieceModelType modelType ) {

    RayCollision rc = getAddRayCollisionFromRay( gw );

    if ( rc.hit && gw->mapPiecesCount < gw->maxMapPieces ) {
        initMapPiece(
            &gw->mapPieces[gw->mapPiecesCount],
            rc.point,
            modelType
        );
        gw->mapPiecesCount++;
    }

}

void removeMapPiece( MapPiece *mp, GameWorld *gw ) {

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
            gw->mapPieces[i-1] = gw->mapPieces[i];
        }
        gw->mapPiecesCount--;
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
            if ( gizmoMode != GIZMO_MODE_SCALE ) {
                return false;
            }
            mp->gizmo.center.selected = true;
            break;
        case GIZMO_AXIS_COLLISION_TYPE_XY:
            // planes only do something in translate/scale, not rotate
            if ( gizmoMode == GIZMO_MODE_ROTATE ) {
                return false;
            }
            mp->gizmo.xyPlane.selected = true;
            break;
        case GIZMO_AXIS_COLLISION_TYPE_XZ:
            if ( gizmoMode == GIZMO_MODE_ROTATE ) {
                return false;
            }
            mp->gizmo.xzPlane.selected = true;
            break;
        case GIZMO_AXIS_COLLISION_TYPE_YZ:
            if ( gizmoMode == GIZMO_MODE_ROTATE ) {
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

        if ( gizmoMode == GIZMO_MODE_TRANSLATE ) {

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

        } else if ( gizmoMode == GIZMO_MODE_SCALE ) {

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

    if ( gizmoMode == GIZMO_MODE_TRANSLATE ) {

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

        if ( gizmoMode == GIZMO_MODE_ROTATE ) {

            float amount = rotateAmount * gizmoDragAccum;
            if ( snap ) amount = roundf( amount / rotateSnap ) * rotateSnap;
            mp->rot = Vector3Add( gizmoDragStartRot, Vector3Scale( axisDir, amount ) );

        } else if ( gizmoMode == GIZMO_MODE_SCALE ) {

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

static Rectangle getMapPiecePropertiesPanelRec( void ) {

    const int mpPropMarginTop = 10;

    return (Rectangle) {
        10,
        GetScreenHeight() - 120 - mpPropMarginTop,
        300,
        120
    };

}

void drawMapPiecePropertiesPanel( void ) {

    MapPiece *mp = selectedMapPiece;
    if ( mp == NULL ) {
        return;
    }

    const int mpPropMarginTop = 10;
    int mpPropMarginLeft = 10;
    Rectangle panelRec = getMapPiecePropertiesPanelRec();
    Vector2 mpPropPos = { panelRec.x, panelRec.y };

    DrawRectangleRounded( panelRec, 0.2f, 10, Fade( WHITE, 0.7f ) );
    DrawRectangleRoundedLinesEx( panelRec, 0.2f, 10, 2.0f, BLACK );

    switch ( gizmoMode ) {
        case GIZMO_MODE_TRANSLATE:
            DrawTextEx(
                rm->baseFont,
                "Gizmo Mode: TRANSLATE",
                (Vector2) { mpPropPos.x + mpPropMarginLeft, mpPropPos.y + mpPropMarginTop },
                20, 0.0f, BLACK
            );
            break;
        case GIZMO_MODE_ROTATE:
            DrawTextEx(
                rm->baseFont,
                "Gizmo Mode: ROTATE",
                (Vector2) { mpPropPos.x + mpPropMarginLeft, mpPropPos.y + mpPropMarginTop },
                20, 0.0f, BLACK
            );
            break;
        case GIZMO_MODE_SCALE:
            DrawTextEx(
                rm->baseFont,
                "Gizmo Mode: SCALE",
                (Vector2) { mpPropPos.x + mpPropMarginLeft, mpPropPos.y + mpPropMarginTop },
                20, 0.0f,
                BLACK
            );
            break;
    }

    #ifndef USE_GUI

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

    #else

        float *mpPropValues[9] = {
            &mp->pos.x, &mp->pos.y, &mp->pos.z,
            &mp->rot.x, &mp->rot.y, &mp->rot.z,
            &mp->sca.x, &mp->sca.y, &mp->sca.z,
        };
        const char *mpPropLabels[9] = { "x:", "y:", "z:", "x:", "y:", "z:", "x:", "y:", "z:" };
        const char *mpPropColumHeaders[3] = { "Position:", "Rotation:", "Scale:" };
        const int boxWidth = 70;
        const int boxHeight = 18;

        for ( int col = 0; col < 3; col++ ) {

            int columnLeft = mpPropMarginLeft + col * 100;

            DrawTextEx(
                rm->baseFont, mpPropColumHeaders[col],
                (Vector2) { mpPropPos.x + columnLeft + 10, mpPropPos.y + mpPropMarginTop + 20 },
                20, 0.0f, BLACK
            );

            for ( int row = 0; row < 3; row++ ) {

                int field = col * 3 + row;

                Rectangle bounds = {
                    mpPropPos.x + columnLeft + 10,
                    mpPropPos.y + mpPropMarginTop + 40 + row * 22,
                    boxWidth, boxHeight
                };

                bool editing = ( mpPropActiveField == field );

                if ( !editing ) {
                    snprintf( mpPropTextBuf[field], sizeof( mpPropTextBuf[field] ), "%.2f", *mpPropValues[field] );
                }

                if ( GuiValueBoxFloat( bounds, mpPropLabels[field], mpPropTextBuf[field], mpPropValues[field], editing ) ) {
                    mpPropActiveField = editing ? -1 : field;
                }

            }

        }

    #endif

}
