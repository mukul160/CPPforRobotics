#include <iostream>

/**
 * VERSION 3: THE DOT PRODUCT
 * Goal: Use the dot product to determine directional alignment.
 */

 struct Vector2D {
    double x;
    double y;

    //The Dot Product Formula: (x1 * x2) + (y1 * y2)
    double dot(const Vector2D& other) const {
        return (x * other.x) + (y * other.y);
    }
 };

 int main() {
    //A robot facing 'Up' (Forward)
    Vector2D robotHeading = {0.0, 1.0};

    //Target A is in front
    Vector2D targetA = {0.5, 0.8};
    //Target B is behind
    Vector2D targetB = {0.2, -0.5};

    std::cout << "---Robotics Math v3: The Dot Product---" << std::endl;

    double resultA = robotHeading.dot(targetA);
    double resultB = robotHeading.dot(targetB);

    std::cout << "Dot product with Target A: " << resultA
            << (resultA > 0 ? " (Target is IN FRONT)" : " (TARGET IS BEHIND)") << std::endl;

    std::cout << "Dot product with Target B: " << resultB
            << (resultB > 0 ? " (Target is IN FRONT)" : " (TARGET IS BEHIND)") << std::endl;

    return 0;    
 }