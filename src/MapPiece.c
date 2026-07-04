#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "raylib/raylib.h"
#include "raylib/raymath.h"

#include "MapPiece.h"
#include "Macros.h"
#include "Gizmo.h"
#include "GizmoAxis.h"
#include "ResourceManager.h"

static void update( MapPiece *mp, Camera3D *camera, float delta );
static void draw( MapPiece *mp );

void initMapPiece( MapPiece *mp, Vector3 pos, MapPieceModelType modelType ) {
    
    mp->pos = pos;
    mp->rot = (Vector3) { 0.0f, 0.0f, 0.0f };
    mp->sca = (Vector3) { 1.0f, 1.0f, 1.0f };

    mp->color = BLUE;
    mp->modelType = modelType;
    mp->model = rm->mapPieceModelAtlas[modelType];
    mp->baseBB = GetModelBoundingBox( mp->model );
    mp->bb.min = Vector3Add( mp->baseBB.min, mp->pos );
    mp->bb.max = Vector3Add( mp->baseBB.max, mp->pos );

    mp->gizmo = (Gizmo) {
        .pos = mp->pos,
        .xAxis = {
            .pos = { 0 },
            .radius = 0.1f,
            .color = RED
        },
        .yAxis = {
            .pos = { 0 },
            .radius = 0.1f,
            .color = GREEN
        },
        .zAxis = {
            .pos = { 0 },
            .radius = 0.1f,
            .color = BLUE
        }
    };

    mp->gizmoOffset = (Vector3) { 0, mp->bb.max.y - mp->bb.min.y + 0.1f, 0 };
    mp->selected = false;

    mp->update = update;
    mp->draw = draw;

}

static void update( MapPiece *mp, Camera3D *camera, float delta ) {

    mp->model.transform = MatrixRotateXYZ( 
        (Vector3) { 
            mp->rot.x * DEG2RAD, 
            mp->rot.y * DEG2RAD, 
            mp->rot.z * DEG2RAD
        }
    );

    // re-derives the world box from the local (unscaled) one every frame,
    // so scaling never leaves the box -- and the gizmo -- behind
    mp->bb.min = Vector3Add( mp->pos, Vector3Multiply( mp->baseBB.min, mp->sca ) );
    mp->bb.max = Vector3Add( mp->pos, Vector3Multiply( mp->baseBB.max, mp->sca ) );

    mp->gizmoOffset = (Vector3) { 0, mp->bb.max.y - mp->bb.min.y + 0.1f, 0 };

    updateGizmo( &mp->gizmo, mp->pos, mp->gizmoOffset );

}

static void draw( MapPiece *mp ) {

    //DrawModel( mp->model, mp->pos, 1.0f, WHITE );
    DrawModelEx( mp->model, mp->pos, (Vector3){ 0 }, 0.0f, mp->sca, WHITE );

    if ( mp->selected ) {
        DrawBoundingBox( mp->bb, BLACK );
        drawGizmo( &mp->gizmo );
    }

}
