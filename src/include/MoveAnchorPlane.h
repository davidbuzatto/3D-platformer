#pragma once

#include <stdbool.h>

#include "raylib/raylib.h"

typedef struct {
    Vector3 pos;
    Vector3 dim;
    Color color;
    bool selected;
} MoveAnchorPlane;