
#pragma once
#include "variables.h"
namespace ChessEngine {

    inline bool IsValidCoord(int r, int c);

    bool IsPathClear(const ChessBoard& b, int r1, int c1, int r2, int c2);

    bool IsPseudoLegalMove(const ChessBoard& b, int r1, int c1, int r2, int c2, bool checkingEnPassant = true);

    std::pair<int, int> LocateKing(const ChessBoard& b, std::string color);

    bool IsKingInCheck(const ChessBoard& b, std::string color);

    bool IsLegalMove(const ChessBoard& b, int r1, int c1, int r2, int c2);

    void CacheLegalMoves(const ChessBoard& b, int r, int c);

    bool HasAnyLegalMoves(const ChessBoard& b, std::string color);

    int GetRepetitionCount(const ChessBoard& b);

    bool CheckInsufficientMaterial(const ChessBoard& b);

    std::string GenerateNotation(const ChessBoard& b, int r1, int c1, int r2, int c2, PieceType promotionChoice, bool isCastlingK, bool isCastlingQ, bool isEP);
    
    void MakeMove(ChessBoard& b, int r1, int c1, int r2, int c2, PieceType promotionChoice = NONE);
}
