#include "raylib.h"
#include "rlgl.h"
#include "main.hpp"
#include "leaves.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <map>
#include <utility>
#include <cstdlib>
#include <random>
#include <sstream>
#include <fstream>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif



#define P_WHITE "WHITE"
#define P_BLACK "BLACK"


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
    //Sounds
    Sound hoversound;
    BoardState savedGameState;
    BoardState savedPuzzleState;
    int puzzleMoveIndex = 0;
    bool puzzleFailed = false;
    bool puzzleSuccess = false;
    bool hasSavedGame = false;
    bool hasSavedPuzzle = false;
    std::vector<HistorySnapshot> savedGameHistory;
    std::vector<HistorySnapshot> savedPuzzleHistory;
    int lastHoveredLeafId = -1;
    int lastHoveredBoardPieceTile;
    int genuineHoveredLeafId = -1;  
    std::unique_ptr<ChessBoard> board;
    Vector2 mousePosition;
    int active_turn_id; 
    float move_log_scroll_ratio;
    bool isGameRunning;
    HistoryState historyView;
    PieceAnimation anim;
    Texture2D backgroundTexture;

    std::map<std::string, Texture2D> pieceSprites; 
    bool useSprites = false;
    
    Font uiFont;
    RenderTexture2D targetScreen; 

    //settings
    bool showMoveHighlights = true;
    bool showBoardCoordinates = true;
    bool fiftymovecounter = false;
    bool threefoldcounter = false;
    bool highcontrast = false;
    bool boardmarkings = true;

    float masterVolume = 0.75f;
    Menus active_menu;
    float sidebarWidth = (float)Config::SIDEBAR_MIN_WIDTH; 
    bool sidebarhovered;
    Puzzle cachedpuzzle;
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
        std::string bgPath = ResolveAssetPath("assets/images/bg.png");
        backgroundTexture = LoadTexture(bgPath.c_str());
        if (backgroundTexture.id == 0) {
            std::cout << "[ERROR] Failed to load background: " << bgPath << std::endl;
        }
        sidebarhovered = false;
        Menus active_menu = PLAY;
        board = std::make_unique<ChessBoard>();
        mousePosition = Vector2{ 0, 0 };
        active_turn_id = 0;
        move_log_scroll_ratio = 0.0f;
        isGameRunning = true;
        historyView = HistoryState{ true, 0 };
        anim = PieceAnimation{ false, nullptr, Vector2{0,0}, Vector2{0,0}, Vector2{0,0}, 0.0f, -1, -1 };
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

