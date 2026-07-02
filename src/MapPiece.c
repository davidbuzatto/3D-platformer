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

static void update( MapPiece *mp, Camera3D *camera, float delta );
static void draw( MapPiece *mp, bool drawDebugInfo );

void initMapPiece( MapPiece *mp, Vector3 pos, Model model ) {
    
    mp->pos = pos;
    mp->rot = (Vector3) { 0 };
    mp->sca = (Vector3) { 1.0f, 1.0f, 1.0f };

    mp->color = BLUE;
    mp->model = model;
    mp->bb = GetModelBoundingBox( mp->model );
    mp->bb.min = Vector3Add( mp->bb.min, mp->pos );
    mp->bb.max = Vector3Add( mp->bb.max, mp->pos );

    mp->moveAnchor = (MoveAnchor) {
        .pos = mp->pos,
        .movePlaneBigSize = MOVE_PLANE_BIG_SIZE,
        .movePlaneSmallSize = MOVE_PLANE_SMALL_SIZE,
        .xmp = {
            .pos = { 0 },
            .dim = {
                MOVE_PLANE_BIG_SIZE,
                MOVE_PLANE_BIG_SIZE,
                MOVE_PLANE_SMALL_SIZE
            },
            .color = GREEN
        },
        .ymp = {
            .pos = { 0 },
            .dim = {
                MOVE_PLANE_SMALL_SIZE,
                MOVE_PLANE_BIG_SIZE,
                MOVE_PLANE_BIG_SIZE
            },
            .color = RED
        },
        .zmp = {
            .pos = { 0 },
            .dim = {
                MOVE_PLANE_BIG_SIZE,
                MOVE_PLANE_SMALL_SIZE,
                MOVE_PLANE_BIG_SIZE
            },
            .color = BLUE
        }
    };

    mp->moveAnchorOffset = (Vector3) { 0, mp->bb.max.y - mp->bb.min.y + 0.1f, 0 };

    mp->update = update;
    mp->draw = draw;

}

static void update( MapPiece *mp, Camera3D *camera, float delta ) {
    mp->model.transform = MatrixRotateXYZ( (Vector3) { mp->rot.x * DEG2RAD, mp->rot.y * DEG2RAD, mp->rot.z * DEG2RAD } );
    updateMoveAnchor( &mp->moveAnchor, mp->pos, mp->moveAnchorOffset );
}

static void draw( MapPiece *mp, bool drawDebugInfo ) {

    //DrawModel( mp->model, mp->pos, 1.0f, WHITE );
    DrawModelEx( mp->model, mp->pos, (Vector3){ 0 }, 0.0f, mp->sca, WHITE );

    if ( drawDebugInfo ) {
        DrawBoundingBox( mp->bb, BLACK );
        drawMoveAnchor( &mp->moveAnchor );
    }

}
