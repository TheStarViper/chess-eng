#include "canvas_renderer.hpp"
#include "main.hpp"
#include "variables.h"
#include "vec_renderer.hpp"
#include "easing_functions.hpp"
#include "variant-switcher.hpp"
#include "puzzles.hpp"

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
            if (g_ctx->duck_phase){
                statusText = (g_ctx->board->turn == P_WHITE) ? "WHITE'S DUCK" : "BLACK'S DUCK";
            }
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

    void draw_duck_placement_dots() {
    if (!g_ctx->duck_phase) return;

    Vector2 mousePos = GetMousePosition();

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            
            if (g_ctx->board->grid[r][c].has_piece) continue;
            if (std::make_pair(r, c) == g_ctx->duck_pos) continue;

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
                playsoundsmart(g_ctx->hoversound, .5, 1.2);
            }

            float minRadius = (float)Config::TILE_SIZE * 0.12f;
            float maxRadius = (float)Config::TILE_SIZE * 0.17f;
            float currentRadius = minRadius + (maxRadius - minRadius) * easedT;
            
            DrawCircle(drawX, drawY, currentRadius, Config::COLOR_DOT);
            tile.isHovered.update();
        }
    }
}

    void draw_legal_moves(){
        if ((g_ctx->board->has_selection || g_ctx->duck_phase) && g_ctx->historyView.useLive) {
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
                else if (tile.has_piece) {
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

    void draw_game_over() {
        GameResult status = VariantSwitcher::Active()->CheckGameOver(*g_ctx->board);
        
        bool isGameOver = (g_ctx->board->state != STATE_PLAYING) || (status != IN_PROGRESS);

        if (isGameOver && g_ctx->historyView.useLive) {
            DrawRectangle(Config::BOARD_OFFSET_X, Config::BOARD_OFFSET_Y, Config::TILE_SIZE * 8, Config::TILE_SIZE * 8, Color{ 20, 10, 5, 160 });
            
            std::string endTitle;
            
            if (status == WHITE_WIN) {
                endTitle = "WHITE WINS!";
            } 
            else if (status == BLACK_WIN) {
                endTitle = "BLACK WINS!";
            } 
            else if (status == DRAW) {
                endTitle = "DRAW";
            } 
            else {
                if (g_ctx->board->state == STATE_RESIGNED) {
                    endTitle = "RESIGNATION";
                } else {
                    endTitle = "GAME OVER";
                }
            }

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
        draw_duck_placement_dots();
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
                    VariantSwitcher::Active()->CacheLegalMoves(*g_ctx->board, hintFromRow, hintFromCol);
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

