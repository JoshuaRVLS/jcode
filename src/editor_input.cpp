#include "editor.h"
#include "autoclose.h"
#include "features.h"
#include <sstream>

struct MEVENT {
    int x, y;
    int bstate;
};

void Editor::handle_input(int ch, bool is_ctrl, bool is_shift) {
    idle_frame_count = 0;
    cursor_visible = true;
    cursor_blink_frame = 0;
    
    if (image_viewer.is_active()) {
        if (ch == 'q' || ch == 27) {
            image_viewer.close();
            needs_redraw = true;
            return;
        } else if (ch == 'o') {
            std::string cmd = "xdg-open '" + image_viewer.get_current() + "' &";
            system(cmd.c_str());
            return;
        }
    }
    
    if (ch == '\t' || ch == 9) {
        insert_char('\t');
        explorer_focused = false;
        needs_redraw = true;
        return;
    }
    
    if (is_ctrl && ch != '\n' && ch != 13 && ch != 9) {
        if (ch == 'p' || ch == 'P') {
            toggle_command_palette(); needs_redraw = true; return;
        } else if (ch == 'f' || ch == 'F') {
            toggle_search(); needs_redraw = true; return;
        } else if (ch == 's' || ch == 'S') {
            save_file(); needs_redraw = true; return;
        } else if (ch == 'w' || ch == 'W') {
            close_buffer(); needs_redraw = true; return;
        } else if (ch == 'q' || ch == 'Q') {
            running = false; return;
        } else if (ch == 'n' || ch == 'N') {
            create_new_buffer(); needs_redraw = true; return;
        } else if (ch == 'o' || ch == 'O') {
            toggle_explorer(); needs_redraw = true; return;
        } else if (ch == 'z' || ch == 'Z') {
            undo(); needs_redraw = true; return;
        } else if (ch == 'y' || ch == 'Y') {
            redo(); needs_redraw = true; return;
        } else if (ch == 'x' || ch == 'X') {
            cut(); needs_redraw = true; return;
        } else if (ch == 'c' || ch == 'C') {
            copy(); return;
        } else if (ch == 'v' || ch == 'V') {
            paste(); needs_redraw = true; return;
        } else if (ch == 'a' || ch == 'A') {
            select_all(); needs_redraw = true; return;
        } else if (ch == 'm' || ch == 'M') {
            toggle_minimap(); needs_redraw = true; return;
        } else if (ch == 'd' || ch == 'D') {
            duplicate_line(); needs_redraw = true; return;
        } else if (ch == 47) {
            toggle_comment(); needs_redraw = true; return;
        } else if (ch == 'b' || ch == 'B') {
            jump_to_matching_bracket(); needs_redraw = true; return;
        } else if (ch == 'l' || ch == 'L') {
            format_document(); needs_redraw = true; return;
        }
    }
    
    if (ch == 1008) {
        move_cursor(0, -1, is_shift);
    } else if (ch == 1009) {
        move_cursor(0, 1, is_shift);
    } else if (ch == 1011) {
        move_cursor(-1, 0, is_shift);
    } else if (ch == 1010) {
        move_cursor(1, 0, is_shift);
    } else if (ch == 1012) {
        move_to_line_start(is_shift);
    } else if (ch == 1013) {
        move_to_line_end(is_shift);
    } else if (ch == 1015) {
        move_cursor(0, -10, false);
    } else if (ch == 1016) {
        move_cursor(0, 10, false);
    } else if (ch == 127 || ch == 8) {
        auto& buf = get_buffer();
        if (buf.cursor.x < (int)buf.lines[buf.cursor.y].length()) {
            char current = buf.lines[buf.cursor.y][buf.cursor.x];
            if (AutoClose::is_closing_bracket(current)) {
                char opening = '\0';
                if (current == ')') opening = '(';
                else if (current == '}') opening = '{';
                else if (current == ']') opening = '[';
                
                if (opening != '\0' && buf.cursor.x > 0) {
                    char prev = buf.lines[buf.cursor.y][buf.cursor.x - 1];
                    if (prev == opening) {
                        delete_char(true);
                        delete_char(false);
                        needs_redraw = true;
                        return;
                    }
                }
            }
        }
        delete_char(false);
    } else if (ch == 1001) {
        delete_char(true);
    } else if (ch == '\n' || ch == 13) {
        if (explorer_focused && show_explorer && !show_command_palette && !show_search) {
            explorer_open();
            needs_redraw = true;
            return;
        }
        
        auto& buf = get_buffer();
        if (buf.cursor.x > 0 && buf.cursor.x < (int)buf.lines[buf.cursor.y].length()) {
            char prev = buf.lines[buf.cursor.y][buf.cursor.x - 1];
            char next = buf.lines[buf.cursor.y][buf.cursor.x];
            
            if (AutoClose::should_auto_close(prev)) {
                char closing = AutoClose::get_closing_bracket(prev);
                if (closing != '\0' && next == closing) {
                    save_state();
                    std::string remaining = buf.lines[buf.cursor.y].substr(buf.cursor.x + 1);
                    buf.lines[buf.cursor.y] = buf.lines[buf.cursor.y].substr(0, buf.cursor.x);
                    
                    int base_indent = EditorFeatures::get_indent_level(buf.lines[buf.cursor.y]);
                    int new_indent = base_indent;
                    if (auto_indent) {
                        new_indent += tab_size;
                    }
                    std::string indent_str = EditorFeatures::get_indent_string(new_indent, tab_size);
                    std::string closing_indent = EditorFeatures::get_indent_string(base_indent, tab_size);
                    
                    buf.lines.insert(buf.lines.begin() + buf.cursor.y + 1, indent_str);
                    buf.lines.insert(buf.lines.begin() + buf.cursor.y + 2, closing_indent + closing + remaining);
                    buf.cursor.y++;
                    buf.cursor.x = indent_str.length();
                    buf.modified = true;
                    needs_redraw = true;
                    explorer_focused = false;
                    return;
                }
            }
        }
        
        new_line();
        explorer_focused = false;
    } else if (ch >= 32 && ch < 127) {
        auto& buf = get_buffer();
        if (AutoClose::is_closing_bracket(ch) && 
            buf.cursor.x < (int)buf.lines[buf.cursor.y].length()) {
            if (AutoClose::should_skip_closing(ch, buf.lines[buf.cursor.y], buf.cursor.x)) {
                buf.cursor.x++;
                needs_redraw = true;
                return;
            }
        }
        insert_char(ch);
        explorer_focused = false;
    }
    
    if (show_explorer && !show_command_palette && !show_search) {
        if (ch == 1008 || ch == 'k') {
            explorer_up();
            explorer_focused = true;
            needs_redraw = true;
        } else if (ch == 1009 || ch == 'j') {
            explorer_down();
            explorer_focused = true;
            needs_redraw = true;
        }
    }
}

