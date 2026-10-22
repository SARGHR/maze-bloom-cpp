CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Iinclude
SDL_FLAGS = $(shell pkg-config --cflags --libs sdl2 SDL2_ttf)
TARGET = maze-bloom
SOURCES = src/gui.cpp src/Maze.cpp
all: $(TARGET)
$(TARGET): $(SOURCES) include/Maze.hpp
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $@ $(SDL_FLAGS)
run: all
	./$(TARGET)
clean:
	rm -f $(TARGET)
.PHONY: all run clean
