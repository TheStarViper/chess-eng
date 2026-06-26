#include "raylib.h"
#include "main.hpp"
#include "leaves.hpp"
#include "easing_functions.hpp"
#include "vec_renderer.hpp"
#include "puzzles.hpp"
#include "gamelogic.hpp"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#define P_WHITE "WHITE"
#define P_BLACK "BLACK"

 


void DrawTextSmooth(const char* text, float posX, float posY, float fontSize, Color color);
Vector2 MeasureTextSmooth(const char* text, float fontSize);

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


void DrawChessPiece(PieceType type, std::string color, int x, int y, int size) {
    if (g_ctx && g_ctx->useSprites) {
        std::string pieceName;
        switch (type) {
            case PAWN:   pieceName = "Pawn";   break;
            case KNIGHT: pieceName = "Knight"; break;
            case BISHOP: pieceName = "Bishop"; break;
            case ROOK:   pieceName = "Rook";   break;
            case QUEEN:  pieceName = "Queen";  break;
            case KING:   pieceName = "King";   break;
            case NONE:   return; 
        }
        std::string key = (color == P_WHITE ? "White" : "Black") + pieceName;
        
        if (g_ctx->pieceSprites.count(key) && g_ctx->pieceSprites[key].id > 0) {
            Texture2D tex = g_ctx->pieceSprites[key];
            Rectangle dest = { (float)x + 2, (float)y + 2, (float)size - 4, (float)size - 4 };
            DrawTexturePro(tex, {0, 0, (float)tex.width, (float)tex.height}, dest, {0,0}, 0.0f, WHITE);
            return;
        }
    }

    VectorRenderer::DrawChessPieceVector(type, color, x, y, size); //fallback to vector bad looking if sprites fail to load
}

namespace CanvasRenderer {

