#include "gamelogic.hpp"
#include "variables.h"
#include "main.hpp"

namespace ChessEngine {
    inline bool IsValidCoord(int r, int c) {
        return r >= 0 && r < 8 && c >= 0 && c < 8;
    }

    bool IsPathClear(const ChessBoard& b, int r1, int c1, int r2, int c2) {
        int dr = r2 - r1;
        int dc = c2 - c1;
        int stepR = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);
        int stepC = (dc == 0) ? 0 : (dc > 0 ? 1 : -1);

        int r = r1 + stepR;
        int c = c1 + stepC;
        while (r != r2 || c != c2) {
            if (!IsValidCoord(r, c)) return false;
            if (b.grid[r][c].has_piece) return false;
            r += stepR;
            c += stepC;
        }
        return true;
    }

    bool IsPseudoLegalMove(const ChessBoard& b, int r1, int c1, int r2, int c2, bool checkingEnPassant) {
        if (!IsValidCoord(r1, c1) || !IsValidCoord(r2, c2)) return false;
        if (r1 == r2 && c1 == c2) return false;

        const auto& cellSrc = b.grid[r1][c1];
        if (!cellSrc.has_piece) return false;

        const auto& piece = cellSrc.piece;
        const auto& cellDest = b.grid[r2][c2];

        if (cellDest.has_piece && cellDest.piece->color == piece->color) return false;

        int dr = r2 - r1;
        int dc = c2 - c1;
        int absR = std::abs(dr);
        int absC = std::abs(dc);

        switch (piece->type) {
            case PAWN: {
                int forward = (piece->color == P_WHITE) ? -1 : 1;
                int startRow = (piece->color == P_WHITE) ? 6 : 1;

                if (dc == 0 && dr == forward && !cellDest.has_piece) {
                    return true;
                }
                if (dc == 0 && r1 == startRow && dr == 2 * forward) {
                    int intermediateR = r1 + forward;
                    if (!b.grid[intermediateR][c1].has_piece && !cellDest.has_piece) {
                        return true;
                    }
                }
                if (absC == 1 && dr == forward) {
                    if (cellDest.has_piece && cellDest.piece->color != piece->color) {
                        return true;
                    }
                    if (checkingEnPassant && b.en_passant_square.first == r2 && b.en_passant_square.second == c2) {
                        return true;
                    }
                }
                return false;
            }
            case KNIGHT:
                return (absR == 2 && absC == 1) || (absR == 1 && absC == 2);

            case BISHOP:
                if (absR == absC) {
                    return IsPathClear(b, r1, c1, r2, c2);
                }
                return false;

            case ROOK:
                if (dr == 0 || dc == 0) {
                    return IsPathClear(b, r1, c1, r2, c2);
                }
                return false;

            case QUEEN:
                if (dr == 0 || dc == 0 || absR == absC) {
                    return IsPathClear(b, r1, c1, r2, c2);
                }
                return false;

            case KING:
                if (absR <= 1 && absC <= 1) {
                    return true;
                }
                return false;
            default:
                break;
        }
        return false;
    }

    std::pair<int, int> LocateKing(const ChessBoard& b, std::string color) {
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                if (b.grid[r][c].has_piece && b.grid[r][c].piece->type == KING && b.grid[r][c].piece->color == color) {
                    return { r, c };
                }
            }
        }
        return { -1, -1 };
    }

    bool IsKingInCheck(const ChessBoard& b, std::string color) {
        std::pair<int, int> kingPos = LocateKing(b, color);
        if (kingPos.first == -1) return false;

        std::string attackerColor = (color == P_WHITE) ? P_BLACK : P_WHITE;

        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                if (b.grid[r][c].has_piece && b.grid[r][c].piece->color == attackerColor) {
                    if (IsPseudoLegalMove(b, r, c, kingPos.first, kingPos.second, false)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool IsLegalMove(const ChessBoard& b, int r1, int c1, int r2, int c2) {
        bool pseudo = IsPseudoLegalMove(b, r1, c1, r2, c2, true);
        const auto& p = b.grid[r1][c1].piece;
        
        if (!pseudo && p && p->type == KING && r1 == r2 && std::abs(c2 - c1) == 2) {
            std::string col = p->color;
            int homeRow = (col == P_WHITE) ? 7 : 0;
            if (r1 != homeRow || c1 != 4) return false;

            if (IsKingInCheck(b, col)) return false;

            if (c2 == 6) {
                bool rights = (col == P_WHITE) ? b.white_king_side_castle : b.black_king_side_castle;
                if (!rights) return false;
                if (b.grid[homeRow][5].has_piece || b.grid[homeRow][6].has_piece) return false;
                
                ChessBoard temp = b;
                temp.grid[homeRow][5] = temp.grid[homeRow][4];
                temp.grid[homeRow][4] = GridData();
                if (IsKingInCheck(temp, col)) return false;

                return true;
            }
            if (c2 == 2) {
                bool rights = (col == P_WHITE) ? b.white_queen_side_castle : b.black_queen_side_castle;
                if (!rights) return false;
                if (b.grid[homeRow][1].has_piece || b.grid[homeRow][2].has_piece || b.grid[homeRow][3].has_piece) return false;

                ChessBoard temp = b;
                temp.grid[homeRow][3] = temp.grid[homeRow][4];
                temp.grid[homeRow][4] = GridData();
                if (IsKingInCheck(temp, col)) return false;

                return true;
            }
        }

        if (!pseudo) return false;

        ChessBoard simulatedBoard = b;
        auto movingPiece = simulatedBoard.grid[r1][c1].piece;
        
        if (movingPiece->type == PAWN && r2 == simulatedBoard.en_passant_square.first && c2 == simulatedBoard.en_passant_square.second) {
            simulatedBoard.grid[r1][c2] = GridData(); 
        }

        simulatedBoard.grid[r2][c2] = simulatedBoard.grid[r1][c1];
        simulatedBoard.grid[r1][c1] = GridData();

        return !IsKingInCheck(simulatedBoard, movingPiece->color);
    }

    void CacheLegalMoves(const ChessBoard& b, int r, int c) {
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

    bool HasAnyLegalMoves(const ChessBoard& b, std::string color) {
        for (int r1 = 0; r1 < 8; ++r1) {
            for (int c1 = 0; c1 < 8; ++c1) {
                if (b.grid[r1][c1].has_piece && b.grid[r1][c1].piece->color == color) {
                    for (int r2 = 0; r2 < 8; ++r2) {
                        for (int c2 = 0; c2 < 8; ++c2) {
                            if (IsLegalMove(b, r1, c1, r2, c2)) {
                                return true;
                            }
                        }
                    }
                }
            }
        }
        return false;
    }

    int GetRepetitionCount(const ChessBoard& b) {
        if (b.move_history.empty()) return 1;
        const BoardState currentState = b.CaptureState();
        int matchingStateCount = 1; 
        
        for (const auto& hist : b.move_history) {
            if (hist.board_state == currentState) {
                matchingStateCount++;
            }
        }
        return matchingStateCount;
    }

    bool CheckInsufficientMaterial(const ChessBoard& b) {
        std::vector<std::shared_ptr<ChessPiece>> whitePieces;
        std::vector<std::shared_ptr<ChessPiece>> blackPieces;

        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                if (b.grid[r][c].has_piece) {
                    auto piece = b.grid[r][c].piece;
                    if (piece->type != KING) {
                        if (piece->color == P_WHITE) whitePieces.push_back(piece);
                        else blackPieces.push_back(piece);
                    }
                }
            }
        }

        size_t wSize = whitePieces.size();
        size_t bSize = blackPieces.size();

        if (wSize == 0 && bSize == 0) return true;

        if ((wSize == 1 && bSize == 0 && (whitePieces[0]->type == BISHOP || whitePieces[0]->type == KNIGHT)) ||
            (bSize == 1 && wSize == 0 && (blackPieces[0]->type == BISHOP || blackPieces[0]->type == KNIGHT))) {
            return true;
        }

        if (wSize == 1 && bSize == 1 && whitePieces[0]->type == BISHOP && blackPieces[0]->type == BISHOP) {
            int wR = -1, wC = -1, bR = -1, bC = -1;
            for (int r = 0; r < 8; ++r) {
                for (int c = 0; c < 8; ++c) {
                    if (b.grid[r][c].has_piece) {
                        if (b.grid[r][c].piece->type == BISHOP) {
                            if (b.grid[r][c].piece->color == P_WHITE) { wR = r; wC = c; }
                            else { bR = r; bC = c; }
                        }
                    }
                }
            }
            if ((wR + wC) % 2 == (bR + bC) % 2) {
                return true;
            }
        }

        return false;
    }

    std::string GenerateNotation(const ChessBoard& b, int r1, int c1, int r2, int c2, PieceType promotionChoice, bool isCastlingK, bool isCastlingQ, bool isEP) {
        if (isCastlingK) return "O-O";
        if (isCastlingQ) return "O-O-O";

        std::stringstream ss;
        auto p = b.grid[r1][c1].piece;
        
        if (p->type != PAWN) {
            switch (p->type) {
                case KNIGHT: ss << "N"; break;
                case BISHOP: ss << "B"; break;
                case ROOK:   ss << "R"; break;
                case QUEEN:  ss << "Q"; break;
                case KING:   ss << "K"; break;
                default: break;
            }
        }

        bool capture = b.grid[r2][c2].has_piece || isEP;
        if (p->type == PAWN && capture) {
            ss << (char)('a' + c1);
        }

        if (capture) {
            ss << "x";
        }

        ss << (char)('a' + c2);
        ss << (char)('8' - r2);

        if (promotionChoice != NONE) {
            ss << "=";
            switch (promotionChoice) {
                case QUEEN: ss << "Q"; break;
                case ROOK: ss << "R"; break;
                case BISHOP: ss << "B"; break;
                case KNIGHT: ss << "N"; break;
                default: break;
            }
        }

        ChessBoard temp = b;
        temp.grid[r2][c2] = temp.grid[r1][c1];
        temp.grid[r1][c1] = GridData();
        std::string nextTurn = (p->color == P_WHITE) ? P_BLACK : P_WHITE;
        if (IsKingInCheck(temp, nextTurn)) {
            if (!HasAnyLegalMoves(temp, nextTurn)) {
                ss << "#";
            } else {
                ss << "+";
            }
        }

        return ss.str();
    }

    void MakeMove(ChessBoard& b, int r1, int c1, int r2, int c2, PieceType promotionChoice) {
        playsoundsmart(g_ctx->movesound,.6,1);
        auto p = b.grid[r1][c1].piece;
        if (!p) return;

        b.last_move_from = { r1, c1 };
        b.last_move_to = { r2, c2 };

        bool isCapture = b.grid[r2][c2].has_piece;
        bool isPawnPush = (p->type == PAWN);
        
        bool isCastlingK = (p->type == KING && c2 - c1 == 2);
        bool isCastlingQ = (p->type == KING && c1 - c2 == 2);
        bool isEP = (p->type == PAWN && r2 == b.en_passant_square.first && c2 == b.en_passant_square.second);

        std::string moveNotation = GenerateNotation(b, r1, c1, r2, c2, promotionChoice, isCastlingK, isCastlingQ, isEP);

        if (isCapture || isPawnPush) {
            b.halfmove_clock = 0;
        } else {
            b.halfmove_clock++;
        }

        if (isEP) {
            b.grid[r1][c2] = GridData(); 
        }

        if (isPawnPush && std::abs(r2 - r1) == 2) {
            b.en_passant_square = { r1 + (r2 - r1)/2, c1 };
        } else {
            b.en_passant_square = { -1, -1 };
        }

        if (isCastlingK) {
            int homeRow = (p->color == P_WHITE) ? 7 : 0;
            b.grid[homeRow][5] = b.grid[homeRow][7];
            b.grid[homeRow][7] = GridData();
            if (b.grid[homeRow][5].has_piece) b.grid[homeRow][5].piece->has_moved = true;
        } else if (isCastlingQ) {
            int homeRow = (p->color == P_WHITE) ? 7 : 0;
            b.grid[homeRow][3] = b.grid[homeRow][0];
            b.grid[homeRow][0] = GridData();
            if (b.grid[homeRow][3].has_piece) b.grid[homeRow][3].piece->has_moved = true;
        }

        if (promotionChoice != NONE) {
            b.grid[r2][c2] = GridData(std::make_shared<ChessPiece>(promotionChoice, p->color));
        } else {
            b.grid[r2][c2] = b.grid[r1][c1];
        }
        b.grid[r2][c2].piece->has_moved = true;
        b.grid[r1][c1] = GridData();

        if (p->type == KING) {
            if (p->color == P_WHITE) {
                b.white_king_side_castle = false;
                b.white_queen_side_castle = false;
            } else {
                b.black_king_side_castle = false;
                b.black_queen_side_castle = false;
            }
        }
        if (p->type == ROOK) {
            if (p->color == P_WHITE) {
                if (r1 == 7 && c1 == 7) b.white_king_side_castle = false;
                if (r1 == 7 && c1 == 0) b.white_queen_side_castle = false;
            } else {
                if (r1 == 0 && c1 == 7) b.black_king_side_castle = false;
                if (r1 == 0 && c1 == 0) b.black_queen_side_castle = false;
            }
        }

        if (b.turn == P_BLACK) {
            b.fullmove_number++;
            b.turn = P_WHITE;
        } else {
            b.turn = P_BLACK;
        }

        b.in_check = IsKingInCheck(b, b.turn);
        b.check_king_pos = b.in_check ? LocateKing(b, b.turn) : std::pair<int,int>{-1, -1};
        b.current_repetition_count = GetRepetitionCount(b);

        HistorySnapshot snap;
        snap.notation = moveNotation;
        snap.board_state = b.CaptureState();

        b.move_history.push_back(snap);
        g_ctx->active_turn_id = (b.turn == P_WHITE) ? 0 : 1;
        g_ctx->cached_legal_moves.clear(); 

        std::string activePlayer = b.turn;
        if (g_ctx->active_menu == PUZZLES){return;}
        if (!HasAnyLegalMoves(b, activePlayer)) {
            if (IsKingInCheck(b, activePlayer)) {
                b.state = STATE_CHECKMATE;
            } else {
                b.state = STATE_STALEMATE;
            }
        } else if (b.halfmove_clock >= 100) { 
            b.state = STATE_DRAW_50_MOVES;
        } else if (b.current_repetition_count >= 3) {
            b.state = STATE_DRAW_REPETITION;
        } else if (CheckInsufficientMaterial(b)) {
            b.state = STATE_DRAW_MATERIAL;
        }
    }
}
