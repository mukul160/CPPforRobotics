#include <iostream>
#include <cmath> //For sqrt and pow

/**
 * VERSION 1: THE COORDINATE POINT
 * Goal: Represent a position in 2D space and calculate the straight-line
 * distance (Euclidean Distance) between two points.
 */

struct Vector2D {
    double x;
    double y;

    //A method to calculate distance to another point
    //Formula: d=sqrt((x2-x1)^2 + (y2-y1)^2)
    double distanceTo(const Vector2D& other) const {
        double dx = other.x - x;
        double dy = other.y - y;
        return std::sqrt(std::pow(dx,2) + std::pow(dy,2));
    }
};

int main() {
    //Imagine these are coordinates from a GPS or Lidar sensor
    Vector2D robotPos = {0.0, 0.0}; //Starting at the origin
    Vector2D targetPos = {3.0, 4.0}; //Target Waypoint

    double distance = robotPos.distanceTo(targetPos);

    std::cout << "---Robotics Math v1: Euclidean Distance---" << std::endl;
    std::cout << "Robot Position: (" << robotPos.x << ", " << targetPos.y << ")" << std::endl;
    std::cout << "Target Position: (" << targetPos.x << ", " << targetPos.y << ")" << std::endl;
    std::cout << "Distance to Target: " << distance << " units" << std::endl;

    return 0;
}