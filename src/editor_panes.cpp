#include "editor.h"

int Editor::create_pane(int x, int y, int w, int h, int buffer_id) {
    SplitPane pane;
    pane.x = x;
    pane.y = y;
    pane.w = w;
    pane.h = h;
    pane.buffer_id = buffer_id == -1 ? buffers.size() : buffer_id;
    pane.active = panes.empty();
    panes.push_back(pane);
    return panes.size() - 1;
}

void Editor::update_pane_layout() {
    int h = ui->get_height();
    int w = ui->get_width();
    
    int editor_w = w - (show_minimap ? minimap_width : 0);
    int editor_h = h - tab_height - status_height;
    
    if (!panes.empty()) {
        panes[0].x = 0;
        panes[0].y = tab_height;
        panes[0].w = editor_w;
        panes[0].h = editor_h;
    }
}

void Editor::split_pane_horizontal() {
    create_new_buffer();
    message = "New buffer created";
}

void Editor::split_pane_vertical() {
    split_pane_horizontal();
}

void Editor::close_pane() {
    if (panes.size() > 1) {
        panes.erase(panes.begin() + current_pane);
        if (current_pane >= panes.size()) current_pane = panes.size() - 1;
    }
}

void Editor::next_pane() {
    if (panes.size() > 1) {
        panes[current_pane].active = false;
        current_pane = (current_pane + 1) % panes.size();
        panes[current_pane].active = true;
        current_buffer = panes[current_pane].buffer_id;
    }
}

void Editor::prev_pane() {
    if (panes.size() > 1) {
        panes[current_pane].active = false;
        current_pane = (current_pane - 1 + panes.size()) % panes.size();
        panes[current_pane].active = true;
        current_buffer = panes[current_pane].buffer_id;
    }
}

