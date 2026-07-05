/**
 * @file MapSerializer.h
 * @author Prof. Dr. David Buzatto
 * @brief Text-based save/load of a GameWorld's map pieces.
 *
 * @copyright Copyright (c) 2026
 */
#pragma once

#include <stdbool.h>

#include "GameWorld.h"

/**
 * @brief Saves all map pieces (model id, position, rotation and scale) to
 * a text file, one piece per line.
 */
void saveMap( const char *filePath, GameWorld *gw );

/**
 * @brief Loads map pieces from a text file, replacing whatever is currently
 * in gw. If centralizeAfterLoad is true, the whole map is shifted so it's
 * centered on the XZ plane and its lowest point sits at y = 0.
 *
 * @note Does not touch any current map piece selection -- deselect before
 * calling this if a piece might be selected (createGameWorld doesn't need
 * to, since nothing is selected yet at startup).
 */
void loadMap( const char *filePath, GameWorld *gw, bool centralizeAfterLoad );