void Editor::handle_mouse(void* event_ptr) {
    MEVENT* event = (MEVENT*)event_ptr;
    if (panes.empty()) return;
    
    auto& pane = get_pane();
    
    int bstate = event->bstate;
    
    if (bstate == 3) {
        show_context_menu = true;
        context_menu_x = event->x;
        context_menu_y = event->y;
        context_menu_selected = 0;
        needs_redraw = true;
        return;
    }
    
    if (event->x < pane.x || event->x >= pane.x + pane.w ||
        event->y < pane.y || event->y >= pane.y + pane.h) {
        return;
    }
    
    auto& buf = get_buffer(pane.buffer_id);
    
    int rel_y = event->y - pane.y - 1;
    int rel_x = event->x - pane.x - 6;
    
    if (rel_y < 0) rel_y = 0;
    if (rel_x < 0) rel_x = 0;
    
    int click_y = rel_y + buf.scroll_offset;
    int click_x = rel_x + buf.scroll_x;
    
    if (click_y < 0) click_y = 0;
    if (click_y >= (int)buf.lines.size()) click_y = buf.lines.size() - 1;
    if (click_y < 0) return;
    
    if (click_x < 0) click_x = 0;
    int line_len = buf.lines[click_y].length();
    if (click_x > line_len) {
        click_x = line_len;
    }
    
    if (bstate == 1) {
        if (show_context_menu) {
            show_context_menu = false;
            needs_redraw = true;
        }
        idle_frame_count = 0;
        cursor_visible = true;
        cursor_blink_frame = 0;
        mouse_selecting = true;
        mouse_start = {click_x, click_y};
        buf.cursor.x = click_x;
        buf.cursor.y = click_y;
        buf.selection.start = mouse_start;
        buf.selection.end = {click_x, click_y};
        buf.selection.active = true;
        needs_redraw = true;
    } else if (bstate == 2) {
        idle_frame_count = 0;
        cursor_visible = true;
        cursor_blink_frame = 0;
        if (mouse_selecting) {
            buf.cursor.x = click_x;
            buf.cursor.y = click_y;
            buf.selection.end = {click_x, click_y};
            mouse_selecting = false;
            needs_redraw = true;
        } else {
            buf.cursor.x = click_x;
            buf.cursor.y = click_y;
            buf.selection.active = false;
            needs_redraw = true;
        }
    } else if (bstate == 32) {
        idle_frame_count = 0;
        cursor_visible = true;
        cursor_blink_frame = 0;
        if (mouse_selecting) {
            buf.cursor.x = click_x;
            buf.cursor.y = click_y;
            buf.selection.end = {click_x, click_y};
            needs_redraw = true;
        }
    } else if (bstate == 3) {
        show_context_menu = true;
        context_menu_x = event->x;
        context_menu_y = event->y;
        context_menu_selected = 0;
        needs_redraw = true;
    }
    
    clamp_cursor(pane.buffer_id);
}

