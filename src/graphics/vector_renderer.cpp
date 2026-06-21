#include "graphics/vector_renderer.hpp"
#include "variables.h"
#include "raylib.h"
#include "rlgl.h"

inline void DrawRectangleRoundedLinesCustom(Rectangle rec, float roundness, int segments, float lineThick, Color color) {
    DrawRectangleRoundedLinesEx(rec, roundness, segments, lineThick, color);
}

struct GameContext {
    void* board; 
    void* historyView;
    void* anim;
    void* g_puzzleDatabase;
    void* g_currentPuzzle;
    void* pieceSprites; 

    Font uiFont;
    Font pieceFont;
    float sidebarWidth;
    bool sidebarhovered;
    int active_menu;
    int lastHoveredLeafId;
    Sound hoversound;
};

namespace VectorRenderer {

    void DrawLeaf(float x, float y, float size, float angleDegrees) {
        rlPushMatrix();
        rlTranslatef(x, y, 0.0f);
        rlRotatef(angleDegrees, 0.0f, 0.0f, 1.0f);

        DrawEllipse(0, 0, size * 1.1f, size * 0.55f, Config::COLOR_LEAF_DARK);
        DrawEllipse(-1, -1, size, size * 0.5f, Config::COLOR_LEAF_LIGHT);
        DrawLineEx(Vector2{ -size * 0.8f, 0 }, Vector2{ size * 0.8f, 0 }, 1.5f, Config::COLOR_LEAF_VEIN);
        
        rlPopMatrix();
    }

    void DrawTileWoodGrain(float x, float y, float w, float h, bool isDark) {
        Color grainColor = isDark ? Color{ 96, 55, 28, 60 } : Color{ 185, 149, 105, 75 };
    
        float offset1 = w * 0.25f;
        float offset2 = w * 0.65f;
        float offset3 = w * 0.80f;

        DrawLineEx(Vector2{ x + offset1, y + 2 }, Vector2{ x + offset1, y + h - 2 }, 1.2f, grainColor);
        DrawLineEx(Vector2{ x + offset2, y + 4 }, Vector2{ x + offset2, y + h - 4 }, 1.0f, grainColor);
        DrawLineEx(Vector2{ x + offset3, y + 3 }, Vector2{ x + offset3, y + h - 3 }, 1.1f, grainColor);
    }

