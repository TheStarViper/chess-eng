#include "raylib.h"
#include "main.hpp"
#include "leaves.hpp"
#include "easing_functions.hpp"
#include "vec_renderer.hpp"
#include "puzzles.hpp"
#include "variant-switcher.hpp"
#include "vanillalogic.hpp"
#include "settings.hpp"
#include "canvas_renderer.hpp"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#define P_WHITE "WHITE"
#define P_BLACK "BLACK"

void playsoundsmart(Sound sound, float volume,float pitch){
    SetSoundPitch(sound,pitch);
    SetSoundVolume(sound,volume);
    PlaySound(sound);
}

Button::Button(Rectangle r, std::string lbl, Color base, Color hover, Color txt, int icon)
    : rect(r), label(lbl), baseColor(base), hoverColor(hover), textColor(txt), isPressed(false), isHovered(false), iconType(icon), soundplayed(false) {}

void Button::Update(Vector2 mousePos) {
    isHovered = CheckCollisionPointRec(mousePos, rect);
    isPressed = isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}


ChessBoard::ChessBoard() {
        Reset();
    }

void ChessBoard::Reset() {
    turn = P_WHITE;
    has_selection = false;
    selected_square = { -1, -1 };
    is_promoting = false;
    promotion_square = { -1, -1 };
    promotion_source = { -1, -1 };
    
    white_king_side_castle = true;
    white_queen_side_castle = true;
    black_king_side_castle = true;
    black_queen_side_castle = true;
    en_passant_square = { -1, -1 };
    halfmove_clock = 0;
    fullmove_number = 1;
    state = STATE_PLAYING;
    move_history.clear();

    last_move_from = { -1, -1 };
    last_move_to = { -1, -1 };
    in_check = false;
    check_king_pos = { -1, -1 };
    current_repetition_count = 1;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            grid[r][c] = GridData();
        }
    }

    for (int c = 0; c < 8; ++c) {
        grid[1][c] = GridData(std::make_shared<ChessPiece>(PAWN, P_BLACK));
        grid[6][c] = GridData(std::make_shared<ChessPiece>(PAWN, P_WHITE));
    }

    std::vector<PieceType> backline = { ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK };
    for (int c = 0; c < 8; ++c) {
        grid[0][c] = GridData(std::make_shared<ChessPiece>(backline[c], P_BLACK));
        grid[7][c] = GridData(std::make_shared<ChessPiece>(backline[c], P_WHITE));
    }
}

BoardState ChessBoard::CaptureState() const {
    BoardState bs;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            bs.grid[r][c] = grid[r][c];
        }
    }
    bs.turn = turn;
    bs.white_king_side_castle = white_king_side_castle;
    bs.white_queen_side_castle = white_queen_side_castle;
    bs.black_king_side_castle = black_king_side_castle;
    bs.black_queen_side_castle = black_queen_side_castle;
    bs.en_passant_square = en_passant_square;
    bs.halfmove_clock = halfmove_clock;
    bs.fullmove_number = fullmove_number;
    
    bs.last_move_from = last_move_from;
    bs.last_move_to = last_move_to;
    bs.in_check = in_check;
    bs.check_king_pos = check_king_pos;
    bs.repetition_count = current_repetition_count;

    return bs;
}

void ChessBoard::LoadState(const BoardState& bs) {
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            grid[r][c] = bs.grid[r][c];
        }
    }
    turn = bs.turn;
    white_king_side_castle = bs.white_king_side_castle;
    white_queen_side_castle = bs.white_queen_side_castle;
    black_king_side_castle = bs.black_king_side_castle;
    black_queen_side_castle = bs.black_queen_side_castle;
    en_passant_square = bs.en_passant_square;
    halfmove_clock = bs.halfmove_clock;
    fullmove_number = bs.fullmove_number;

    last_move_from = bs.last_move_from;
    last_move_to = bs.last_move_to;
    in_check = bs.in_check;
    check_king_pos = bs.check_king_pos;
    current_repetition_count = bs.repetition_count;
}


