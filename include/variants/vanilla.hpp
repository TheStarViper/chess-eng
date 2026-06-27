#pragma once
#include "variant-linker.hpp"
#include "vanillalogic.hpp"

class StandardChessVariant : public ChessVariantInterface {
public:
    ~StandardChessVariant() override = default;

    bool IsPseudoLegalMove(const ChessBoard& b, int r1, int c1, int r2, int c2, bool checkingEnPassant) override {
        return VanillaLogic::IsPseudoLegalMove(b, r1, c1, r2, c2, checkingEnPassant);
    }

    bool IsLegalMove(const ChessBoard& b, int r1, int c1, int r2, int c2) override {
        return VanillaLogic::IsLegalMove(b, r1, c1, r2, c2);
    }

    void MakeMove(ChessBoard& b, int r1, int c1, int r2, int c2, PieceType promotionChoice) override {
        VanillaLogic::MakeMove(b, r1, c1, r2, c2, promotionChoice);
    }

    bool CheckInsufficientMaterial(const ChessBoard& b) override {
        return VanillaLogic::CheckInsufficientMaterial(b);
    }

    void CacheLegalMoves(const ChessBoard& b, int r, int c) override {
        VanillaLogic::CacheLegalMoves(b, r, c);
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
                        case KING:   val = 20000; break;
                        default: break;
                    }
                    if (piece->color == P_WHITE) score += val;
                    else score -= val;
                }
            }
        }
        return score;
    }
};