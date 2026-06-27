#pragma once
#include <string>
#include <vector>
#include <utility>
#include "variables.h"

class ChessVariantInterface {
public:
    virtual ~ChessVariantInterface();

    virtual bool IsPseudoLegalMove(const ChessBoard& b, int r1, int c1, int r2, int c2, bool checkingEnPassant) = 0;
    virtual bool IsLegalMove(const ChessBoard& b, int r1, int c1, int r2, int c2) = 0;
    virtual void MakeMove(ChessBoard& b, int r1, int c1, int r2, int c2, PieceType promotionChoice=NONE) = 0;
    virtual void CacheLegalMoves(const ChessBoard& b, int r, int c) = 0;
    virtual bool CheckInsufficientMaterial(const ChessBoard& b) = 0;
    virtual int EvaluateBoard(const ChessBoard& b) = 0;
};