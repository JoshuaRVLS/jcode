#ifndef UI_H
#define UI_H

#include <string>
#include <vector>
#include <functional>
#include "terminal.h"

struct UIRect {
    int x, y, w, h;
};

struct UICell {
    char ch;
    int fg;
    int bg;
    bool bold;
    bool reverse;
};

class UI {
private:
    Terminal* term;
    std::vector<std::vector<UICell>> grid;
    int width, height;
    
    void set_cell(int x, int y, const UICell& cell);
    UICell get_cell(int x, int y) const;
    
public:
    UI(Terminal* t);
    void resize(int w, int h);
    
    void clear();
    void render();
    
    void draw_text(int x, int y, const std::string& text, int fg = 7, int bg = 0, bool bold = false);
    void draw_rect(const UIRect& rect, int fg, int bg);
    void draw_border(const UIRect& rect, int fg, int bg);
    void fill_rect(const UIRect& rect, char ch, int fg, int bg);
    
    void set_cursor(int x, int y);
    void hide_cursor();
    
    int get_width() const { return width; }
    int get_height() const { return height; }
};

#endif

