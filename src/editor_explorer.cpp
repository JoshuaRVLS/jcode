#include "editor.h"
#include <algorithm>
#include <cctype>

void Editor::toggle_explorer() {
    show_explorer = !show_explorer;
    explorer_focused = false;
}

void Editor::explorer_up() {
    if (explorer_selected > 0) explorer_selected--;
}

void Editor::explorer_down() {
    if (explorer_selected < file_list.size() - 1) explorer_selected++;
}

void Editor::explorer_open() {
    if (file_list.empty()) return;
    fs::path p = fs::path(current_dir) / file_list[explorer_selected];
    
    if (fs::is_directory(p)) {
        if (file_list[explorer_selected] == "..") {
            fs::path parent = fs::path(current_dir).parent_path();
            if (!parent.empty()) {
                load_directory(parent.string());
            }
        } else {
            load_directory(p.string());
        }
    } else {
        std::string ext = p.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || 
            ext == ".gif" || ext == ".bmp" || ext == ".svg" ||
            ext == ".webp" || ext == ".ico") {
            image_viewer.open(p.string());
            needs_redraw = true;
        } else {
            open_file(p.string());
            show_explorer = false;
        }
    }
    explorer_focused = false;
}

