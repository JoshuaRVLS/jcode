#include "editor.h"
#include <cctype>

void Editor::move_cursor(int dx, int dy, bool extend_selection) {
    auto& buf = get_buffer();
    
    if (!extend_selection && buf.selection.active) {
        clear_selection();
    }
    
    if (extend_selection && !buf.selection.active) {
        buf.selection.start = buf.cursor;
        buf.selection.active = true;
    }
    
    buf.cursor.y = std::max(0, std::min((int)buf.lines.size() - 1, buf.cursor.y + dy));
    buf.cursor.x = std::max(0, buf.cursor.x + dx);
    clamp_cursor(get_pane().buffer_id);
    
    if (extend_selection) {
        buf.selection.end = buf.cursor;
    }
    needs_redraw = true;
}

void Editor::clamp_cursor(int buffer_id) {
    auto& buf = get_buffer(buffer_id);
    if (buf.cursor.y >= buf.lines.size()) buf.cursor.y = buf.lines.size() - 1;
    if (buf.cursor.y < 0) buf.cursor.y = 0;
    int line_len = buf.lines[buf.cursor.y].length();
    buf.cursor.x = std::max(0, std::min(line_len, buf.cursor.x));
}

void Editor::move_word_forward(bool extend_selection) {
    auto& buf = get_buffer();
    int len = buf.lines[buf.cursor.y].length();
    while (buf.cursor.x < len && !std::isalnum(buf.lines[buf.cursor.y][buf.cursor.x])) buf.cursor.x++;
    while (buf.cursor.x < len && std::isalnum(buf.lines[buf.cursor.y][buf.cursor.x])) buf.cursor.x++;
    clamp_cursor(get_pane().buffer_id);
    if (extend_selection) {
        if (!buf.selection.active) {
            buf.selection.start = buf.cursor;
            buf.selection.active = true;
        }
        buf.selection.end = buf.cursor;
    }
}

void Editor::move_word_backward(bool extend_selection) {
    auto& buf = get_buffer();
    buf.cursor.x--;
    if (buf.cursor.x < 0) {
        if (buf.cursor.y > 0) {
            buf.cursor.y--;
            buf.cursor.x = buf.lines[buf.cursor.y].length();
        } else {
            buf.cursor.x = 0;
        }
    } else {
        while (buf.cursor.x > 0 && !std::isalnum(buf.lines[buf.cursor.y][buf.cursor.x])) buf.cursor.x--;
        while (buf.cursor.x > 0 && std::isalnum(buf.lines[buf.cursor.y][buf.cursor.x - 1])) buf.cursor.x--;
    }
    clamp_cursor(get_pane().buffer_id);
    if (extend_selection) {
        if (!buf.selection.active) {
            buf.selection.start = buf.cursor;
            buf.selection.active = true;
        }
        buf.selection.end = buf.cursor;
    }
}

void Editor::move_to_line_start(bool extend_selection) {
    auto& buf = get_buffer();
    buf.cursor.x = 0;
    if (extend_selection) {
        if (!buf.selection.active) {
            buf.selection.start = buf.cursor;
            buf.selection.active = true;
        }
        buf.selection.end = buf.cursor;
    }
}

void Editor::move_to_line_end(bool extend_selection) {
    auto& buf = get_buffer();
    buf.cursor.x = buf.lines[buf.cursor.y].length();
    clamp_cursor(get_pane().buffer_id);
    if (extend_selection) {
        if (!buf.selection.active) {
            buf.selection.start = buf.cursor;
            buf.selection.active = true;
        }
        buf.selection.end = buf.cursor;
    }
}

void Editor::move_to_file_start(bool extend_selection) {
    auto& buf = get_buffer();
    buf.cursor.y = 0;
    buf.cursor.x = 0;
    if (extend_selection) {
        if (!buf.selection.active) {
            buf.selection.start = buf.cursor;
            buf.selection.active = true;
        }
        buf.selection.end = buf.cursor;
    }
}

void Editor::move_to_file_end(bool extend_selection) {
    auto& buf = get_buffer();
    buf.cursor.y = buf.lines.size() - 1;
    buf.cursor.x = buf.lines[buf.cursor.y].length();
    clamp_cursor(get_pane().buffer_id);
    if (extend_selection) {
        if (!buf.selection.active) {
            buf.selection.start = buf.cursor;
            buf.selection.active = true;
        }
        buf.selection.end = buf.cursor;
    }
}

