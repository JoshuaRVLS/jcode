#include "editor.h"
#include "bracket.h"
#include <sstream>
#include <cstdio>

void Editor::render() {
    if (!needs_redraw) {
        if (!show_command_palette && !show_search) {
            auto& pane = get_pane();
            auto& buf = get_buffer(pane.buffer_id);
            int display_y = buf.cursor.y - buf.scroll_offset + pane.y + 1;
            int display_x = buf.cursor.x - buf.scroll_x + pane.x + 6;
            if (display_y >= pane.y && display_y < pane.y + pane.h &&
                display_x >= pane.x && display_x < pane.x + pane.w) {
                ui->set_cursor(display_x, display_y);
            }
        }
        return;
    }
    
    ui->clear();
    int h = ui->get_height();
    int w = ui->get_width();
    
    render_tabs();
    
    if (show_explorer) {
        render_explorer(0, tab_height, explorer_width, h - tab_height - status_height);
    }
    
    update_pane_layout();
    
    if (image_viewer.is_active()) {
        int img_w = w / 2;
        int editor_w = w - explorer_width - img_w;
        if (!panes.empty()) {
            panes[0].w = editor_w;
        }
        render_panes();
        if (image_viewer.is_active()) {
            render_image_viewer();
        }
    } else {
        render_panes();
        if (show_minimap && !panes.empty()) {
            int minimap_x = w - minimap_width;
            render_minimap(minimap_x, tab_height, minimap_width, h - tab_height - status_height, get_pane().buffer_id);
        }
    }
    
    if (show_search) {
        render_search_panel();
    }
    
    if (show_command_palette) {
        render_command_palette();
    }
    
    render_status_line();
    
    if (show_context_menu) {
        render_context_menu();
    }
    
    idle_frame_count++;
    
    if (idle_frame_count <= 30) {
        cursor_visible = true;
    } else {
        cursor_blink_frame++;
        if (cursor_blink_frame >= 15) {
            cursor_visible = !cursor_visible;
            cursor_blink_frame = 0;
            needs_redraw = true;
        } else {
            needs_redraw = true;
        }
    }
    
    ui->hide_cursor();
    
    bool should_redraw = needs_redraw;
    needs_redraw = false;
    ui->render();
    
    if (should_redraw || idle_frame_count > 30) {
        needs_redraw = true;
    }
}

void Editor::render_tabs() {
    int w = ui->get_width();
    int x = 0;
    
    for (size_t i = 0; i < buffers.size(); i++) {
        std::string name = buffers[i].filepath.empty() ? "[New File]" : get_filename(buffers[i].filepath);
        if (buffers[i].modified) name += " ●";
        
        int fg = 7, bg = 0;
        if (i == current_buffer) {
            fg = 0;
            bg = 6;
        }
        
        ui->draw_text(x, 0, " " + name + " ", fg, bg);
        x += name.length() + 2;
    }
    
    for (int i = x; i < w; i++) {
        ui->draw_text(i, 0, " ", 7, 0);
    }
}

void Editor::render_explorer(int x, int y, int w, int h) {
    for (int i = 0; i < h; i++) {
        ui->draw_text(x + w - 1, y + i, "|", COLOR_PANEL_BORDER_FG, COLOR_PANEL_BORDER_BG);
    }
    ui->draw_text(x, y, " EXPLORER ", COLOR_PANEL_BORDER_FG, COLOR_PANEL_BORDER_BG);
    
    load_directory(current_dir);
    
    for (int i = 0; i < h - 1 && i < (int)file_list.size(); i++) {
        int file_idx = i;
        int fg = 7, bg = 0;
        if (file_idx == explorer_selected) {
            fg = 0;
            bg = 7;
        }
        
        std::string icon = " ";
        fs::path p = fs::path(current_dir) / file_list[file_idx];
        if (fs::is_directory(p)) {
            icon = "D";
        } else {
            icon = "F";
        }
        
        std::string display = icon + " " + file_list[file_idx];
        if ((int)display.length() > w - 3) {
            display = display.substr(0, w - 6) + "...";
        }
        
        ui->draw_text(x + 1, y + i + 1, display, fg, bg);
    }
}

