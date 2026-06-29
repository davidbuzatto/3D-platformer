#include <stdlib.h>
#include <math.h>

#include "raylib/raylib.h"

#include "Entity.h"
#include "Macros.h"
#include "MoveAnchor.h"
#include "MoveAnchorPlane.h"

static void update( Entity *e, Camera3D *camera, float delta );
static void draw( Entity *e );

// TODO: move to MoveAnchor.c
static void updateMoveAnchor( Entity *e );
static void drawMoveAnchor( MoveAnchor *ma );
static MoveAnchorCollisionType checkCollisionMouseMoveAnchor( MoveAnchor *ma, Camera3D *camera );

static float const MOVE_PLANE_BIG_SIZE = 0.6f;
static float const MOVE_PLANE_SMALL_SIZE = 0.05f;

void initEntity( Entity *e, Vector3 pos ) {
    
    e->pos = pos;
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

    //e->moveAnchorOffset = (Vector3) { 0, e->dim.y / 2, e->pos.z };
    e->moveAnchorOffset = (Vector3) { 0, e->dim.y / 2, 0 };

    e->update = update;
    e->draw = draw;

}

static void update( Entity *e, Camera3D *camera, float delta ) {

    /*int h = ( IsKeyDown( KEY_LEFT ) ? -1 : 0 ) + ( IsKeyDown( KEY_RIGHT ) ? 1 : 0 );
    int f = ( IsKeyDown( KEY_UP )   ? -1 : 0 ) + ( IsKeyDown( KEY_DOWN )  ? 1 : 0 );

    e->vel.x = e->baseSpeed * h;
    e->vel.z = e->baseSpeed * f;

    e->pos.x += e->vel.x * delta;
    e->pos.z += e->vel.z * delta;*/

    updateMoveAnchor( e );

    switch ( checkCollisionMouseMoveAnchor( &e->moveAnchor, camera ) ) {
        case MOVE_ANCHOR_COLLISION_TYPE_NONE:
            e->moveAnchor.xymp.mouseHover = false;
            e->moveAnchor.xzmp.mouseHover = false;
            e->moveAnchor.yzmp.mouseHover = false;
            break;
        case MOVE_ANCHOR_COLLISION_TYPE_XY:
            e->moveAnchor.xymp.mouseHover = true;
            e->moveAnchor.xzmp.mouseHover = false;
            e->moveAnchor.yzmp.mouseHover = false;
            break;
        case MOVE_ANCHOR_COLLISION_TYPE_XZ:
            e->moveAnchor.xymp.mouseHover = false;
            e->moveAnchor.xzmp.mouseHover = true;
            e->moveAnchor.yzmp.mouseHover = false;
            break;
        case MOVE_ANCHOR_COLLISION_TYPE_YZ:
            e->moveAnchor.xymp.mouseHover = false;
            e->moveAnchor.xzmp.mouseHover = false;
            e->moveAnchor.yzmp.mouseHover = true;
            break;
    }

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

    const float alpha = 0.5f;

    if ( ma->xymp.mouseHover ) {
        DrawCubeV( ma->xymp.pos, ma->xymp.dim, ma->xymp.color );
        DrawCubeWiresV( ma->xymp.pos, ma->xymp.dim, BLACK );
    } else {
        DrawCubeV( ma->xymp.pos, ma->xymp.dim, Fade( ma->xymp.color, alpha ) );
        DrawCubeWiresV( ma->xymp.pos, ma->xymp.dim, Fade( BLACK, alpha ) );
    }

    if ( ma->xzmp.mouseHover ) {
        DrawCubeV( ma->xzmp.pos, ma->xzmp.dim, ma->xzmp.color );
        DrawCubeWiresV( ma->xzmp.pos, ma->xzmp.dim, BLACK );
    } else {
        DrawCubeV( ma->xzmp.pos, ma->xzmp.dim, Fade( ma->xzmp.color, alpha ) );
        DrawCubeWiresV( ma->xzmp.pos, ma->xzmp.dim, Fade( BLACK, alpha ) );
    }

    if ( ma->yzmp.mouseHover ) {
        DrawCubeV( ma->yzmp.pos, ma->yzmp.dim, ma->yzmp.color );
        DrawCubeWiresV( ma->yzmp.pos, ma->yzmp.dim, BLACK );
    } else {
        DrawCubeV( ma->yzmp.pos, ma->yzmp.dim, Fade( ma->yzmp.color, alpha ) );
        DrawCubeWiresV( ma->yzmp.pos, ma->yzmp.dim, Fade( BLACK, alpha ) );
    }

}

static MoveAnchorCollisionType checkCollisionMouseMoveAnchor( MoveAnchor *ma, Camera3D *camera ) {

    Ray ray = GetScreenToWorldRay( GetMousePosition(), *camera );

    RayCollision xyCol = GetRayCollisionBox( 
        ray,
        (BoundingBox) {
            .min = { 
                ma->xymp.pos.x - ma->movePlaneBigSize / 2,
                ma->xymp.pos.y - ma->movePlaneBigSize / 2,
                ma->xymp.pos.z - ma->movePlaneSmallSize / 2,
            },
            .max = {
                ma->xymp.pos.x + ma->movePlaneBigSize / 2,
                ma->xymp.pos.y + ma->movePlaneBigSize / 2,
                ma->xymp.pos.z + ma->movePlaneSmallSize / 2,
            },
        }
    );

    RayCollision xzCol = GetRayCollisionBox( 
        ray,
        (BoundingBox) {
            .min = { 
                ma->xzmp.pos.x - ma->movePlaneBigSize / 2,
                ma->xzmp.pos.y - ma->movePlaneSmallSize / 2,
                ma->xzmp.pos.z - ma->movePlaneBigSize / 2,
            },
            .max = {
                ma->xzmp.pos.x + ma->movePlaneBigSize / 2,
                ma->xzmp.pos.y + ma->movePlaneSmallSize / 2,
                ma->xzmp.pos.z + ma->movePlaneBigSize / 2,
            },
        }
    );

    RayCollision yzCol = GetRayCollisionBox( 
        ray,
        (BoundingBox) {
            .min = { 
                ma->yzmp.pos.x - ma->movePlaneSmallSize / 2,
                ma->yzmp.pos.y - ma->movePlaneBigSize / 2,
                ma->yzmp.pos.z - ma->movePlaneBigSize / 2,
            },
            .max = {
                ma->yzmp.pos.x + ma->movePlaneSmallSize / 2,
                ma->yzmp.pos.y + ma->movePlaneBigSize / 2,
                ma->yzmp.pos.z + ma->movePlaneBigSize / 2,
            },
        }
    );

    float minDist = INFINITY;
    MoveAnchorCollisionType collType = MOVE_ANCHOR_COLLISION_TYPE_NONE;

    float xyDist = INFINITY;
    float xzDist = INFINITY;
    float yzDist = INFINITY;

    if ( xyCol.hit ) xyDist = xyCol.distance;
    if ( xzCol.hit ) xzDist = xzCol.distance;
    if ( yzCol.hit ) yzDist = yzCol.distance;

    if ( xyDist < minDist ) { 
        minDist = xyDist;
        collType = MOVE_ANCHOR_COLLISION_TYPE_XY;
    }

    if ( xzDist < minDist ) {
        minDist = xzDist;
        collType = MOVE_ANCHOR_COLLISION_TYPE_XZ;
    }

    if ( yzDist < minDist ) {
        minDist = yzDist;
        collType = MOVE_ANCHOR_COLLISION_TYPE_YZ;
    }

    return collType;

}