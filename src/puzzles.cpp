#include "puzzles.hpp"
#include <vector>
#include <sstream>
#include <fstream>
#include <utility>
#include <cstdlib>
#include <random>
#include "variables.h"
#include "main.hpp"
#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif
std::vector<unsigned char> g_fileDataBuffer;

void load_puzzles() {
    int dataSize = 0;
    unsigned char *fileData = LoadFileData(puzzlefilepath.c_str(), &dataSize);

    if (fileData == nullptr || dataSize == 0) {
        TraceLog(LOG_ERROR, "CSV_LOADER: Could not open or find %s", puzzlefilepath.c_str());
        return;
    }

#if defined(PLATFORM_WEB)
    unsigned int seed = static_cast<unsigned int>(emscripten_get_now());
    srand(seed);
#else
    srand(GetRandomValue(0, 100000));
#endif

    size_t fileSize = static_cast<size_t>(dataSize);
    if (fileSize < 500) {
        TraceLog(LOG_ERROR, "CSV_LOADER: File too small to parse.");
        UnloadFileData(fileData);
        return;
    }

    g_fileDataBuffer.assign(fileData, fileData + fileSize);
    UnloadFileData(fileData);
    
    TraceLog(LOG_INFO, "CSV_LOADER: Successfully loaded puzzle file into memory.");
}


Puzzle get_random_puzzle() {
    Puzzle selectedpuzzle{};

    if (g_fileDataBuffer.empty()) {
        TraceLog(LOG_ERROR, "CSV_LOADER: Cannot get puzzle. Buffer is empty. Did you call load_puzzles()?");
        return selectedpuzzle;
    }

    size_t fileSize = g_fileDataBuffer.size();

    if (fileSize <= 250) {
        TraceLog(LOG_ERROR, "CSV_LOADER: File buffer is too small to safely pick a random offset.");
        return selectedpuzzle;
    }

    static std::random_device rd;
    static std::mt19937 gen(rd()); 

    std::uniform_int_distribution<size_t> distr(0, fileSize - 250);
    size_t randomOffset = distr(gen);

    size_t startPos = randomOffset;
    while (startPos < fileSize && g_fileDataBuffer[startPos] != '\n') {
        startPos++;
    }
    startPos++;

    if (startPos >= fileSize - 10) {
        startPos = 0;
        while (startPos < fileSize && g_fileDataBuffer[startPos] != '\n') {
            startPos++;
        }
        startPos++;
    }

    size_t endPos = startPos;
    while (endPos < fileSize && g_fileDataBuffer[endPos] != '\n') {
        endPos++;
    }

    std::string line(reinterpret_cast<char*>(&g_fileDataBuffer[startPos]), endPos - startPos);

    line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
    if (line.empty()) {
        TraceLog(LOG_ERROR, "CSV_LOADER: Landed on empty row string slice.");
        return selectedpuzzle;
    }

    std::vector<std::string> row;
    size_t commaPrev = 0;

    for (int i = 0; i < 4; i++) {
        size_t commaPos = line.find(',', commaPrev);
        if (commaPos == std::string::npos) break;

        std::string segment = line.substr(commaPrev, commaPos - commaPrev);
        segment.erase(0, segment.find_first_not_of(" \t"));
        size_t last = segment.find_last_not_of(" \t");
        if (last != std::string::npos) segment.erase(last + 1);

        row.push_back(segment);
        commaPrev = commaPos + 1;
    }

    if (commaPrev < line.length()) {
        std::string ratingSegment = line.substr(commaPrev);
        ratingSegment.erase(0, ratingSegment.find_first_not_of(" \t"));
        size_t last = ratingSegment.find_last_not_of(" \t");
        if (last != std::string::npos) ratingSegment.erase(last + 1);
        row.push_back(ratingSegment);
    }

    if (row.size() == 5) {
        selectedpuzzle.puzzleid = row[0];
        selectedpuzzle.gameid = row[1];
        selectedpuzzle.boardsetup = row[2];
        selectedpuzzle.solution = row[3];
        
        char* endptr;
        selectedpuzzle.rating = static_cast<int>(std::strtol(row[4].c_str(), &endptr, 10));

        TraceLog(LOG_INFO, "--- Randomly Selected Row ---");
        TraceLog(LOG_INFO, "Puzzle ID: %s", selectedpuzzle.puzzleid.c_str());
        TraceLog(LOG_INFO, "Game ID:   %s", selectedpuzzle.gameid.c_str());
        TraceLog(LOG_INFO, "Setup:     %s", selectedpuzzle.boardsetup.c_str());
        TraceLog(LOG_INFO, "Solution:  %s", selectedpuzzle.solution.c_str());
        TraceLog(LOG_INFO, "Rating:    %d", selectedpuzzle.rating);
    } else {
        TraceLog(LOG_ERROR, "CSV_LOADER: Extracted line slice was invalid.");
    }

    return selectedpuzzle;
}

