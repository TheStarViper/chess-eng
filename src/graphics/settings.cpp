#include "settings.hpp"
#include "main.hpp"
#include "raylib.h"
#include "variables.h"

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
    DrawSettingsCheckbox("High Contrast (unimplemented)", &Config::highcontrast, labelX, row2Y, mousePos);
    DrawSettingsCheckbox("Board Markings", &Config::boardmarkings, labelXright, row2Y, mousePos);
    std::vector<Color> availableColors = { MAROON, LIME, DARKBLUE, ORANGE, PURPLE };
    int boxSize = 35;
    int boxSpacing = 15;

    int row3Y = row2Y + 50;
    DrawSettingsCheckbox("50 Move Counter", &Config::fiftymovecounter, labelX, row3Y, mousePos);
    DrawSettingsCheckbox("Three Fold Counter", &Config::threefoldcounter, labelXright, row3Y, mousePos);
    int row4Y = row3Y + 50;
    DrawSettingsCheckbox("placeholder", &Config::highcontrast, labelXright, row4Y, mousePos);
    DrawSettingsCheckbox("placeholder", &Config::highcontrast, labelXright, row4Y, mousePos);

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