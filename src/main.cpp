#include "raylib.h"
#include "rlgl.h"
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <map>
#include <utility>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

namespace Config {
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 720;

    constexpr int TILE_SIZE = 75;                // Size of chessboard squares (75 * 8 = 600px)
    constexpr int BOARD_OFFSET_X = 180;          // Left buffer
    constexpr int BOARD_OFFSET_Y = 60;          // Top buffer

    constexpr int PANEL_X = BOARD_OFFSET_X+TILE_SIZE*8+50;                // Sidebar alignment x-coordinate
    constexpr int PANEL_Y = 60;                 // Sidebar alignment y-coordinate
    constexpr int PANEL_WIDTH = 380;            // Sidebar width
    constexpr int PANEL_HEIGHT = TILE_SIZE*8;           // Panel matches board footprint
    constexpr int ROW_HEIGHT = 35;              // Row spacing for historical lists

    // Aesthetic color schemes matching Board.png
    const Color COLOR_LIGHT_SQ = Color{ 205, 171, 128, 255 };  
    const Color COLOR_DARK_SQ  = Color{ 116, 75, 48, 255 };    
    const Color COLOR_FRAME_DARK = Color{ 48, 28, 16, 255 };   
    const Color COLOR_FRAME_MID  = Color{ 78, 48, 30, 255 };
    const Color COLOR_LEAF_DARK  = Color{ 76, 91, 55, 255 }; 
    const Color COLOR_LEAF_LIGHT = Color{ 151, 163, 69, 255 }; 
    const Color COLOR_LEAF_VEIN  = Color{ 113, 129, 63, 255 }; 
    const Color COLOR_GEM_BASE   = Color{ 42, 24, 18, 255 };    
    const Color COLOR_GEM_GLINT  = Color{ 162, 103, 56, 255 };  

    // NEW THEMATIC UI COLORS
    const Color COLOR_UI_PANEL_BG   = Color{ 36, 22, 14, 255 }; 
    const Color COLOR_UI_BORDER     = Color{ 78, 48, 30, 255 }; 
    const Color COLOR_UI_ROW_A      = Color{ 48, 30, 20, 255 }; 
    const Color COLOR_UI_ROW_B      = Color{ 38, 24, 16, 255 }; 
    const Color COLOR_UI_TEXT       = Color{ 240, 220, 190, 255 }; 
    const Color COLOR_UI_TEXT_DIM   = Color{ 180, 150, 120, 255 }; 
    const Color COLOR_UI_BUTTON     = Color{ 96, 55, 28, 255 };
    const Color COLOR_UI_BUTTON_HOV = Color{ 120, 75, 40, 255 }; 
    
    const Color COLOR_HIGHLIGHT= Color{ 123, 97, 255, 120 };   
    const Color COLOR_CHECK    = Color{ 230, 90, 90, 220 };    
    const Color COLOR_DOT      = Color{ 100, 149, 237, 180 }; 
    const Color COLOR_DOT_RING = Color{ 100, 149, 237, 100 };
    const Color COLOR_LAST_MOVE= Color{ 246, 235, 120, 100 };  
    
    const Color BOARD_MARKINGS_TEXT = Color{0,0,0,255};
    constexpr float ANIMATION_DURATION = 0.12f;
}

#define P_WHITE "WHITE"
#define P_BLACK "BLACK"

enum PieceType {
    NONE = 0,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

enum GameState {
    STATE_PLAYING,
    STATE_PROMOTING,
    STATE_CHECKMATE,
    STATE_STALEMATE,
    STATE_DRAW_REPETITION,
    STATE_DRAW_50_MOVES,
    STATE_DRAW_MATERIAL,
    STATE_RESIGNED,
    STATE_MUTUAL_DRAW
};


inline void DrawRectangleRoundedLinesCustom(Rectangle rec, float roundness, int segments, float lineThick, Color color) {
    DrawRectangleRoundedLines(rec, roundness, segments, color);
}

std::string ResolveAssetPath(const std::string& relativePath) {
    if (FileExists(relativePath.c_str())) {
        return relativePath;
    }
    std::string prefix = "";
    for (int i = 0; i < 4; ++i) {
        prefix += "../";
        std::string testPath = prefix + relativePath;
        if (FileExists(testPath.c_str())) {
            return testPath;
        }
    }
    std::string appDir = GetApplicationDirectory();
    std::string testPath = appDir + relativePath;
    if (FileExists(testPath.c_str())) {
        return testPath;
    }
    testPath = appDir + "../" + relativePath;
    if (FileExists(testPath.c_str())) {
        return testPath;
    }
    return relativePath; 
}


struct ChessPiece {
    PieceType type;
    std::string color; 
    bool has_moved;    

