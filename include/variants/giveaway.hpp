#pragma once
#include "variant-linker.hpp"
#include "vanillalogic.hpp"
#include "main.hpp"

class GiveawayChessVariant : public ChessVariantInterface {
private:
    bool HasAnyCaptures(const ChessBoard& b) {
        for (int r1 = 0; r1 < 8; ++r1) {
            for (int c1 = 0; c1 < 8; ++c1) {
                if (!b.grid[r1][c1].has_piece || b.grid[r1][c1].piece->color != b.turn) {
                    continue;
                }
                
                for (int r2 = 0; r2 < 8; ++r2) {
                    for (int c2 = 0; c2 < 8; ++c2) {
                        if (VanillaLogic::IsPseudoLegalMove(b, r1, c1, r2, c2, true)) {
                            if (b.grid[r2][c2].has_piece && b.grid[r2][c2].piece->color != b.turn) {
                                return true; 
                            }
                            if (b.grid[r1][c1].piece->type == PAWN && c1 != c2 && !b.grid[r2][c2].has_piece) {
                                if (VanillaLogic::IsLegalMove(b, r1, c1, r2, c2)) {
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
        return false;
    }

public:
    ~GiveawayChessVariant() override = default;

    bool IsPseudoLegalMove(const ChessBoard& b, int r1, int c1, int r2, int c2, bool checkingEnPassant) override {
        return VanillaLogic::IsPseudoLegalMove(b, r1, c1, r2, c2, checkingEnPassant);
    }

    bool IsLegalMove(const ChessBoard& b, int r1, int c1, int r2, int c2) override {
        if (!VanillaLogic::IsPseudoLegalMove(b, r1, c1, r2, c2, true)) {
            return false;
        }

        bool captureAvailable = HasAnyCaptures(b);
        bool isThisMoveACapture = b.grid[r2][c2].has_piece && b.grid[r2][c2].piece->color != b.turn;

        if (b.grid[r1][c1].piece->type == PAWN && c1 != c2 && !b.grid[r2][c2].has_piece) {
            isThisMoveACapture = true; 
        }

        if (captureAvailable && !isThisMoveACapture) {
            return false; 
        }

        return true;
    }

    void MakeMove(ChessBoard& b, int r1, int c1, int r2, int c2, PieceType promotionChoice) override {
        VanillaLogic::MakeMove(b, r1, c1, r2, c2, promotionChoice);
    }

    bool CheckInsufficientMaterial(const ChessBoard& b) override {
        return false; 
    }

    void CacheLegalMoves(const ChessBoard& b, int r, int c) override {
        g_ctx->cached_legal_moves.clear();
        if (!b.grid[r][c].has_piece) return;

        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                if (IsLegalMove(b, r, c, row, col)) {
                    g_ctx->cached_legal_moves.push_back({row, col});
                }
            }
        }
    }

    int EvaluateBoard(const ChessBoard& b) override {
        int score = 0;
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                if (b.grid[r][c].has_piece) {
                    auto piece = b.grid[r][c].piece;
                    int val = 0;
                    switch (piece->type) {
                        case PAWN:   val = 100;  break;
                        case KNIGHT: val = 320;  break;
                        case BISHOP: val = 330;  break;
                        case ROOK:   val = 500;  break;
                        case QUEEN:  val = 900;  break;
                        case KING:   val = 300; break;
                        default: break;
                    }
                    if (piece->color == P_WHITE) score -= val;
                    else score += val;
                }
            }
        }
        return score;
    }
    
    GameResult CheckGameOver(const ChessBoard& b) override {
        int whitePieces = 0;
        int blackPieces = 0;
        bool hasAnyLegalMove = false;

        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                if (b.grid[r][c].has_piece) {
                    if (b.grid[r][c].piece->color == P_WHITE) whitePieces++;
                    else blackPieces++;

                    if (b.grid[r][c].piece->color == b.turn && !hasAnyLegalMove) {
                        for (int targetR = 0; targetR < 8; ++targetR) {
                            for (int targetC = 0; targetC < 8; ++targetC) {
                                if (IsLegalMove(b, r, c, targetR, targetC)) {
                                    hasAnyLegalMove = true;
                                    break;
                                }
                            }
                            if (hasAnyLegalMove) break;
                        }
                    }
                }
            }
        }

        if (whitePieces == 0) return WHITE_WIN;
        if (blackPieces == 0) return BLACK_WIN;

        if (!hasAnyLegalMove) {
            return (b.turn == P_WHITE) ? WHITE_WIN : BLACK_WIN;
        }

        return IN_PROGRESS;
    }
};