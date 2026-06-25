#pragma once
#include <vector>
#include <string>
#include "variables.h"

void load_puzzles();
std::vector<std::string> SplitMoveString(const std::string& movesStr);
std::string ConvertToUci(int fromRow, int fromCol, int toRow, int toCol);
void do_the_puzzle_stuff(BoardState& displayState, float dt, Menus& last_menu);