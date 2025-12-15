#include "imageviewer.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace fs = std::filesystem;

ImageViewer::ImageViewer() {
    is_open = false;
    view_x = view_y = view_w = view_h = 0;
}

bool ImageViewer::is_image_file(const std::string& path) {
    std::string ext = path;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    size_t dot = ext.find_last_of('.');
    if (dot == std::string::npos) return false;
    
    ext = ext.substr(dot);
    return ext == ".jpg" || ext == ".jpeg" || ext == ".png" || 
           ext == ".gif" || ext == ".bmp" || ext == ".svg" ||
           ext == ".webp" || ext == ".ico";
}

std::string ImageViewer::get_image_info(const std::string& path) {
    if (!fs::exists(path)) return "File not found";
    
    try {
        auto size = fs::file_size(path);
        std::string size_str;
        if (size < 1024) {
            size_str = std::to_string(size) + " B";
        } else if (size < 1024 * 1024) {
            size_str = std::to_string(size / 1024) + " KB";
        } else {
            size_str = std::to_string(size / (1024 * 1024)) + " MB";
        }
        
        std::string ext = path.substr(path.find_last_of('.'));
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        return ext + " | " + size_str;
    } catch (...) {
        return "Unknown";
    }
}

void ImageViewer::generate_ascii_preview(const std::string& path) {
    ascii_preview.clear();
    
    if (!fs::exists(path)) {
        ascii_preview.push_back("Image not found");
        return;
    }
    
    std::string ext = path.substr(path.find_last_of('.'));
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    std::string info = get_image_info(path);
    ascii_preview.push_back("IMAGE: " + fs::path(path).filename().string());
    ascii_preview.push_back(info);
    ascii_preview.push_back("");
    
    std::string cmd;
    if (ext == ".svg") {
        cmd = "convert '" + path + "' -resize 40x20 txt:- 2>/dev/null | head -20";
    } else {
        cmd = "convert '" + path + "' -resize 40x20 -colorspace Gray txt:- 2>/dev/null | head -20";
    }
    
    FILE* pipe = popen(cmd.c_str(), "r");
    if (pipe) {
        char buffer[128];
        int line_count = 0;
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr && line_count < 15) {
            std::string line(buffer);
            if (line.find("#") == 0) continue;
            
            size_t comma = line.find(',');
            if (comma != std::string::npos) {
                size_t rgb = line.find("rgb(", comma);
                if (rgb != std::string::npos) {
                    size_t close = line.find(')', rgb);
                    if (close != std::string::npos) {
                        std::string rgb_val = line.substr(rgb + 4, close - rgb - 4);
                        std::istringstream iss(rgb_val);
                        int r, g, b;
                        char comma1, comma2;
                        if (iss >> r >> comma1 >> g >> comma2 >> b) {
                            int gray = (r + g + b) / 3;
                            char ch = ' ';
                            if (gray < 30) ch = '█';
                            else if (gray < 60) ch = '▓';
                            else if (gray < 90) ch = '▒';
                            else if (gray < 120) ch = '░';
                            else if (gray < 150) ch = '.';
                            
                            ascii_preview.push_back(std::string(1, ch));
                            line_count++;
                        }
                    }
                }
            }
        }
        pclose(pipe);
    }
    
    if (ascii_preview.size() < 5) {
        ascii_preview.clear();
        ascii_preview.push_back("IMAGE PREVIEW");
        ascii_preview.push_back(fs::path(path).filename().string());
        ascii_preview.push_back("");
        ascii_preview.push_back("Use external viewer:");
        ascii_preview.push_back("  feh " + path);
        ascii_preview.push_back("  sxiv " + path);
        ascii_preview.push_back("  xdg-open " + path);
    }
}

void ImageViewer::open(const std::string& path) {
    if (!is_image_file(path)) return;
    if (!fs::exists(path)) return;
    
    current_image = path;
    is_open = true;
    generate_ascii_preview(path);
}

void ImageViewer::close() {
    is_open = false;
    current_image.clear();
    ascii_preview.clear();
}

void ImageViewer::render(int x, int y, int w, int h) {
    if (!is_open) return;
    
    view_x = x;
    view_y = y;
    view_w = w;
    view_h = h;
}