void Button::Draw() {
        Color currentBg = isHovered ? hoverColor : baseColor;
        if (isHovered&&!soundplayed){
            playsoundsmart(g_ctx->hoversound,.4,.8);
            soundplayed = true;
        }
        if (!isHovered){
            soundplayed = false;
        }
        DrawRectangleRounded(rect, 0.15f, 4, currentBg);
        DrawRectangleRoundedLinesCustom(rect, 0.15f, 4, 1.5f, Config::COLOR_FRAME_DARK);

        int fontSize = 16;
        Vector2 textSize = MeasureTextSmooth(label.c_str(), (float)fontSize);
        float textX = rect.x + (rect.width - textSize.x) / 2.0f;
        float textY = rect.y + (rect.height - textSize.y) / 2.0f;

        if (iconType == 1) { 
            float flagX = rect.x + 18;
            float flagY = rect.y + rect.height/2.0f;
            DrawLineEx(Vector2{flagX, flagY - 10}, Vector2{flagX, flagY + 12}, 2.5f, textColor);
            DrawTriangle(Vector2{flagX, flagY - 10}, Vector2{flagX, flagY}, Vector2{flagX + 12, flagY - 5}, textColor);
            textX += 8;
        } else if (iconType == 2) { 
            float handX = rect.x + 16;
            float handY = rect.y + rect.height/2.0f;
            DrawCircle((int)handX, (int)handY - 2, 3, textColor);
            DrawCircle((int)handX + 10, (int)handY - 2, 3, textColor);
            DrawLineEx(Vector2{handX - 4, handY + 4}, Vector2{handX + 14, handY + 4}, 2.0f, textColor);
            textX += 10;
        }

        DrawTextSmooth(label.c_str(), textX, textY, (float)fontSize, textColor);
    }




void DrawTextSmooth(const char* text, float posX, float posY, float fontSize, Color color) {
    if (g_ctx && g_ctx->uiFont.texture.id > 0) {
        DrawTextEx(g_ctx->uiFont, text, Vector2{ posX, posY }, fontSize, 1.0f, color);
    } else {
        DrawText(text, (int)posX, (int)posY, (int)fontSize, color);
    }
}

Vector2 MeasureTextSmooth(const char* text, float fontSize) {
    if (g_ctx && g_ctx->uiFont.texture.id > 0) {
        return MeasureTextEx(g_ctx->uiFont, text, fontSize, 1.0f);
    } else {
        return Vector2{ (float)MeasureText(text, (int)fontSize), fontSize };
    }
}

