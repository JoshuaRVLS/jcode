#include "editor.h"

Editor::Editor() {
    config.load();
    
    running = true;
    current_pane = 0;
    current_buffer = 0;
    show_explorer = config.get_bool("show_explorer", true);
    explorer_focused = false;
    explorer_width = config.get_int("explorer_width", 25);
    explorer_selected = 0;
    current_dir = fs::current_path().string();
    show_minimap = config.get_bool("show_minimap", true);
    minimap_width = config.get_int("minimap_width", 15);
    show_search = false;
    show_command_palette = false;
    command_palette_selected = 0;
    search_result_index = -1;
    search_case_sensitive = false;
    status_height = 2;
    tab_height = 1;
    tab_size = config.get_int("tab_size", 4);
    auto_indent = config.get_bool("auto_indent", true);
    needs_redraw = true;
    mouse_selecting = false;
    idle_frame_count = 0;
    cursor_blink_frame = 0;
    cursor_visible = true;
    show_context_menu = false;
    context_menu_x = 0;
    context_menu_y = 0;
    context_menu_selected = 0;
    
    terminal.init();
    ui = new UI(&terminal);
    ui->resize(terminal.get_width(), terminal.get_height());
    
    int h = terminal.get_height();
    int w = terminal.get_width();
    create_pane(explorer_width, tab_height, w - explorer_width - minimap_width, h - tab_height - status_height, -1);
    
    FileBuffer fb;
    fb.lines.push_back("");
    fb.cursor = {0, 0};
    fb.selection = {{0, 0}, {0, 0}, false};
    fb.scroll_offset = 0;
    fb.scroll_x = 0;
    fb.modified = false;
    buffers.push_back(fb);
    panes[0].buffer_id = 0;
}

Editor::~Editor() {
    delete ui;
    terminal.cleanup();
}
