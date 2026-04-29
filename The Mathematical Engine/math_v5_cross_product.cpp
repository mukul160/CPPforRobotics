#include <iostream>
#include <string>

/**
 * VERSION 5: THE 2D CROSS PRODUCT (SENSE OF DIRECTION)
 * Goal: Determine the "Side" of a target to choose turn direction.
 */

struct Vector2D {
    double x;
    double y;

    //2D Cross Product: (x1 * x2) - (y1 * y2)
    double cross(const Vector2D& other) const {
        return (x * other.y) - (y * other.x);
    }
};

int main() {
    //Robot is facing 'Forward' (Up)
    Vector2D heading = {0.0, 1.0};

    //Target A is to the right
    Vector2D targetA = {1.0, 1.0};

    //Target B is to the left
    Vector2D targetB = {-1.0, 1.0};

    std::cout << "---Robotics Math v5: 2D Cross Product---" << std::endl;

    double sideA = heading.cross(targetA);
    double sideB = heading.cross(targetB);

    std::cout << "Target A Cross Result: " << sideA
                << (sideA < 0 ? " -> Turn RIGHT" : " -> Turn LEFT") << std::endl;

                
    std::cout << "Target B Cross Result: " << sideB
                << (sideB < 0 ? " -> TURN RIGHT " : " -> Turn LEFT ") << std::endl;

    return 0;
}