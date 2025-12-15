CXX = g++
CXXFLAGS = -std=c++17 -Wall
LIBS = -lstdc++fs

TARGET = bin/jcode
SRCDIR = src
BINDIR = bin

SOURCES = $(SRCDIR)/main.cpp $(SRCDIR)/editor.cpp $(SRCDIR)/syntax.cpp $(SRCDIR)/editor_utils.cpp $(SRCDIR)/editor_file.cpp $(SRCDIR)/editor_edit.cpp $(SRCDIR)/editor_cursor.cpp $(SRCDIR)/editor_undo.cpp $(SRCDIR)/editor_search.cpp $(SRCDIR)/editor_panes.cpp $(SRCDIR)/editor_bookmarks.cpp $(SRCDIR)/editor_command.cpp $(SRCDIR)/editor_render.cpp $(SRCDIR)/editor_input.cpp $(SRCDIR)/config.cpp $(SRCDIR)/features.cpp $(SRCDIR)/imageviewer.cpp $(SRCDIR)/terminal.cpp $(SRCDIR)/ui.cpp $(SRCDIR)/autoclose.cpp $(SRCDIR)/bracket.cpp $(SRCDIR)/telescope.cpp
HEADERS = $(SRCDIR)/editor.h $(SRCDIR)/config.h $(SRCDIR)/features.h $(SRCDIR)/imageviewer.h $(SRCDIR)/terminal.h $(SRCDIR)/ui.h $(SRCDIR)/autoclose.h $(SRCDIR)/bracket.h $(SRCDIR)/telescope.h
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(SRCDIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
