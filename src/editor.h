#ifndef EDITOR_H
#define EDITOR_H

#include <vector>
#include <string>
#include <stack>
#include <map>
#include <regex>
#include <filesystem>
#include <set>
#include "config.h"
#include "features.h"
#include "imageviewer.h"
#include "terminal.h"
#include "ui.h"
#include "autoclose.h"
#include "bracket.h"
#include "telescope.h"

namespace fs = std::filesystem;

enum PanelType {
    PANEL_EDITOR,
    PANEL_MINIMAP,
    PANEL_SEARCH,
    PANEL_COMMAND_PALETTE,
    PANEL_TELESCOPE
};

enum {
    COLOR_DEFAULT_FG = 7, COLOR_DEFAULT_BG = 0,
    COLOR_KEYWORD_FG = 6, COLOR_KEYWORD_BG = 0,
    COLOR_STRING_FG = 2, COLOR_STRING_BG = 0,
    COLOR_COMMENT_FG = 4, COLOR_COMMENT_BG = 0,
    COLOR_NUMBER_FG = 5, COLOR_NUMBER_BG = 0,
    COLOR_FUNCTION_FG = 3, COLOR_FUNCTION_BG = 0,
    COLOR_TYPE_FG = 6, COLOR_TYPE_BG = 0,
    COLOR_PANEL_BORDER_FG = 7, COLOR_PANEL_BORDER_BG = 4,
    COLOR_SELECTION_FG = 0, COLOR_SELECTION_BG = 6,
    COLOR_LINE_NUM_FG = 3, COLOR_LINE_NUM_BG = 0,
    COLOR_CURSOR_FG = 0, COLOR_CURSOR_BG = 7,
    COLOR_STATUS_FG = 0, COLOR_STATUS_BG = 6,
    COLOR_COMMAND_FG = 7, COLOR_COMMAND_BG = 5,
    COLOR_MINIMAP_FG = 4, COLOR_MINIMAP_BG = 0,
    COLOR_IMAGE_BORDER_FG = 7, COLOR_IMAGE_BORDER_BG = 5,
    COLOR_BRACKET1_FG = 1, COLOR_BRACKET2_FG = 2, COLOR_BRACKET3_FG = 3,
    COLOR_BRACKET4_FG = 4, COLOR_BRACKET5_FG = 5, COLOR_BRACKET6_FG = 6,
    COLOR_BRACKET_MATCH_FG = 3, COLOR_BRACKET_MATCH_BG = 0,
    COLOR_TELESCOPE_FG = 7, COLOR_TELESCOPE_BG = 0,
    COLOR_TELESCOPE_SELECTED_FG = 0, COLOR_TELESCOPE_SELECTED_BG = 6,
    COLOR_TELESCOPE_PREVIEW_FG = 7, COLOR_TELESCOPE_PREVIEW_BG = 0
};

struct Cursor {
    int x, y;
    bool operator==(const Cursor& other) const { return x == other.x && y == other.y; }
};

struct Selection {
    Cursor start;
    Cursor end;
    bool active;
};

struct State {
    std::vector<std::string> lines;
    Cursor cursor;
    Selection selection;
};

struct FileBuffer {
    std::vector<std::string> lines;
    Cursor cursor;
    Selection selection;
    int scroll_offset;
    int scroll_x;
    std::string filepath;
    bool modified;
    std::stack<State> undo_stack;
    std::stack<State> redo_stack;
    std::set<int> bookmarks;
};

struct SplitPane {
    int x, y, w, h;
    int buffer_id;
    bool active;
};

struct SyntaxRule {
    std::regex pattern;
    int color;
};

class SyntaxHighlighter {
private:
    std::vector<SyntaxRule> rules;
    std::string file_extension;
    
public:
    void set_language(const std::string& ext);
    std::vector<std::pair<int, int>> get_colors(const std::string& line);
};

class Editor {
private:
    std::vector<FileBuffer> buffers;
    std::vector<SplitPane> panes;
    int current_pane;
    int current_buffer;
    
    bool running;
    std::string message;
    std::string clipboard;
    
    // Command palette
    bool show_command_palette;
    std::string command_palette_query;
    std::vector<std::string> command_palette_results;
    int command_palette_selected;
    
    // Telescope finder
    Telescope telescope;
    bool waiting_for_space_f;
    
    // Search panel
    bool show_search;
    std::string search_query;
    std::vector<std::pair<int, int>> search_results; // (line, col)
    int search_result_index;
    bool search_case_sensitive;
    
    // Minimap
    bool show_minimap;
    int minimap_width;
    
    SyntaxHighlighter highlighter;
    Config config;
    ImageViewer image_viewer;
    Terminal terminal;
    UI* ui;
    
    int status_height;
    int tab_height;
    int tab_size;
    bool auto_indent;
    bool needs_redraw;
    bool mouse_selecting;
    Cursor mouse_start;
    
    int idle_frame_count;
    int cursor_blink_frame;
    bool cursor_visible;
    
    bool show_context_menu;
    int context_menu_x;
    int context_menu_y;
    int context_menu_selected;
    
    void render();
    void render_tabs();
    void render_panes();
    void render_pane(const SplitPane& pane);
    void render_telescope();
    void render_minimap(int x, int y, int w, int h, int buffer_id);
    void render_image_viewer();
    void render_status_line();
    void render_command_palette();
    void render_search_panel();
    void render_context_menu();
    void render_buffer_content(const SplitPane& pane, int buffer_id);
    
    void handle_input(int ch, bool is_ctrl = false, bool is_shift = false);
    void handle_command_palette(int ch);
    void handle_search_panel(int ch);
    void handle_telescope(int ch);
    void handle_mouse(void* event);
    
    void move_cursor(int dx, int dy, bool extend_selection = false);
    void insert_char(char c);
    void insert_string(const std::string& str);
    void delete_char(bool forward = true);
    void delete_selection();
    void delete_line();
    void new_line();
    void duplicate_line();
    void toggle_comment();
    
    void copy();
    void cut();
    void paste();
    
    void save_state();
    void undo();
    void redo();
    
    void clamp_cursor(int buffer_id);
    void move_word_forward(bool extend_selection = false);
    void move_word_backward(bool extend_selection = false);
    void move_to_line_start(bool extend_selection = false);
    void move_to_line_end(bool extend_selection = false);
    void move_to_file_start(bool extend_selection = false);
    void move_to_file_end(bool extend_selection = false);
    
    void select_all();
    void clear_selection();
    
    void open_file(const std::string& path);
    void close_buffer();
    void create_new_buffer();
    void save_file();
    void save_file_as();
    
    void toggle_minimap();
    void toggle_search();
    void toggle_command_palette();
    void execute_command(const std::string& cmd);
    
    void find_next();
    void find_prev();
    void perform_search();
    
    void split_pane_horizontal();
    void split_pane_vertical();
    void close_pane();
    void next_pane();
    void prev_pane();
    
    void toggle_bookmark();
    void next_bookmark();
    void prev_bookmark();
    
    void jump_to_matching_bracket();
    void format_document();
    
    FileBuffer& get_buffer(int id = -1);
    SplitPane& get_pane(int id = -1);
    std::string get_file_extension(const std::string& path);
    std::string get_filename(const std::string& path);
    
    int create_pane(int x, int y, int w, int h, int buffer_id);
    void update_pane_layout();
    void refresh_command_palette();

public:
    Editor();
    ~Editor();
    void load_file(const std::string& fname);
    void run();
};

#endif
