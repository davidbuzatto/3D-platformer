/**
 * @file MapPieceEditor.h
 * @author Prof. Dr. David Buzatto
 * @brief Map piece selection, gizmo dragging and the properties panel.
 *
 * @copyright Copyright (c) 2026
 */
#pragma once

#include "raylib/raylib.h"

#include "GameWorld.h"
#include "MapPiece.h"
#include "Gizmo.h"

typedef enum GizmoMode {
    GIZMO_MODE_TRANSLATE,
    GIZMO_MODE_ROTATE,
    GIZMO_MODE_SCALE,
} GizmoMode;

typedef enum MapPieceDuplicateOperationAxis {
    DUPLICATE_OPERATION_X,
    DUPLICATE_OPERATION_Y,
    DUPLICATE_OPERATION_Z,
} DuplicateOperation;

MapPiece *getSelectedMapPiece( void );
void deselectSelectedMapPiece( void );

GizmoMode getGizmoMode( void );
void setGizmoMode( GizmoMode mode );
GizmoOperationMode toGizmoOperationMode( GizmoMode mode );

/**
 * @brief Handles map piece selection and gizmo dragging for one frame:
 * clicking picks a gizmo axis (priority) or selects a new piece, dragging
 * performs the current gizmo operation, releasing clears the gizmo state.
 */
void updateMapPieceSelectionAndGizmo( GameWorld *gw, Camera *camera );

void addMapPiece( GameWorld *gw, MapPieceModelType modelType );
void removeMapPiece( MapPiece *mp, GameWorld *gw );

/**
 * @brief Draws the position/rotation/scale panel for the selected map
 * piece. Does nothing if no piece is selected.
 */
void drawMapPiecePropertiesPanel( void );

/**
 * @brief Rectangle of the properties panel (bottom-left corner), so other
 * panels (e.g. the player debug panel in play mode) can share the exact
 * same spot.
 */
Rectangle getMapPiecePropertiesPanelRec( void );

/**
 * @brief Clones the selected map piece (same model, rotation and scale) and
 * offsets the copy along the given axis by its own bounding-box size, so it
 * sits flush against the original with no gap or overlap. The copy becomes
 * the new selection, so repeating the same call chains a row/column/stack.
 * Does nothing if no piece is selected.
 */
void duplicateSelectedMapPiece( GameWorld *gw, DuplicateOperation axis, float sign );
