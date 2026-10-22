#pragma once
#include <vector>
#include <string>
#include <random>

struct Cell {
    bool walls[4] = {true,true,true,true}; // haut, droite, bas, gauche
    bool visited = false;
};

class Maze {
public:
    Maze(int width, int height);
    void generate();
    std::vector<int> solveBFS() const;
    std::vector<int> solveDFS() const;
    std::string render(const std::vector<int>& path = {}) const;
    bool wall(int x, int y, int direction) const { return cells_.at(y*w_+x).walls[direction]; }
    int width() const { return w_; }
    int height() const { return h_; }
private:
    int w_, h_;
    std::vector<Cell> cells_;
    std::mt19937 rng_;
    std::vector<int> neighbors(int index) const;
    std::vector<int> solve(bool bfs) const;
};