    void DrawOvergrownVines(int boardX, int boardY, float boardSize) {
        Vector2 mousePos = GetMousePosition();
        int currentLeafId = 0;
        int hoveredLeafThisFrame = -1;

        auto UpdateAndDrawLeaf = [&](float x, float y, float size, float angleDegrees) {
            int id = currentLeafId++;
            float finalSize = size;

            float hoverRadius = size * 1.2f; 
            bool isHovered = CheckCollisionPointCircle(mousePos, Vector2{ x, y }, hoverRadius);

            if (isHovered) {
                hoveredLeafThisFrame = id;
                finalSize = size * 1.30f;

                if (g_ctx->lastHoveredLeafId != id) {
                    PlaySound(g_ctx->hoversound);
                }
            }

            DrawLeaf(x, y, finalSize, angleDegrees);
        };

        //TOP
      // --- BOARD TOP LEAVES ---
        UpdateAndDrawLeaf((float)boardX + 75,  (float)boardY - 6,  14, -15);
        UpdateAndDrawLeaf((float)boardX + 95,  (float)boardY - 12, 17, 10);
        UpdateAndDrawLeaf((float)boardX + 115, (float)boardY - 5,  13, 35);
        UpdateAndDrawLeaf((float)boardX + 135, (float)boardY - 9,  15, -10);
        UpdateAndDrawLeaf((float)boardX + 160, (float)boardY - 6,  12, 45);

        // --- Board Top Grouping (Spread across mid-to-right) ---
        UpdateAndDrawLeaf((float)boardX + 380, (float)boardY - 10, 15, -25);
        UpdateAndDrawLeaf((float)boardX + 405, (float)boardY - 6,  13, 5);
        UpdateAndDrawLeaf((float)boardX + 430, (float)boardY - 13, 18, 20);
        UpdateAndDrawLeaf((float)boardX + 455, (float)boardY - 5,  14, -15);
        UpdateAndDrawLeaf((float)boardX + 480, (float)boardY + 2,  16, 55);

        // --- Board Bottom Grouping (Spread left-to-mid) ---
        UpdateAndDrawLeaf((float)boardX + 65,  (float)boardY + boardSize + 6,  14, 160);
        UpdateAndDrawLeaf((float)boardX + 90,  (float)boardY + boardSize + 12, 18, 195);
        UpdateAndDrawLeaf((float)boardX + 115, (float)boardY + boardSize + 4,  13, 140);
        UpdateAndDrawLeaf((float)boardX + 140, (float)boardY + boardSize + 9,  15, 175);

        // --- Board Bottom Grouping (Spread mid-to-right) ---
        UpdateAndDrawLeaf((float)boardX + 420, (float)boardY + boardSize + 5,  13, 150);
        UpdateAndDrawLeaf((float)boardX + 445, (float)boardY + boardSize + 11, 17, 215);
        UpdateAndDrawLeaf((float)boardX + 470, (float)boardY + boardSize + 4,  14, 135);
        UpdateAndDrawLeaf((float)boardX + 495, (float)boardY + boardSize + 8,  16, 185);

        // --- Board Left Side Grouping (Spread vertically down) ---
        UpdateAndDrawLeaf((float)boardX - 6,   (float)boardY + 210, 14, -75);
        UpdateAndDrawLeaf((float)boardX - 10,  (float)boardY + 235, 16, -100);
        UpdateAndDrawLeaf((float)boardX - 13,  (float)boardY + 260, 18, -120);
        UpdateAndDrawLeaf((float)boardX - 8,   (float)boardY + 285, 13, -60);
        UpdateAndDrawLeaf((float)boardX - 5,   (float)boardY + 310, 15, -85);

        // --- Board Right Side Grouping (Spread vertically down) ---
        UpdateAndDrawLeaf((float)boardX + boardSize + 6,  (float)boardY + 310, 13, 65);
        UpdateAndDrawLeaf((float)boardX + boardSize + 11, (float)boardY + 335, 17, 90);
        UpdateAndDrawLeaf((float)boardX + boardSize + 14, (float)boardY + 360, 19, 120);
        UpdateAndDrawLeaf((float)boardX + boardSize + 9,  (float)boardY + 385, 14, 55);
        UpdateAndDrawLeaf((float)boardX + boardSize + 7,  (float)boardY + 410, 15, 100);

        
        // --- Move Log Top Border (Spanning left half) ---
        UpdateAndDrawLeaf((float)Config::PANEL_X + 10,  (float)Config::PANEL_Y + 6,   13, -45);
        UpdateAndDrawLeaf((float)Config::PANEL_X + 30,  (float)Config::PANEL_Y + 1,   16, 10);
        UpdateAndDrawLeaf((float)Config::PANEL_X + 50,  (float)Config::PANEL_Y + 4,   12, -20);
        UpdateAndDrawLeaf((float)Config::PANEL_X + 70,  (float)Config::PANEL_Y + 1,   15, 30);
        UpdateAndDrawLeaf((float)Config::PANEL_X + 90,  (float)Config::PANEL_Y + 5,   13, -10);

        // --- Move Log Top Border (Spanning right half) ---
        UpdateAndDrawLeaf((float)Config::PANEL_X + Config::PANEL_WIDTH - 100, (float)Config::PANEL_Y + 4,   14, -15);
        UpdateAndDrawLeaf((float)Config::PANEL_X + Config::PANEL_WIDTH - 80,  (float)Config::PANEL_Y + 1,   15, 35);
        UpdateAndDrawLeaf((float)Config::PANEL_X + Config::PANEL_WIDTH - 60,  (float)Config::PANEL_Y + 6,   12, 10);
        UpdateAndDrawLeaf((float)Config::PANEL_X + Config::PANEL_WIDTH - 40,  (float)Config::PANEL_Y + 2,   17, 75);
        UpdateAndDrawLeaf((float)Config::PANEL_X + Config::PANEL_WIDTH - 15,  (float)Config::PANEL_Y + 8,   13, 115);

        // --- Move Log Left Side (Cascading down the edge) ---
        UpdateAndDrawLeaf((float)Config::PANEL_X+3,   (float)Config::PANEL_Y + 120, 13, -80);
        UpdateAndDrawLeaf((float)Config::PANEL_X+2,   (float)Config::PANEL_Y + 145, 16, -110);
        UpdateAndDrawLeaf((float)Config::PANEL_X+4,  (float)Config::PANEL_Y + 170, 15, -70);
        UpdateAndDrawLeaf((float)Config::PANEL_X+2,   (float)Config::PANEL_Y + 195, 14, -95);
        UpdateAndDrawLeaf((float)Config::PANEL_X+1,   (float)Config::PANEL_Y + 220, 12, -120);

        // --- Move Log Right Side (Cascading down the edge) ---
        UpdateAndDrawLeaf((float)Config::PANEL_X + Config::PANEL_WIDTH + 1, (float)Config::PANEL_Y + 140, 12, 60);
        UpdateAndDrawLeaf((float)Config::PANEL_X + Config::PANEL_WIDTH + 1, (float)Config::PANEL_Y + 165, 15, 95);
        UpdateAndDrawLeaf((float)Config::PANEL_X + Config::PANEL_WIDTH + 2, (float)Config::PANEL_Y + 190, 17, 115);
        UpdateAndDrawLeaf((float)Config::PANEL_X + Config::PANEL_WIDTH + 1, (float)Config::PANEL_Y + 215, 13, 50);
        UpdateAndDrawLeaf((float)Config::PANEL_X + Config::PANEL_WIDTH + 2, (float)Config::PANEL_Y + 240, 14, 85);

        // --- Move Log Bottom Border (Spanning along the base) ---
        UpdateAndDrawLeaf((float)Config::PANEL_X + 12,  (float)Config::PANEL_Y + Config::PANEL_HEIGHT + 3, 14, -135);
        UpdateAndDrawLeaf((float)Config::PANEL_X + 37,  (float)Config::PANEL_Y + Config::PANEL_HEIGHT + 2,  16, 185);
        UpdateAndDrawLeaf((float)Config::PANEL_X + 62,  (float)Config::PANEL_Y + Config::PANEL_HEIGHT + 5, 13, 150);
        
        UpdateAndDrawLeaf((float)Config::PANEL_X + Config::PANEL_WIDTH - 75, (float)Config::PANEL_Y + Config::PANEL_HEIGHT +2, 14, 210);
        UpdateAndDrawLeaf((float)Config::PANEL_X + Config::PANEL_WIDTH - 50, (float)Config::PANEL_Y + Config::PANEL_HEIGHT +3, 15, 145);
        UpdateAndDrawLeaf((float)Config::PANEL_X + Config::PANEL_WIDTH - 20, (float)Config::PANEL_Y + Config::PANEL_HEIGHT+1,  13, 70);
        
        g_ctx->lastHoveredLeafId = hoveredLeafThisFrame;
    }

