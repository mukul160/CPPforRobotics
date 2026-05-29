#include <iostream>
#include <cmath>
#include <algorithm> //For std::clamp

/**
 * VERSION 4: CALCULATING ANGLES
 * Goal: Find the precise angle (in degree) between two vector.
 */
struct Vector2D {
    double x;
    double y;

    //Magnitude (Length) of the vector
    double length() const {
        return std::sqrt(x*x + y*y);
    }

    double dot(const Vector2D& other) const {
        return (x*other.x) + (y*other.y);
    }

    //Angle in degrees
    double angleWidth(const Vector2D& other) const {
        double magA = this->length();
        double magB = other.length();

        if (magA == 0 || magB == 0) return 0.0;

        //Calculate cosine and clamp to [-1,1] to avoid math errors
        double cosTheta = this->dot(other) / (magA * magB);
        cosTheta = std::clamp(cosTheta, -1.0, 1.0);

        //acos returns radians, so we convert to degrees
        return std::acos(cosTheta) * (180.0/M_PI);
    }
};

int main() {
    Vector2D currentHeading = {1.0, 0.0}; //Facing East
    Vector2D targetVector = {0.0, 1.0}; //Target is North

    double angle = currentHeading.angleWidth(targetVector);

    std::cout << "---Robotics Math v4: Angle Calculation---" << std::endl;
    std::cout << "Angle to turn: " << angle << " degrees" << std::endl;

    return 0;   
}