BoardState ParseFenToState(const std::string& fen) {
    BoardState newState;
    std::stringstream ss(fen);
    std::string pieces, turn, castling, en_passant;
    int halfmove = 0, fullmove = 1;

    ss >> pieces >> turn >> castling >> en_passant;
    if (ss >> halfmove) ss >> fullmove;

    int row = 0;
    int col = 0; 
    
    for (char c : pieces) {
        if (c == '/') {
            row++;
            col = 0;
        } else if (std::isdigit(c)) {
            int empty_spaces = c - '0';
            for (int i = 0; i < empty_spaces; ++i) {
                newState.grid[row][col] = GridData(); 
                col++;
            }
        } else {
            bool isWhite = std::isupper(c);
            char typeChar = std::toupper(c);
            
            PieceType type = PieceType::PAWN; 
            if (typeChar == 'P') type = PieceType::PAWN;
            else if (typeChar == 'N') type = PieceType::KNIGHT;
            else if (typeChar == 'B') type = PieceType::BISHOP;
            else if (typeChar == 'R') type = PieceType::ROOK;
            else if (typeChar == 'Q') type = PieceType::QUEEN;
            else if (typeChar == 'K') type = PieceType::KING;

            std::string colorStr = isWhite ? P_WHITE : P_BLACK;
            auto new_piece = std::make_shared<ChessPiece>(type, colorStr);

            newState.grid[row][col] = GridData(new_piece);

            col++;
        }
    }
    newState.turn = (turn == "w") ? P_WHITE : P_BLACK;
    newState.white_king_side_castle = (castling.find('K') != std::string::npos);
    newState.white_queen_side_castle = (castling.find('Q') != std::string::npos);
    newState.black_king_side_castle = (castling.find('k') != std::string::npos);
    newState.black_queen_side_castle = (castling.find('q') != std::string::npos);

    if (en_passant == "-") {
        newState.en_passant_square = {-1, -1};
    } else {
        int ep_col = en_passant[0] - 'a';
        int ep_row = 8 - (en_passant[1] - '0');
        newState.en_passant_square = {ep_row, ep_col};
    }

    newState.halfmove_clock = halfmove;
    newState.fullmove_number = fullmove;

    return newState;
}

std::vector<std::string> SplitMoveString(const std::string& movesStr) {
    std::vector<std::string> moves;
    std::stringstream ss(movesStr);
    std::string move;
    while (ss >> move) {
        moves.push_back(move);
    }
    return moves;
}

std::string ConvertToUci(int fromRow, int fromCol, int toRow, int toCol) { //universal chess interface
    char fromFile = 'a' + fromCol;
    char fromRank = '8' - fromRow;
    char toFile = 'a' + toCol;
    char toRank = '8' - toRow;
    
    std::string uci = "";
    uci += fromFile;
    uci += fromRank;
    uci += toFile;
    uci += toRank;
    return uci;
}

