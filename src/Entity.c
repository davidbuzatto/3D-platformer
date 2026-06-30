#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "raylib/raylib.h"
#include "raylib/raymath.h"

#include "Entity.h"
#include "Macros.h"
#include "MoveAnchor.h"
#include "MoveAnchorPlane.h"
#include "ResourceManager.h"

static void update( Entity *e, Camera3D *camera, float delta );
static void draw( Entity *e, bool drawDebugInfo );

void initEntity( Entity *e, Vector3 pos, Model model ) {
    
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

static void update( Entity *e, Camera3D *camera, float delta ) {

    /*float xAmount = e->vel.x * delta;
    float yAmount = e->vel.y * delta;
    float zAmount = e->vel.z * delta;

    e->pos.x += xAmount;
    e->pos.y += yAmount;
    e->pos.z += zAmount;

    e->bb.min.x += xAmount;
    e->bb.max.x += xAmount;
    e->bb.min.y += yAmount;
    e->bb.max.y += yAmount;
    e->bb.min.z += zAmount;
    e->bb.max.z += zAmount;*/

    updateMoveAnchor( &e->moveAnchor, e->pos, e->moveAnchorOffset );

}

static void draw( Entity *e, bool drawDebugInfo ) {

    DrawModel( e->model, e->pos, 1.0, WHITE );

    if ( drawDebugInfo ) {
        DrawBoundingBox( e->bb, BLACK );
        drawMoveAnchor( &e->moveAnchor );
    }

}
