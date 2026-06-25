#include "variables.h"
#include "main.hpp"

std::unique_ptr<GameContext> g_ctx = nullptr;

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



GameContext::GameContext() {
    std::string bgPath = "assets/images/bg.png";
    backgroundTexture = LoadTexture(bgPath.c_str());
    if (backgroundTexture.id == 0) {
        std::cout << "[ERROR] Failed to load background: " << bgPath << std::endl;
    }
    sidebarhovered = false;
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
    
    btnpuzzlehint = std::make_unique<Button>(Rectangle{ (float)Config::PANEL_X + 25, (float)Config::PANEL_Y + 500, (Config::PANEL_WIDTH-50), 40 }, "Hint", Color{ 55, 60, 45, 255 }, Color{ 75, 80, 55, 255 }, Config::COLOR_UI_TEXT, 2);
    btnpuzzleretry = std::make_unique<Button>(Rectangle{ (float)Config::PANEL_X + 25, (float)Config::PANEL_Y + 550, (Config::PANEL_WIDTH-50)/2-10, 40 }, "Retry", Color{ 120, 35, 35, 255 }, Color{ 150, 45, 45, 255 }, Config::COLOR_UI_TEXT, 1);
    btnpuzzlenext = std::make_unique<Button>(Rectangle{ (float)Config::PANEL_X + (Config::PANEL_WIDTH-50)/2+25+10, (float)Config::PANEL_Y + 550, (Config::PANEL_WIDTH-50)/2-10, 40 }, "Next Puzzle", Color{ 55, 60, 45, 255 }, Color{ 75, 80, 55, 255 }, Config::COLOR_UI_TEXT, 2);
    
    float overlayCenterX = Config::BOARD_OFFSET_X + (Config::TILE_SIZE * 8) / 2.0f;
    btnOverlayRematch = std::make_unique<Button>(Rectangle{ overlayCenterX - 125, Config::BOARD_OFFSET_Y + (Config::TILE_SIZE * 8) / 2.0f + 25, 250, 50 }, "REMATCH", Config::COLOR_LEAF_DARK, Config::COLOR_LEAF_LIGHT, Config::COLOR_UI_TEXT);

    ResetPromotionButtons();
}


void GameContext::ResetPromotionButtons() {
    btnPromotionTrays.clear();
    float btnW = 90;
    float startX = Config::BOARD_OFFSET_X + 4 * Config::TILE_SIZE - (btnW * 4)/2.0f;
    float startY = Config::BOARD_OFFSET_Y + 4 * Config::TILE_SIZE - 25;

    btnPromotionTrays.push_back(std::make_unique<Button>(Rectangle{ startX, startY, btnW, 50 }, "QUEEN", Config::COLOR_UI_BUTTON, Config::COLOR_UI_BUTTON_HOV, Config::COLOR_UI_TEXT));
    btnPromotionTrays.push_back(std::make_unique<Button>(Rectangle{ startX + btnW, startY, btnW, 50 }, "ROOK", Config::COLOR_UI_BUTTON, Config::COLOR_UI_BUTTON_HOV, Config::COLOR_UI_TEXT));
    btnPromotionTrays.push_back(std::make_unique<Button>(Rectangle{ startX + btnW * 2, startY, btnW, 50 }, "BISHOP", Config::COLOR_UI_BUTTON, Config::COLOR_UI_BUTTON_HOV, Config::COLOR_UI_TEXT));
    btnPromotionTrays.push_back(std::make_unique<Button>(Rectangle{ startX + btnW * 3, startY, btnW, 50 }, "KNIGHT", Config::COLOR_UI_BUTTON, Config::COLOR_UI_BUTTON_HOV, Config::COLOR_UI_TEXT));
}
