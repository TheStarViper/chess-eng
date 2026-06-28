#pragma once
#include "main.hpp"
#include "variables.h"

namespace CanvasRenderer {

    void DrawCapturedTrays(BoardState& displayState);
    void DrawMoveLog();
    void draw_right_side(std::string title);
    void DrawGameMetrics();
    void draw_board_markings();
    void draw_board_tiles(BoardState& displayState, std::pair<int, int> checkKingSquare);

    void DrawGlowTargetRing(float x, float y, float radius, float thickness, Color baseColor, bool isHovered);

    void draw_duck_placement_dots();
    void draw_legal_moves();
    void draw_static_pieces(BoardState& displayState);
    void draw_promotion_panel();

    void draw_game_over();
    void DrawChessboard(BoardState displayState);

    void draw_puzzle_streak_dots();
    void DrawPuzzleSideBar();
}