void DrawCollapsibleSidebar(Vector2 mousePos) {
    g_ctx->sidebarhovered = (mousePos.x <= g_ctx->sidebarWidth);
    
    float targetWidth = g_ctx->sidebarhovered ? Config::SIDEBAR_MAX_WIDTH : Config::SIDEBAR_MIN_WIDTH;
    g_ctx->sidebarWidth += (targetWidth - g_ctx->sidebarWidth) * Config::ANIMATION_DURATION*50 * GetFrameTime();

    int currentWidth = (int)g_ctx->sidebarWidth;
    DrawRectangle(0, 0, currentWidth, Config::WINDOW_HEIGHT, Config::COLOR_UI_PANEL_BG); 
    DrawRectangle(currentWidth - 2, 0, 2, Config::WINDOW_HEIGHT, Config::COLOR_LEAF_LIGHT);

    bool isFullyOpen = (currentWidth > Config::SIDEBAR_MAX_WIDTH - 20);
    bool canClickEarly = (g_ctx->sidebarWidth > (float)Config::SIDEBAR_MIN_WIDTH + 15);
    bool showText = (currentWidth > Config::SIDEBAR_MAX_WIDTH - 40);

    int startY = Config::WINDOW_HEIGHT / 4; 
    int itemSpacing = 60;
    int btnHeight = 45;
    int iconPadding = 15;
    struct MenuItem { Menus mode; const char* icon; const char* label; };
    std::vector<MenuItem> items = {
        { PLAY, " P ", "PLAY"},
        { GAME, " G ", "GAME" },
        { OPENINGS, " O ", "OPENINGS" },
        { PUZZLES, " X ", "PUZZLES" }
    };

    for (size_t i = 0; i < items.size(); ++i) {
        int itemY = startY + (i * itemSpacing);
        Rectangle btnRec = { 5, (float)itemY, (float)(currentWidth - 10), (float)btnHeight };
        
        bool isItemHovered = CheckCollisionPointRec(mousePos, btnRec);
        Color btnColor = (g_ctx->active_menu == items[i].mode) ? Fade(GOLD, 0.3f) :
                        (isItemHovered && g_ctx->sidebarhovered) ? Fade(WHITE, 0.1f) : BLANK;

        DrawRectangleRec(btnRec, btnColor);

        DrawText(items[i].icon, iconPadding, itemY + 12, 20, RAYWHITE);
        if (showText) {
            DrawText(items[i].label, 50, itemY + 14, 16, RAYWHITE);
        }
        if (isItemHovered && canClickEarly) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                g_ctx->active_menu = items[i].mode;
            }
        }
    }
    int bottomY = Config::WINDOW_HEIGHT - 70;
    Rectangle settingsRec = { 5, (float)bottomY, (float)(currentWidth - 10), (float)btnHeight };
    
    bool isSettingsHovered = CheckCollisionPointRec(mousePos, settingsRec);
    Color settingsBtnColor = (g_ctx->active_menu == SETTINGS) ? Fade(GOLD, 0.3f) : 
                             (isSettingsHovered && g_ctx->sidebarhovered) ? Fade(WHITE, 0.1f) : BLANK;

    DrawRectangleRec(settingsRec, settingsBtnColor);
    DrawText(" S ", iconPadding, bottomY + 12, 20, RAYWHITE);

    if (isFullyOpen) {
        DrawText("SETTINGS", 50, bottomY + 14, 16, RAYWHITE);
        if (isSettingsHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            g_ctx->active_menu = SETTINGS;
        }
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

    void DrawOvergrownVines(int boardX, int boardY, float boardSize, int id) {
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

        float boardLeaves[][4] = {
            // --- Board Top Grouping (Left-to-mid) ---
            { (float)boardX + 75,  (float)boardY - 6,  14, -15 },
            { (float)boardX + 95,  (float)boardY - 12, 17, 10 },
            { (float)boardX + 115, (float)boardY - 5,  13, 35 },
            { (float)boardX + 135, (float)boardY - 9,  15, -10 },
            { (float)boardX + 160, (float)boardY - 6,  12, 45 },

            // --- Board Top Grouping (Spread across mid-to-right) ---
            { (float)boardX + 380, (float)boardY - 10, 15, -25 },
            { (float)boardX + 405, (float)boardY - 6,  13, 5 },
            { (float)boardX + 430, (float)boardY - 13, 18, 20 },
            { (float)boardX + 455, (float)boardY - 5,  14, -15 },
            { (float)boardX + 480, (float)boardY + 2,  16, 55 },

            // --- Board Bottom Grouping (Spread left-to-mid) ---
            { (float)boardX + 65,  (float)boardY + boardSize + 6,  14, 160 },
            { (float)boardX + 90,  (float)boardY + boardSize + 12, 18, 195 },
            { (float)boardX + 115, (float)boardY + boardSize + 4,  13, 140 },
            { (float)boardX + 140, (float)boardY + boardSize + 9,  15, 175 },

            // --- Board Bottom Grouping (Spread mid-to-right) ---
            { (float)boardX + 420, (float)boardY + boardSize + 5,  13, 150 },
            { (float)boardX + 445, (float)boardY + boardSize + 11, 17, 215 },
            { (float)boardX + 470, (float)boardY + boardSize + 4,  14, 135 },
            { (float)boardX + 495, (float)boardY + boardSize + 8,  16, 185 },

            // --- Board Left Side Grouping (Spread vertically down) ---
            { (float)boardX - 6,   (float)boardY + 210, 14, -75 },
            { (float)boardX - 10,  (float)boardY + 235, 16, -100 },
            { (float)boardX - 13,  (float)boardY + 260, 18, -120 },
            { (float)boardX - 8,   (float)boardY + 285, 13, -60 },
            { (float)boardX - 5,   (float)boardY + 310, 15, -85 },

            // --- Board Right Side Grouping (Spread vertically down) ---
            { (float)boardX + boardSize + 6,  (float)boardY + 310, 13, 65 },
            { (float)boardX + boardSize + 11, (float)boardY + 335, 17, 90 },
            { (float)boardX + boardSize + 14, (float)boardY + 360, 19, 120 },
            { (float)boardX + boardSize + 9,  (float)boardY + 385, 14, 55 },
            { (float)boardX + boardSize + 7,  (float)boardY + 410, 15, 100 }
        };

        float panelLeaves[][4] = {
            // --- Move Log Top Border (Spanning left half) ---
            { (float)Config::PANEL_X + 10,  (float)Config::PANEL_Y + 6, 13, -45 },
            { (float)Config::PANEL_X + 30,  (float)Config::PANEL_Y + 1, 16, 10 },
            { (float)Config::PANEL_X + 50,  (float)Config::PANEL_Y + 4, 12, -20 },
            { (float)Config::PANEL_X + 70,  (float)Config::PANEL_Y + 1, 15, 30 },
            { (float)Config::PANEL_X + 90,  (float)Config::PANEL_Y + 5, 13, -10 },

            // --- Move Log Top Border (Spanning right half) ---
            { (float)Config::PANEL_X + Config::PANEL_WIDTH - 100, (float)Config::PANEL_Y + 4, 14, -15 },
            { (float)Config::PANEL_X + Config::PANEL_WIDTH - 80,  (float)Config::PANEL_Y + 1, 15, 35 },
            { (float)Config::PANEL_X + Config::PANEL_WIDTH - 60,  (float)Config::PANEL_Y + 6, 12, 10 },
            { (float)Config::PANEL_X + Config::PANEL_WIDTH - 40,  (float)Config::PANEL_Y + 2, 17, 75 },
            { (float)Config::PANEL_X + Config::PANEL_WIDTH - 15,  (float)Config::PANEL_Y + 8, 13, 115 },

            // --- Move Log Left Side (Cascading down the edge) ---
            { (float)Config::PANEL_X + 3,  (float)Config::PANEL_Y + 120, 13, -80 },
            { (float)Config::PANEL_X + 2,  (float)Config::PANEL_Y + 145, 16, -110 },
            { (float)Config::PANEL_X + 4,  (float)Config::PANEL_Y + 170, 15, -70 },
            { (float)Config::PANEL_X + 2,  (float)Config::PANEL_Y + 195, 14, -95 },
            { (float)Config::PANEL_X + 1,  (float)Config::PANEL_Y + 220, 12, -120 },

            // --- Move Log Right Side (Cascading down the edge) ---
            { (float)Config::PANEL_X + Config::PANEL_WIDTH + 1, (float)Config::PANEL_Y + 140, 12, 60 },
            { (float)Config::PANEL_X + Config::PANEL_WIDTH + 1, (float)Config::PANEL_Y + 165, 15, 95 },
            { (float)Config::PANEL_X + Config::PANEL_WIDTH + 2, (float)Config::PANEL_Y + 190, 17, 115 },
            { (float)Config::PANEL_X + Config::PANEL_WIDTH + 1, (float)Config::PANEL_Y + 215, 13, 50 },
            { (float)Config::PANEL_X + Config::PANEL_WIDTH + 2, (float)Config::PANEL_Y + 240, 14, 85 },

            // --- Move Log Bottom Border (Spanning along the base) ---
            { (float)Config::PANEL_X + 12, (float)Config::PANEL_Y + Config::PANEL_HEIGHT + 3, 14, -135 },
            { (float)Config::PANEL_X + 37, (float)Config::PANEL_Y + Config::PANEL_HEIGHT + 2, 16, 185 },
            { (float)Config::PANEL_X + 62, (float)Config::PANEL_Y + Config::PANEL_HEIGHT + 5, 13, 150 },
            
            { (float)Config::PANEL_X + Config::PANEL_WIDTH - 75, (float)Config::PANEL_Y + Config::PANEL_HEIGHT + 2, 14, 210 },
            { (float)Config::PANEL_X + Config::PANEL_WIDTH - 50, (float)Config::PANEL_Y + Config::PANEL_HEIGHT + 3, 15, 145 },
            { (float)Config::PANEL_X + Config::PANEL_WIDTH - 20, (float)Config::PANEL_Y + Config::PANEL_HEIGHT + 1, 13, 70 }
        };
        switch (id){
            case 1:
            for (const auto& leaf : boardLeaves) {
                UpdateAndDrawLeaf((float)leaf[0], (float)leaf[1], (float)leaf[2], (float)leaf[3]);
            }
            break;
        case 2:
            for (const auto& leaf : panelLeaves) {
                UpdateAndDrawLeaf((float)leaf[0], (float)leaf[1], (float)leaf[2], (float)leaf[3]);
            }
            break;
        }
        

        
        
        
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
        VectorRenderer::DrawOvergrownVines(Config::BOARD_OFFSET_X, Config::BOARD_OFFSET_Y, Config::TILE_SIZE*8 ,2);
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

    void draw_legal_moves(){
        if (g_ctx->board->has_selection && g_ctx->historyView.useLive) {
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

                bool isHovered = CheckCollisionPointRec(mousePos, tileRect);

                if (g_ctx->board->grid[r][c].has_piece) {
                    float baseRadius1 = (float)Config::TILE_SIZE * 0.4f;
                    float baseRadius2 = (float)Config::TILE_SIZE * 0.38f;
                    
                    if (isHovered) {
                        baseRadius1 += 4.0f; 
                        baseRadius2 += 2.0f;
                    }

                    DrawCircleLines(drawX, drawY, baseRadius1, Config::COLOR_DOT_RING);
                    DrawCircleLines(drawX, drawY, baseRadius2, Config::COLOR_DOT_RING);
                } else {
                    float radius = (float)Config::TILE_SIZE * 0.12f;
                    if (isHovered) {
                        radius = (float)Config::TILE_SIZE * 0.17f; 
                    }

                    DrawCircle(drawX, drawY, radius, Config::COLOR_DOT);
                }
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

    void draw_game_over(){
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
        int targetCursor = MOUSE_CURSOR_DEFAULT;
        int gridX = (int)((g_ctx->mousePosition.x - Config::BOARD_OFFSET_X) / Config::TILE_SIZE);
        int gridY = (int)((g_ctx->mousePosition.y - Config::BOARD_OFFSET_Y) / Config::TILE_SIZE);
        bool isMouseOnBoard = (gridX >= 0 && gridX < 8 && gridY >= 0 && gridY < 8);
        
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

            g_ctx->isGameRunning = true;                
            g_ctx->btnOverlayRematch->isPressed = false; 
            
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

                    if (g_ctx->active_menu == PUZZLES && !g_ctx->puzzleSuccess && !g_ctx->puzzleFailed) {
                        std::vector<std::string> solutionMoves = SplitMoveString(g_ctx->cachedpuzzle.solution);
                        
                        std::string playerMoveUci = ConvertToUci(g_ctx->board->promotion_source.first, g_ctx->board->promotion_source.second, g_ctx->board->promotion_square.first, g_ctx->board->promotion_square.second);
                        if (choice == QUEEN) playerMoveUci += "q";
                        else if (choice == ROOK) playerMoveUci += "r";
                        else if (choice == BISHOP) playerMoveUci += "b";
                        else if (choice == KNIGHT) playerMoveUci += "n";

                        if (g_ctx->puzzleMoveIndex < (int)solutionMoves.size() && playerMoveUci == solutionMoves[g_ctx->puzzleMoveIndex]) {
                            g_ctx->puzzleMoveIndex++;
                            if (g_ctx->puzzleMoveIndex >= (int)solutionMoves.size()) {
                                g_ctx->puzzleSuccess = true;
                            } else {
                                std::string opponentMoveUci = solutionMoves[g_ctx->puzzleMoveIndex];
                                int opFromCol = opponentMoveUci[0] - 'a', opFromRow = '8' - opponentMoveUci[1];
                                int opToCol   = opponentMoveUci[2] - 'a', opToRow   = '8' - opponentMoveUci[3];
                                g_ctx->board->grid[opToRow][opToCol] = g_ctx->board->grid[opFromRow][opFromCol];
                                g_ctx->board->grid[opFromRow][opFromCol] = GridData();
                                g_ctx->board->turn = (g_ctx->board->turn == P_WHITE) ? P_BLACK : P_WHITE;
                                g_ctx->puzzleMoveIndex++;
                            }
                        } else {
                            g_ctx->puzzleFailed = true;
                            g_ctx->board->LoadState(g_ctx->savedPuzzleState); 
                        }
                    }

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
                        if (g_ctx->active_menu == PUZZLES && !g_ctx->puzzleSuccess && !g_ctx->puzzleFailed) {
                            std::vector<std::string> solutionMoves = SplitMoveString(g_ctx->cachedpuzzle.solution);
                            std::string playerMoveUci = ConvertToUci(srcR, srcC, gridY, gridX);
                            
                            if (g_ctx->puzzleMoveIndex < (int)solutionMoves.size() && playerMoveUci == solutionMoves[g_ctx->puzzleMoveIndex]) {
                                g_ctx->puzzleMoveIndex++;
                                
                                if (g_ctx->puzzleMoveIndex >= (int)solutionMoves.size()) {
                                    g_ctx->puzzleSuccess = true;
                                } else {
                                    std::string opponentMoveUci = solutionMoves[g_ctx->puzzleMoveIndex];
                                    int opFromCol = opponentMoveUci[0] - 'a', opFromRow = '8' - opponentMoveUci[1];
                                    int opToCol   = opponentMoveUci[2] - 'a', opToRow   = '8' - opponentMoveUci[3];
                                    
                                    g_ctx->board->grid[opToRow][opToCol] = g_ctx->board->grid[opFromRow][opFromCol];
                                    g_ctx->board->grid[opFromRow][opFromCol] = GridData();
                                    
                                    g_ctx->board->turn = (g_ctx->board->turn == P_WHITE) ? P_BLACK : P_WHITE;
                                    g_ctx->puzzleMoveIndex++;
                                }
                            } else {
                                g_ctx->puzzleFailed = true;
                                g_ctx->board->LoadState(g_ctx->savedPuzzleState); 
                            }
                        }
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
        SetMouseCursor(targetCursor);
    }
}

void DrawSettingsCheckbox(const char* label, bool* variablePointer, int x, int y, Vector2 mousePos) {
    DrawTextSmooth(label, x, y, 20.0f, Color{ 215, 195, 140, 255 });

    int boxX = x + 350; 
    Rectangle boxRec = { (float)boxX, (float)y - 4, 28.0f, 28.0f };
    bool isHovered = CheckCollisionPointRec(mousePos, boxRec);

    DrawRectangleRec(boxRec, isHovered ? Color{ 45, 45, 48, 255 } : Color{ 24, 24, 26, 255 });
    DrawRectangleLinesEx(boxRec, 2, isHovered ? GOLD : Color{ 80, 75, 70, 255 });

    if (*variablePointer == true) {
        DrawRectangle(boxRec.x + 6, boxRec.y + 6, 16, 16, GOLD);
    }

    if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        *variablePointer = !(*variablePointer); 
    }
}

void DrawSettingsMenu(Vector2 mousePos) {

    int paneX = 200;
    int paneY = 80;
    int paneWidth = Config::WINDOW_WIDTH - 300;
    int paneHeight = Config::WINDOW_HEIGHT - 160;

    DrawRectangle(paneX, paneY, paneWidth, paneHeight, Config::COLOR_UI_PANEL_BG);
    DrawRectangleLinesEx(Rectangle{ (float)paneX, (float)paneY, (float)paneWidth, (float)paneHeight }, 3, Config::COLOR_LEAF_LIGHT);
    
    DrawTextSmooth("SETTINGS", paneX + 40, paneY + 30, 32.0f, Color{ 215, 195, 140, 255 });
    DrawLineEx(Vector2{ (float)paneX + 40, (float)paneY + 75 }, Vector2{ (float)paneX + paneWidth - 40, (float)paneY + 75 }, 2, Config::COLOR_LEAF_LIGHT);

    int startY = paneY + 110;
    int labelX = paneX + 60;
    int labelXright = paneX +60+ paneWidth/2;
    int controlX = paneX + 400;

    float minVal = 0.0f;
    float maxVal = 1.0f;
    
    DrawTextSmooth("Master Volume", labelX, startY, 20.0f, RAYWHITE);
    
    int sliderWidth = 300;
    int sliderHeight = 10;
    Rectangle sliderBar = { (float)controlX, (float)startY + 6, (float)sliderWidth, (float)sliderHeight };
    DrawRectangleRec(sliderBar, Color{ 45, 45, 48, 255 }); 
    DrawRectangleLinesEx(sliderBar, 1, Color{ 90, 85, 80, 255 });

    float currentPercentage = (g_ctx->masterVolume - minVal) / (maxVal - minVal);
    int handleX = sliderBar.x + (currentPercentage * sliderWidth);
    Rectangle sliderHandle = { (float)handleX - 8, (float)startY, 16, 22 };

    DrawRectangle(sliderBar.x, sliderBar.y, handleX - sliderBar.x, sliderHeight, Color{ 180, 160, 110, 255 });

    bool sliderHover = CheckCollisionPointRec(mousePos, sliderBar) || CheckCollisionPointRec(mousePos, sliderHandle);
    DrawRectangleRec(sliderHandle, sliderHover ? Color{ 230, 210, 160, 255 } : Color{ 140, 125, 95, 255 });

    if (sliderHover && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float mouseXRelative = mousePos.x - sliderBar.x;
        if (mouseXRelative < 0) mouseXRelative = 0;
        if (mouseXRelative > sliderWidth) mouseXRelative = sliderWidth;
        
        g_ctx->masterVolume = minVal + (mouseXRelative / sliderWidth) * (maxVal - minVal);
    }

    DrawTextSmooth(TextFormat("%d%%", (int)(g_ctx->masterVolume * 100)), controlX + sliderWidth + 25, startY, 18.0f, Color{ 180, 160, 110, 255 });

    int row2Y = startY + 50;
    DrawSettingsCheckbox("High Contrast (unimplemented)", &g_ctx->highcontrast, labelX, row2Y, mousePos);
    DrawSettingsCheckbox("Board Markings", &g_ctx->boardmarkings, labelXright, row2Y, mousePos);
    std::vector<Color> availableColors = { MAROON, LIME, DARKBLUE, ORANGE, PURPLE };
    int boxSize = 35;
    int boxSpacing = 15;

    int row3Y = row2Y + 50;
    DrawSettingsCheckbox("50 Move Counter", &g_ctx->fiftymovecounter, labelX, row3Y, mousePos);
    DrawSettingsCheckbox("Three Fold Counter", &g_ctx->threefoldcounter, labelXright, row3Y, mousePos);
    int row4Y = row3Y + 50;
    DrawSettingsCheckbox("placeholder", &g_ctx->highcontrast, labelXright, row4Y, mousePos);
    DrawSettingsCheckbox("placeholder", &g_ctx->highcontrast, labelXright, row4Y, mousePos);

    int btnY = paneY + paneHeight - 75;
    Rectangle backBtn = { (float)(paneX + (paneWidth / 2) - 100), (float)btnY, 200.0f, 45.0f };
    bool backHover = CheckCollisionPointRec(mousePos, backBtn);

    DrawRectangleRec(backBtn, backHover ? Color{ 42, 40, 38, 255 } : Color{ 34, 32, 30, 255 });
    DrawRectangleLinesEx(backBtn, 2, backHover ? GOLD : Color{ 100, 95, 90, 255 });
    
    DrawTextSmooth("CONFIRM", backBtn.x + (backBtn.width / 2) - 40, backBtn.y + 12, 18.0f, backHover ? GOLD : Color{ 215, 195, 140, 255 });

    if (backHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        g_ctx->active_menu = GAME; 
    }
}


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


void UpdateDrawFrame() { // rendering
    float dt = GetFrameTime();
    Vector2 rawMousePos = GetMousePosition();
    Vector2 mousePos = {
        rawMousePos.x * ((float)Config::WINDOW_WIDTH / (float)GetScreenWidth()),
        rawMousePos.y * ((float)Config::WINDOW_HEIGHT / (float)GetScreenHeight())
    };

    if (!g_puzzlesLoaded) {
        load_puzzles();
        g_puzzlesLoaded = true;
    }
    static Menus last_menu = PLAY; 
    BoardState puzzleState;

    if (load_new_puzzle) {
        g_ctx->cachedpuzzle = get_random_puzzle();
        g_ctx->savedPuzzleState = ParseFenToState(g_ctx->cachedpuzzle.boardsetup);
        g_ctx->savedPuzzleHistory.clear(); 
        g_ctx->hasSavedPuzzle = true;
        
        g_ctx->puzzleMoveIndex = 0;
        g_ctx->puzzleFailed = false;
        g_ctx->puzzleSuccess = false;

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
    BoardState displayState;
    displayState = g_ctx->board->CaptureState();
    g_ctx->active_turn_id = (g_ctx->board->turn == P_WHITE) ? 0 : 1;



    if (audio_loaded && IsKeyPressed(KEY_B)) {
        
        #if defined(PLATFORM_WEB)
        if (!IsAudioDeviceReady()) {
            InitAudioDevice(); 
        }
        #endif
        
        PlaySound(g_ctx->hoversound);
        TraceLog(LOG_INFO, "AUDIO: PlaySound executed for KEY_B");
    }

    TickEngine::UpdateAnimations(dt);
    if ((g_ctx->active_menu == GAME || g_ctx->active_menu == PUZZLES) && mousePos.x > g_ctx->sidebarWidth) {
        TickEngine::ProcessInput();
    }
    
    BeginTextureMode(g_ctx->targetScreen);
    ClearBackground(Color{ 18, 12, 10, 255 });
    DrawTexturePro(
    g_ctx->backgroundTexture,
    Rectangle{ 0, 0, (float)g_ctx->backgroundTexture.width, (float)g_ctx->backgroundTexture.height },
    Rectangle{ 0, 0, (float)Config::WINDOW_WIDTH, (float)Config::WINDOW_HEIGHT },
    Vector2{ 0, 0 },
    0.0f,
    Color{ 255, 255, 255, 90}
    );
    
    
    
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
    switch (g_ctx->active_menu) {
        case PLAY:
            DrawRectangle(0, 0, Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT, Fade(BLACK, 0.6f));
            DrawTextSmooth("TO BE ADDED", 250.0f, 200.0f, 32.0f, RAYWHITE);
            break;
        case SETTINGS:
            DrawRectangle(0, 0, Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT, Fade(BLACK, 0.2f));
            DrawSettingsMenu(mousePos);
            break;
        case PUZZLES:
            CanvasRenderer::DrawChessboard(displayState);
            if (g_ctx->puzzleSuccess) {
                DrawRectangle(0, 0, Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT, Fade(GREEN, 0.3f));
                DrawTextSmooth("SUCCESS! PUZZLE SOLVED", 300.0f, 400.0f, 40.0f, GREEN);
            } 
            else if (g_ctx->puzzleFailed) {
                DrawRectangle(0, 0, Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT, Fade(RED, 0.15f));
                DrawTextSmooth("WRONG MOVE. TRY AGAIN!", 300.0f, 400.0f, 40.0f, RED);
            }
            break;
        case OPENINGS:
            DrawRectangle(0, 0, Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT, Fade(BLACK, 0.6f));
            DrawTextSmooth("TO BE ADDED", 250.0f, 200.0f, 32.0f, RAYWHITE);
            break;
        case GAME:
            CanvasRenderer::DrawGameMetrics();
            CanvasRenderer::DrawChessboard(displayState);
            break;
        default:
            break;
    }
    DrawCollapsibleSidebar(mousePos); 
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
    InitWindow(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT, "chess");
    InitAudioDevice();
    g_ctx = std::make_unique<GameContext>();
    g_ctx->ResetPromotionButtons();
    if (IsAudioDeviceReady()) {
        g_ctx->hoversound = LoadSound(hoversoundfilepath);
        audio_loaded= true;
        TraceLog(LOG_INFO, "AUDIO: ZA BLUETOOTH DEWICE HAS BEEN CONNECTED");
    } 
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    while (g_ctx->isGameRunning) {
        UpdateDrawFrame();
    }
    //unload sounds but idk how many i will have so if you run this on pc youre cooked
    CloseAudioDevice();
    CloseWindow();
#endif

    return 0;
}