    void DrawBoardOrnateFrame(int x, int y, int size) {
        int borderSize = 20;
        DrawRectangle(x - borderSize, y - borderSize, size + (borderSize * 2), size + (borderSize * 2), Config::COLOR_FRAME_DARK);
        DrawRectangleLinesEx(Rectangle{ (float)x - 4, (float)y - 4, (float)size + 8, (float)size + 8 }, 3.0f, Config::COLOR_FRAME_MID);
    }


    //fallback function if sprites fail to load
    void DrawChessPieceVector(PieceType type, std::string color, int x, int y, int size) { 
        float fX = (float)x;
        float fY = (float)y;
        float fS = (float)size;

        if (g_ctx && g_ctx->pieceFont.texture.id > 0) {
            char c = ' ';
            switch (type) {
                case PAWN:   c = 'O'; break;
                case KNIGHT: c = 'J'; break;
                case BISHOP: c = 'N'; break;
                case ROOK:   c = 'T'; break;
                case QUEEN:  c = 'W'; break;
                case KING:   c = 'L'; break;
                case NONE:   return;
            }
            if (color == P_BLACK) {
                c = tolower(c);
            }
            char codepointStr[2] = { c, '\0' };

            Vector2 sizeVec = MeasureTextEx(g_ctx->pieceFont, codepointStr, (float)size, 0.0f);
            float drawX = fX + (fS - sizeVec.x) / 2.0f;
            float drawY = fY + (fS - sizeVec.y) / 2.0f;

            Color tintColor = (color == P_WHITE) ? Color{ 255, 255, 255, 255 } : Color{ 35, 35, 40, 255 };

            DrawTextEx(g_ctx->pieceFont, codepointStr, Vector2{ drawX + 2, drawY + 2 }, (float)size, 0.0f, Color{ 0, 0, 0, 60 });
            DrawTextEx(g_ctx->pieceFont, codepointStr, Vector2{ drawX, drawY }, (float)size, 0.0f, tintColor);
            return;
        }

        Color fill = (color == P_WHITE) ? Color{ 255, 255, 255, 255 } : Color{ 55, 55, 60, 255 };
        Color stroke = (color == P_WHITE) ? Color{ 140, 140, 145, 255 } : Color{ 225, 225, 230, 255 };
        float thick = 3.0f;

        float cx = fX + fS / 2.0f;
        float cy = fY + fS / 2.0f;

        switch (type) {
            case PAWN: {
                DrawRectangleRounded(Rectangle{ cx - fS*0.22f, cy + fS*0.24f, fS*0.44f, fS*0.08f }, 0.4f, 4, fill);
                DrawRectangleRoundedLinesCustom(Rectangle{ cx - fS*0.22f, cy + fS*0.24f, fS*0.44f, fS*0.08f }, 0.4f, 4, thick, stroke);

                Vector2 p1 = Vector2{ cx, cy - fS * 0.10f };
                Vector2 p2 = Vector2{ cx - fS * 0.14f, cy + fS * 0.24f };
                Vector2 p3 = Vector2{ cx + fS * 0.14f, cy + fS * 0.24f };
                DrawTriangle(p1, p2, p3, fill);
                DrawLineEx(p1, p2, thick, stroke);
                DrawLineEx(p1, p3, thick, stroke);

                DrawCircle((int)cx, (int)(cy - fS * 0.12f), fS * 0.14f, fill);
                DrawCircleLines((int)cx, (int)(cy - fS * 0.12f), fS * 0.14f, stroke);
                break;
            }
            case KNIGHT: {
                DrawRectangleRounded(Rectangle{ cx - fS*0.25f, cy + fS*0.24f, fS*0.5f, fS*0.08f }, 0.4f, 4, fill);
                DrawRectangleRoundedLinesCustom(Rectangle{ cx - fS*0.25f, cy + fS*0.24f, fS*0.5f, fS*0.08f }, 0.4f, 4, thick, stroke);

                Vector2 top = Vector2{ cx - fS*0.18f, cy + fS*0.24f };
                Vector2 snout = Vector2{ cx - fS*0.28f, cy - fS*0.08f };
                Vector2 head = Vector2{ cx + fS*0.12f, cy - fS*0.28f };
                Vector2 back = Vector2{ cx + fS*0.18f, cy + fS*0.24f };

                DrawTriangle(top, snout, head, fill);
                DrawTriangle(top, head, back, fill);

                DrawLineEx(top, snout, thick, stroke);
                DrawLineEx(snout, head, thick, stroke);
                DrawLineEx(head, back, thick, stroke);

                Color eyeColor = (color == P_WHITE) ? Color{ 40, 40, 40, 255 } : WHITE;
                DrawCircle((int)(cx - fS*0.08f), (int)(cy - fS*0.13f), fS*0.035f, eyeColor);
                break;
            }
            case BISHOP: {
                DrawRectangleRounded(Rectangle{ cx - fS*0.24f, cy + fS*0.24f, fS*0.48f, fS*0.08f }, 0.4f, 4, fill);
                DrawRectangleRoundedLinesCustom(Rectangle{ cx - fS*0.24f, cy + fS*0.24f, fS*0.48f, fS*0.08f }, 0.4f, 4, thick, stroke);
                
                DrawCircle((int)cx, (int)cy, fS*0.20f, fill);
                DrawCircleLines((int)cx, (int)cy, fS*0.20f, stroke);

                DrawCircle((int)cx, (int)(cy - fS*0.25f), fS*0.06f, fill);
                DrawCircleLines((int)cx, (int)(cy - fS*0.25f), fS*0.06f, stroke);

                DrawLineEx(Vector2{ cx - fS*0.08f, cy - fS*0.08f }, Vector2{ cx + fS*0.08f, cy + fS*0.08f }, thick + 0.5f, stroke);
                break;
            }
            case ROOK: {
                DrawRectangleRounded(Rectangle{ cx - fS*0.24f, cy + fS*0.24f, fS*0.48f, fS*0.08f }, 0.4f, 4, fill);
                DrawRectangleRoundedLinesCustom(Rectangle{ cx - fS*0.24f, cy + fS*0.24f, fS*0.48f, fS*0.08f }, 0.4f, 4, thick, stroke);

                DrawRectangleRounded(Rectangle{ cx - fS*0.20f, cy - fS*0.16f, fS*0.40f, fS*0.40f }, 0.1f, 4, fill);
                DrawRectangleRoundedLinesCustom(Rectangle{ cx - fS*0.20f, cy - fS*0.16f, fS*0.40f, fS*0.40f }, 0.1f, 4, thick, stroke);

                float batW = fS * 0.10f;
                DrawRectangleRec(Rectangle{ cx - fS*0.20f, cy - fS*0.26f, batW, fS*0.10f }, fill);
                DrawRectangleLinesEx(Rectangle{ cx - fS*0.20f, cy - fS*0.26f, batW, fS*0.10f }, thick, stroke);

                DrawRectangleRec(Rectangle{ cx - batW/2.0f, cy - fS*0.26f, batW, fS*0.10f }, fill);
                DrawRectangleLinesEx(Rectangle{ cx - batW/2.0f, cy - fS*0.26f, batW, fS*0.10f }, thick, stroke);

                DrawRectangleRec(Rectangle{ cx + fS*0.20f - batW, cy - fS*0.26f, batW, fS*0.10f }, fill);
                DrawRectangleLinesEx(Rectangle{ cx + fS*0.20f - batW, cy - fS*0.26f, batW, fS*0.10f }, thick, stroke);
                break;
            }
            case QUEEN: {
                DrawRectangleRounded(Rectangle{ cx - fS*0.28f, cy + fS*0.24f, fS*0.56f, fS*0.08f }, 0.4f, 4, fill);
                DrawRectangleRoundedLinesCustom(Rectangle{ cx - fS*0.28f, cy + fS*0.24f, fS*0.56f, fS*0.08f }, 0.4f, 4, thick, stroke);

                Vector2 pLeft = Vector2{ cx - fS*0.26f, cy + fS*0.24f };
                Vector2 pMid = Vector2{ cx, cy - fS*0.22f };
                Vector2 pRight = Vector2{ cx + fS*0.26f, cy + fS*0.24f };

                DrawTriangle(pLeft, pMid, pRight, fill);
                DrawLineEx(pLeft, pMid, thick, stroke);
                DrawLineEx(pMid, pRight, thick, stroke);

                DrawCircle((int)(cx - fS*0.22f), (int)(cy - fS*0.02f), fS*0.05f, fill);
                DrawCircleLines((int)(cx - fS*0.22f), (int)(cy - fS*0.02f), fS*0.05f, stroke);

                DrawCircle((int)(cx + fS*0.22f), (int)(cy - fS*0.02f), fS*0.05f, fill);
                DrawCircleLines((int)(cx + fS*0.22f), (int)(cy - fS*0.02f), fS*0.05f, stroke);

                DrawCircle((int)cx, (int)(cy - fS*0.22f), fS*0.07f, fill);
                DrawCircleLines((int)cx, (int)(cy - fS*0.22f), fS*0.07f, stroke);
                break;
            }
            case KING: {
                DrawRectangleRounded(Rectangle{ cx - fS*0.28f, cy + fS*0.24f, fS*0.56f, fS*0.08f }, 0.4f, 4, fill);
                DrawRectangleRoundedLinesCustom(Rectangle{ cx - fS*0.28f, cy + fS*0.24f, fS*0.56f, fS*0.08f }, 0.4f, 4, thick, stroke);

                DrawRectangleRounded(Rectangle{ cx - fS*0.18f, cy - fS*0.14f, fS*0.36f, fS*0.38f }, 0.15f, 4, fill);
                DrawRectangleRoundedLinesCustom(Rectangle{ cx - fS*0.18f, cy - fS*0.14f, fS*0.36f, fS*0.38f }, 0.15f, 4, thick, stroke);

                float crossThick = 4.0f;
                DrawLineEx(Vector2{ cx, cy - fS*0.18f }, Vector2{ cx, cy - fS*0.36f }, crossThick, stroke);
                DrawLineEx(Vector2{ cx - fS*0.09f, cy - fS*0.27f }, Vector2{ cx + fS*0.09f, cy - fS*0.27f }, crossThick, stroke);
                break;
            }
            case NONE: break;
        }
    }
}