void DrawCollapsibleSidebar(Vector2 mousePos) {
    g_ctx->sidebarhovered = (mousePos.x <= g_ctx->sidebarWidth);
    
    float targetWidth = g_ctx->sidebarhovered ? Config::SIDEBAR_MAX_WIDTH : Config::SIDEBAR_MIN_WIDTH;
    g_ctx->sidebarWidth += (targetWidth - g_ctx->sidebarWidth) * Config::ANIMATION_DURATION*50 * GetFrameTime();

    int currentWidth = (int)g_ctx->sidebarWidth;
    if (currentWidth>Config::SIDEBAR_MAX_WIDTH){currentWidth=Config::SIDEBAR_MAX_WIDTH;}
    if (currentWidth<Config::SIDEBAR_MIN_WIDTH){currentWidth=Config::SIDEBAR_MIN_WIDTH;}
    DrawRectangle(0, 0, currentWidth, Config::WINDOW_HEIGHT, Config::COLOR_UI_PANEL_BG); 
    DrawRectangle(currentWidth - 2, 0, 2, Config::WINDOW_HEIGHT, Config::COLOR_LEAF_LIGHT);

    bool isFullyOpen = (currentWidth > Config::SIDEBAR_MAX_WIDTH - 20);
    bool canClickEarly = (g_ctx->sidebarWidth > (float)Config::SIDEBAR_MIN_WIDTH + 15);
    bool showText = (currentWidth > Config::SIDEBAR_MAX_WIDTH - 40);

    int startY = Config::WINDOW_HEIGHT / 4; 
    int itemSpacing = 60;
    int btnHeight = 45;
    int iconPadding = 15;
    struct MenuItem { Menus mode; const char* icon; const char* label; };
    std::vector<MenuItem> items = {
        { PLAY, " P ", "PLAY"},
        { GAME, " G ", "GAME" },
        { OPENINGS, " O ", "OPENINGS" },
        { PUZZLES, " X ", "PUZZLES" }
    };

    for (size_t i = 0; i < items.size(); ++i) {
        int itemY = startY + (i * itemSpacing);
        Rectangle btnRec = { 5, (float)itemY, (float)(currentWidth - 10), (float)btnHeight };
        
        bool isItemHovered = CheckCollisionPointRec(mousePos, btnRec);
        Color btnColor = (g_ctx->active_menu == items[i].mode) ? Fade(GOLD, 0.3f) :
                        (isItemHovered && g_ctx->sidebarhovered) ? Fade(WHITE, 0.1f) : BLANK;

        DrawRectangleRec(btnRec, btnColor);

        DrawText(items[i].icon, iconPadding, itemY + 12, 20, RAYWHITE);
        if (showText) {
            DrawText(items[i].label, 50, itemY + 14, 16, RAYWHITE);
        }
        if (isItemHovered && canClickEarly) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (g_ctx->active_menu == PUZZLES && g_ctx->anim.active && g_ctx->anim.piece != GridData().piece) {
                    if (g_ctx->anim.targetRow >= 0 && g_ctx->anim.targetRow < 8 &&
                        g_ctx->anim.targetCol >= 0 && g_ctx->anim.targetCol < 8) {
                        
                        g_ctx->board->grid[g_ctx->anim.targetRow][g_ctx->anim.targetCol] = GridData(g_ctx->anim.piece);
                    }
                }

                g_ctx->active_menu = items[i].mode;
                g_ctx->anim.active = false;
                g_ctx->anim.piece = GridData().piece; 
                g_ctx->anim.targetRow = -1;
                g_ctx->anim.targetCol = -1;
            }
        }
    }
    int bottomY = Config::WINDOW_HEIGHT - 70;
    Rectangle settingsRec = { 5, (float)bottomY, (float)(currentWidth - 10), (float)btnHeight };
    
    bool isSettingsHovered = CheckCollisionPointRec(mousePos, settingsRec);
    Color settingsBtnColor = (g_ctx->active_menu == SETTINGS) ? Fade(GOLD, 0.3f) : 
                             (isSettingsHovered && g_ctx->sidebarhovered) ? Fade(WHITE, 0.1f) : BLANK;

    DrawRectangleRec(settingsRec, settingsBtnColor);
    DrawText(" S ", iconPadding, bottomY + 12, 20, RAYWHITE);

    if (isFullyOpen) {
        DrawText("SETTINGS", 50, bottomY + 14, 16, RAYWHITE);
        if (isSettingsHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            g_ctx->active_menu = SETTINGS;
        }
    }
}


namespace TickEngine {

    void UpdateAnimations(float dt) {
        if (!g_ctx->anim.active) return;

        g_ctx->anim.elapsedTime += dt;
        float progress = g_ctx->anim.elapsedTime / Config::ANIMATION_DURATION;

        if (progress >= 1.0f) {
            g_ctx->anim.active = false;
            
            VariantSwitcher::Active()->MakeMove(
                *g_ctx->board, 
                (int)((g_ctx->anim.startPos.y - Config::BOARD_OFFSET_Y) / Config::TILE_SIZE),
                (int)((g_ctx->anim.startPos.x - Config::BOARD_OFFSET_X) / Config::TILE_SIZE),
                g_ctx->anim.targetRow, 
                g_ctx->anim.targetCol,
                NONE
            );
        } else {
            float t = 1.0f - std::pow(1.0f - progress, 3.0f);
            g_ctx->anim.currentPos.x = g_ctx->anim.startPos.x + (g_ctx->anim.endPos.x - g_ctx->anim.startPos.x) * t;
            g_ctx->anim.currentPos.y = g_ctx->anim.startPos.y + (g_ctx->anim.endPos.y - g_ctx->anim.startPos.y) * t;
        }
    }

