#pragma once
#include <string>
#include "variables.h"
#include "raylib.h"
void DrawRectangleRoundedLinesCustom(Rectangle rec, float roundness, int segments, float lineThick, Color color);
namespace VectorRenderer {
    void DrawLeaf(float x, float y, float size, float angleDegrees);
    void DrawTileWoodGrain(float x, float y, float w, float h, bool isDark);
    void DrawOvergrownVines(int boardX, int boardY, float boardSize, int id);
    void DrawBoardOrnateFrame(int x, int y, int size);
    void DrawChessPieceVector(PieceType type, std::string color, int x, int y, int size);
}
