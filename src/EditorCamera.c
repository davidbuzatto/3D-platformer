/**
 * @file EditorCamera.c
 * @author Prof. Dr. David Buzatto
 * @brief EditorCamera implementation.
 *
 * @copyright Copyright (c) 2026
 */
#include <math.h>

#include "raylib/raylib.h"
#include "raylib/raymath.h"

#include "EditorCamera.h"

static float cameraYaw               = 90.0f;   // XZ plane angle (degrees)
static float cameraPitch             = 20.0f;   // pitch angle (degrees)
static float cameraDistance          = 16.0f;    // current (smoothed) distance to the target
static float cameraTargetDistance    = 16.0f;   // desired distance, set instantly by the wheel
static float cameraOrbitSpeed        = 0.2f;    // degrees per pixel (mouse movement)
static float cameraZoomSpeed         = 1.0f;    // units per wheel notch
static float cameraZoomSmoothing     = 10.0f;   // how fast distance chases the target (per second)
static float cameraPanSpeed          = 5.0f;    // units per second
static float cameraMousePanSpeed     = 0.005f;  // world units per pixel of drag
static const float cameraPitchMin    = -85.0f;
static const float cameraPitchMax    = 85.0f;
static const float cameraDistanceMin = 1.5f;

void updateEditorCamera( Camera *camera, float delta ) {

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
    Vector3 up      = { -cosf( yawRad ) * sinf( pitchRad ), cosf( pitchRad ), -sinf( yawRad ) * sinf( pitchRad ) };

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

    // vertical movement is plain world Y, no yaw-dependent direction needed
    if ( IsKeyDown( KEY_E ) ) {
        camera->target.y += panAmount;
    }
    if ( IsKeyDown( KEY_Q ) ) {
        camera->target.y -= panAmount;
    }

    if ( IsMouseButtonDown( MOUSE_BUTTON_MIDDLE ) ) {

        Vector2 mouseDelta = GetMouseDelta();

        // scaled by distance so a pixel of drag covers the same apparent
        // screen-space amount whether zoomed in close or far away
        float panScale = cameraMousePanSpeed * cameraDistance;

        // moves opposite to the drag on the right axis, and with the drag on
        // the up axis -- makes the point under the cursor stay under the
        // cursor, like grabbing the viewport
        Vector3 offset = Vector3Add(
            Vector3Scale( right, -mouseDelta.x * panScale ),
            Vector3Scale( up,     mouseDelta.y * panScale )
        );

        camera->target = Vector3Add( camera->target, offset );

    }

    camera->position.x = camera->target.x + cameraDistance * cosf( pitchRad ) * cosf( yawRad );
    camera->position.y = camera->target.y + cameraDistance * sinf( pitchRad );
    camera->position.z = camera->target.z + cameraDistance * cosf( pitchRad ) * sinf( yawRad );

}
