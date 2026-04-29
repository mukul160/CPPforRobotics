#include <iostream>

/**
 * VERSION 2: Displacement & Operator Overloading
 * Goal: Move the robot by adding a velocity vector to its position.
 */

 struct Vector2D {
    double x;
    double y;

    //Operator overloading: This tells C++ how to handle 'Vector + Vector'
    Vector2D operator+(const Vector2D& other) const {
        return {x + other.x, y + other.y};
    }

    //Scalar Multiplication: Scaling a vector (e.g., Velocity * Time)
    Vector2D operator*(double scalar) const {
        return {x * scalar, y * scalar};
    }
 };

 int main() {
    Vector2D position = {10.0, 5.0};
    Vector2D velocity = {2.0, -1.0}; //Moving right and slightly down
    double dt = 2.0; //Move for two seconds

    //Professional Syntax: New Position = Old Position + (Velocity * Time)
    //Without overloading, this would be a messy chain of function calls.
    Vector2D displacement = velocity * dt;
    Vector2D newPosition = position + displacement;

    std::cout << "---Robotics Math v2: Vector Addition---" << std::endl;
    std::cout << "Initial Pos: (" << position.x << ", " << position.y << ")" << std::endl;
    std::cout << "Velocity: (" << velocity.x << ", " << velocity.y << ")" << std::endl;
    std::cout << "Final Pos: (" << newPosition.x << ", " << newPosition.y << ")" << std::endl;

    return 0;
 }