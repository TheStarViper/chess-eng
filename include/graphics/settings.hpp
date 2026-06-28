#pragma once
#include "raylib.h"


void DrawSettingsCheckbox(const char* label, bool* variablePointer, int x, int y, Vector2 mousePos);
void DrawSettingsMenu(Vector2 mousePos);