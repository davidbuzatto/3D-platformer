/**
 * @file ResourceManager.c
 * @author Prof. Dr. David Buzatto
 * @brief ResourceManager implementation.
 * 
 * @copyright Copyright (c) 2026
 */
#include <stdio.h>
#include <stdlib.h>

#include "raylib/raylib.h"

#include "ResourceManager.h"

static ResourceManager _rm = { 0 };  // mutable; owned exclusively by this module
const ResourceManager * const rm = &_rm;

void loadResourcesResourceManager( void ) {

    _rm.baseFont = LoadFontEx( "resources/fonts/space-mono/SpaceMono-Bold.ttf", 20, NULL, 250 );

    _rm.blockGrassModel = LoadModel( "resources/models/block-grass.glb" );
    _rm.blockGrassCornerModel = LoadModel( "resources/models/block-grass-corner.glb" );
    _rm.blockGrassCurveModel = LoadModel( "resources/models/block-grass-curve.glb" );
    _rm.blockGrassEdgeModel = LoadModel( "resources/models/block-grass-edge.glb" );
    _rm.blockGrassLongModel = LoadModel( "resources/models/block-grass-long.glb" );
    _rm.blockGrassLargeModel = LoadModel( "resources/models/block-grass-large.glb" );
    _rm.blockGrassLargeTallModel = LoadModel( "resources/models/block-grass-large-tall.glb" );

    /*_rm.textureExample = LoadTexture( "resources/images/image.png" );
    _rm.soundExample = LoadSound( "resources/sfx/sound.wav" );
    _rm.musicExample = LoadMusicStream( "resources/musics/music.ogg" );*/

}

void unloadResourcesResourceManager( void ) {

    UnloadFont( _rm.baseFont );

    UnloadModel( _rm.blockGrassModel );
    UnloadModel( _rm.blockGrassCornerModel );
    UnloadModel( _rm.blockGrassCurveModel );
    UnloadModel( _rm.blockGrassEdgeModel );
    UnloadModel( _rm.blockGrassLongModel );
    UnloadModel( _rm.blockGrassLargeModel );
    UnloadModel( _rm.blockGrassLargeTallModel );

    /*UnloadTexture( _rm.textureExample );
    UnloadSound( _rm.soundExample );
    UnloadMusicStream( _rm.musicExample );*/

}