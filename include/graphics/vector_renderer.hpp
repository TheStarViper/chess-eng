#pragma once

#include "raylib.h"
#include "variables.h"
#include <string>
namespace VectorRenderer {

    void DrawLeaf(float x, float y, float size, float angleDegrees);

    void DrawTileWoodGrain(float x, float y, float w, float h, bool isDark);

    void DrawOvergrownVines(int boardX, int boardY, float boardSize);


    void DrawBoardOrnateFrame(int x, int y, int size);

    void DrawChessPieceVector(PieceType type, std::string color, int x, int y, int size);

}