    void ProcessInput() {
        int targetCursor = MOUSE_CURSOR_DEFAULT;
        int gridX = (int)((g_ctx->mousePosition.x - Config::BOARD_OFFSET_X) / Config::TILE_SIZE);
        int gridY = (int)((g_ctx->mousePosition.y - Config::BOARD_OFFSET_Y) / Config::TILE_SIZE);
        
        bool isMouseOnBoard = (gridX >= 0 && gridX < 8 && gridY >= 0 && gridY < 8);
        g_ctx->mousePosition = GetMousePosition();
        if (g_ctx->active_menu==GAME){
            g_ctx->btnResign->Update(g_ctx->mousePosition);
            g_ctx->btnDraw->Update(g_ctx->mousePosition);
            
            g_ctx->btnFirst->Update(g_ctx->mousePosition);
            g_ctx->btnPrev->Update(g_ctx->mousePosition);
            g_ctx->btnNext->Update(g_ctx->mousePosition);
            g_ctx->btnLast->Update(g_ctx->mousePosition);
        }
        if (gridX >= 0 && gridX < 8 && gridY >= 0 && gridY < 8) {
        GridData clickedTile = g_ctx->board->grid[gridY][gridX];
        std::pair<int, int> current_from = g_ctx->board->last_move_from;
        std::pair<int, int> current_to = g_ctx->board->last_move_to;


        g_ctx->board->last_move_from = {gridY, gridX};
        g_ctx->board->last_move_to = {gridY, gridX};
    }

        if (!g_ctx->historyView.useLive && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            int gridX = (int)((g_ctx->mousePosition.x - Config::BOARD_OFFSET_X) / Config::TILE_SIZE);
            int gridY = (int)((g_ctx->mousePosition.y - Config::BOARD_OFFSET_Y) / Config::TILE_SIZE);
            if (VanillaLogic::IsValidCoord(gridY, gridX)) {
                VariantSwitcher::Active()->MakeMove(
                    *g_ctx->board, 
                    g_ctx->board->promotion_source.first, 
                    g_ctx->board->promotion_source.second, 
                    g_ctx->board->promotion_square.first, 
                    g_ctx->board->promotion_square.second
                );
            }
        }


        if (g_ctx->btnOverlayRematch->isPressed) { //restart game
            g_ctx->board->Reset();
            g_ctx->historyView.useLive = true;
            g_ctx->historyView.viewingIndex = 0;
            g_ctx->anim.active = false;
            g_ctx->cached_legal_moves.clear();

            g_ctx->isGameRunning = true;                
            g_ctx->btnOverlayRematch->isPressed = false; 
            
            return;
        }

        if (g_ctx->btnFirst->isPressed && !g_ctx->board->move_history.empty()) {
            g_ctx->historyView.useLive = false;
            g_ctx->historyView.viewingIndex = 0;
        }
        if (g_ctx->btnLast->isPressed && !g_ctx->board->move_history.empty()) {
            g_ctx->historyView.useLive = true;
        }
        if (g_ctx->btnPrev->isPressed && !g_ctx->board->move_history.empty()) {
            
            const auto& hist = g_ctx->board->move_history;
    
            if (g_ctx->historyView.useLive) {
                g_ctx->historyView.useLive = false;
                
                if (!hist.empty()) {
                    g_ctx->historyView.viewingIndex = (int)hist.size() - 2; 
                } else {
                    g_ctx->historyView.viewingIndex = -1;
                }
            } else {
                if (g_ctx->historyView.viewingIndex > 0) {
                    g_ctx->historyView.viewingIndex--;
                }
            }
            
            size_t totalPairs = (hist.size() + 1) / 2;
            float itemHeight = (float)Config::ROW_HEIGHT;
            float totalContentHeight = (float)totalPairs * itemHeight;
            float maxScroll = totalContentHeight - (220 - 20);
            
            if (maxScroll > 0) {
                int currentPair = (g_ctx->historyView.viewingIndex) / 2;
                float targetScroll = currentPair * itemHeight;
                if (targetScroll > maxScroll) targetScroll = maxScroll;
                g_ctx->move_log_scroll_ratio = targetScroll / maxScroll;
            }
        }
        if (g_ctx->btnNext->isPressed && !g_ctx->historyView.useLive) {
            if (g_ctx->historyView.viewingIndex < (int)g_ctx->board->move_history.size() - 1) {
                g_ctx->historyView.viewingIndex++;
            } else {
                g_ctx->historyView.useLive = true;
            }
        }
        if (g_ctx->board->state == STATE_PLAYING) {
            if (g_ctx->btnResign->isPressed) {
                g_ctx->board->state = STATE_RESIGNED;
            }
            if (g_ctx->btnDraw->isPressed) {
                g_ctx->board->state = STATE_MUTUAL_DRAW;
            }
        } else if (g_ctx->historyView.useLive) {
            g_ctx->btnOverlayRematch->Update(g_ctx->mousePosition);
            
        }

        int logContainerX = Config::PANEL_X + 25;
        int logContainerY = Config::PANEL_Y + 95;
        int logW = Config::PANEL_WIDTH - 50;
        int logH = 220;
        
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (g_ctx->mousePosition.x >= logContainerX + logW - 25 && g_ctx->mousePosition.x <= logContainerX + logW) {
                if (g_ctx->mousePosition.y >= logContainerY && g_ctx->mousePosition.y <= logContainerY + logH) {
                    float relativeY = g_ctx->mousePosition.y - logContainerY;
                    g_ctx->move_log_scroll_ratio = relativeY / (float)logH;
                    if (g_ctx->move_log_scroll_ratio < 0) g_ctx->move_log_scroll_ratio = 0;
                    if (g_ctx->move_log_scroll_ratio > 1) g_ctx->move_log_scroll_ratio = 1;
                }
            }
        }

        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            g_ctx->move_log_scroll_ratio -= wheel * 0.08f;
            if (g_ctx->move_log_scroll_ratio < 0) g_ctx->move_log_scroll_ratio = 0;
            if (g_ctx->move_log_scroll_ratio > 1) g_ctx->move_log_scroll_ratio = 1;
        }

        if (g_ctx->board->is_promoting) {
            for (size_t i = 0; i < g_ctx->btnPromotionTrays.size(); ++i) {
                g_ctx->btnPromotionTrays[i]->Update(g_ctx->mousePosition);
                if (g_ctx->btnPromotionTrays[i]->isPressed) {
                    PieceType choice = QUEEN;
                    if (i == 1) choice = ROOK;
                    if (i == 2) choice = BISHOP;
                    if (i == 3) choice = KNIGHT;

                    g_ctx->board->is_promoting = false;
                    VanillaLogic::MakeMove(
                        *g_ctx->board, 
                        g_ctx->board->promotion_source.first, 
                        g_ctx->board->promotion_source.second, 
                        g_ctx->board->promotion_square.first, 
                        g_ctx->board->promotion_square.second, 
                        choice
                    );

                    if (g_ctx->active_menu == PUZZLES && !g_ctx->puzzleSuccess && !g_ctx->puzzleFailed) {
                        std::vector<std::string> solutionMoves = SplitMoveString(g_ctx->cachedpuzzle.solution);
                        
                        std::string playerMoveUci = ConvertToUci(g_ctx->board->promotion_source.first, g_ctx->board->promotion_source.second, g_ctx->board->promotion_square.first, g_ctx->board->promotion_square.second);
                        if (choice == QUEEN) playerMoveUci += "q";
                        else if (choice == ROOK) playerMoveUci += "r";
                        else if (choice == BISHOP) playerMoveUci += "b";
                        else if (choice == KNIGHT) playerMoveUci += "n";

                        if (g_ctx->puzzleMoveIndex < (int)solutionMoves.size() && playerMoveUci == solutionMoves[g_ctx->puzzleMoveIndex]) {
                            g_ctx->puzzleMoveIndex++;
                            if (g_ctx->puzzleMoveIndex >= (int)solutionMoves.size()) {
                                g_ctx->puzzleSuccess = true;
                                g_ctx->puzzle_win_count++;
                            } else {
                                std::string opponentMoveUci = solutionMoves[g_ctx->puzzleMoveIndex];
                                int opFromCol = opponentMoveUci[0] - 'a', opFromRow = '8' - opponentMoveUci[1];
                                int opToCol   = opponentMoveUci[2] - 'a', opToRow   = '8' - opponentMoveUci[3];
                                g_ctx->board->grid[opToRow][opToCol] = g_ctx->board->grid[opFromRow][opFromCol];
                                g_ctx->board->grid[opFromRow][opFromCol] = GridData();
                                g_ctx->board->turn = (g_ctx->board->turn == P_WHITE) ? P_BLACK : P_WHITE;
                                g_ctx->puzzleMoveIndex++;
                            }
                        } else {
                            g_ctx->puzzleFailed = true;
                            g_ctx->board->LoadState(g_ctx->savedPuzzleState); 
                        }
                    }

                    return;
                }
            }
            return;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            int gridX = (int)((g_ctx->mousePosition.x - Config::BOARD_OFFSET_X) / Config::TILE_SIZE);
            int gridY = (int)((g_ctx->mousePosition.y - Config::BOARD_OFFSET_Y) / Config::TILE_SIZE);

            if (VariantSwitcher::GetCurrentName() == "duck" && g_ctx->duck_phase) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    Vector2 mousePos = GetMousePosition();
                    int gridX = (int)((mousePos.x - Config::BOARD_OFFSET_X) / Config::TILE_SIZE);
                    int gridY = (int)((mousePos.y - Config::BOARD_OFFSET_Y) / Config::TILE_SIZE);

                    if (VanillaLogic::IsValidCoord(gridY, gridX) && !g_ctx->board->grid[gridY][gridX].has_piece) {
                        if (std::make_pair(gridY, gridX) != g_ctx->duck_pos) {
                            g_ctx->duck_pos = {gridY, gridX};
                            g_ctx->duck_phase = false;
                            g_ctx->board->turn = (g_ctx->board->turn == P_WHITE) ? P_BLACK : P_WHITE;
                            g_ctx->cached_legal_moves.clear();
                        }
                    }
                }
                return; 
            }
            if (VanillaLogic::IsValidCoord(gridY, gridX)) {
                if (!g_ctx->board->has_selection) {
                    const auto& piece = g_ctx->board->grid[gridY][gridX].piece;
                    if (piece && piece->color == g_ctx->board->turn) {
                        g_ctx->board->selected_square = { gridY, gridX };
                        g_ctx->board->has_selection = true;
                        VariantSwitcher::Active()->CacheLegalMoves(*g_ctx->board, gridY, gridX);
                    }
                } else { 
                    int srcR = g_ctx->board->selected_square.first;
                    int srcC = g_ctx->board->selected_square.second;

                    g_ctx->board->has_selection = false;

                    if (srcR == gridY && srcC == gridX) {
                        g_ctx->cached_legal_moves.clear();
                        return; 
                    }

                    if (VariantSwitcher::Active()->IsLegalMove(*g_ctx->board, srcR, srcC, gridY, gridX)) {
                        auto p = g_ctx->board->grid[srcR][srcC].piece;
                        
                        bool isPawnPush = (p && p->type == PAWN);
                        bool reachesEnd = (p && p->color == P_WHITE && gridY == 0) || (p && p->color == P_BLACK && gridY == 7);

                        if (isPawnPush && reachesEnd) {
                            g_ctx->board->is_promoting = true;
                            g_ctx->board->promotion_source = { srcR, srcC };
                            g_ctx->board->promotion_square = { gridY, gridX };
                            return;
                        }

                        g_ctx->anim.active = true;
                        g_ctx->anim.piece = p;
                        g_ctx->anim.startPos = { 
                            (float)Config::BOARD_OFFSET_X + srcC * Config::TILE_SIZE, 
                            (float)Config::BOARD_OFFSET_Y + srcR * Config::TILE_SIZE 
                        };
                        g_ctx->anim.currentPos = g_ctx->anim.startPos;
                        g_ctx->anim.endPos = { 
                            (float)Config::BOARD_OFFSET_X + gridX * Config::TILE_SIZE, 
                            (float)Config::BOARD_OFFSET_Y + gridY * Config::TILE_SIZE 
                        };
                        g_ctx->anim.elapsedTime = 0.0f;
                        g_ctx->anim.targetRow = gridY;
                        g_ctx->anim.targetCol = gridX;

                        if (g_ctx->active_menu == PUZZLES && !g_ctx->puzzleSuccess && !g_ctx->puzzleFailed) {
                            std::vector<std::string> solutionMoves = SplitMoveString(g_ctx->cachedpuzzle.solution);
                            std::string playerMoveUci = ConvertToUci(srcR, srcC, gridY, gridX);
                            g_ctx->puzzle_win_square = Vector2{(float)gridX, (float)gridY};
                            if (g_ctx->puzzleMoveIndex < (int)solutionMoves.size() && playerMoveUci == solutionMoves[g_ctx->puzzleMoveIndex]) {
                                g_ctx->puzzleMoveIndex++;
                                
                                if (g_ctx->puzzleMoveIndex >= (int)solutionMoves.size()) {
                                    g_ctx->puzzleSuccess = true;
                                } else {
                                    g_ctx->puzzleOpponentTimer = 0.5f; 
                                }
                            } else {
                                g_ctx->puzzleFailed = true;
                            }
                        }
                    } else {
                        const auto& nextPiece = g_ctx->board->grid[gridY][gridX].piece;
                        if (nextPiece && nextPiece->color == g_ctx->board->turn) {
                            g_ctx->board->selected_square = { gridY, gridX };
                            g_ctx->board->has_selection = true;
                            
                            VanillaLogic::CacheLegalMoves(*g_ctx->board, gridY, gridX);
                            
                        } else {
                            g_ctx->cached_legal_moves.clear();
                        }
                    }
                }
            } else {
                g_ctx->board->has_selection = false;
                g_ctx->cached_legal_moves.clear();
            }
        }
        SetMouseCursor(targetCursor);
    }
}



