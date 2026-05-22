#include <iostream>
#include <vector>
#include <iomanip>

/**
 * VERSION 1: THE LOW PASS FILTER
 * Goal: Smooth out "jittery" sensor data
 */

class LowPassFilter {
private:
    double alpha; //Smoothing factor (0.0 to 1.0)
    double filteredValue;
    bool initialized = false;

public:
    LowPassFilter(double a) : alpha(a), filteredValue(0.0) {}

    double update(double rawValue) {
        if(!initialized) {
            filteredValue = rawValue;
            initialized = true;
            return filteredValue;
        }

        // The LPF Formula
        filteredValue = (alpha * rawValue) + (1.0 - alpha) * filteredValue;
        return filteredValue;
    }
};

int main() {
    LowPassFilter lpf(0.2); //20% New Data, 80% Old Data

    //Simulated Noisy Sensor Data (Target is 10.0, but sensor is jittering)
    std::vector<double> noisyData = {10.5, 9.8, 11.2, 9.5, 10.1, 10.8, 9.2, 10.3};

    std::cout << "---Sensor v1: Low-Pass Filter---" << std::endl;
    std::cout << "RAW | FILTERED" << std::endl;

    for (double raw : noisyData) {
        double clean = lpf.update(raw);
        std::cout << std::fixed << std::setprecision(2)
                    << raw << " | " << clean << std::endl;
    }

    return 0;
}