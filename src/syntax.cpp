#include "editor.h"
#include <regex>

void SyntaxHighlighter::set_language(const std::string& ext) {
    file_extension = ext;
    rules.clear();
    
    if (ext == ".cpp" || ext == ".h" || ext == ".c" || ext == ".hpp") {
        rules.push_back({std::regex("\\b(int|char|void|float|double|bool|long|short|unsigned|signed|const|static|struct|class|namespace|public|private|protected|virtual|override|return|if|else|for|while|do|switch|case|break|continue|sizeof|typedef|using|template|typename|auto|nullptr|new|delete|try|catch|throw)\\b"), 1});
        rules.push_back({std::regex("\"[^\"]*\""), 2});
        rules.push_back({std::regex("'[^']*'"), 2});
        rules.push_back({std::regex("//.*"), 3});
        rules.push_back({std::regex("\\b[0-9]+\\b"), 4});
        rules.push_back({std::regex("\\b[A-Za-z_][A-Za-z0-9_]*\\s*\\("), 5});
    } else if (ext == ".py") {
        rules.push_back({std::regex("\\b(def|class|if|elif|else|for|while|return|import|from|as|try|except|finally|with|lambda|yield|assert|break|continue|pass|raise|global|nonlocal|True|False|None|and|or|not|in|is)\\b"), 1});
        rules.push_back({std::regex("\"\"\"[^\"]*\"\"\"|'''[^']*'''|\"[^\"]*\"|'[^']*'"), 2});
        rules.push_back({std::regex("#.*"), 3});
        rules.push_back({std::regex("\\b[0-9]+\\b"), 4});
    } else if (ext == ".js" || ext == ".ts") {
        rules.push_back({std::regex("\\b(var|let|const|function|return|if|else|for|while|do|switch|case|break|continue|class|extends|import|export|from|async|await|try|catch|finally|throw|new|typeof|instanceof|this|super|static|get|set|yield|void|null|undefined|true|false)\\b"), 1});
        rules.push_back({std::regex("`[^`]*`|\"[^\"]*\"|'[^']*'"), 2});
        rules.push_back({std::regex("//.*"), 3});
        rules.push_back({std::regex("\\b[0-9]+\\b"), 4});
    } else if (ext == ".html" || ext == ".xml") {
        rules.push_back({std::regex("<[^>]*>"), 1});
        rules.push_back({std::regex("\"[^\"]*\"|'[^']*'"), 2});
        rules.push_back({std::regex("<!--.*?-->"), 3});
    } else if (ext == ".rs") {
        rules.push_back({std::regex("\\b(fn|let|mut|const|static|struct|enum|impl|trait|type|pub|use|mod|crate|self|super|if|else|match|for|while|loop|return|break|continue|async|await|move|ref|where|unsafe|extern|as|dyn)\\b"), 1});
        rules.push_back({std::regex("\"[^\"]*\""), 2});
        rules.push_back({std::regex("//.*"), 3});
        rules.push_back({std::regex("\\b[0-9]+\\b"), 4});
    }
}

std::vector<std::pair<int, int>> SyntaxHighlighter::get_colors(const std::string& line) {
    std::vector<std::pair<int, int>> colors(line.length(), {0, 0});
    
    for (const auto& rule : rules) {
        auto words_begin = std::sregex_iterator(line.begin(), line.end(), rule.pattern);
        auto words_end = std::sregex_iterator();
        
        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            std::smatch match = *i;
            for (size_t pos = match.position(); pos < match.position() + match.length() && pos < line.length(); pos++) {
                colors[pos] = {1, rule.color};
            }
        }
    }
    
    return colors;
}

