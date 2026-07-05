/**
 * @file MapSerializer.c
 * @author Prof. Dr. David Buzatto
 * @brief MapSerializer implementation.
 *
 * @copyright Copyright (c) 2026
 */
#include <stdio.h>
#include <stdlib.h>

#include "raylib/raylib.h"

#include "MapSerializer.h"
#include "MapPiece.h"
#include "ResourceManager.h"

void saveMap( const char *filePath, GameWorld *gw ) {

    int bufferSize = gw->mapPiecesCount * 128 + 1;   // upper bound per line
    char *buffer = (char*) malloc( bufferSize );
    int offset = 0;

    for ( int i = 0; i < gw->mapPiecesCount; i++ ) {

        MapPiece *mp = &gw->mapPieces[i];

        offset += snprintf(
            buffer + offset,
            bufferSize - offset,
            "%d %.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f\n",
            (int) mp->modelType,
            mp->pos.x, mp->pos.y, mp->pos.z,
            mp->rot.x, mp->rot.y, mp->rot.z,
            mp->sca.x, mp->sca.y, mp->sca.z
        );

    }

    SaveFileText( filePath, buffer );
    free( buffer );

}

void loadMap( const char *filePath, GameWorld *gw, bool centralizeAfterLoad ) {

    char *fileText = LoadFileText( filePath );
    if ( fileText == NULL ) {
        return;
    }

    gw->mapPiecesCount = 0;

    const char *cursor = fileText;
    int modelType;
    Vector3 pos;
    Vector3 rot;
    Vector3 sca;
    int charsRead = 0;

    while ( gw->mapPiecesCount < gw->maxMapPieces &&
            sscanf( cursor, "%d %f %f %f %f %f %f %f %f %f%n",
                    &modelType,
                    &pos.x, &pos.y, &pos.z,
                    &rot.x, &rot.y, &rot.z,
                    &sca.x, &sca.y, &sca.z, &charsRead ) == 10 ) {

        if ( modelType >= 0 && modelType < rm->mapPieceModelAtlasCount ) {
            MapPiece *mp = &gw->mapPieces[gw->mapPiecesCount];
            initMapPiece( mp, pos, (MapPieceModelType) modelType );
            mp->rot = rot;
            mp->sca = sca;
            gw->mapPiecesCount++;
        }

        cursor += charsRead;

    }

    if ( centralizeAfterLoad && gw->mapPiecesCount > 0 ) {

        Vector3 min = { 0 };
        Vector3 max = { 0 };

        for ( int i = 0; i < gw->mapPiecesCount; i++ ) {

            MapPiece *mp = &gw->mapPieces[i];

            if ( i == 0 ) {
                min.x = max.x = mp->bb.min.x;
                min.y = max.y = mp->bb.min.y;
                min.z = max.z = mp->bb.min.z;
            } else {
                if ( min.x > mp->bb.min.x ) min.x = mp->bb.min.x;
                if ( min.y > mp->bb.min.y ) min.y = mp->bb.min.y;
                if ( min.z > mp->bb.min.z ) min.z = mp->bb.min.z;
                if ( max.x < mp->bb.max.x ) max.x = mp->bb.max.x;
                if ( max.y < mp->bb.max.y ) max.y = mp->bb.max.y;
                if ( max.z < mp->bb.max.z ) max.z = mp->bb.max.z;
            }

        }

        float xShift = -( min.x + max.x ) / 2.0f;
        float yShift = -min.y;
        float zShift = -( min.z + max.z ) / 2.0f;

        for ( int i = 0; i < gw->mapPiecesCount; i++ ) {
            gw->mapPieces[i].pos.x += xShift;
            gw->mapPieces[i].pos.y += yShift;
            gw->mapPieces[i].pos.z += zShift;
        }

    }

    UnloadFileText( fileText );

}
