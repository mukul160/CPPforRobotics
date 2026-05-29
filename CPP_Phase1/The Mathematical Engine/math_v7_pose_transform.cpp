#include <iostream>
#include <cmath>
#include <iomanip>

struct Vector2D {
    double x,y;

    //From v6: Rotating a point
    Vector2D rotated(double degrees) const {
        double rad = degrees * (M_PI / 180.0);
        return {
            x*std::cos(rad) - y*std::sin(rad),
            x*std::sin(rad) + y*std::cos(rad) 
        };
    }

    //From v2: Adding a displacement
    Vector2D operator+(const Vector2D& other) const {
        return {x + other.x, y + other.y};
    }
};

/**
 * VERSION 7: THE POSE (POSITION + ORIENTATION)
 * Goal: Transform a local sensor reading into a global map coordinate.
 */

 struct Pose2D {
    Vector2D position;
    double theta; //Orientation in Degrees

    //The 'Transform' - the bread and butter of robotics navigation
    Vector2D transformToGlobal(const Vector2D& localPoint) const {
        // Step 1: Rotate the local point by the robot's current heading
        Vector2D rotatedPoint = localPoint.rotated(theta);

        //Step 2: Translate (add) the robot's current position
        return position + rotatedPoint;
    }
 };

 int main() {
    //The Robot's state in the world
    Pose2D robotPose = {{10.0, 5.0}, 90.0}; //At (10.5) facing "North"

    //A sensor detects a wall 2 meters directly in front of the robot
    Vector2D sensorReading = {2.0, 0.0};

    //Where is that wall on the global map
    Vector2D wallInGlobal = robotPose.transformToGlobal(sensorReading);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "---Robotics Math v7: The Pose Transform---" << std::endl;
    std::cout << "Robot Pose: Pos(" << robotPose.position.x << ", " << robotPose.position.y
                << ") Heading: " << robotPose.theta << " deg" << std::endl;
    std::cout << "Local Sensor Point: (" << sensorReading.x << ", " << sensorReading.y << ")" << std::endl;
    std::cout << "Global Map Point: (" << wallInGlobal.x << ", " << wallInGlobal.y << ")" << std::endl;

    return 0;
 }