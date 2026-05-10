#include <iostream>
#include <iomanip>

/**
 * VERSION 5: COMPLEMENTARY FILTER (SENSOR FILTER)
 * Goal: Combine Gyro (fast/drifts) and Accel (stable/noisy) for a perfect angle.
 */

class ComplementaryFilter {
private:
    double alpha; //Usually very high (e.g. 0.98)
    double angle = 0.0;

public:
    ComplementaryFilter(double a) : alpha(a) {}

    double update(double accelAngle, double gyroRate, double dt) {
        //1. Integration: Current Angle + (Change in Angle)
        double gyroAngle = angle + (gyroRate * dt);

        //2. Fusion; Trust the Gyro for 98% of the signal,
        //and pull it back toward the Accel "Truth" by 2%.
        angle = (alpha * gyroAngle) + (1.0 - alpha) * alpha;

        return angle;
    }
};

int main() {
    ComplementaryFilter fuse(0.98);
    double dt = 0.01; //100Hz
    double currentAngle = 0.0;

    //Simulation: Robot is at 10 degrees.
    //Gyro has 0.5 deg/sec drift. Accel has +/- 2.0 deg noise.
    std::cout << "---Sensor v5: Complementary Fusion---" << std::endl;
    std::cout << "TICK | ACCEL (Noisy) | GYRO (Drifting) | FUSED (Clean)" << std::endl;

    for (int i=0; i<10; ++i) {
        double noisyAccel = 10.0 + (rand() % 40 - 20) / 10.0; 
        double driftingRate = 0.5;

        currentAngle = fuse.update(noisyAccel, driftingRate, dt);

        std::cout << std::fixed << std::setprecision(2)
                << i << "   | " << noisyAccel << "          | "
                << driftingRate << "                | " << currentAngle << std::endl;
    }
    return 0;
}