void Editor::render_image_viewer() {
    int h = ui->get_height();
    int w = ui->get_width();
    
    int img_w = w / 2;
    int img_h = h - tab_height - status_height;
    int img_x = w - img_w;
    int img_y = tab_height;
    
    image_viewer.render(img_x, img_y, img_w, img_h);
}

void Editor::render_minimap(int x, int y, int w, int h, int buffer_id) {
    for (int i = 0; i < h; i++) {
        ui->draw_text(x, y + i, "|", COLOR_PANEL_BORDER_FG, COLOR_PANEL_BORDER_BG);
    }
    ui->draw_text(x + 1, y, " MINIMAP ", COLOR_PANEL_BORDER_FG, COLOR_PANEL_BORDER_BG);
    
    auto& buf = get_buffer(buffer_id);
    int total_lines = buf.lines.size();
    if (total_lines == 0) return;
    
    int visible_lines = h - 1;
    if (visible_lines <= 0) return;
    
    double scale = total_lines > 0 ? (double)total_lines / visible_lines : 1.0;
    if (scale < 1.0) scale = 1.0;
    
    for (int i = 0; i < visible_lines; i++) {
        int line_idx = (int)(i * scale);
        if (line_idx >= total_lines) line_idx = total_lines - 1;
        
        int fg = COLOR_MINIMAP_FG, bg = COLOR_MINIMAP_BG;
        if (line_idx == buf.cursor.y) {
            fg = 0;
            bg = 7;
        }
        
        std::string preview = buf.lines[line_idx];
        if ((int)preview.length() > w - 2) {
            preview = preview.substr(0, w - 2);
        }
        ui->draw_text(x + 1, y + i + 1, preview, fg, bg);
    }
}

void Editor::render_panes() {
    for (auto& pane : panes) {
        render_pane(pane);
    }
}

void Editor::render_pane(const SplitPane& pane) {
    for (int i = 0; i < pane.h; i++) {
        if (i == 0) {
            for (int j = 0; j < pane.w; j++) {
                ui->draw_text(pane.x + j, pane.y, "-", COLOR_PANEL_BORDER_FG, COLOR_PANEL_BORDER_BG);
            }
        }
        ui->draw_text(pane.x, pane.y + i, "|", COLOR_PANEL_BORDER_FG, COLOR_PANEL_BORDER_BG);
        ui->draw_text(pane.x + pane.w - 1, pane.y + i, "|", COLOR_PANEL_BORDER_FG, COLOR_PANEL_BORDER_BG);
    }
    
    render_buffer_content(pane, pane.buffer_id);
}

