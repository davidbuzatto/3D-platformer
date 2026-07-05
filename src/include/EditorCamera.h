/**
 * @file EditorCamera.h
 * @author Prof. Dr. David Buzatto
 * @brief Free orbital camera used by the stage editor.
 *
 * @copyright Copyright (c) 2026
 */
#pragma once

#include "raylib/raylib.h"

/**
 * @brief Reads mouse/keyboard input and updates the camera's position,
 * orbiting and panning its target.
 */
void updateEditorCamera( Camera *camera, float delta );