void UpdateDrawFrame() { // rendering
    float dt = GetFrameTime();
    BoardState displayState;
    Vector2 rawMousePos = GetMousePosition();
    Vector2 mousePos = {
        rawMousePos.x * ((float)Config::WINDOW_WIDTH / (float)GetScreenWidth()),
        rawMousePos.y * ((float)Config::WINDOW_HEIGHT / (float)GetScreenHeight())
    };
    
    if (!g_puzzlesLoaded) {
        load_puzzles();
        g_puzzlesLoaded = true;
    }
    static Menus last_menu = PLAY;
    do_the_puzzle_stuff(displayState,dt,last_menu);
    if ((g_ctx->active_menu == GAME || g_ctx->active_menu == PUZZLES) && mousePos.x > g_ctx->sidebarWidth) {
        TickEngine::ProcessInput();
    }
    TickEngine::UpdateAnimations(dt);
    BeginTextureMode(g_ctx->targetScreen);
    ClearBackground(Color{ 18, 12, 10, 255 });
    DrawTexturePro(
    g_ctx->backgroundTexture,
    Rectangle{ 0, 0, (float)g_ctx->backgroundTexture.width, (float)g_ctx->backgroundTexture.height },
    Rectangle{ 0, 0, (float)Config::WINDOW_WIDTH, (float)Config::WINDOW_HEIGHT },
    Vector2{ 0, 0 },
    0.0f,
    Color{ 255, 255, 255, 90}
    );
    
    if (!g_ctx->historyView.useLive) {
        int idx = g_ctx->historyView.viewingIndex;
        if (idx >= 0 && idx < (int)g_ctx->board->move_history.size()) {
            displayState = g_ctx->board->move_history[idx].board_state;
        }
    } else {
        if (g_ctx->active_menu == PUZZLES) {
        } else {
            displayState = g_ctx->board->CaptureState();
        }
    }
    switch (g_ctx->active_menu) {
        case PLAY:
            DrawRectangle(0, 0, Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT, Fade(BLACK, 0.6f));
            DrawTextSmooth("TO BE ADDED", 250.0f, 200.0f, 32.0f, RAYWHITE);
            break;
        case SETTINGS:
            DrawRectangle(0, 0, Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT, Fade(BLACK, 0.2f));
            DrawSettingsMenu(mousePos);
            break;
        case PUZZLES:
            VariantSwitcher::SetVariant("vanilla oooga booga raaaa");
            CanvasRenderer::draw_right_side("Puzzle Stats");
            CanvasRenderer::DrawPuzzleSideBar();
            CanvasRenderer::DrawChessboard(displayState);

            if (g_ctx->puzzleSuccess.is_new_true()) {
                g_ctx->puzzle_win_count++;
                g_ctx->puzzle_streak++;
                g_ctx->streak_anim_timer = 0.0f;
                PlaySound(g_ctx->winsound);
            }

            if (g_ctx->puzzleSuccess) {
                DrawCircle(g_ctx->puzzle_win_square.x * Config::TILE_SIZE + Config::BOARD_OFFSET_X + Config::TILE_SIZE * .85,
                        g_ctx->puzzle_win_square.y * Config::TILE_SIZE + Config::BOARD_OFFSET_Y + Config::TILE_SIZE * .15, 
                        15, GREEN);
            }
            else if (g_ctx->puzzleFailed) {
                DrawRectangle(0, 0, Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT, Fade(RED, 0.15f));
                DrawTextSmooth("WRONG MOVE. TRY AGAIN!", 300.0f, 400.0f, 40.0f, RED);
            }
            g_ctx->puzzleSuccess.update();
            break;
        case OPENINGS:
            DrawRectangle(0, 0, Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT, Fade(BLACK, 0.6f));
            DrawTextSmooth("TO BE ADDED", 250.0f, 200.0f, 32.0f, RAYWHITE);
            break;
        case GAME:
            VariantSwitcher::SetVariant("duck");
            
            CanvasRenderer::DrawGameMetrics();
            CanvasRenderer::DrawChessboard(displayState);
            if (VariantSwitcher::GetCurrentName() == "duck" && g_ctx->duck_pos.first != -1) {
                float drawX = Config::BOARD_OFFSET_X + (g_ctx->duck_pos.second * Config::TILE_SIZE);
                float drawY = Config::BOARD_OFFSET_Y + (g_ctx->duck_pos.first * Config::TILE_SIZE);
                
                float radius = Config::TILE_SIZE * 0.35f;
                float centerX = drawX + Config::TILE_SIZE / 2.0f;
                float centerY = drawY + Config::TILE_SIZE / 2.0f;
                DrawCircle((int)centerX, (int)centerY, radius, YELLOW);
                DrawCircleLines((int)centerX, (int)centerY, radius, ORANGE);
            }
            VariantSwitcher::Active()->DrawExtra(*g_ctx->board);
            break;
        default:
            break;
    }
    DrawCollapsibleSidebar(mousePos); 
    DrawTextSmooth(TextFormat("%d", GetFPS()), 25.0f, 20.0f, 24.0f, Config::COLOR_LEAF_LIGHT);
    EndTextureMode();
    BeginDrawing();
        ClearBackground(BLACK);
        Rectangle srcRec = { 0, 0, (float)g_ctx->targetScreen.texture.width, -(float)g_ctx->targetScreen.texture.height };
        Rectangle destRec = { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() };
        DrawTexturePro(g_ctx->targetScreen.texture, srcRec, destRec, Vector2{ 0, 0 }, 0.0f, WHITE);
    EndDrawing();

    if (WindowShouldClose()) {
        g_ctx->isGameRunning = false;
    }
}

int main(int argc, char* argv[]) {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT, "chess");
    InitAudioDevice();
    g_ctx = std::make_unique<GameContext>();
    g_ctx->ResetPromotionButtons();
    if (IsAudioDeviceReady()) {
        g_ctx->hoversound = LoadSound(hoversoundfilepath);
        g_ctx->winsound = LoadSound(winsoundfilepath);
        g_ctx->movesound = LoadSound(movesoundfilepath);
        audio_loaded= true;
        TraceLog(LOG_INFO, "AUDIO: ZA BLUETOOTH DEWICE HAS BEEN CONNECTED");
    }
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    while (g_ctx->isGameRunning) {
        UpdateDrawFrame();
    }
    //unload sounds but idk how many i will have so if you run this on pc youre cooked
    CloseAudioDevice(); 
    CloseWindow();
#endif
    return 0;
}