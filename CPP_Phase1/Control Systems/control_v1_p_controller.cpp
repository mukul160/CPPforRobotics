#include <iostream>
#include <iomanip>

/**
 * VERSION 1: THE PROPORTIONAL (P) CONTROLLER
 * Goal: Move a robot arm to a target angle.
 */
class P_Controller {
private:
    double kp;       // Proportional Gain (How "aggressive" the robot is)
    double target;   // The Setpoint

public:
    P_Controller(double gain, double setpoint) : kp(gain), target(setpoint) {}

    double calculate(double currentPosition) {
        // Step 1: Calculate the Error
        double error = target - currentPosition;

        // Step 2: Output = Gain * Error
        return kp * error;
    }
};

int main() {
    // Target: 90 degrees. Gain: 0.5
    P_Controller motorControl(0.5, 90.0);
    double currentAngle = 0.0;

    std::cout << "--- Control v1: The P-Controller ---" << std::endl;

    // Simulate 10 "ticks" of the control loop
    for (int i = 0; i < 10; ++i) {
        double power = motorControl.calculate(currentAngle);
        
        // Simulate the motor moving (it won't perfectly reach the goal instantly)
        currentAngle += power; 

        std::cout << "Tick " << i << " | Power: " << std::setw(6) << power 
                  << " | Angle: " << currentAngle << " deg" << std::endl;
    }

    return 0;
} 