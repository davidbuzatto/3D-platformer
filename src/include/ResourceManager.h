/**
 * @file ResourceManager.h
 * @author Prof. Dr. David Buzatto
 * @brief ResourceManager struct and function declarations.
 * 
 * @copyright Copyright (c) 2026
 */
#pragma once

#include "raylib/raylib.h"

#include "MapPiece.h"

typedef struct ResourceManager {

    int mapPieceModelAtlasCount;
    Model *mapPieceModelAtlas;

    Font baseFont;

    Texture2D mapPieceModelAtlasPreviewTexture;

    /*Texture2D textureExample;
    Sound soundExample;
    Music musicExample;*/

} ResourceManager;

// Read-only outside this module. Use rm->field to access resources.
extern const ResourceManager * const rm;

/**
 * @brief Load global game resources, linking them in the global instance of
 * ResourceManager called rm.
 */
void loadResourcesResourceManager( void );

/**
 * @brief Unload global game resources.
 */
void unloadResourcesResourceManager( void );

/**
 * @brief Get a human-readable name for a map piece model type, used for
 * tooltips in the model picker.
 */
const char *getMapPieceModelTypeName( MapPieceModelType type );
