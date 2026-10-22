#include "Maze.hpp"
#include <algorithm>
#include <queue>
#include <stack>
#include <sstream>
#include <stdexcept>

Maze::Maze(int width, int height) : w_(width), h_(height), cells_(width*height), rng_(std::random_device{}()) {
    if (width < 2 || height < 2 || width > 60 || height > 30) throw std::invalid_argument("Dimensions entre 2x2 et 60x30.");
}

std::vector<int> Maze::neighbors(int i) const {
    std::vector<int> result;
    int x=i%w_, y=i/w_;
    if(y>0) result.push_back(i-w_);
    if(x<w_-1) result.push_back(i+1);
    if(y<h_-1) result.push_back(i+w_);
    if(x>0) result.push_back(i-1);
    return result;
}

void Maze::generate() {
    cells_.assign(w_*h_, Cell{});
    std::vector<int> stack{0};
    cells_[0].visited=true;
    while (!stack.empty()) {
        int current=stack.back();
        auto adjacent=neighbors(current);
        std::vector<int> options;
        for(int n: adjacent) if(!cells_[n].visited) options.push_back(n);
        if(options.empty()) { stack.pop_back(); continue; }
        std::uniform_int_distribution<int> pick(0, static_cast<int>(options.size())-1);
        int next=options[pick(rng_)];
        int direction = next == current-w_ ? 0 : next == current+1 ? 1 : next == current+w_ ? 2 : 3;
        cells_[current].walls[direction]=false;
        cells_[next].walls[(direction+2)%4]=false;
        cells_[next].visited=true;
        stack.push_back(next);
    }
}

std::vector<int> Maze::solve(bool bfs) const {
    const int end=w_*h_-1;
    std::vector<int> parent(w_*h_, -1);
    std::vector<bool> seen(w_*h_, false);
    std::vector<int> frontier{0};
    seen[0]=true;
    size_t head=0;
    while (head < frontier.size()) {
        int current = bfs ? frontier[head++] : frontier.back();
        if(!bfs) frontier.pop_back();
        if(current==end) break;
        for(int n: neighbors(current)) {
            int direction = n==current-w_ ? 0 : n==current+1 ? 1 : n==current+w_ ? 2 : 3;
            if(!cells_[current].walls[direction] && !seen[n]) {
                seen[n]=true;
                parent[n]=current;
                frontier.push_back(n);
            }
        }
    }
    if(!seen[end]) return {};
    std::vector<int> path;
    for(int i=end;i!=-1;i=parent[i]) path.push_back(i);
    std::reverse(path.begin(),path.end());
    return path;
}
std::vector<int> Maze::solveBFS() const { return solve(true); }
std::vector<int> Maze::solveDFS() const { return solve(false); }

std::string Maze::render(const std::vector<int>& path) const {
    std::vector<std::string> grid(h_*2+1, std::string(w_*2+1,'#'));
    for(int y=0;y<h_;++y) for(int x=0;x<w_;++x) {
        int i=y*w_+x;
        grid[y*2+1][x*2+1]=' ';
        if(!cells_[i].walls[1]) grid[y*2+1][x*2+2]=' ';
        if(!cells_[i].walls[2]) grid[y*2+2][x*2+1]=' ';
    }
    for(size_t k=0;k<path.size();++k) {
        int i=path[k];
        if(i<0 || i>=w_*h_) continue;
        int row=(i/w_)*2+1, col=(i%w_)*2+1;
        grid[row][col]='.';
        if(k>0) {
            int prev=path[k-1];
            int prevRow=(prev/w_)*2+1, prevCol=(prev%w_)*2+1;
            grid[(row+prevRow)/2][(col+prevCol)/2]='.';
        }
    }
    grid[1][1]='S';
    grid[h_*2-1][w_*2-1]='E';
    std::ostringstream out;
    for(const auto& line: grid) out<<line<<'\n';
    return out.str();
}
