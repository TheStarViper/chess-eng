#pragma once
#include <memory>
#include "raylib.h"
#include <iostream>
#define P_WHITE "WHITE"
#define P_BLACK "BLACK"

//security checks
bool g_puzzlesLoaded = false;
bool audio_loaded = false;
bool load_new_puzzle = true;

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


    
struct ChessPiece {
    PieceType type;
    std::string color; 
    bool has_moved;    

    ChessPiece(PieceType t, std::string col) : type(t), color(col), has_moved(false) {}
};

struct GridData {
    bool has_piece;
    std::shared_ptr<ChessPiece> piece;

    GridData() : has_piece(false), piece(nullptr) {}
    GridData(std::shared_ptr<ChessPiece> p) : has_piece(p != nullptr), piece(p) {}
};

struct Puzzle{
    std::string puzzleid;
    std::string gameid;
    std::string boardsetup;
    std::string solution;
    int rating;
};

struct BoardState {
    GridData grid[8][8];
    std::string turn;
    
    bool white_king_side_castle;
    bool white_queen_side_castle;
    bool black_king_side_castle;
    bool black_queen_side_castle;
    
    std::pair<int, int> en_passant_square;
    int halfmove_clock; 
    int fullmove_number;

    std::pair<int, int> last_move_from{-1, -1};
    std::pair<int, int> last_move_to{-1, -1};
    bool in_check = false;
    std::pair<int, int> check_king_pos{-1, -1};
    int repetition_count = 1;

    bool operator==(const BoardState& other) const {
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                if (grid[r][c].has_piece != other.grid[r][c].has_piece) return false;
                if (grid[r][c].has_piece) {
                    if (grid[r][c].piece->type != other.grid[r][c].piece->type ||
                        grid[r][c].piece->color != other.grid[r][c].piece->color) {
                        return false;
                    }
                }
            }
        }
        return turn == other.turn &&
               white_king_side_castle == other.white_king_side_castle &&
               white_queen_side_castle == other.white_queen_side_castle &&
               black_king_side_castle == other.black_king_side_castle &&
               black_queen_side_castle == other.black_queen_side_castle &&
               en_passant_square == other.en_passant_square;
    }
};

struct HistorySnapshot {
    std::string notation; 
    BoardState board_state;
};

struct PieceAnimation {
    bool active;
    std::shared_ptr<ChessPiece> piece;
    Vector2 startPos; 
    Vector2 currentPos;
    Vector2 endPos;
    float elapsedTime;
    int targetRow;
    int targetCol;
};

struct HistoryState {
    bool useLive;            
    int viewingIndex;       
};

//asset file paths
std::string puzzlefilepath = "assets/puzzles_new.csv";
const char * hoversoundfilepath = "assets/sfx/hover.ogg";