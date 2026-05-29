#include <iostream>
#include <vector>
#include <iomanip>

/**
 * CHALLENGE - THE INFLATION COSTMAP
 * Goal: Take raw obstacles and "inflates" them so the robot stays safe.
 */

class Costmap2D {
private:
    int width, height;
    std::vector<std::vector<int>> grid;

public:
    Costmap2D(int w, int h) : width(w), height(h),
                                grid(h, std::vector<int>(w,0)) {}

    void setObstacle(int x, int y) {
        if (x >= 0 && x < width && y >=0 && y < height) {
            grid[y][x] = 255; //Lethal
            inflate(x,y);
        }
    }

    void inflate(int x, int y) {
        //Higher Complexity: Apply a "Buffer Zone" around obstacle
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int nx = x + dx, ny = y + dy;
                if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                    if (grid[ny][nx] != 255) {
                        grid[ny][nx] = std::max(grid[ny][nx], 128); //"Caution" zone
                    }
                }
            }
        }
    }

    void display() {
        for (const auto& row : grid) {
            for (int cell : row) {
                if (cell == 255) std::cout << " # "; //Wall
                else if (cell == 128) std::cout << " . "; //Buffer
                else std::cout << "  "; //Free
            }
            std::cout << "|" << std::endl;
        }
    }
};

int main() {
    Costmap2D map(10, 5);

    //Create a "Wall"
    map.setObstacle(5,1);
    map.setObstacle(5,2);
    map.setObstacle(5,3);

    std::cout << "--- Nav v1: Occupy Grid with Inflation ---" << std::endl;
    map.display();

    return 0;
}