    ChessPiece(PieceType t, std::string col) : type(t), color(col), has_moved(false) {}
};

struct GridData {
    bool has_piece;
    std::shared_ptr<ChessPiece> piece;

    GridData() : has_piece(false), piece(nullptr) {}
    GridData(std::shared_ptr<ChessPiece> p) : has_piece(p != nullptr), piece(p) {}
};

struct BoardState {
    GridData grid[8][8];
    std::string turn;
    
    bool white_king_side_castle;
    bool white_queen_side_castle;
    bool black_king_side_castle;
    bool black_queen_side_castle;
    
    std::pair<int, int> en_passant_square;
    int halfmove_clock; 
    int fullmove_number;

    std::pair<int, int> last_move_from{-1, -1};
    std::pair<int, int> last_move_to{-1, -1};
    bool in_check = false;
    std::pair<int, int> check_king_pos{-1, -1};
    int repetition_count = 1;

    bool operator==(const BoardState& other) const {
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                if (grid[r][c].has_piece != other.grid[r][c].has_piece) return false;
                if (grid[r][c].has_piece) {
                    if (grid[r][c].piece->type != other.grid[r][c].piece->type ||
                        grid[r][c].piece->color != other.grid[r][c].piece->color) {
                        return false;
                    }
                }
            }
        }
        return turn == other.turn &&
               white_king_side_castle == other.white_king_side_castle &&
               white_queen_side_castle == other.white_queen_side_castle &&
               black_king_side_castle == other.black_king_side_castle &&
               black_queen_side_castle == other.black_queen_side_castle &&
               en_passant_square == other.en_passant_square;
    }
};

struct HistorySnapshot {
    std::string notation; 
    BoardState board_state;
};

struct PieceAnimation {
    bool active;
    std::shared_ptr<ChessPiece> piece;
    Vector2 startPos; 
    Vector2 currentPos;
    Vector2 endPos;
    float elapsedTime;
    int targetRow;
    int targetCol;
};

struct HistoryState {
    bool useLive;            
    int viewingIndex;       
};


struct GameContext;
static std::unique_ptr<GameContext> g_ctx = nullptr;

void DrawTextSmooth(const char* text, float posX, float posY, float fontSize, Color color);
Vector2 MeasureTextSmooth(const char* text, float fontSize);

Font LoadSystemUIFont() {
    std::vector<std::string> paths = {
        "C:\\Windows\\Fonts\\arial.ttf",
        "C:\\Windows\\Fonts\\segoeui.ttf",
        "C:\\Windows\\Fonts\\tahoma.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/Library/Fonts/Arial.ttf",
        "/System/Library/Fonts/HelveticaNeue.ttc",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf"
    };
    
    for (const auto& path : paths) {
        if (FileExists(path.c_str())) {
            Font font = LoadFontEx(path.c_str(), 64, nullptr, 0);
            if (font.texture.id > 0) {
                return font;
            }
        }
    }
    return GetFontDefault();
}


class Button {
public:
    Rectangle rect;
    std::string label;
    Color baseColor;
    Color hoverColor;
    Color textColor;
    bool isPressed;
    bool isHovered;
    int iconType; 

    Button(Rectangle r, std::string lbl, Color base = Color{ 40, 41, 45, 255 }, Color hover = Color{ 60, 62, 68, 255 }, Color txt = WHITE, int icon = 0)
        : rect(r), label(lbl), baseColor(base), hoverColor(hover), textColor(txt), isPressed(false), isHovered(false), iconType(icon) {}