void Editor::render_buffer_content(const SplitPane& pane, int buffer_id) {
    auto& buf = get_buffer(buffer_id);
    
    if (buf.cursor.y < buf.scroll_offset) {
        buf.scroll_offset = buf.cursor.y;
    }
    if (buf.cursor.y >= buf.scroll_offset + pane.h - 2) {
        buf.scroll_offset = buf.cursor.y - pane.h + 3;
    }
    
    if (buf.cursor.x < buf.scroll_x) {
        buf.scroll_x = buf.cursor.x;
    }
    if (buf.cursor.x >= buf.scroll_x + pane.w - 7) {
        buf.scroll_x = buf.cursor.x - pane.w + 8;
    }
    
    for (int i = 0; i < pane.h - 1; i++) {
        int line_num = i + buf.scroll_offset;
        if (line_num >= (int)buf.lines.size()) break;
        
        char line_num_str[16];
        snprintf(line_num_str, sizeof(line_num_str), "%4d ", line_num + 1);
        ui->draw_text(pane.x + 1, pane.y + i + 1, line_num_str, COLOR_LINE_NUM_FG, COLOR_LINE_NUM_BG);
        
        bool in_selection = false;
        if (buf.selection.active) {
            int sel_start_y = std::min(buf.selection.start.y, buf.selection.end.y);
            int sel_end_y = std::max(buf.selection.start.y, buf.selection.end.y);
            if (line_num >= sel_start_y && line_num <= sel_end_y) {
                in_selection = true;
            }
        }
        
        int x = pane.x + 6;
        auto colors = highlighter.get_colors(buf.lines[line_num]);
        auto bracket_pairs = BracketHighlighter::find_all_pairs(buf.lines);
        auto matching_bracket = BracketHighlighter::find_matching_bracket_near_cursor(
            buf.lines, buf.cursor.y, buf.cursor.x
        );
        
        for (size_t j = buf.scroll_x; j < buf.lines[line_num].length() && x < pane.x + pane.w - 1; j++) {
            bool is_cursor = (line_num == buf.cursor.y && (int)j == buf.cursor.x);
            bool selected = in_selection;
            if (selected && line_num == buf.selection.start.y && (int)j < buf.selection.start.x) selected = false;
            if (selected && line_num == buf.selection.end.y && (int)j > buf.selection.end.x) selected = false;
            if (selected && line_num == buf.selection.start.y && line_num == buf.selection.end.y) {
                selected = (int)j >= std::min(buf.selection.start.x, buf.selection.end.x) &&
                          (int)j <= std::max(buf.selection.start.x, buf.selection.end.x);
            }
            
            bool is_matching_bracket = (matching_bracket.first == line_num && 
                                       matching_bracket.second == (int)j);
            bool is_near_bracket = ((line_num == buf.cursor.y && (int)j == buf.cursor.x - 1) ||
                                   (line_num == buf.cursor.y && (int)j == buf.cursor.x) ||
                                   (line_num == buf.cursor.y && (int)j == buf.cursor.x + 1)) &&
                                   BracketHighlighter::is_bracket(buf.lines[line_num][j]);
            
            int fg = 7, bg = 0;
            if (is_cursor && cursor_visible) {
                fg = COLOR_CURSOR_FG;
                bg = COLOR_CURSOR_BG;
            } else if (selected) {
                fg = COLOR_SELECTION_FG;
                bg = COLOR_SELECTION_BG;
            } else if (is_matching_bracket || (is_near_bracket && matching_bracket.first != -1)) {
                fg = COLOR_BRACKET_MATCH_FG;
                bg = COLOR_BRACKET_MATCH_BG;
            } else {
                int bracket_color = BracketHighlighter::get_bracket_color_at(
                    buf.lines, line_num, j, bracket_pairs
                );
                if (bracket_color > 0) {
                    switch(bracket_color) {
                        case 1: fg = COLOR_BRACKET1_FG; break;
                        case 2: fg = COLOR_BRACKET2_FG; break;
                        case 3: fg = COLOR_BRACKET3_FG; break;
                        case 4: fg = COLOR_BRACKET4_FG; break;
                        case 5: fg = COLOR_BRACKET5_FG; break;
                        case 6: fg = COLOR_BRACKET6_FG; break;
                        default: fg = 7;
                    }
                } else if (colors[j].first) {
                    switch(colors[j].second) {
                        case 1: fg = COLOR_KEYWORD_FG; break;
                        case 2: fg = COLOR_STRING_FG; break;
                        case 3: fg = COLOR_COMMENT_FG; break;
                        case 4: fg = COLOR_NUMBER_FG; break;
                        case 5: fg = COLOR_FUNCTION_FG; break;
                        case 6: fg = COLOR_TYPE_FG; break;
                        default: fg = 7;
                    }
                }
            }
            
            char ch = buf.lines[line_num][j];
            ui->draw_text(x, pane.y + i + 1, std::string(1, ch), fg, bg);
            x++;
        }
        
        if (line_num == buf.cursor.y && buf.cursor.x >= (int)buf.lines[line_num].length()) {
            int cursor_x = pane.x + 6 + (buf.cursor.x - buf.scroll_x);
            if (cursor_x >= pane.x + 6 && cursor_x < pane.x + pane.w - 1) {
                if (cursor_visible) {
                    ui->draw_text(cursor_x, pane.y + i + 1, " ", COLOR_CURSOR_FG, COLOR_CURSOR_BG);
                }
            }
        }
        
    }
}

