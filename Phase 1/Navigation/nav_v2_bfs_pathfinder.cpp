#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <algorithm>

struct Node {
    int x,y;
    // Overloading < for map usage
    bool operator<(const Node& other) const {
        return std::tie(x,y) < std::tie(other.x,other.y);
    }
    bool operator==(const Node& other) const {
        return x == other.x && y == other.y;
    }
};

class BFSPathFinder {
private:
    int width, height;
    std::vector<Node> directions = {{0,1},{1,0},{0,-1},{-1,0}}; //Up, Right, Down, Left

public:
    BFSPathFinder(int w, int h) : width(w), height(h) {}

    std::vector<Node> findPath(Node start, Node goal, const std::vector<std::vector<int>>& map) {
        std::queue<Node> q;
        q.push(start);

        std::map<Node, Node> parent; //To backtrack the path
        std::map<Node, bool> visited;
        visited[start] = true;

        bool found = false;
        while (!q.empty()) {
            Node current = q.front();
            q.pop();

            if(current == goal){
                found = true;
                break;
            }

            for(auto& d : directions) {
                Node next = {current.x + d.x, current.y + d.y};

                //Check bounds, obstacles (255), and visited status
                if (next.x >= 0 && next.x < width && next.y >= 0 && next.y < height &&
                    map[next.y][next.x] != 255 && !visited[next]) {
                        visited[next] = true;
                        parent[next] = current;
                        q.push(next);
                    }
            }
        }

        //Backtrack from goal to start
        std::vector<Node> path;
        if(found){
            Node curr = goal;
            while (!(curr == start))
            {
                path.push_back(curr);
                curr = parent[curr];
            }
            path.push_back(start);
            std::reverse(path.begin(), path.end());
        }
        return path;
    }
};

int main() {
    int W = 10, H = 5;
    //Create a wall with a vertical wall in the middle
    std::vector<std::vector<int>> map(H, std::vector<int>(W,0));
    for(int i=0; i<4; ++i) map[i][5] = 255;

    BFSPathFinder pathFinder(W,H);
    auto path = pathFinder.findPath({0,0},{9,0},map);

    std::cout << "---Nav v2: BFS Pathfinding Around a Wall---" << std::endl;
    for(int y=0; y<H; ++y) {
        for(int x=0; x<W; ++x){
            bool isPath = false;
            for (auto& p : path) if (p.x == x && p.y == y) isPath = true;

            if(map[y][x] == 255) std::cout << " # ";
            else if(isPath) std::cout << " * ";
            else std::cout << " . ";
        }
        std::cout << std::endl;
    }
    return 0;
}