    void DrawCapturedTrays(BoardState& displayState) {
    auto getVal = [](PieceType t) {
        if (t == PAWN) return 1;
        if (t == KNIGHT || t == BISHOP) return 3;
        if (t == ROOK) return 5;
        if (t == QUEEN) return 9;
        return 0;
    };

    auto getStartCount = [](PieceType t) {
        if (t == PAWN) return 8;
        if (t == KNIGHT || t == BISHOP) return 2;
        if (t == ROOK) return 2;
        if (t == QUEEN) return 1;
        return 0;
    };

    int currentWhiteVal = 0, currentBlackVal = 0;
    std::map<PieceType, int> activeWhite, activeBlack;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (displayState.grid[r][c].has_piece) {
                auto p = displayState.grid[r][c].piece;
                if (p->color == P_WHITE) {
                    activeWhite[p->type]++;
                    currentWhiteVal += getVal(p->type);
                } else {
                    activeBlack[p->type]++;
                    currentBlackVal += getVal(p->type);
                }
            }
        }
    }

    const int START_VAL = 39;
    int capturedWhiteVal = START_VAL - currentWhiteVal;
    int capturedBlackVal = START_VAL - currentBlackVal;

    int topY = Config::BOARD_OFFSET_Y - 55;
    int bottomY = Config::BOARD_OFFSET_Y + (Config::TILE_SIZE * 8) + 15;
    int capSize = 35;
    int startX = Config::BOARD_OFFSET_X + 110;

    int curX = startX;
    for (int t = PAWN; t <= QUEEN; t++) {
        int capturedCount = getStartCount((PieceType)t) - activeWhite[(PieceType)t];
        for (int i = 0; i < capturedCount; i++) {
            DrawChessPiece((PieceType)t, P_WHITE, curX, topY, capSize);
            curX += 28;
        }
    }
    if (capturedWhiteVal > capturedBlackVal) {
        DrawTextSmooth(TextFormat("+%d", capturedWhiteVal - capturedBlackVal), (float)curX + 10, (float)topY + 10, 16.0f, Config::COLOR_LEAF_LIGHT);
    }

    curX = startX;
    for (int t = PAWN; t <= QUEEN; t++) {
        int capturedCount = getStartCount((PieceType)t) - activeBlack[(PieceType)t];
        for (int i = 0; i < capturedCount; i++) {
            DrawChessPiece((PieceType)t, P_BLACK, curX, bottomY, capSize);
            curX += 28;
        }
    }
    if (capturedBlackVal > capturedWhiteVal) {
        DrawTextSmooth(TextFormat("+%d", capturedBlackVal - capturedWhiteVal), (float)curX + 10, (float)bottomY + 10, 16.0f, Config::COLOR_LEAF_LIGHT);
    }
}   


    void DrawMoveLog() {
        int logContainerX = Config::PANEL_X + 25;
        int logContainerY = Config::PANEL_Y + 95;
        int logW = Config::PANEL_WIDTH - 50;
        int logH = 360;

        DrawRectangleRounded(Rectangle{ (float)logContainerX, (float)logContainerY, (float)logW, (float)logH }, 0.08f, 4, Color{ 24, 14, 8, 255 });
        DrawRectangleRoundedLinesCustom(Rectangle{ (float)logContainerX, (float)logContainerY, (float)logW, (float)logH }, 0.08f, 4, 1.5f, Config::COLOR_FRAME_DARK);

        BeginScissorMode(logContainerX, logContainerY + 10, logW, logH - 20);

        const auto& hist = g_ctx->board->move_history;
        size_t totalMoves = hist.size();
        size_t totalPairs = (totalMoves + 1) / 2;

        float itemHeight = (float)Config::ROW_HEIGHT;
        float totalContentHeight = (float)totalPairs * itemHeight;

        float maxScroll = totalContentHeight - (logH - 20);
        if (maxScroll < 0) maxScroll = 0;

        static size_t lastTotalMoves = 0;
        if (totalMoves > lastTotalMoves) {
            if (g_ctx->historyView.useLive && maxScroll > 0) {
                g_ctx->move_log_scroll_ratio = 1.0f;
            }
            lastTotalMoves = totalMoves;
        }

        float scrollOffset = g_ctx->move_log_scroll_ratio * maxScroll;
        
        Vector2 mousePos = GetMousePosition();
        bool anyMoveHovered = false;

        Rectangle visibleScissorRect = { (float)logContainerX, (float)logContainerY + 10, (float)logW, (float)logH - 20 };

        for (size_t i = 0; i < totalPairs; ++i) {
            float yPos = logContainerY + 15 + (i * itemHeight) - scrollOffset;

            Color rowBg = (i % 2 == 0) ? Config::COLOR_UI_ROW_A : Config::COLOR_UI_ROW_B;
            DrawRectangle(logContainerX + 10, (int)yPos, logW - 35, (int)itemHeight - 3, rowBg);

            std::string stepStr = std::to_string(i + 1) + ".";
            DrawTextSmooth(stepStr.c_str(), (float)logContainerX + 25, yPos + 8.0f, 16.0f, Config::COLOR_UI_TEXT_DIM);

            size_t wIndex = i * 2;
            std::string whiteMove = hist[wIndex].notation;
            
            Rectangle whiteClickRect = { (float)logContainerX + 85, yPos + 3, 80, itemHeight - 9 };
            bool isWhiteHovered = CheckCollisionPointRec(mousePos, whiteClickRect) && CheckCollisionPointRec(mousePos, visibleScissorRect);

            if (isWhiteHovered) {
                anyMoveHovered = true;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    g_ctx->historyView.useLive = false;
                    g_ctx->historyView.viewingIndex = (int)wIndex;
                }
            }

            bool isWhiteHighlighted = (!g_ctx->historyView.useLive && g_ctx->historyView.viewingIndex == (int)wIndex) ||
                                      (g_ctx->historyView.useLive && wIndex == totalMoves - 1);
            
            if (isWhiteHighlighted || isWhiteHovered) {
                DrawRectangleLines(logContainerX + 85, (int)yPos + 3, 80, (int)itemHeight - 9, Config::COLOR_LEAF_VEIN);
            }
            DrawTextSmooth(whiteMove.c_str(), (float)logContainerX + 95, yPos + 8.0f, 16.0f, isWhiteHighlighted ? Config::COLOR_LEAF_VEIN : Config::COLOR_UI_TEXT);

            size_t bIndex = wIndex + 1;
            if (bIndex < totalMoves) {
                std::string blackMove = hist[bIndex].notation;
                
                Rectangle blackClickRect = { (float)logContainerX + 195, yPos + 3, 80, itemHeight - 9 };
                bool isBlackHovered = CheckCollisionPointRec(mousePos, blackClickRect) && CheckCollisionPointRec(mousePos, visibleScissorRect);

                if (isBlackHovered) {
                    anyMoveHovered = true;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        g_ctx->historyView.useLive = false;
                        g_ctx->historyView.viewingIndex = (int)bIndex;
                    }
                }

                bool isBlackHighlighted = (!g_ctx->historyView.useLive && g_ctx->historyView.viewingIndex == (int)bIndex) ||
                                          (g_ctx->historyView.useLive && bIndex == totalMoves - 1);
                
                if (isBlackHighlighted || isBlackHovered) {
                    DrawRectangleLines(logContainerX + 195, (int)yPos + 3, 80, (int)itemHeight - 9, Config::COLOR_LEAF_VEIN);
                }
                DrawTextSmooth(blackMove.c_str(), (float)logContainerX + 205, yPos + 8.0f, 16.0f, isBlackHighlighted ? Config::COLOR_LEAF_VEIN : Config::COLOR_UI_TEXT);
            }
        }
        
        EndScissorMode();

        float scrollbarX = (float)logContainerX + logW - 15;
        float scrollbarY = (float)logContainerY + 15;
        float scrollbarH = (float)logH - 30;
        
        DrawRectangle((int)scrollbarX, (int)scrollbarY, 6, (int)scrollbarH, Color{ 36, 22, 14, 255 });
        
        float handleH = (scrollbarH / (totalContentHeight > 0 ? totalContentHeight : 1.0f)) * scrollbarH;
        if (handleH > scrollbarH) handleH = scrollbarH;
        if (handleH < 30) handleH = 30;

        float handleY = scrollbarY + (g_ctx->move_log_scroll_ratio * (scrollbarH - handleH));
        DrawRectangleRounded(Rectangle{ scrollbarX - 1, handleY, 8, handleH }, 0.5f, 4, Config::COLOR_GEM_GLINT);
    }

    void draw_right_side(std::string title){
        DrawRectangleRounded(Rectangle{ (float)Config::PANEL_X, (float)Config::PANEL_Y, (float)Config::PANEL_WIDTH, (float)Config::PANEL_HEIGHT }, 0.03f, 4, Config::COLOR_UI_PANEL_BG);
        DrawRectangleRoundedLinesCustom(Rectangle{ (float)Config::PANEL_X, (float)Config::PANEL_Y, (float)Config::PANEL_WIDTH, (float)Config::PANEL_HEIGHT }, 0.03f, 4, 3.0f, Config::COLOR_UI_BORDER);
        DrawRectangleRoundedLinesCustom(Rectangle{ (float)Config::PANEL_X + 6, (float)Config::PANEL_Y + 6, (float)Config::PANEL_WIDTH - 12, (float)Config::PANEL_HEIGHT - 12 }, 0.03f, 4, 1.0f, Config::COLOR_FRAME_DARK);
        DrawTextSmooth(title.c_str(), (float)Config::PANEL_X + 25, (float)Config::PANEL_Y + 25, 36.0f, Config::COLOR_UI_TEXT);
        VectorRenderer::DrawOvergrownVines(Config::BOARD_OFFSET_X, Config::BOARD_OFFSET_Y, Config::TILE_SIZE*8 ,2);
    }

    void DrawGameMetrics() {
        draw_right_side("Game Log");
        std::string statusText;
        Color statusColor = Config::COLOR_UI_TEXT;

        if (g_ctx->board->state == STATE_PLAYING) {
            statusText = (g_ctx->board->turn == P_WHITE) ? "WHITE TO PLAY" : "BLACK TO PLAY";
            statusColor = (g_ctx->board->turn == P_WHITE) ? Config::COLOR_UI_TEXT : Config::COLOR_UI_TEXT_DIM;
        } else {
            statusColor = Config::COLOR_CHECK;
            switch (g_ctx->board->state) {
                case STATE_CHECKMATE:
                    statusText = (g_ctx->board->turn == P_WHITE) ? "MATE! BLACK WINS" : "MATE! WHITE WINS";
                    break;
                case STATE_STALEMATE:
                    statusText = "DRAW (STALEMATE)";
                    break;
                case STATE_DRAW_REPETITION:
                    statusText = "DRAW (THREEFOLD)";
                    break;
                case STATE_DRAW_50_MOVES:
                    statusText = "DRAW (50-MOVES)";
                    break;
                case STATE_DRAW_MATERIAL:
                    statusText = "DRAW (DEAD MATERIAL)";
                    break;
                case STATE_RESIGNED:
                    statusText = (g_ctx->board->turn == P_WHITE) ? "WHITE RESIGNED" : "BLACK RESIGNED";
                    break;
                case STATE_MUTUAL_DRAW:
                    statusText = "MUTUAL AGREED DRAW";
                    break;
                default:
                    statusText = "GAME OVER";
                    break;
            }
        }

        Vector2 statusSize = MeasureTextSmooth(statusText.c_str(), 16.0f);
        DrawTextSmooth(statusText.c_str(), (float)Config::PANEL_X + Config::PANEL_WIDTH - statusSize.x - 25, (float)Config::PANEL_Y + 40, 16.0f, statusColor);

        DrawMoveLog();

        int repCount;
        if (g_ctx->historyView.useLive) {
            repCount = g_ctx->board->current_repetition_count;
        } else {
            repCount = g_ctx->board->move_history[g_ctx->historyView.viewingIndex].board_state.repetition_count;
        }
        float warning_info_y = Config::PANEL_Y + Config::PANEL_HEIGHT-130;
        if (!g_ctx->historyView.useLive) {
            DrawRectangleRounded(Rectangle{ (float)Config::PANEL_X + 25, warning_info_y, (float)Config::PANEL_WIDTH - 50, 22 }, 0.1f, 4, Config::COLOR_GEM_GLINT);
            std::string viewStr = "HISTORICAL INSPECTION: INDEX #" + std::to_string(g_ctx->historyView.viewingIndex + 1);
            Vector2 vSize = MeasureTextSmooth(viewStr.c_str(), 14.0f);
            DrawTextSmooth(viewStr.c_str(), (float)Config::PANEL_X + 25 + (Config::PANEL_WIDTH - 50 - vSize.x)/2, warning_info_y+4, 14.0f, Config::COLOR_FRAME_DARK);
        } else {
            if (g_ctx->fiftymovecounter){
            char stats[128];
            snprintf(stats, sizeof(stats), "50 Move Rule: %d / 50", (int)std::round(g_ctx->board->halfmove_clock/2.1)); //2.1 to avoid rounding up .5
            DrawTextSmooth(stats, (float)Config::PANEL_X + 30, warning_info_y, 14.0f, Config::COLOR_UI_TEXT_DIM);
            }
            if (g_ctx->threefoldcounter){
            if (repCount == 2) {
                DrawTextSmooth("[!] 3-Fold Warning (2x Same)", (float)Config::PANEL_X + Config::PANEL_WIDTH-200, warning_info_y, 14.0f, Config::COLOR_GEM_GLINT);
            }}
        }

        g_ctx->btnFirst->Draw();
        g_ctx->btnPrev->Draw();
        g_ctx->btnNext->Draw();
        g_ctx->btnLast->Draw();

        g_ctx->btnResign->Draw();
        g_ctx->btnDraw->Draw();

    }
    void draw_board_markings(){
        if (g_ctx->boardmarkings){
            for (int i = 0; i < 8; ++i) {
                char fileStr[2] = { (char)('a' + i), '\0' };
                char rankStr[2] = { (char)('8' - i), '\0' };

                DrawTextSmooth(fileStr, (float)Config::BOARD_OFFSET_X + i * Config::TILE_SIZE + Config::TILE_SIZE-10, (float)Config::BOARD_OFFSET_Y + 8 * Config::TILE_SIZE - 17, 15.0f, Config::BOARD_MARKINGS_TEXT);
                DrawTextSmooth(rankStr, (float)Config::BOARD_OFFSET_X+3, (float)Config::BOARD_OFFSET_Y + i * Config::TILE_SIZE + 3, 15.0f,Config::BOARD_MARKINGS_TEXT);
            }
        }
    }
    void draw_board_tiles(BoardState& displayState, std::pair<int, int> checkKingSquare){
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                int drawX = Config::BOARD_OFFSET_X + c * Config::TILE_SIZE;
                int drawY = Config::BOARD_OFFSET_Y + r * Config::TILE_SIZE;

                bool isDark = ((r + c) % 2 != 0);
                Color sqColor = isDark ? Config::COLOR_DARK_SQ : Config::COLOR_LIGHT_SQ;
                DrawRectangle(drawX, drawY, Config::TILE_SIZE, Config::TILE_SIZE, sqColor);
                VectorRenderer::DrawTileWoodGrain((float)drawX, (float)drawY, (float)Config::TILE_SIZE, (float)Config::TILE_SIZE, isDark);

                if ((r == displayState.last_move_from.first && c == displayState.last_move_from.second) ||
                    (r == displayState.last_move_to.first && c == displayState.last_move_to.second)) {
                    DrawRectangle(drawX, drawY, Config::TILE_SIZE, Config::TILE_SIZE, Config::COLOR_LAST_MOVE);
                }

                if (r == checkKingSquare.first && c == checkKingSquare.second) {
                    DrawRectangle(drawX, drawY, Config::TILE_SIZE, Config::TILE_SIZE, Config::COLOR_CHECK);
                }

                if (g_ctx->board->has_selection && g_ctx->historyView.useLive) {
                    if (g_ctx->board->selected_square.first == r && g_ctx->board->selected_square.second == c) {
                        DrawRectangle(drawX, drawY, Config::TILE_SIZE, Config::TILE_SIZE, Config::COLOR_HIGHLIGHT);
                    }
                }
            }
        }
    }

    void DrawGlowTargetRing(float x, float y, float radius, float thickness, Color baseColor, bool isHovered) {
        float innerRadius = radius - (thickness / 2.0f);
        float outerRadius = radius + (thickness / 2.0f);
        DrawRing({ x, y }, innerRadius, outerRadius, 0, 360, 48, baseColor);

        if (isHovered) {
            BeginBlendMode(BLEND_ADDITIVE);

            Color glowColor = { 235, 87, 43, 255 };
            int glowLayers = 12;

            for (int i = 1; i <= glowLayers; i++) {
                float alpha = 0.08f * (1.0f - ((float)i / (float)glowLayers));
                Color fadedGlow = Fade(glowColor, alpha);

                float outerGlow = outerRadius + ((float)i * 1.5f);
                DrawRing({ x, y }, outerRadius, outerGlow, 0, 360, 48, fadedGlow);
                float innerGlow = innerRadius - ((float)i * 1.5f);
                if (innerGlow > 0) {
                    DrawRing({ x, y }, innerGlow, innerRadius, 0, 360, 48, fadedGlow);
                }
            }

            EndBlendMode();
        }
    }

    void draw_legal_moves(){
        if (g_ctx->board->has_selection && g_ctx->historyView.useLive) {
            Vector2 mousePos = GetMousePosition(); 

            for (const auto& move : g_ctx->cached_legal_moves) {
                int r = move.first;
                int c = move.second;
                int drawX = Config::BOARD_OFFSET_X + c * Config::TILE_SIZE + Config::TILE_SIZE / 2;
                int drawY = Config::BOARD_OFFSET_Y + r * Config::TILE_SIZE + Config::TILE_SIZE / 2;

                Rectangle tileRect = {
                    (float)(Config::BOARD_OFFSET_X + c * Config::TILE_SIZE),
                    (float)(Config::BOARD_OFFSET_Y + r * Config::TILE_SIZE),
                    (float)Config::TILE_SIZE,
                    (float)Config::TILE_SIZE
                };

                auto& tile = g_ctx->board->grid[r][c];

                tile.isHovered.set(CheckCollisionPointRec(mousePos, tileRect));

                float animSpeed = 12.0f;
                if (tile.isHovered) {
                    tile.hoverProgress += GetFrameTime() * animSpeed;
                    if (tile.hoverProgress > 1.0f) tile.hoverProgress = 1.0f;
                } else {
                    tile.hoverProgress -= GetFrameTime() * animSpeed;
                    if (tile.hoverProgress < 0.0f) tile.hoverProgress = 0.0f;
                }

                float easedT = Easings::EaseInOutCubic(tile.hoverProgress);
                
                if (tile.isHovered.is_new_true()) {
                    playsoundsmart(g_ctx->hoversound,.5,1.2);
                }

                if (tile.has_piece) {
                    float radius = (float)Config::TILE_SIZE * 0.38f;
                    float bigradius = (float)Config::TILE_SIZE * 0.38f+1.5f;
                    float thickness = (float)Config::TILE_SIZE * 0.08f; 
                    
                    float radiuss = radius + (bigradius - radius) * easedT;
                    DrawGlowTargetRing(drawX, drawY, radiuss, thickness, Config::COLOR_DOT_RING, tile.isHovered);
                } else {
                    float minRadius = (float)Config::TILE_SIZE * 0.12f;
                    float maxRadius = (float)Config::TILE_SIZE * 0.17f;
                    
                    float currentRadius = minRadius + (maxRadius - minRadius) * easedT;
                    DrawCircle(drawX, drawY, currentRadius, Config::COLOR_DOT);
                }
                tile.isHovered.update();
            }
        }
    }

    void draw_static_pieces(BoardState& displayState){
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                const auto& cell = displayState.grid[r][c];
                if (cell.has_piece) {
                    
                    if (g_ctx->anim.active) {
                        int animStartCol = (int)((g_ctx->anim.startPos.x - Config::BOARD_OFFSET_X) / Config::TILE_SIZE);
                        int animStartRow = (int)((g_ctx->anim.startPos.y - Config::BOARD_OFFSET_Y) / Config::TILE_SIZE);

                        if (animStartRow == r && animStartCol == c) {
                            continue;
                        }
                    }

                    int drawX = Config::BOARD_OFFSET_X + c * Config::TILE_SIZE;
                    int drawY = Config::BOARD_OFFSET_Y + r * Config::TILE_SIZE;
                    DrawChessPiece(cell.piece->type, cell.piece->color, drawX, drawY, Config::TILE_SIZE);
                }
            }
        }
    }

    void draw_promotion_panel(){
        if (g_ctx->board->is_promoting) {
            DrawRectangle(0, 0, Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT, Color{ 10, 5, 2, 180 });
            
            float trayW = 420;
            float trayH = 160;
            float trayX = (float)Config::BOARD_OFFSET_X + 4 * Config::TILE_SIZE - trayW / 2.0f;
            float trayY = (float)Config::BOARD_OFFSET_Y + 4 * Config::TILE_SIZE - trayH / 2.0f;

            DrawRectangleRounded(Rectangle{ trayX, trayY, trayW, trayH }, 0.12f, 4, Config::COLOR_UI_PANEL_BG);
            DrawRectangleRoundedLinesCustom(Rectangle{ trayX, trayY, trayW, trayH }, 0.12f, 4, 3.0f, Config::COLOR_FRAME_MID);

            DrawTextSmooth("PROMOTION PIECE SELECTOR", trayX + 50, trayY + 25, 18.0f, Config::COLOR_UI_TEXT);

            for (auto& btn : g_ctx->btnPromotionTrays) {
                btn->Draw();
            }
        }
    }

    void draw_game_over(){
        if (g_ctx->board->state != STATE_PLAYING && g_ctx->historyView.useLive) {
            DrawRectangle(Config::BOARD_OFFSET_X, Config::BOARD_OFFSET_Y, Config::TILE_SIZE * 8, Config::TILE_SIZE * 8, Color{ 20, 10, 5, 160 });
            
            std::string endTitle;
            if (g_ctx->board->state == STATE_CHECKMATE) endTitle = "CHECKMATE";
            else if (g_ctx->board->state == STATE_RESIGNED) endTitle = "RESIGNATION";
            else endTitle = "DRAW";

            float centerX = (float)Config::BOARD_OFFSET_X + (Config::TILE_SIZE * 8) / 2.0f;
            float centerY = (float)Config::BOARD_OFFSET_Y + (Config::TILE_SIZE * 8) / 2.0f - 40.0f;

            Vector2 tSize = MeasureTextSmooth(endTitle.c_str(), 50.0f);
            DrawTextSmooth(endTitle.c_str(), centerX - tSize.x / 2.0f, centerY - 25.0f, 50.0f, Config::COLOR_UI_TEXT);
            
            g_ctx->btnOverlayRematch->Draw();
        }
    }
    
    void DrawChessboard(BoardState displayState) {
        if (g_ctx->active_menu==GAME){
            DrawCapturedTrays(displayState);
        }

        

        std::pair<int, int> checkKingSquare = displayState.check_king_pos;
        int boardSize = Config::TILE_SIZE * 8;
        VectorRenderer::DrawBoardOrnateFrame(Config::BOARD_OFFSET_X, Config::BOARD_OFFSET_Y, boardSize);
        draw_board_tiles(displayState,checkKingSquare);
        VectorRenderer::DrawOvergrownVines(Config::BOARD_OFFSET_X, Config::BOARD_OFFSET_Y, boardSize,1);
        draw_legal_moves();
        draw_static_pieces(displayState);
        

        if (g_ctx->anim.active) {
            DrawChessPiece(
                g_ctx->anim.piece->type, 
                g_ctx->anim.piece->color, 
                (int)g_ctx->anim.currentPos.x, 
                (int)g_ctx->anim.currentPos.y, 
                Config::TILE_SIZE
            );
        }
        draw_board_markings();
        draw_promotion_panel();
        draw_game_over();
    }

    

    void draw_puzzle_streak_dots() {
        int streakX = Config::PANEL_X + 40;
        int streakY = Config::PANEL_Y + 85;
        int dotRadius = 12;
        int dotSpacing = 30;
        int amount_of_dots_width = 10;

        if (g_ctx->streak_anim_timer < 1.0f) {
            g_ctx->streak_anim_timer += GetFrameTime() / 0.4f; 
            if (g_ctx->streak_anim_timer > 1.0f) g_ctx->streak_anim_timer = 1.0f;
        }
        
        float scaleFactor = Easings::EaseOutBack(g_ctx->streak_anim_timer);

        int goldCount = g_ctx->puzzle_streak / amount_of_dots_width; 
        int leafCount = g_ctx->puzzle_streak % amount_of_dots_width;

        for (int i = 0; i < amount_of_dots_width; i++) {
            int cx = streakX + (i * dotSpacing);
            int cy = streakY; 
            
            if (i < goldCount) {
                if (i == goldCount - 1 && leafCount == 0) {
                    DrawCircle(cx, cy, (float)dotRadius * scaleFactor, Config::COLOR_STREAK_GOLD);
                } else {
                    DrawCircle(cx, cy, dotRadius, Config::COLOR_STREAK_GOLD);
                }
            } else {
                DrawCircleLines(cx, cy, dotRadius, DARKGRAY); 
            }
        }

        for (int i = 0; i < amount_of_dots_width; i++) {
            int cx = streakX + (i * dotSpacing);
            int cy = streakY + 30;
            
            if (i < leafCount) {
                if (i == leafCount - 1) {
                    DrawCircle(cx, cy, (float)dotRadius * scaleFactor, Config::COLOR_LEAF_VEIN);
                } else {
                    DrawCircle(cx, cy, dotRadius, Config::COLOR_LEAF_VEIN);
                }
            } else {
                DrawCircleLines(cx, cy, dotRadius, DARKGRAY);
            }
        }
    }

    void DrawPuzzleSideBar(){
        DrawRectangleRounded(Rectangle{ (float)Config::PANEL_X+25, (float)Config::PANEL_Y+425, (float)Config::PANEL_WIDTH-50, (float)60 }, 0.08f, 4, Color{ 24, 14, 8, 255 });
        DrawRectangleRoundedLinesCustom(Rectangle{ (float)Config::PANEL_X+25, (float)Config::PANEL_Y+425, (float)Config::PANEL_WIDTH-50, (float)60 }, 0.08f, 4, 1.5f, Config::COLOR_FRAME_DARK);
        draw_puzzle_streak_dots();
        g_ctx->btnpuzzlehint->Draw();
        g_ctx->btnpuzzleretry->Draw();
        g_ctx->btnpuzzlenext->Draw();
        if (g_ctx->active_menu==PUZZLES){
            g_ctx->btnpuzzlehint->Update(g_ctx->mousePosition);
            g_ctx->btnpuzzleretry->Update(g_ctx->mousePosition);
            g_ctx->btnpuzzlenext->Update(g_ctx->mousePosition);
            if (g_ctx->btnpuzzlehint->isPressed&&!g_ctx->puzzleSuccess){
                std::vector<std::string> solutionMoves = SplitMoveString(g_ctx->cachedpuzzle.solution);
                if (g_ctx->puzzleMoveIndex < (int)solutionMoves.size()) {
                    g_ctx->hintActive = true;
                    g_ctx->currentHintUci = solutionMoves[g_ctx->puzzleMoveIndex];
                    
                    int hintFromCol = g_ctx->currentHintUci[0] - 'a';
                    int hintFromRow = '8' - g_ctx->currentHintUci[1];
                    
                    g_ctx->board->selected_square = { hintFromRow, hintFromCol };
                    g_ctx->board->has_selection = true;
                    ChessEngine::CacheLegalMoves(*g_ctx->board, hintFromRow, hintFromCol);
                }
            }
            if (g_ctx->btnpuzzleretry->isPressed&&!g_ctx->puzzleSuccess){
                g_ctx->board->LoadState(g_ctx->savedPuzzleState);
                g_ctx->board->move_history.clear(); 
                g_ctx->board->current_repetition_count = 1;
                
                g_ctx->puzzleMoveIndex = 0;
                g_ctx->hintActive = false;
                g_ctx->puzzleFailed = false;
                g_ctx->puzzleSuccess = false;
                
                g_ctx->board->turn = g_ctx->savedPuzzleState.turn;
                g_ctx->active_turn_id = (g_ctx->board->turn == P_WHITE) ? 0 : 1;
                g_ctx->puzzleOpponentTimer = 0.4f; 
            }
            if (g_ctx->btnpuzzlenext->isPressed){
                if (g_ctx->active_menu == PUZZLES && g_ctx->anim.active && g_ctx->anim.piece != nullptr) {
                    if (g_ctx->anim.targetRow >= 0 && g_ctx->anim.targetRow < 8 &&
                        g_ctx->anim.targetCol >= 0 && g_ctx->anim.targetCol < 8) {
                        
                        g_ctx->board->grid[g_ctx->anim.targetRow][g_ctx->anim.targetCol] = GridData(g_ctx->anim.piece);
                    }
                }

                g_ctx->anim.active = false;
                g_ctx->anim.piece = nullptr; 
                g_ctx->anim.targetRow = -1;
                g_ctx->anim.targetCol = -1;

                load_new_puzzle = true; 
                g_ctx->hintActive = false;
            }
        }

        int panelX = Config::PANEL_X + 25;
        int panelY = Config::PANEL_Y + 330; 
        int btnWidth = (Config::PANEL_WIDTH - 70) / 2; 
        int btnHeight = 40;

        int totalGames = g_ctx->puzzle_win_count + g_ctx->puzzle_fail_count;

        int winRatePercent = 0;
        if (totalGames > 0) {
            winRatePercent = std::round(100.0 * g_ctx->puzzle_win_count / totalGames);
        }

        std::string winratetext = "Solve Rate: " + std::to_string(winRatePercent) + "%";
        DrawText(winratetext.c_str(), panelX+85, Config::PANEL_Y+425+23, 20, WHITE);
    }
}


