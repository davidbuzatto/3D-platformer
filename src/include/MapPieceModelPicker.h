/**
 * @file MapPieceModelPicker.h
 * @author Prof. Dr. David Buzatto
 * @brief Grid UI for picking which map piece model to place next (F3 mode).
 *
 * @copyright Copyright (c) 2026
 */
#pragma once

#include "MapPiece.h"

/**
 * @brief Sets up the picker layout from the preview texture already loaded
 * in the ResourceManager. Call once after resources are loaded.
 */
void initMapPieceModelPicker( void );

/**
 * @brief Reads mouse input to hover/select a model type from the picker
 * grid. Call every frame while the picker is the active editor mode.
 */
void updateMapPieceModelPicker( void );

/**
 * @brief Draws the model picker: preview texture, hover/selection
 * highlights and a tooltip with the hovered model's name.
 */
void drawMapPieceModelPicker( void );

/**
 * @brief Gets the model type currently selected in the picker, used when
 * adding new map pieces.
 */
MapPieceModelType getSelectedMapPieceModelType( void );