void do_the_puzzle_stuff(BoardState& displayState, float dt, Menus& last_menu){
    BoardState puzzleState;
    if (load_new_puzzle) {
        g_ctx->cachedpuzzle = get_random_puzzle();
        g_ctx->savedPuzzleState = ParseFenToState(g_ctx->cachedpuzzle.boardsetup);
        g_ctx->savedPuzzleHistory.clear(); 
        g_ctx->hasSavedPuzzle = true;
        
        g_ctx->puzzleMoveIndex = 0;
        g_ctx->puzzleFailed = false;
        g_ctx->puzzleSuccess = false;
        g_ctx->puzzleOpponentTimer = 0.5f;
        if (g_ctx->active_menu == PUZZLES) {
            g_ctx->board->LoadState(g_ctx->savedPuzzleState);
            g_ctx->board->move_history.clear();
        }
        
        g_ctx->historyView.useLive = true;
        g_ctx->historyView.viewingIndex = -1;
        load_new_puzzle = false;
    } 

    if (g_ctx->active_menu != last_menu) {
        if (last_menu == PUZZLES) {
            g_ctx->savedPuzzleState = g_ctx->board->CaptureState();
            g_ctx->savedPuzzleHistory = g_ctx->board->move_history;
            g_ctx->hasSavedPuzzle = true;
        } else if (last_menu == GAME) {
            g_ctx->savedGameState = g_ctx->board->CaptureState();
            g_ctx->savedGameHistory = g_ctx->board->move_history;
            g_ctx->hasSavedGame = true;
        }

        if (g_ctx->active_menu == PUZZLES) {
            if (!g_ctx->hasSavedPuzzle) {
                std::string defaultFen = g_ctx->cachedpuzzle.boardsetup.empty() ? 
                    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" : g_ctx->cachedpuzzle.boardsetup;
                g_ctx->savedPuzzleState = ParseFenToState(defaultFen);
                g_ctx->savedPuzzleHistory.clear();
                g_ctx->hasSavedPuzzle = true;
            }
            g_ctx->board->LoadState(g_ctx->savedPuzzleState);
            g_ctx->board->move_history = g_ctx->savedPuzzleHistory;
        } 
        else if (g_ctx->active_menu == GAME) {
            if (!g_ctx->hasSavedGame) {
                g_ctx->board->Reset(); 
                g_ctx->savedGameState = g_ctx->board->CaptureState();
                g_ctx->savedGameHistory.clear();
                g_ctx->hasSavedGame = true;
            } else {
                g_ctx->board->LoadState(g_ctx->savedGameState);
                g_ctx->board->move_history = g_ctx->savedGameHistory;
            }
        }

        g_ctx->active_turn_id = (g_ctx->board->turn == P_WHITE) ? 0 : 1;
        last_menu = g_ctx->active_menu;
        g_ctx->historyView.useLive = true;
        g_ctx->historyView.viewingIndex = -1;
    }
    
    displayState = g_ctx->board->CaptureState();
    g_ctx->active_turn_id = (g_ctx->board->turn == P_WHITE) ? 0 : 1;
    
    if (g_ctx->active_menu == PUZZLES && !g_ctx->anim.active && g_ctx->anim.piece != GridData().piece) {
        if (g_ctx->active_menu == PUZZLES && !g_ctx->anim.active && g_ctx->anim.piece != GridData().piece) {
        if (g_ctx->puzzleMoveIndex % 2 != 0) {
            g_ctx->board->grid[g_ctx->anim.targetRow][g_ctx->anim.targetCol] = GridData(g_ctx->anim.piece);
            g_ctx->anim.piece = GridData().piece;
            
            if (g_ctx->board->turn == P_WHITE) {
                g_ctx->board->turn = P_BLACK;
                g_ctx->active_turn_id = 1;
            } else {
                g_ctx->board->turn = P_WHITE;
                g_ctx->active_turn_id = 0;
            }
        }
    }
    }

    if (g_ctx->active_menu == PUZZLES) {
        if (g_ctx->puzzleOpponentTimer > 0.0f) {
            g_ctx->puzzleOpponentTimer -= dt;
            if (g_ctx->anim.active) {
                g_ctx->anim.elapsedTime += dt;
                float duration = 0.25f;
                
                if (g_ctx->anim.elapsedTime >= duration) {
                    g_ctx->anim.active = false;
                    g_ctx->board->grid[g_ctx->anim.targetRow][g_ctx->anim.targetCol] = GridData(g_ctx->anim.piece);
                    g_ctx->anim.piece = GridData().piece;
                    g_ctx->savedPuzzleState = g_ctx->board->CaptureState();
                }
            }
            if (g_ctx->puzzleOpponentTimer <= 0.0f) {
                std::vector<std::string> solutionMoves = SplitMoveString(g_ctx->cachedpuzzle.solution);
                std::string opponentMoveUci = solutionMoves[g_ctx->puzzleMoveIndex];
                
                int opFromCol = opponentMoveUci[0] - 'a', opFromRow = '8' - opponentMoveUci[1];
                int opToCol   = opponentMoveUci[2] - 'a', opToRow   = '8' - opponentMoveUci[3];
                
                auto p = g_ctx->board->grid[opFromRow][opFromCol].piece;
                
                g_ctx->anim.active = true;
                g_ctx->anim.piece = p;
                g_ctx->anim.startPos = { 
                    (float)Config::BOARD_OFFSET_X + opFromCol * Config::TILE_SIZE, 
                    (float)Config::BOARD_OFFSET_Y + opFromRow * Config::TILE_SIZE 
                };
                g_ctx->anim.currentPos = g_ctx->anim.startPos;
                g_ctx->anim.endPos = { 
                    (float)Config::BOARD_OFFSET_X + opToCol * Config::TILE_SIZE, 
                    (float)Config::BOARD_OFFSET_Y + opToRow * Config::TILE_SIZE 
                };
                g_ctx->anim.elapsedTime = 0.0f;
                g_ctx->anim.targetRow = opToRow;
                g_ctx->anim.targetCol = opToCol;

                g_ctx->board->grid[opFromRow][opFromCol] = GridData();
                
                g_ctx->puzzleMoveIndex++; 
                g_ctx->puzzleOpponentTimer = -1.0f;
            }
        }

        if (g_ctx->puzzleFailed && !g_ctx->anim.active) {
            g_ctx->puzzle_streak = 0;
            g_ctx->puzzle_fail_count++;
            g_ctx->board->LoadState(g_ctx->savedPuzzleState);
            g_ctx->board->move_history.clear(); 
            g_ctx->board->current_repetition_count = 1;
            
            g_ctx->puzzleMoveIndex = 0; 
            g_ctx->board->turn = g_ctx->savedPuzzleState.turn;
            g_ctx->active_turn_id = (g_ctx->board->turn == P_WHITE) ? 0 : 1;
            
            g_ctx->puzzleOpponentTimer = 0.4f;
            g_ctx->puzzleFailed = false;
        }

        if (g_ctx->anim.active) {
            if (g_ctx->anim.elapsedTime > 0.0f) {
                std::vector<std::string> solutionMoves = SplitMoveString(g_ctx->cachedpuzzle.solution);
                if (g_ctx->puzzleMoveIndex > 0 && (g_ctx->puzzleMoveIndex - 1) < (int)solutionMoves.size()) {
                    std::string currentMove = solutionMoves[g_ctx->puzzleMoveIndex - 1];
                    int opFromCol = currentMove[0] - 'a';
                    int opFromRow = '8' - currentMove[1];
                    
                    displayState.grid[opFromRow][opFromCol] = GridData();
                }
            }
        } else {
            displayState = g_ctx->board->CaptureState();
        }
    }
}