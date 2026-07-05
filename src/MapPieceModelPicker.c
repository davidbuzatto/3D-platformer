/**
 * @file MapPieceModelPicker.c
 * @author Prof. Dr. David Buzatto
 * @brief MapPieceModelPicker implementation.
 *
 * @copyright Copyright (c) 2026
 */
#include "raylib/raylib.h"

#include "MapPieceModelPicker.h"
#include "ResourceManager.h"

static MapPieceModelType selectedMapPieceModelType = MODEL_TYPE_BLOCK_GRASS;
static Rectangle mouseHoverMapPieceModelRect = { 0, 0, 0, 0 };
static int hoveredMapPieceModelType = -1;  // -1 while the mouse isn't over any tile

static int targetWSelect;
static int targetHSelect;
static int piecesPerLineSelect;
static int pieceSizeSelect;
static int startXSelect;
static int startYSelect;
static int marginSelect;
static int spacingSelect;

void initMapPieceModelPicker( void ) {

    targetWSelect = rm->mapPieceModelAtlasPreviewTexture.width / 2;
    targetHSelect = rm->mapPieceModelAtlasPreviewTexture.height / 2;
    piecesPerLineSelect = 15;
    pieceSizeSelect = 32;
    startYSelect = 55;
    marginSelect = 5;
    spacingSelect = 3;

}

void updateMapPieceModelPicker( void ) {

    int mouseX = GetMouseX();
    int mouseY = GetMouseY();
    startXSelect = GetScreenWidth() / 2 - targetWSelect / 2;

    hoveredMapPieceModelType = -1;

    if ( mouseX >= startXSelect + marginSelect && mouseX <= startXSelect + targetWSelect - marginSelect &&
         mouseY >= startYSelect + marginSelect && mouseY <= startYSelect + targetHSelect - marginSelect ) {

        int offsetX = mouseX - ( startXSelect + marginSelect );
        int offsetY = mouseY - ( startYSelect + marginSelect );

        int col = offsetX / ( pieceSizeSelect + spacingSelect );
        int row = offsetY / ( pieceSizeSelect + spacingSelect );

        mouseHoverMapPieceModelRect.x = startXSelect + marginSelect + col * ( pieceSizeSelect + spacingSelect );
        mouseHoverMapPieceModelRect.y = startYSelect + marginSelect + row * ( pieceSizeSelect + spacingSelect );
        mouseHoverMapPieceModelRect.width = mouseHoverMapPieceModelRect.height = pieceSizeSelect;

        int modelPos = row * piecesPerLineSelect + col;
        if ( modelPos < rm->mapPieceModelAtlasCount ) {
            hoveredMapPieceModelType = modelPos;
            if ( IsMouseButtonPressed( MOUSE_BUTTON_LEFT ) ) {
                selectedMapPieceModelType = modelPos;
            }
        }

    }

}

void drawMapPieceModelPicker( void ) {

    DrawTexturePro(
        rm->mapPieceModelAtlasPreviewTexture,
        (Rectangle) { 0, 0, rm->mapPieceModelAtlasPreviewTexture.width, rm->mapPieceModelAtlasPreviewTexture.height },
        (Rectangle) { GetScreenWidth() / 2 - targetWSelect / 2, startYSelect, targetWSelect, targetHSelect },
        (Vector2) { 0 },
        0,
        WHITE
    );

    int pieceModelPos = (int) selectedMapPieceModelType;
    int row = pieceModelPos / piecesPerLineSelect;
    int col = pieceModelPos % piecesPerLineSelect;

    Rectangle selectedMapPieceModelRect = {
        startXSelect + marginSelect + ( pieceSizeSelect + spacingSelect ) * col,
        startYSelect + marginSelect + ( pieceSizeSelect + spacingSelect ) * row,
        pieceSizeSelect,
        pieceSizeSelect
    };
    DrawRectangleLinesEx( selectedMapPieceModelRect, 2.0f, BLUE );
    DrawRectangleLinesEx( mouseHoverMapPieceModelRect, 2.0f, WHITE );

    if ( hoveredMapPieceModelType >= 0 ) {

        const char *name = getMapPieceModelTypeName( (MapPieceModelType) hoveredMapPieceModelType );
        const int fontSize = 20;
        const int tooltipMargin = 8;
        const int tooltipOffset = 16;

        Vector2 mousePos = GetMousePosition();
        Vector2 textSize = MeasureTextEx( rm->baseFont, name, fontSize, 0.0f );

        Rectangle tooltipRect = {
            mousePos.x + tooltipOffset,
            mousePos.y + tooltipOffset,
            textSize.x + tooltipMargin * 2,
            textSize.y + tooltipMargin * 2
        };

        DrawRectangleRounded( tooltipRect, 0.2f, 10, Fade( WHITE, 0.7f ) );
        DrawRectangleRoundedLinesEx( tooltipRect, 0.2f, 10, 2.0f, BLACK );
        DrawTextEx(
            rm->baseFont, name,
            (Vector2) { tooltipRect.x + tooltipMargin, tooltipRect.y + tooltipMargin },
            fontSize, 0.0f, BLACK
        );

    }

}

MapPieceModelType getSelectedMapPieceModelType( void ) {
    return selectedMapPieceModelType;
}