namespace TickEngine {

    void UpdateAnimations(float dt) {
        if (!g_ctx->anim.active) return;

        g_ctx->anim.elapsedTime += dt;
        float progress = g_ctx->anim.elapsedTime / Config::ANIMATION_DURATION;

        if (progress >= 1.0f) {
            g_ctx->anim.active = false;
            
            ChessEngine::MakeMove(
                *g_ctx->board, 
                (int)((g_ctx->anim.startPos.y - Config::BOARD_OFFSET_Y) / Config::TILE_SIZE),
                (int)((g_ctx->anim.startPos.x - Config::BOARD_OFFSET_X) / Config::TILE_SIZE),
                g_ctx->anim.targetRow, 
                g_ctx->anim.targetCol
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

            if (ChessEngine::IsValidCoord(gridY, gridX)) {
                g_ctx->historyView.useLive = true;
                g_ctx->board->has_selection = false;
                g_ctx->cached_legal_moves.clear();
                return; 
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
                    ChessEngine::MakeMove(
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

            if (ChessEngine::IsValidCoord(gridY, gridX)) {
                if (!g_ctx->board->has_selection) {
                    const auto& piece = g_ctx->board->grid[gridY][gridX].piece;
                    if (piece && piece->color == g_ctx->board->turn) {
                        g_ctx->board->selected_square = { gridY, gridX };
                        g_ctx->board->has_selection = true;
                        ChessEngine::CacheLegalMoves(*g_ctx->board, gridY, gridX);
                    }
                } else { 
                    int srcR = g_ctx->board->selected_square.first;
                    int srcC = g_ctx->board->selected_square.second;

                    g_ctx->board->has_selection = false;

                    if (srcR == gridY && srcC == gridX) {
                        g_ctx->cached_legal_moves.clear();
                        return; 
                    }

                    if (ChessEngine::IsLegalMove(*g_ctx->board, srcR, srcC, gridY, gridX)) {
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
                            
                            ChessEngine::CacheLegalMoves(*g_ctx->board, gridY, gridX);
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

void DrawSettingsCheckbox(const char* label, bool* variablePointer, int x, int y, Vector2 mousePos) {
    DrawTextSmooth(label, x, y, 20.0f, Color{ 215, 195, 140, 255 });

    int boxX = x + 350; 
    Rectangle boxRec = { (float)boxX, (float)y - 4, 28.0f, 28.0f };
    bool isHovered = CheckCollisionPointRec(mousePos, boxRec);

    DrawRectangleRec(boxRec, isHovered ? Color{ 45, 45, 48, 255 } : Color{ 24, 24, 26, 255 });
    DrawRectangleLinesEx(boxRec, 2, isHovered ? GOLD : Color{ 80, 75, 70, 255 });

    if (*variablePointer == true) {
        DrawRectangle(boxRec.x + 6, boxRec.y + 6, 16, 16, GOLD);
    }

    if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        *variablePointer = !(*variablePointer); 
    }
}

void DrawSettingsMenu(Vector2 mousePos) {

    int paneX = 200;
    int paneY = 80;
    int paneWidth = Config::WINDOW_WIDTH - 300;
    int paneHeight = Config::WINDOW_HEIGHT - 160;

    DrawRectangle(paneX, paneY, paneWidth, paneHeight, Config::COLOR_UI_PANEL_BG);
    DrawRectangleLinesEx(Rectangle{ (float)paneX, (float)paneY, (float)paneWidth, (float)paneHeight }, 3, Config::COLOR_LEAF_LIGHT);
    
    DrawTextSmooth("SETTINGS", paneX + 40, paneY + 30, 32.0f, Color{ 215, 195, 140, 255 });
    DrawLineEx(Vector2{ (float)paneX + 40, (float)paneY + 75 }, Vector2{ (float)paneX + paneWidth - 40, (float)paneY + 75 }, 2, Config::COLOR_LEAF_LIGHT);

    int startY = paneY + 110;
    int labelX = paneX + 60;
    int labelXright = paneX +60+ paneWidth/2;
    int controlX = paneX + 400;

    float minVal = 0.0f;
    float maxVal = 1.0f;
    
    DrawTextSmooth("Master Volume", labelX, startY, 20.0f, RAYWHITE);
    
    int sliderWidth = 300;
    int sliderHeight = 10;
    Rectangle sliderBar = { (float)controlX, (float)startY + 6, (float)sliderWidth, (float)sliderHeight };
    DrawRectangleRec(sliderBar, Color{ 45, 45, 48, 255 }); 
    DrawRectangleLinesEx(sliderBar, 1, Color{ 90, 85, 80, 255 });

    float currentPercentage = (g_ctx->masterVolume - minVal) / (maxVal - minVal);
    int handleX = sliderBar.x + (currentPercentage * sliderWidth);
    Rectangle sliderHandle = { (float)handleX - 8, (float)startY, 16, 22 };

    DrawRectangle(sliderBar.x, sliderBar.y, handleX - sliderBar.x, sliderHeight, Color{ 180, 160, 110, 255 });

    bool sliderHover = CheckCollisionPointRec(mousePos, sliderBar) || CheckCollisionPointRec(mousePos, sliderHandle);
    DrawRectangleRec(sliderHandle, sliderHover ? Color{ 230, 210, 160, 255 } : Color{ 140, 125, 95, 255 });

    if (sliderHover && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float mouseXRelative = mousePos.x - sliderBar.x;
        if (mouseXRelative < 0) mouseXRelative = 0;
        if (mouseXRelative > sliderWidth) mouseXRelative = sliderWidth;
        
        g_ctx->masterVolume = minVal + (mouseXRelative / sliderWidth) * (maxVal - minVal);
    }

    DrawTextSmooth(TextFormat("%d%%", (int)(g_ctx->masterVolume * 100)), controlX + sliderWidth + 25, startY, 18.0f, Color{ 180, 160, 110, 255 });

    int row2Y = startY + 50;
    DrawSettingsCheckbox("High Contrast (unimplemented)", &g_ctx->highcontrast, labelX, row2Y, mousePos);
    DrawSettingsCheckbox("Board Markings", &g_ctx->boardmarkings, labelXright, row2Y, mousePos);
    std::vector<Color> availableColors = { MAROON, LIME, DARKBLUE, ORANGE, PURPLE };
    int boxSize = 35;
    int boxSpacing = 15;

    int row3Y = row2Y + 50;
    DrawSettingsCheckbox("50 Move Counter", &g_ctx->fiftymovecounter, labelX, row3Y, mousePos);
    DrawSettingsCheckbox("Three Fold Counter", &g_ctx->threefoldcounter, labelXright, row3Y, mousePos);
    int row4Y = row3Y + 50;
    DrawSettingsCheckbox("placeholder", &g_ctx->highcontrast, labelXright, row4Y, mousePos);
    DrawSettingsCheckbox("placeholder", &g_ctx->highcontrast, labelXright, row4Y, mousePos);

    int btnY = paneY + paneHeight - 75;
    Rectangle backBtn = { (float)(paneX + (paneWidth / 2) - 100), (float)btnY, 200.0f, 45.0f };
    bool backHover = CheckCollisionPointRec(mousePos, backBtn);

    DrawRectangleRec(backBtn, backHover ? Color{ 42, 40, 38, 255 } : Color{ 34, 32, 30, 255 });
    DrawRectangleLinesEx(backBtn, 2, backHover ? GOLD : Color{ 100, 95, 90, 255 });
    
    DrawTextSmooth("CONFIRM", backBtn.x + (backBtn.width / 2) - 40, backBtn.y + 12, 18.0f, backHover ? GOLD : Color{ 215, 195, 140, 255 });

    if (backHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        g_ctx->active_menu = GAME; 
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
            CanvasRenderer::DrawGameMetrics();
            CanvasRenderer::DrawChessboard(displayState);
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