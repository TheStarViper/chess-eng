#pragma once
#include <memory>
#include "raylib.h"
#define P_WHITE "WHITE"
#define P_BLACK "BLACK"

namespace Config {
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 720;

    constexpr int TILE_SIZE = 75;                // Size of chessboard squares (75 * 8 = 600px)
    constexpr int BOARD_OFFSET_X = 210;          // Left buffer
    constexpr int BOARD_OFFSET_Y = 60;          // Top buffer

    constexpr int PANEL_X = BOARD_OFFSET_X+TILE_SIZE*8+50;                // Sidebar alignment x-coordinate
    constexpr int PANEL_Y = 60;                 // Sidebar alignment y-coordinate
    constexpr int PANEL_WIDTH = 380;            // Sidebar width
    constexpr int PANEL_HEIGHT = TILE_SIZE*8;           // Panel matches board footprint
    constexpr int ROW_HEIGHT = 35;              // Row spacing for historical lists

    // Aesthetic color schemes matching Board.png
    inline const Color COLOR_LIGHT_SQ = Color{ 205, 171, 128, 255 };  
    inline const Color COLOR_DARK_SQ  = Color{ 116, 75, 48, 255 };    
    inline const Color COLOR_FRAME_DARK = Color{ 48, 28, 16, 255 };   
    inline const Color COLOR_FRAME_MID  = Color{ 78, 48, 30, 255 };
    inline const Color COLOR_LEAF_DARK  = Color{ 76, 91, 55, 255 }; 
    inline const Color COLOR_LEAF_LIGHT = Color{ 151, 163, 69, 255 }; 
    inline const Color COLOR_LEAF_VEIN  = Color{ 113, 129, 63, 255 }; 
    inline const Color COLOR_GEM_BASE   = Color{ 42, 24, 18, 255 };    
    inline const Color COLOR_GEM_GLINT  = Color{ 162, 103, 56, 255 };

    // NEW THEMATIC UI COLORS
    inline const Color COLOR_UI_PANEL_BG   = Color{ 36, 22, 14, 255 }; 
    inline const Color COLOR_UI_BORDER     = Color{ 78, 48, 30, 255 }; 
    inline const Color COLOR_UI_ROW_A      = Color{ 48, 30, 20, 255 }; 
    inline const Color COLOR_UI_ROW_B      = Color{ 38, 24, 16, 255 }; 
    inline const Color COLOR_UI_TEXT       = Color{ 240, 220, 190, 255 }; 
    inline const Color COLOR_UI_TEXT_DIM   = Color{ 180, 150, 120, 255 }; 
    inline const Color COLOR_UI_BUTTON     = Color{ 96, 55, 28, 255 };
    inline const Color COLOR_UI_BUTTON_HOV = Color{ 120, 75, 40, 255 }; 
    
    inline const Color COLOR_HIGHLIGHT= Color{ 123, 97, 255, 120 };   
    inline const Color COLOR_CHECK    = Color{ 230, 90, 90, 220 };    
    inline const Color COLOR_DOT      = Color{ 100, 149, 237, 180 }; 
    inline const Color COLOR_DOT_RING = Color{ 100, 149, 237, 100 };
    inline const Color COLOR_LAST_MOVE= Color{ 246, 235, 120, 100 };  
    
    const Color COLOR_WOOD_DARK   = Color{ 43, 24, 16, 255 };   
    const Color COLOR_WOOD_LIGHT  = Color{ 115, 74, 50, 255 };  
    const Color COLOR_WOOD_GRAIN  = Color{ 74, 44, 28, 255 };   
    const Color COLOR_IVORY       = Color{ 240, 225, 200, 255 }; 

    inline const Color BOARD_MARKINGS_TEXT = Color{ 36, 22, 14, 255 };
    constexpr float ANIMATION_DURATION = 0.12f;

    inline const int SIDEBAR_MAX_WIDTH = 160;
    inline const int SIDEBAR_MIN_WIDTH = 60;
}

enum PieceType {
    NONE = 0,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

enum GameState {
    STATE_PLAYING,
    STATE_PROMOTING,
    STATE_CHECKMATE,
    STATE_STALEMATE,
    STATE_DRAW_REPETITION,
    STATE_DRAW_50_MOVES,
    STATE_DRAW_MATERIAL,
    STATE_RESIGNED,
    STATE_MUTUAL_DRAW
};

enum Menus { 
    PLAY,
    GAME, 
    SETTINGS, 
    PUZZLES, 
    OPENINGS };

struct GameContext; 
extern std::unique_ptr<GameContext> g_ctx;

