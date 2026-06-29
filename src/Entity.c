#include <stdlib.h>

#include "raylib/raylib.h"

#include "Entity.h"
#include "MoveAnchor.h"
#include "MoveAnchorPlane.h"

static void update( Entity *e, float delta );
static void draw( Entity *e );

static void updateMoveAnchor( Entity *e );
static void drawMoveAnchor( MoveAnchor *ma );

static float const MOVE_PLANE_BIG_SIZE = 0.6f;
static float const MOVE_PLANE_SMALL_SIZE = 0.05f;

void initEntity( Entity *e ) {
    
    e->pos = (Vector3) { 0 };
    e->dim = (Vector3) { 1, 1, 1 };
    e->vel = (Vector3) { 0 };
    e->baseSpeed = 10;

    e->color = BLUE;

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
            .color = ORANGE
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

    e->moveAnchorOffset = (Vector3) { 0, e->dim.y / 2, e->pos.z };

    e->update = update;
    e->draw = draw;

}

static void update( Entity *e, float delta ) {

    int h = ( IsKeyDown( KEY_LEFT ) ? -1 : 0 ) + ( IsKeyDown( KEY_RIGHT ) ? 1 : 0 );
    int f = ( IsKeyDown( KEY_UP )   ? -1 : 0 ) + ( IsKeyDown( KEY_DOWN )  ? 1 : 0 );

    e->vel.x = e->baseSpeed * h;
    e->vel.z = e->baseSpeed * f;

    e->pos.x += e->vel.x * delta;
    e->pos.z += e->vel.z * delta;

    updateMoveAnchor( e );

}

static void draw( Entity *e ) {

    DrawCubeV( e->pos, e->dim, e->color );
    DrawCubeWiresV( e->pos, e->dim, BLACK );

    drawMoveAnchor( &e->moveAnchor );

}

static void updateMoveAnchor( Entity *e ) {

    MoveAnchor *ma = &e->moveAnchor;
    ma->pos.x = e->pos.x + e->moveAnchorOffset.x;
    ma->pos.y = e->pos.y + e->moveAnchorOffset.y;
    ma->pos.z = e->pos.z + e->moveAnchorOffset.z;

    ma->xymp.pos = (Vector3) { 
        ma->pos.x, 
        ma->pos.y + MOVE_PLANE_BIG_SIZE / 2 + MOVE_PLANE_SMALL_SIZE / 2, 
        ma->pos.z - ( MOVE_PLANE_BIG_SIZE / 2 + MOVE_PLANE_SMALL_SIZE / 2 )
    };

    ma->xzmp.pos = (Vector3) { 
        ma->pos.x, 
        ma->pos.y, 
        ma->pos.z
    };

    ma->yzmp.pos = (Vector3) { 
        ma->pos.x + MOVE_PLANE_BIG_SIZE / 2 + MOVE_PLANE_SMALL_SIZE / 2, 
        ma->pos.y + MOVE_PLANE_BIG_SIZE / 2 + MOVE_PLANE_SMALL_SIZE / 2,
        ma->pos.z
    };

}

static void drawMoveAnchor( MoveAnchor *ma ) {

    DrawCubeV( ma->xymp.pos, ma->xymp.dim, ma->xymp.color );
    DrawCubeWiresV( ma->xymp.pos, ma->xymp.dim, BLACK );

    DrawCubeV( ma->xzmp.pos, ma->xzmp.dim, ma->xzmp.color );
    DrawCubeWiresV( ma->xzmp.pos, ma->xzmp.dim, BLACK );

    DrawCubeV( ma->yzmp.pos, ma->yzmp.dim, ma->yzmp.color );
    DrawCubeWiresV( ma->yzmp.pos, ma->yzmp.dim, BLACK );

}