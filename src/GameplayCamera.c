/**
 * @file GameplayCamera.c
 * @author Prof. Dr. David Buzatto
 * @brief GameplayCamera implementation.
 *
 * @copyright Copyright (c) 2026
 */
#include <math.h>
#include <stdbool.h>

#include "raylib/raylib.h"
#include "raylib/raymath.h"

#include "GameplayCamera.h"

#define GAMEPAD_ID       0
#define GAMEPAD_DEADZONE 0.2f

static float yaw            = 90.0f;    // degrees, around Y
static float pitch          = 20.0f;    // degrees, above the horizon
static float distance       = 8.0f;     // current follow distance
static float rotSpeed       = 120.0f;   // degrees per second at full stick deflection
static const float pitchMin = -10.0f;
static const float pitchMax = 80.0f;

static const float zoomSpeed   = 8.0f; // units per second at full trigger press
static const float distanceMin = 3.0f;
static const float distanceMax = 15.0f;

static bool invertY = true; // flip this to change vertical look direction

void updateGameplayCamera( Camera3D *camera, Vector3 target, float delta ) {

    float lookX = 0.0f;
    float lookY = 0.0f;
    float zoomIn = 0.0f;
    float zoomOut = 0.0f;

    if ( IsGamepadAvailable( GAMEPAD_ID ) ) {
        lookX = GetGamepadAxisMovement( GAMEPAD_ID, GAMEPAD_AXIS_RIGHT_X );
        lookY = GetGamepadAxisMovement( GAMEPAD_ID, GAMEPAD_AXIS_RIGHT_Y );
        if ( fabsf( lookX ) < GAMEPAD_DEADZONE ) lookX = 0.0f;
        if ( fabsf( lookY ) < GAMEPAD_DEADZONE ) lookY = 0.0f;

        // triggers report in the [-1..1] range like other axes; raylib's
        // own header comments it as "[1..-1]" without saying which end is
        // "pressed", so this assumes -1 = released, 1 = fully pressed --
        // remapped here to a plain 0..1 "how pressed" amount. If zooming
        // feels backwards on your controller, flip the sign here.
        zoomIn  = ( GetGamepadAxisMovement( GAMEPAD_ID, GAMEPAD_AXIS_RIGHT_TRIGGER ) + 1.0f ) * 0.5f;
        zoomOut = ( GetGamepadAxisMovement( GAMEPAD_ID, GAMEPAD_AXIS_LEFT_TRIGGER  ) + 1.0f ) * 0.5f;
    }

    // raw stick Y is negative when pushed "up" -- flipping it here makes
    // "up" look up by default; invertY flips it back to the traditional
    // flight-stick convention
    float pitchInput = invertY ? -lookY : lookY;

    yaw   += lookX * rotSpeed * delta;
    pitch += pitchInput * rotSpeed * delta;
    pitch  = Clamp( pitch, pitchMin, pitchMax );

    // right trigger pulls the camera in, left trigger pushes it away
    distance += ( zoomOut - zoomIn ) * zoomSpeed * delta;
    distance  = Clamp( distance, distanceMin, distanceMax );

    float yawRad   = DEG2RAD * yaw;
    float pitchRad = DEG2RAD * pitch;

    camera->target = target;
    camera->position.x = target.x + distance * cosf( pitchRad ) * cosf( yawRad );
    camera->position.y = target.y + distance * sinf( pitchRad );
    camera->position.z = target.z + distance * cosf( pitchRad ) * sinf( yawRad );

}