#include "ui.h"
#include <algorithm>

UI::UI(Terminal* t) : term(t), width(80), height(24) {
    grid.resize(height);
    for (auto& row : grid) {
        row.resize(width);
    }
}

void UI::resize(int w, int h) {
    width = w;
    height = h;
    grid.resize(height);
    for (auto& row : grid) {
        row.resize(width);
        for (auto& cell : row) {
            cell = {' ', 7, 0, false, false};
        }
    }
}

void UI::clear() {
    for (auto& row : grid) {
        for (auto& cell : row) {
            cell = {' ', 7, 0, false, false};
        }
    }
}

void UI::set_cell(int x, int y, const UICell& cell) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        grid[y][x] = cell;
    }
}

UICell UI::get_cell(int x, int y) const {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        return grid[y][x];
    }
    return {' ', 7, 0, false, false};
}

void UI::render() {
    term->clear();
    
    int last_fg = -1, last_bg = -1;
    bool last_bold = false, last_reverse = false;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const auto& cell = grid[y][x];
            
            if (cell.fg != last_fg || cell.bg != last_bg || 
                cell.bold != last_bold || cell.reverse != last_reverse) {
                term->reset_color();
                if (cell.bold) term->set_bold(true);
                if (cell.reverse) term->set_reverse(true);
                term->set_color(cell.fg, cell.bg);
                last_fg = cell.fg;
                last_bg = cell.bg;
                last_bold = cell.bold;
                last_reverse = cell.reverse;
            }
            
            term->move_cursor(x, y);
            term->write_char(cell.ch);
        }
    }
    
    term->reset_color();
    term->flush();
}

void UI::draw_text(int x, int y, const std::string& text, int fg, int bg, bool bold) {
    for (size_t i = 0; i < text.length() && x + (int)i < width; i++) {
        UICell cell;
        cell.ch = text[i];
        cell.fg = fg;
        cell.bg = bg;
        cell.bold = bold;
        cell.reverse = false;
        set_cell(x + i, y, cell);
    }
}

void UI::draw_rect(const UIRect& rect, int fg, int bg) {
    for (int y = rect.y; y < rect.y + rect.h && y < height; y++) {
        for (int x = rect.x; x < rect.x + rect.w && x < width; x++) {
            UICell cell;
            cell.ch = ' ';
            cell.fg = fg;
            cell.bg = bg;
            cell.bold = false;
            cell.reverse = false;
            set_cell(x, y, cell);
        }
    }
}

void UI::draw_border(const UIRect& rect, int fg, int bg) {
    for (int x = rect.x; x < rect.x + rect.w && x < width; x++) {
        if (x == rect.x || x == rect.x + rect.w - 1) {
            UICell cell;
            cell.ch = '|';
            cell.fg = fg;
            cell.bg = bg;
            set_cell(x, rect.y, cell);
            if (rect.y + rect.h - 1 < height) {
                set_cell(x, rect.y + rect.h - 1, cell);
            }
        }
    }
    for (int y = rect.y; y < rect.y + rect.h && y < height; y++) {
        UICell cell;
        cell.ch = '-';
        cell.fg = fg;
        cell.bg = bg;
        set_cell(rect.x, y, cell);
        if (rect.x + rect.w - 1 < width) {
            set_cell(rect.x + rect.w - 1, y, cell);
        }
    }
}

void UI::fill_rect(const UIRect& rect, char ch, int fg, int bg) {
    for (int y = rect.y; y < rect.y + rect.h && y < height; y++) {
        for (int x = rect.x; x < rect.x + rect.w && x < width; x++) {
            UICell cell;
            cell.ch = ch;
            cell.fg = fg;
            cell.bg = bg;
            set_cell(x, y, cell);
        }
    }
}

void UI::set_cursor(int x, int y) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        term->move_cursor(x, y);
        term->show_cursor();
        term->flush();
    }
}

void UI::hide_cursor() {
    term->hide_cursor();
    term->flush();
}

