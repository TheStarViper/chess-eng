#pragma once

#include "variables.h"
#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <memory>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <utility>
#include <cstdlib>
#include <random>
#include <fstream>
#include <unordered_map>

void playsoundsmart(Sound sound, float volume = 1.0,float pitch = 1.0);
class ChessBoard {
public:
    GridData grid[8][8];
    std::string turn; 
    std::vector<HistorySnapshot> move_history;
    
    std::pair<int, int> selected_square;
    bool has_selection;
    bool is_promoting;
    std::pair<int, int> promotion_square;
    std::pair<int, int> promotion_source;
    
    bool white_king_side_castle;
    bool white_queen_side_castle;
    bool black_king_side_castle;
    bool black_queen_side_castle;
    std::pair<int, int> en_passant_square; 
    int halfmove_clock;
    int fullmove_number;
    GameState state;

    std::pair<int, int> last_move_from;
    std::pair<int, int> last_move_to;
    bool in_check;
    std::pair<int, int> check_king_pos;
    int current_repetition_count;

    ChessBoard();
    void Reset();
    BoardState CaptureState() const;
    void LoadState(const BoardState& bs);
};

class Button {
public:
    Rectangle rect;
    std::string label;
    Color baseColor;
    Color hoverColor;
    Color textColor;
    bool isPressed;
    bool isHovered;
    int iconType; 

    Button(Rectangle r, std::string lbl, Color base = Color{ 40, 41, 45, 255 }, Color hover = Color{ 60, 62, 68, 255 }, Color txt = WHITE, int icon = 0);
    void Update(Vector2 mousePos);

    void Draw();
private:
    bool soundplayed;
};