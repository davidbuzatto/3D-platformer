#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "raylib/raylib.h"
#include "raylib/raymath.h"

#include "MapPiece.h"
#include "Macros.h"
#include "MoveAnchor.h"
#include "MoveAnchorPlane.h"
#include "ResourceManager.h"

static void update( MapPiece *e, Camera3D *camera, float delta );
static void draw( MapPiece *e, bool drawDebugInfo );

void initMapPiece( MapPiece *e, Vector3 pos, Model model ) {
    
    e->pos = pos;
    e->vel = (Vector3) { 0 };
    e->baseSpeed = 10;

    e->color = BLUE;
    e->model = model;
    e->bb = GetModelBoundingBox( e->model );
    e->bb.min = Vector3Add( e->bb.min, e->pos );
    e->bb.max = Vector3Add( e->bb.max, e->pos );

    e->moveAnchor = (MoveAnchor) {
        .pos = e->pos,
        .movePlaneBigSize = MOVE_PLANE_BIG_SIZE,
        .movePlaneSmallSize = MOVE_PLANE_SMALL_SIZE,
        .xymp = {
            .pos = { 0 },
            .dim = {
                MOVE_PLANE_BIG_SIZE,
                MOVE_PLANE_BIG_SIZE,
                MOVE_PLANE_SMALL_SIZE
            },
            .color = GREEN
        },
        .xzmp = {
            .pos = { 0 },
            .dim = {
                MOVE_PLANE_BIG_SIZE,
                MOVE_PLANE_SMALL_SIZE,
                MOVE_PLANE_BIG_SIZE
            },
            .color = BLUE
        },
        .yzmp = {
            .pos = { 0 },
            .dim = {
                MOVE_PLANE_SMALL_SIZE,
                MOVE_PLANE_BIG_SIZE,
                MOVE_PLANE_BIG_SIZE
            },
            .color = RED
        }
    };

    e->moveAnchorOffset = (Vector3) { 0, e->bb.max.y - e->bb.min.y + 0.1f, 0 };

    e->update = update;
    e->draw = draw;

}

static void update( MapPiece *e, Camera3D *camera, float delta ) {
    updateMoveAnchor( &e->moveAnchor, e->pos, e->moveAnchorOffset );
}

static void draw( MapPiece *e, bool drawDebugInfo ) {

    DrawModel( e->model, e->pos, 1.0, WHITE );

    if ( drawDebugInfo ) {
        DrawBoundingBox( e->bb, BLACK );
        drawMoveAnchor( &e->moveAnchor );
    }

}
