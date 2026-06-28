#include "main.hpp"
#include "vec_renderer.hpp"

void draw_play_screen(){
    DrawRectangleRounded(Rectangle{ (float)Config::BOARD_OFFSET_X+25, (float)Config::PANEL_Y, (float)Config::PANEL_WIDTH*2.5, (float)720-Config::PANEL_Y*2 }, 0.03f, 4, Color{ 24, 14, 8, 255 });
    DrawRectangleRoundedLinesCustom(Rectangle{ (float)Config::BOARD_OFFSET_X+25, (float)Config::PANEL_Y, (float)Config::PANEL_WIDTH*2.5, (float)720-Config::PANEL_Y*2 }, 0.03f, 4, 1.5f, Config::COLOR_FRAME_DARK);

}