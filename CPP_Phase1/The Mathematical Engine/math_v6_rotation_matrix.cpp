#include <iostream>
#include <cmath>
#include <iomanip>

/**
 * VERSION 6: ROTATION MATRICES
 * Goal: Rotate a vector by a specific angle.
 * This is how a robot "maps" a sensor reading back to the real world.
 */

 struct Vector2D {
    double x;
    double y;

    //Rotate the vector counter-clockwise by 'degrees'
    Vector2D rotate(double degrees) const {
        //Step 1: Convert to radians
        double rad = degrees * (M_PI / 180.0);

        //Step 2: Apply the rotation matix formula
        double newX = x*std::cos(rad) - y*std::sin(rad);
        double newY = x*std::sin(rad) + y*std::cos(rad);

        return {newX, newY};
    }
 };

 int main() {
    //A Lidar sensor detected an obstacle 5 meters directly in front of the robot
    Vector2D localObstacle = {1.0, 0.0};

    //But the robot itself is currently rotated 45 degrees!
    double robotRotation = 90.0;

    Vector2D globalObstacle = localObstacle.rotate(robotRotation);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "---Robot Math v6: Rotation---" << std::endl;
    std::cout << "Local (Sensor) Obstacle: (" << localObstacle.x << ", " << localObstacle.y << ")" << std::endl;
    std::cout << "Global (World) Obstacle: (" << globalObstacle.x << ", " << globalObstacle.y << ")" << std::endl;

    return 0;
 }