    void Update(Vector2 mousePos) {
        isHovered = CheckCollisionPointRec(mousePos, rect);
        isPressed = isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    }

    void Draw() {
        Color currentBg = isHovered ? hoverColor : baseColor;
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
};


class ChessBoard {
public:
    GridData grid[8][8];
    std::string turn; 
    std::vector<HistorySnapshot> move_history;
    
    std::pair<int, int> selected_square;
    bool has_selection;
    bool is_promoting;
    std::pair<int, int> promotion_square;
    std::pair<int, int> promotion_source;
    
    bool white_king_side_castle;
    bool white_queen_side_castle;
    bool black_king_side_castle;
    bool black_queen_side_castle;
    std::pair<int, int> en_passant_square; 
    int halfmove_clock;
    int fullmove_number;
    GameState state;

    std::pair<int, int> last_move_from;
    std::pair<int, int> last_move_to;
    bool in_check;
    std::pair<int, int> check_king_pos;
    int current_repetition_count;

    ChessBoard() {
        Reset();
    }

    void Reset() {
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

    BoardState CaptureState() const {
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

    void LoadState(const BoardState& bs) {
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
};


struct GameContext {
    std::unique_ptr<ChessBoard> board;
    Vector2 mousePosition;
    int active_turn_id; 
    float move_log_scroll_ratio;
    bool isGameRunning;

    HistoryState historyView;
    PieceAnimation anim;

    std::map<std::string, Texture2D> pieceSprites; 
    bool useSprites = false;

    Font pieceFont;
    Font uiFont;
    RenderTexture2D targetScreen; 

    std::vector<std::pair<int, int>> cached_legal_moves;

    std::unique_ptr<Button> btnResign;
    std::unique_ptr<Button> btnDraw;
    std::unique_ptr<Button> btnPrev;
    std::unique_ptr<Button> btnNext;
    std::unique_ptr<Button> btnLive;
    std::unique_ptr<Button> btnFirst;
    std::unique_ptr<Button> btnLast;
    std::unique_ptr<Button> btnOverlayRematch;

    std::vector<std::unique_ptr<Button>> btnPromotionTrays;

    GameContext() {
        board = std::make_unique<ChessBoard>();
        mousePosition = Vector2{ 0, 0 };
        active_turn_id = 0;
        move_log_scroll_ratio = 0.0f;
        isGameRunning = true;
        historyView = HistoryState{ true, 0 };
        anim = PieceAnimation{ false, nullptr, Vector2{0,0}, Vector2{0,0}, Vector2{0,0}, 0.0f, -1, -1 };

        std::string resolvedPiecesFont = ResolveAssetPath("assets/fonts/pieces.ttf");
        pieceFont = LoadFontEx(resolvedPiecesFont.c_str(), 128, nullptr, 0);

        uiFont = LoadSystemUIFont();

        auto LoadPieceTex = [&](std::string name) {
            std::string path = ResolveAssetPath("assets/images/set/" + name + ".png");
            if (FileExists(path.c_str())) {
                pieceSprites[name] = LoadTexture(path.c_str());
            }
        };

        std::vector<std::string> pieces = { 
            "WhitePawn", "WhiteKnight", "WhiteBishop", "WhiteRook", "WhiteQueen", "WhiteKing",
            "BlackPawn", "BlackKnight", "BlackBishop", "BlackRook", "BlackQueen", "BlackKing" 
        };
        for (const auto& name : pieces) {
            LoadPieceTex(name);
        }
        if (!pieceSprites.empty()) {
            useSprites = true;
        }

        targetScreen = LoadRenderTexture(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT);
        SetTextureFilter(targetScreen.texture, TEXTURE_FILTER_BILINEAR);

        float navY = (float)Config::PANEL_Y + Config::PANEL_HEIGHT - 100;
        float navW = 80;
        float navH = 35;
        float startNavX = (float)Config::PANEL_X + (Config::PANEL_WIDTH - (navW * 4 + 15)) / 2.0f;

        btnFirst   = std::make_unique<Button>(Rectangle{ startNavX, navY, navW, navH }, "|<", Config::COLOR_UI_BUTTON, Config::COLOR_UI_BUTTON_HOV, Config::COLOR_UI_TEXT);
        btnPrev    = std::make_unique<Button>(Rectangle{ startNavX + navW + 5, navY, navW, navH }, "<", Config::COLOR_UI_BUTTON, Config::COLOR_UI_BUTTON_HOV, Config::COLOR_UI_TEXT);
        btnNext    = std::make_unique<Button>(Rectangle{ startNavX + (navW + 5) * 2, navY, navW, navH }, ">", Config::COLOR_UI_BUTTON, Config::COLOR_UI_BUTTON_HOV, Config::COLOR_UI_TEXT);
        btnLast    = std::make_unique<Button>(Rectangle{ startNavX + (navW + 5) * 3, navY, navW, navH }, ">|", Config::COLOR_UI_BUTTON, Config::COLOR_UI_BUTTON_HOV, Config::COLOR_UI_TEXT);

        btnResign  = std::make_unique<Button>(Rectangle{ (float)Config::PANEL_X + 25, (float)Config::PANEL_Y + 550, (Config::PANEL_WIDTH-50)/2-10, 40 }, "Resign", Color{ 120, 35, 35, 255 }, Color{ 150, 45, 45, 255 }, Config::COLOR_UI_TEXT, 1);
        btnDraw    = std::make_unique<Button>(Rectangle{ (float)Config::PANEL_X + (Config::PANEL_WIDTH-50)/2+25+10, (float)Config::PANEL_Y + 550, (Config::PANEL_WIDTH-50)/2-10, 40 }, "Offer Draw", Color{ 55, 60, 45, 255 }, Color{ 75, 80, 55, 255 }, Config::COLOR_UI_TEXT, 2);
        
        float overlayCenterX = Config::BOARD_OFFSET_X + (Config::TILE_SIZE * 8) / 2.0f;
        btnOverlayRematch = std::make_unique<Button>(Rectangle{ overlayCenterX - 125, Config::BOARD_OFFSET_Y + (Config::TILE_SIZE * 8) / 2.0f + 25, 250, 50 }, "REMATCH", Config::COLOR_LEAF_DARK, Config::COLOR_LEAF_LIGHT, Config::COLOR_UI_TEXT);

        ResetPromotionButtons();
    }

    ~GameContext() {
        UnloadRenderTexture(targetScreen);
        if (pieceFont.texture.id > 0) {
            UnloadFont(pieceFont);
        }
        if (uiFont.texture.id > 0) {
            UnloadFont(uiFont);
        }
        for (auto& [name, tex] : pieceSprites) {
            if (tex.id > 0) UnloadTexture(tex);
        }
    }

    void ResetPromotionButtons() {
        btnPromotionTrays.clear();
        float btnW = 90;
        float startX = Config::BOARD_OFFSET_X + 4 * Config::TILE_SIZE - (btnW * 4)/2.0f;
        float startY = Config::BOARD_OFFSET_Y + 4 * Config::TILE_SIZE - 25;

        btnPromotionTrays.push_back(std::make_unique<Button>(Rectangle{ startX, startY, btnW, 50 }, "QUEEN", Config::COLOR_UI_BUTTON, Config::COLOR_UI_BUTTON_HOV, Config::COLOR_UI_TEXT));
        btnPromotionTrays.push_back(std::make_unique<Button>(Rectangle{ startX + btnW, startY, btnW, 50 }, "ROOK", Config::COLOR_UI_BUTTON, Config::COLOR_UI_BUTTON_HOV, Config::COLOR_UI_TEXT));
        btnPromotionTrays.push_back(std::make_unique<Button>(Rectangle{ startX + btnW * 2, startY, btnW, 50 }, "BISHOP", Config::COLOR_UI_BUTTON, Config::COLOR_UI_BUTTON_HOV, Config::COLOR_UI_TEXT));
        btnPromotionTrays.push_back(std::make_unique<Button>(Rectangle{ startX + btnW * 3, startY, btnW, 50 }, "KNIGHT", Config::COLOR_UI_BUTTON, Config::COLOR_UI_BUTTON_HOV, Config::COLOR_UI_TEXT));
    }
};

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

    bool IsPseudoLegalMove(const ChessBoard& b, int r1, int c1, int r2, int c2, bool checkingEnPassant = true) {
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

    void ExecuteMove(ChessBoard& b, int r1, int c1, int r2, int c2, PieceType promotionChoice = NONE) {
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

    void DrawOvergrownVines(int boardX, int boardY, int boardSize) {
        Color stemColor = Color{ 35, 48, 22, 255 };
        float framePad = 12.0f;
        
        DrawLineBezier(
            Vector2{ (float)boardX - framePad, (float)boardY - 6.0f }, 
            Vector2{ (float)boardX + boardSize + framePad, (float)boardY - 4.0f }, 
            3.0f, stemColor
        );
        DrawLineBezier(
            Vector2{ (float)boardX - framePad, (float)boardY + boardSize + 4.0f }, 
            Vector2{ (float)boardX + boardSize + framePad, (float)boardY + boardSize + 6.0f }, 
            3.2f, stemColor
        );
        DrawLineBezier(
            Vector2{ (float)boardX - 4.0f, (float)boardY - framePad }, 
            Vector2{ (float)boardX - 6.0f, (float)boardY + boardSize + framePad }, 
            3.0f, stemColor
        );
        DrawLineBezier(
            Vector2{ (float)boardX + boardSize + 6.0f, (float)boardY - framePad }, 
            Vector2{ (float)boardX + boardSize + 4.0f, (float)boardY + boardSize + framePad }, 
            3.0f, stemColor
        );

        //top
        DrawLeaf((float)boardX + 80,  (float)boardY - 8,  14, -15);
        DrawLeaf((float)boardX + 110, (float)boardY - 14, 18, 5);
        DrawLeaf((float)boardX + 130, (float)boardY - 6,  12, 45);

        DrawLeaf((float)boardX + 220, (float)boardY - 10, 16, -30);
        DrawLeaf((float)boardX + 250, (float)boardY - 8,  15, 20);

        DrawLeaf((float)boardX + 410, (float)boardY - 12, 17, -10);
        DrawLeaf((float)boardX + 440, (float)boardY - 8,  13, 30);

        DrawLeaf((float)boardX + 530, (float)boardY - 12, 19, -20);
        DrawLeaf((float)boardX + 560, (float)boardY - 6,  14, 15);

        //bottom
        DrawLeaf((float)boardX + 60,  (float)boardY + boardSize + 8,  15, 170);
        DrawLeaf((float)boardX + 90,  (float)boardY + boardSize + 12, 18, 195);

        DrawLeaf((float)boardX + 260, (float)boardY + boardSize + 6,  14, 160);
        DrawLeaf((float)boardX + 285, (float)boardY + boardSize + 10, 16, 210);

        DrawLeaf((float)boardX + 440, (float)boardY + boardSize + 8,  19, 175);
        DrawLeaf((float)boardX + 470, (float)boardY + boardSize + 14, 15, 150);

        DrawLeaf((float)boardX + 540, (float)boardY + boardSize + 10, 17, 190);

        //left
        DrawLeaf((float)boardX - 8,  (float)boardY + 50,  15, -80);
        DrawLeaf((float)boardX - 12, (float)boardY + 80,  17, -105);
        
        DrawLeaf((float)boardX - 10, (float)boardY + 230, 16, -95);
        DrawLeaf((float)boardX - 8,  (float)boardY + 260, 14, -60);

        DrawLeaf((float)boardX - 14, (float)boardY + 410, 18, -110);
        DrawLeaf((float)boardX - 10, (float)boardY + 440, 15, -75);

        DrawLeaf((float)boardX - 12, (float)boardY + 550, 16, -90);

        //right
        DrawLeaf((float)boardX + boardSize + 10, (float)boardY + 120, 17, 85);
        DrawLeaf((float)boardX + boardSize + 8,  (float)boardY + 150, 14, 110);

        DrawLeaf((float)boardX + boardSize + 12, (float)boardY + 330, 18, 95);
        DrawLeaf((float)boardX + boardSize + 9,  (float)boardY + 360, 15, 65);

        DrawLeaf((float)boardX + boardSize + 10, (float)boardY + 510, 16, 90);
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
        std::vector<PieceType> standardSet = { QUEEN, ROOK, ROOK, BISHOP, BISHOP, KNIGHT, KNIGHT, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN };
        
        std::map<PieceType, int> activeWhite;
        std::map<PieceType, int> activeBlack;
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                if (displayState.grid[r][c].has_piece) {
                    auto piece = displayState.grid[r][c].piece;
                    if (piece->color == P_WHITE) activeWhite[piece->type]++;
                    else activeBlack[piece->type]++;
                }
            }
        }

        auto getVal = [](PieceType t) {
            if (t == PAWN) return 1;
            if (t == KNIGHT || t == BISHOP) return 3;
            if (t == ROOK) return 5;
            if (t == QUEEN) return 9;
            return 0;
        };

        std::vector<PieceType> capturedWhite; 
        std::vector<PieceType> capturedBlack; 

        int whiteTotalVal = 0;
        int blackTotalVal = 0;

        for (auto t : standardSet) {
            whiteTotalVal += activeWhite[t] * getVal(t);
            blackTotalVal += activeBlack[t] * getVal(t);
        }

        std::map<PieceType, int> tempWhite = activeWhite;
        std::map<PieceType, int> tempBlack = activeBlack;

        for (auto t : standardSet) {
            if (tempWhite[t] > 0) tempWhite[t]--;
            else capturedWhite.push_back(t);

            if (tempBlack[t] > 0) tempBlack[t]--;
            else capturedBlack.push_back(t);
        }

        int topY = Config::BOARD_OFFSET_Y - 55;
        int capSize = 35;
        int startX = Config::BOARD_OFFSET_X;
        int currentX = startX + 110;
        for (auto t : capturedWhite) {
            DrawChessPiece(t, P_WHITE, currentX, topY, capSize);
            currentX += 28;
        }
        if (blackTotalVal > whiteTotalVal) {
            DrawTextSmooth(TextFormat("+%d", blackTotalVal - whiteTotalVal), (float)currentX + 10, (float)topY + 10, 16.0f, Config::COLOR_LEAF_LIGHT);
        }

        int bottomY = Config::BOARD_OFFSET_Y + (Config::TILE_SIZE * 8) + 15;
        currentX = startX + 110;
        for (auto t : capturedBlack) {
            DrawChessPiece(t, P_BLACK, currentX, bottomY, capSize);
            currentX += 28;
        }
        if (whiteTotalVal > blackTotalVal) {
            DrawTextSmooth(TextFormat("+%d", whiteTotalVal - blackTotalVal), (float)currentX + 10, (float)bottomY + 10, 16.0f, Config::COLOR_LEAF_LIGHT);
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
        
        // Track mouse state
        Vector2 mousePos = GetMousePosition();
        bool anyMoveHovered = false;

        // Bounding box for the visible scroll region to prevent clicking text scrolled out of view
        Rectangle visibleScissorRect = { (float)logContainerX, (float)logContainerY + 10, (float)logW, (float)logH - 20 };

        for (size_t i = 0; i < totalPairs; ++i) {
            float yPos = logContainerY + 15 + (i * itemHeight) - scrollOffset;

            Color rowBg = (i % 2 == 0) ? Config::COLOR_UI_ROW_A : Config::COLOR_UI_ROW_B;
            DrawRectangle(logContainerX + 10, (int)yPos, logW - 35, (int)itemHeight - 3, rowBg);

            std::string stepStr = std::to_string(i + 1) + ".";
            DrawTextSmooth(stepStr.c_str(), (float)logContainerX + 25, yPos + 8.0f, 16.0f, Config::COLOR_UI_TEXT_DIM);

            // --- WHITE MOVE INTERACTION ---
            size_t wIndex = i * 2;
            std::string whiteMove = hist[wIndex].notation;
            
            // Define clickable boundary box for White's move column
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

            // --- BLACK MOVE INTERACTION ---
            size_t bIndex = wIndex + 1;
            if (bIndex < totalMoves) {
                std::string blackMove = hist[bIndex].notation;
                
                // Define clickable boundary box for Black's move column
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

    void DrawGameMetrics() {
        DrawRectangleRounded(Rectangle{ (float)Config::PANEL_X, (float)Config::PANEL_Y, (float)Config::PANEL_WIDTH, (float)Config::PANEL_HEIGHT }, 0.03f, 4, Config::COLOR_UI_PANEL_BG);
        DrawRectangleRoundedLinesCustom(Rectangle{ (float)Config::PANEL_X, (float)Config::PANEL_Y, (float)Config::PANEL_WIDTH, (float)Config::PANEL_HEIGHT }, 0.03f, 4, 3.0f, Config::COLOR_UI_BORDER);
        DrawRectangleRoundedLinesCustom(Rectangle{ (float)Config::PANEL_X + 6, (float)Config::PANEL_Y + 6, (float)Config::PANEL_WIDTH - 12, (float)Config::PANEL_HEIGHT - 12 }, 0.03f, 4, 1.0f, Config::COLOR_FRAME_DARK);

        DrawTextSmooth("Game Log", (float)Config::PANEL_X + 25, (float)Config::PANEL_Y + 25, 36.0f, Config::COLOR_UI_TEXT);

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
            char stats[128];
            snprintf(stats, sizeof(stats), "Halfmoves: %d / 100", g_ctx->board->halfmove_clock);
            DrawTextSmooth(stats, (float)Config::PANEL_X + 30, warning_info_y, 14.0f, Config::COLOR_UI_TEXT_DIM);

            if (repCount == 2) {
                DrawTextSmooth("[!] 3-Fold Warning (2x Same)", (float)Config::PANEL_X + Config::PANEL_WIDTH-200, warning_info_y, 14.0f, Config::COLOR_GEM_GLINT);
            }
        }

        g_ctx->btnFirst->Draw();
        g_ctx->btnPrev->Draw();
        g_ctx->btnNext->Draw();
        g_ctx->btnLast->Draw();

        g_ctx->btnResign->Draw();
        g_ctx->btnDraw->Draw();

    }

    void DrawChessboard() {
        BoardState displayState;
        if (g_ctx->historyView.useLive) {
            displayState = g_ctx->board->CaptureState();
        } else {
            int idx = g_ctx->historyView.viewingIndex;
            if (idx >= 0 && idx < (int)g_ctx->board->move_history.size()) {
                displayState = g_ctx->board->move_history[idx].board_state;
            } else {
                displayState = g_ctx->board->CaptureState();
            }
        }

        DrawCapturedTrays(displayState);

        std::pair<int, int> checkKingSquare = displayState.check_king_pos;
        int boardSize = Config::TILE_SIZE * 8;

        VectorRenderer::DrawBoardOrnateFrame(Config::BOARD_OFFSET_X, Config::BOARD_OFFSET_Y, boardSize);

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

        VectorRenderer::DrawOvergrownVines(Config::BOARD_OFFSET_X, Config::BOARD_OFFSET_Y, boardSize);

        if (g_ctx->board->has_selection && g_ctx->historyView.useLive) {
            for (const auto& move : g_ctx->cached_legal_moves) {
                int r = move.first;
                int c = move.second;
                int drawX = Config::BOARD_OFFSET_X + c * Config::TILE_SIZE + Config::TILE_SIZE / 2;
                int drawY = Config::BOARD_OFFSET_Y + r * Config::TILE_SIZE + Config::TILE_SIZE / 2;

                if (g_ctx->board->grid[r][c].has_piece) {
                    DrawCircleLines(drawX, drawY, (float)Config::TILE_SIZE * 0.4f, Config::COLOR_DOT_RING);
                    DrawCircleLines(drawX, drawY, (float)Config::TILE_SIZE * 0.38f, Config::COLOR_DOT_RING);
                } else {
                    DrawCircle(drawX, drawY, (float)Config::TILE_SIZE * 0.12f, Config::COLOR_DOT);
                }
            }
        }

        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                const auto& cell = displayState.grid[r][c];
                if (cell.has_piece) {
                    if (g_ctx->anim.active && g_ctx->anim.targetRow == r && g_ctx->anim.targetCol == c) {
                        continue;
                    }

                    int drawX = Config::BOARD_OFFSET_X + c * Config::TILE_SIZE;
                    int drawY = Config::BOARD_OFFSET_Y + r * Config::TILE_SIZE;
                    DrawChessPiece(cell.piece->type, cell.piece->color, drawX, drawY, Config::TILE_SIZE);
                }
            }
        }

        if (g_ctx->anim.active) {
            DrawChessPiece(
                g_ctx->anim.piece->type, 
                g_ctx->anim.piece->color, 
                (int)g_ctx->anim.currentPos.x, 
                (int)g_ctx->anim.currentPos.y, 
                Config::TILE_SIZE
            );
        }

        for (int i = 0; i < 8; ++i) {
            char fileStr[2] = { (char)('a' + i), '\0' };
            char rankStr[2] = { (char)('8' - i), '\0' };

            DrawTextSmooth(fileStr, (float)Config::BOARD_OFFSET_X + i * Config::TILE_SIZE + Config::TILE_SIZE-10, (float)Config::BOARD_OFFSET_Y + 8 * Config::TILE_SIZE - 17, 15.0f, Config::BOARD_MARKINGS_TEXT);
            DrawTextSmooth(rankStr, (float)Config::BOARD_OFFSET_X+3, (float)Config::BOARD_OFFSET_Y + i * Config::TILE_SIZE + 3, 15.0f,Config::BOARD_MARKINGS_TEXT);
        }

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
}


namespace TickEngine {

    void UpdateAnimations(float dt) {
        if (!g_ctx->anim.active) return;

        g_ctx->anim.elapsedTime += dt;
        float progress = g_ctx->anim.elapsedTime / Config::ANIMATION_DURATION;

        if (progress >= 1.0f) {
            g_ctx->anim.active = false;
            
            ChessEngine::ExecuteMove(
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
        g_ctx->mousePosition = GetMousePosition();

        g_ctx->btnResign->Update(g_ctx->mousePosition);
        g_ctx->btnDraw->Update(g_ctx->mousePosition);
        
        g_ctx->btnFirst->Update(g_ctx->mousePosition);
        g_ctx->btnPrev->Update(g_ctx->mousePosition);
        g_ctx->btnNext->Update(g_ctx->mousePosition);
        g_ctx->btnLast->Update(g_ctx->mousePosition);

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
                    ChessEngine::ExecuteMove(
                        *g_ctx->board, 
                        g_ctx->board->promotion_source.first, 
                        g_ctx->board->promotion_source.second, 
                        g_ctx->board->promotion_square.first, 
                        g_ctx->board->promotion_square.second, 
                        choice
                    );
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
    }
}


void UpdateDrawFrame() {
    float dt = GetFrameTime();

    TickEngine::UpdateAnimations(dt);
    TickEngine::ProcessInput();

    BeginTextureMode(g_ctx->targetScreen);
        ClearBackground(Color{ 18, 12, 10, 255 }); 
        CanvasRenderer::DrawChessboard();
        CanvasRenderer::DrawGameMetrics();
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
    InitWindow(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT, "CHESS ARCHITECTURE - PROCEDURAL WOOD & OVERGROWN VINES ENGINE");

    g_ctx = std::make_unique<GameContext>();
    g_ctx->ResetPromotionButtons();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    while (g_ctx->isGameRunning) {
        UpdateDrawFrame();
    }
    CloseWindow();
#endif

    return 0;
}