void Editor::render_status_line() {
    int h = ui->get_height();
    int w = ui->get_width();
    auto& buf = get_buffer();
    
    std::string status = " Line " + std::to_string(buf.cursor.y + 1) + 
                        " Col " + std::to_string(buf.cursor.x + 1);
    if (buf.selection.active) {
        status += " [SELECTED]";
    }
    if (buf.modified) {
        status += " [MODIFIED]";
    }
    if (!buf.filepath.empty()) {
        status += " | " + buf.filepath;
    }
    if ((int)status.length() > w) {
        status = status.substr(0, w);
    }
    
    ui->draw_text(0, h - 2, status, COLOR_STATUS_FG, COLOR_STATUS_BG);
    
    if (!message.empty()) {
        ui->draw_text(0, h - 1, message, 7, 0);
        message.clear();
    } else {
        std::string help = "Ctrl+P: Command | Ctrl+F: Search | Ctrl+S: Save | Ctrl+W: Close | Ctrl+Q: Quit";
        if ((int)help.length() > w) {
            help = help.substr(0, w);
        }
        ui->draw_text(0, h - 1, help, 7, 0);
    }
}

void Editor::render_command_palette() {
    int h = ui->get_height();
    int w = ui->get_width();
    
    int palette_h = std::min(10, (int)command_palette_results.size() + 2);
    int palette_y = h / 2 - palette_h / 2;
    
    UIRect rect = {0, palette_y, w, palette_h};
    ui->draw_rect(rect, COLOR_COMMAND_FG, COLOR_COMMAND_BG);
    
    ui->draw_text(w / 2 - 10, palette_y, " COMMAND PALETTE ", COLOR_COMMAND_FG, COLOR_COMMAND_BG);
    ui->draw_text(1, palette_y + 1, "> " + command_palette_query, COLOR_COMMAND_FG, COLOR_COMMAND_BG);
    
    for (size_t i = 0; i < command_palette_results.size() && i < (size_t)(palette_h - 2); i++) {
        int fg = COLOR_COMMAND_FG, bg = COLOR_COMMAND_BG;
        if ((int)i == command_palette_selected) {
            fg = 0;
            bg = 7;
        }
        ui->draw_text(3, palette_y + 2 + (int)i, command_palette_results[i], fg, bg);
    }
}

void Editor::render_search_panel() {
    int h = ui->get_height();
    int w = ui->get_width();
    
    UIRect rect = {0, h - 3, w, 1};
    ui->draw_rect(rect, COLOR_COMMAND_FG, COLOR_COMMAND_BG);
    
    std::string search_text = "Search: " + search_query;
    ui->draw_text(1, h - 3, search_text, COLOR_COMMAND_FG, COLOR_COMMAND_BG);
    if (search_result_index >= 0) {
        char result_str[32];
        snprintf(result_str, sizeof(result_str), "[%d/%zu]", search_result_index + 1, search_results.size());
        ui->draw_text(w - 20, h - 3, result_str, COLOR_COMMAND_FG, COLOR_COMMAND_BG);
    }
}

void Editor::render_context_menu() {
    int menu_w = 20;
    int menu_h = 4;
    int x = context_menu_x;
    int y = context_menu_y;
    
    if (x + menu_w > ui->get_width()) x = ui->get_width() - menu_w;
    if (y + menu_h > ui->get_height()) y = ui->get_height() - menu_h;
    
    UIRect rect = {x, y, menu_w, menu_h};
    ui->draw_rect(rect, COLOR_COMMAND_FG, COLOR_COMMAND_BG);
    
    std::vector<std::string> items = {"Copy", "Cut", "Paste"};
    for (size_t i = 0; i < items.size(); i++) {
        int fg = COLOR_COMMAND_FG, bg = COLOR_COMMAND_BG;
        if ((int)i == context_menu_selected) {
            fg = 0;
            bg = 7;
        }
        ui->draw_text(x + 1, y + 1 + (int)i, items[i], fg, bg);
    }
}

