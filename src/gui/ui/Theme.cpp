/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Modern flat raygui theme implementation
*/

#include "gui/ui/Theme.hpp"

#include <raylib.h>

#include "raygui.h"

namespace zappy::gui::ui {

namespace {

constexpr Color PANEL_BG = {29, 26, 31, 255};
constexpr Color TEXT_MAIN = {10, 132, 255, 255};
constexpr Color TEXT_MUTED = {251, 251, 255, 255};
constexpr Color INPUT_BASE = {28, 28, 30, 255};
constexpr Color BORDER = {72, 72, 74, 255};
constexpr Color LINE = {10, 132, 255, 255};
constexpr Color ACCENT = {58, 58, 60, 255};
constexpr Color ACCENT_PRESSED = {0, 96, 223, 255};
constexpr Color ACCENT_SOFT = {232, 241, 255, 255};
constexpr Color BAR_BG = {58, 58, 60, 255};
constexpr Color WHITE_TXT = {255, 255, 255, 255};

int col(Color c)
{
    return static_cast<int>(ColorToInt(c));
}

void apply_base_palette()
{
    GuiSetStyle(DEFAULT, BORDER_WIDTH, 1);
    GuiSetStyle(DEFAULT, TEXT_PADDING, 6);
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, col(BORDER));
    GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, col(INPUT_BASE));
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, col(TEXT_MAIN));
    GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, col(ACCENT));
    GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, col(ACCENT_SOFT));
    GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, col(TEXT_MAIN));
    GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, col(ACCENT));
    GuiSetStyle(DEFAULT, BASE_COLOR_PRESSED, col(ACCENT_SOFT));
    GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, col(ACCENT_PRESSED));
    GuiSetStyle(DEFAULT, BORDER_COLOR_DISABLED, col(BORDER));
    GuiSetStyle(DEFAULT, BASE_COLOR_DISABLED, col(PANEL_BG));
    GuiSetStyle(DEFAULT, TEXT_COLOR_DISABLED, col(TEXT_MUTED));
}

void apply_buttons()
{
    GuiSetStyle(BUTTON, BORDER_WIDTH, 1);
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, col(BORDER));
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, col(INPUT_BASE));
    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, col(TEXT_MAIN));
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, col(ACCENT));
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, col(ACCENT));
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, col(WHITE_TXT));
    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, col(ACCENT_PRESSED));
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, col(ACCENT_PRESSED));
    GuiSetStyle(BUTTON, TEXT_COLOR_PRESSED, col(WHITE_TXT));
}

} // namespace

void apply_modern_theme()
{
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 1);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, col(PANEL_BG));
    GuiSetStyle(DEFAULT, LINE_COLOR, col(LINE));
    apply_base_palette();
    apply_buttons();

    GuiSetStyle(STATUSBAR, BASE_COLOR_NORMAL, col(BAR_BG));
    GuiSetStyle(STATUSBAR, TEXT_COLOR_NORMAL, col(TEXT_MAIN));
    GuiSetStyle(STATUSBAR, BORDER_COLOR_NORMAL, col(BORDER));
    GuiSetStyle(STATUSBAR, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    // Text inputs and labels read better left-aligned.
    GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, col(TEXT_MAIN));
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
}

} // namespace zappy::gui::ui