void Editor::run() {
    while (running) {
        render();
        
        Event ev = terminal.poll_event();
        
        if (ev.type == EVENT_REDRAW) {
            needs_redraw = true;
        } else if (ev.type == EVENT_RESIZE) {
            ui->resize(ev.resize.width, ev.resize.height);
            needs_redraw = true;
        } else if (ev.type == EVENT_KEY) {
            int ch = ev.key.key;
            bool is_ctrl = ev.key.ctrl;
            bool is_shift = ev.key.shift;
            
            if (ch == '\n' || ch == 13) {
                is_ctrl = false;
            }
            
            if (is_ctrl && ch >= 1 && ch <= 26) {
                ch = ch + 96;
            }
            
            if (show_context_menu) {
                if (ch == 27) {
                    show_context_menu = false;
                    needs_redraw = true;
                }
            } else if (show_command_palette) {
                handle_command_palette(ch);
            } else if (show_search) {
                handle_search_panel(ch);
            } else {
                handle_input(ch, is_ctrl, is_shift);
            }
        } else if (ev.type == EVENT_MOUSE) {
            if (show_context_menu) {
                if (ev.mouse.pressed || ev.mouse.released) {
                    int menu_x = context_menu_x;
                    int menu_y = context_menu_y;
                    int menu_w = 20;
                    int menu_h = 4;
                    
                    if (ev.mouse.x >= menu_x && ev.mouse.x < menu_x + menu_w &&
                        ev.mouse.y >= menu_y && ev.mouse.y < menu_y + menu_h) {
                        int item = ev.mouse.y - menu_y - 1;
                        if (item >= 0 && item < 3) {
                            if (ev.mouse.released) {
                                if (item == 0) {
                                    copy();
                                } else if (item == 1) {
                                    cut();
                                } else if (item == 2) {
                                    paste();
                                }
                                show_context_menu = false;
                                needs_redraw = true;
                            } else {
                                context_menu_selected = item;
                                needs_redraw = true;
                            }
                        } else {
                            show_context_menu = false;
                            needs_redraw = true;
                        }
                    } else {
                        show_context_menu = false;
                        needs_redraw = true;
                    }
                }
            } else {
                MEVENT mevent;
                mevent.x = ev.mouse.x;
                mevent.y = ev.mouse.y;
                int bstate = 0;
                
                int button_code = ev.mouse.button & 0x03;
                bool is_motion = (ev.mouse.button & 0x20) != 0;
                
                if (is_motion) {
                    bstate = 32;
                } else if (ev.mouse.pressed) {
                    if (button_code == 0) {
                        bstate = 1;
                    } else if (button_code == 1 || button_code == 2) {
                        bstate = 3;
                    } else {
                        bstate = 1;
                    }
                } else if (ev.mouse.released) {
                    if (button_code == 0) {
                        bstate = 2;
                    } else {
                        bstate = 2;
                    }
                }
                
                mevent.bstate = bstate;
                handle_mouse(&mevent);
            }
        }
    }
    config.save();
}

