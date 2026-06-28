#pragma once
#include "variant-linker.hpp"
#include "vanillalogic.hpp"
#include "main.hpp"

class DuckChessVariant : public ChessVariantInterface {
public:
    ~DuckChessVariant() override = default;

    bool IsPseudoLegalMove(const ChessBoard& b, int r1, int c1, int r2, int c2, bool checkingEnPassant) override {
        if (r1 < 0 || r1 >= 8 || c1 < 0 || c1 >= 8 || r2 < 0 || r2 >= 8 || c2 < 0 || c2 >= 8) return false;
        if (g_ctx->duck_pos.first == r2 && g_ctx->duck_pos.second == c2) return false;
        if (!b.grid[r1][c1].has_piece || !b.grid[r1][c1].piece) return false;

        if (!VanillaLogic::IsPseudoLegalMove(b, r1, c1, r2, c2, checkingEnPassant)) return false;

        if (b.grid[r1][c1].piece->type != KNIGHT) {
            int deltaR = (r2 > r1) ? 1 : ((r2 < r1) ? -1 : 0);
            int deltaC = (c2 > c1) ? 1 : ((c2 < c1) ? -1 : 0);
            int currR = r1 + deltaR;
            int currC = c1 + deltaC;
            
            while (currR >= 0 && currR < 8 && currC >= 0 && currC < 8) {
                if (currR == r2 && currC == c2) break;
                if (g_ctx->duck_pos.first == currR && g_ctx->duck_pos.second == currC) return false;
                currR += deltaR;
                currC += deltaC;
            }
        }
        return true;
    }

    bool IsLegalMove(const ChessBoard& b, int r1, int c1, int r2, int c2) override {
        if (g_ctx->duck_phase) return false;
        return IsPseudoLegalMove(b, r1, c1, r2, c2, true);
    }

    void MakeMove(ChessBoard& b, int r1, int c1, int r2, int c2, PieceType promotionChoice) override {
        VanillaLogic::MakeMove(b, r1, c1, r2, c2, promotionChoice);
        g_ctx->duck_phase = true;
        b.turn = (b.turn == P_WHITE) ? P_BLACK : P_WHITE;
    }

    void CacheLegalMoves(const ChessBoard& b, int r, int c) override {
        g_ctx->cached_legal_moves.clear();
        if (g_ctx->duck_phase || !b.grid[r][c].has_piece) return;

        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                if (IsLegalMove(b, r, c, row, col)) {
                    g_ctx->cached_legal_moves.push_back({row, col});
                }
            }
        }
    }

    void DrawExtra(const ChessBoard& b) override {
        if (g_ctx->duck_pos.first != -1) {
            static Texture2D duckTexture = LoadTexture("assets/images/set/duck-ai-generated.png");
            float drawX = Config::BOARD_OFFSET_X + (g_ctx->duck_pos.second * Config::TILE_SIZE);
            float drawY = Config::BOARD_OFFSET_Y + (g_ctx->duck_pos.first * Config::TILE_SIZE);

            DrawTexturePro(
                duckTexture,
                Rectangle{ 0, 0, (float)duckTexture.width, (float)duckTexture.height },
                Rectangle{ drawX, drawY, (float)Config::TILE_SIZE, (float)Config::TILE_SIZE },
                Vector2{ 0, 0 }, 0.0f, WHITE
            );
        }
    }

    GameResult CheckGameOver(const ChessBoard& b) override {
        bool whiteKingAlive = false, blackKingAlive = false;
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                if (b.grid[r][c].has_piece && b.grid[r][c].piece->type == KING) {
                    if (b.grid[r][c].piece->color == P_WHITE) whiteKingAlive = true;
                    else blackKingAlive = true;
                }
            }
        }
        if (!whiteKingAlive) return BLACK_WIN;
        if (!blackKingAlive) return WHITE_WIN;
        return IN_PROGRESS;
    }

    bool CheckInsufficientMaterial(const ChessBoard& b) override { return false; }
    int EvaluateBoard(const ChessBoard& b) override { return 0; }
};