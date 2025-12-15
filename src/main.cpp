#include "editor.h"
#include <iostream>

int main(int argc, char* argv[]) {
    Editor editor;
    
    if (argc > 1) {
        editor.load_file(argv[1]);
    }
    
    
    editor.run();
    
    return 0;
}
