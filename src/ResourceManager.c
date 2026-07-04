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

static void prepareMapPieceModelAtlas( void );

void loadResourcesResourceManager( void ) {

    SetTraceLogLevel( LOG_WARNING );
    prepareMapPieceModelAtlas();
    SetTraceLogLevel( LOG_ALL );

    _rm.baseFont = LoadFontEx( "resources/fonts/space-mono/SpaceMono-Bold.ttf", 20, NULL, 250 );

    _rm.mapPieceModelAtlasPreviewTexture = LoadTexture( "resources/images/mapPieceModelAtlasPreview.png" );
    SetTextureFilter( _rm.mapPieceModelAtlasPreviewTexture, TEXTURE_FILTER_BILINEAR );
    
    /*_rm.textureExample = LoadTexture( "resources/images/image.png" );
    _rm.soundExample = LoadSound( "resources/sfx/sound.wav" );
    _rm.musicExample = LoadMusicStream( "resources/musics/music.ogg" );*/

}

void unloadResourcesResourceManager( void ) {

    SetTraceLogLevel( LOG_WARNING );
    for ( int i = 0; i < _rm.mapPieceModelAtlasCount; i++ ) {
        UnloadModel( _rm.mapPieceModelAtlas[i] );
    }
    free( _rm.mapPieceModelAtlas );
    SetTraceLogLevel( LOG_ALL );

    UnloadFont( _rm.baseFont );
    UnloadTexture( _rm.mapPieceModelAtlasPreviewTexture );

    /*UnloadTexture( _rm.textureExample );
    UnloadSound( _rm.soundExample );
    UnloadMusicStream( _rm.musicExample );*/

}

static void prepareMapPieceModelAtlas( void ) {

    int modelCount = (int) MODEL_TYPE_TREE_SNOW + 1;
    _rm.mapPieceModelAtlas = (Model*) malloc( sizeof( Model ) * modelCount );

    int currentModel = 0;
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/arrow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/arrows.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/barrel.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrass.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassCorner.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassCornerLow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassCornerOverhang.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassCornerOverhangLow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassCurve.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassCurveHalf.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassCurveLow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassEdge.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassHexagon.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassLarge.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassLargeSlope.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassLargeSlopeNarrow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassLargeSlopeSteep.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassLargeSlopeSteepNarrow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassLargeTall.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassLong.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassLow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassLowHexagon.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassLowLarge.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassLowLong.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassLowNarrow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassNarrow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassOverhangCorner.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassOverhangEdge.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassOverhangHexagon.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassOverhangLarge.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassOverhangLargeSlope.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassOverhangLargeSlopeNarrow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassOverhangLargeSlopeSteep.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassOverhangLargeSlopeSteepNarrow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassOverhangLargeTall.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassOverhangLong.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassOverhangLow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassOverhangLowHexagon.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassOverhangLowLarge.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassOverhangLowLong.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassOverhangLowNarrow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockGrassOverhangNarrow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockMoving.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockMovingBlue.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockMovingLarge.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowCorner.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowCornerLow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowCornerOverhang.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowCornerOverhangLow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowCurve.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowCurveHalf.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowCurveLow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowEdge.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowHexagon.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowLarge.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowLargeSlope.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowLargeSlopeNarrow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowLargeSlopeSteep.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowLargeSlopeSteepNarrow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowLargeTall.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowLong.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowLow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowLowHexagon.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowLowLarge.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowLowLong.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowLowNarrow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowNarrow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowOverhangCorner.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowOverhangEdge.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowOverhangHexagon.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowOverhangLarge.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowOverhangLargeSlope.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowOverhangLargeSlopeNarrow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowOverhangLargeSlopeSteep.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowOverhangLargeSlopeSteepNarrow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowOverhangLargeTall.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowOverhangLong.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowOverhangLow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowOverhangLowHexagon.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowOverhangLowLarge.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowOverhangLowLong.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowOverhangLowNarrow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/blockSnowOverhangNarrow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/bomb.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/brick.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/buttonRound.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/buttonSquare.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/characterOobi.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/characterOodi.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/characterOoli.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/characterOopi.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/characterOozi.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/chest.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/coinBronze.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/coinGold.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/coinSilver.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/conveyorBelt.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/crate.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/crateItem.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/crateItemStrong.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/crateStrong.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/doorLargeOpen.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/doorOpen.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/doorRotate.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/doorRotateLarge.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/fenceBroken.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/fenceCorner.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/fenceCornerCurved.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/fenceLowBroken.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/fenceLowCorner.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/fenceLowCornerCurved.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/fenceLowStraight.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/fenceRope.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/fenceStraight.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/flag.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/flowers.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/flowersTall.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/grass.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/heart.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/hedge.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/hedgeCorner.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/jewel.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/key.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/ladder.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/ladderBroken.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/ladderLong.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/lever.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/lock.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/mushrooms.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/pipe.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/plant.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/platform.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/platformFortified.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/platformOverhang.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/platformRamp.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/poles.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/rocks.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/saw.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/sign.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/spikeBlock.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/spikeBlockWide.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/spring.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/star.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/stones.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/trapSpikes.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/trapSpikesLarge.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/tree.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/treePine.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/treePineSmall.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/treePineSnow.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/treePineSnowSmall.glb" );
    _rm.mapPieceModelAtlas[currentModel++] = LoadModel( "resources/models/treeSnow.glb" );
    _rm.mapPieceModelAtlasCount = currentModel;

}