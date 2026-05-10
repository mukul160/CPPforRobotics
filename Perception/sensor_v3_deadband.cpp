#include <iostream>
#include <cmath>
#include <vector>

/**
 * VERSION 3: THE DEADBAND
 * Goal: Eliminate "Drift" by ignoring values near zero
 */

class DeadbandFilter {
private:
    double threshold;

public:
    DeadbandFilter(double t) : threshold(t) {}

    double apply(double rawValue) {
        //If the absolute value is within the threshold, return 0
        if (std::abs(rawValue) < threshold) {
            return 0.0;
        }

        //Professional Touch: Scaling
        //We don't just return 'rawValue' because that could cause a "jump"
        //at the threshold. We scale the output to start smoothly from 0.

        if (rawValue > 0) {
            return (rawValue - threshold) / (1.0 - threshold);
        }

        else {
            return (rawValue + threshold) / (1.0 - threshold);
        }
    }
};


int main() {
    //Treat anything between -0.1 and 0.1 as zero
    DeadbandFilter stickFilter(0.1);

    //Simulated joystick values (returning to center)
    std::vector<double> joystickInputs = {0.8, 0.5, 0.12, 0.08, -0.05, -0.15};

    std::cout << "---Sensor v3: Deadband Filter---" << std::endl;

    for (double input : joystickInputs) {
        std::cout << "Input: " << input << " | Output: " << stickFilter.apply(input) << std::endl;
    }
    return 0;
}