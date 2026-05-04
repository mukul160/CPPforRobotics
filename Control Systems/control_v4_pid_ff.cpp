#include <iostream>
#include <cmath>

/**
 * VERSION 4: PID + FEED-FORWARD
 * Goal: Use knowledge of physics (gravity) to assist the controller.
 */
class PID_FF_Controller {
private:
    double kp, ki, kd, kf; // kf is the Feed-Forward gain
    double target;
    double integralSum = 0.0;
    double lastError = 0.0;

public:
    PID_FF_Controller(double p, double i, double d, double f, double setpoint) 
        : kp(p), ki(i), kd(d), kf(f), target(setpoint) {}

    double calculate(double currentPosition, double dt) {
        double error = target - currentPosition;

        // PID Terms
        double p_term = kp * error;
        
        integralSum += error * dt;
        double i_term = kp * integralSum;

        double derivative = (error - lastError) / dt;
        double d_term = kd * derivative;
        
        lastError = error;

        // Feed-Forward Term: kf * target 
        // (In an arm, this might be kf * cos(angle) to fight gravity)
        double ff_term = kf * target;

        return ff_term + p_term + i_term + d_term;
    }
};

int main() {
    // Imagine an arm that needs 10 units of power just to hold itself up
    // kf = 0.1 means if target is 100, we start with 10 units of power instantly
    PID_FF_Controller armControl(0.5, 0.1, 0.002, 0.1, 100.0);
    
    double currentPos = 0.0;
    double dt = 0.1;

    std::cout << "--- Control v4: PID + Feed-Forward ---" << std::endl;

    for (int i = 0; i < 101; ++i) {
        double power = armControl.calculate(currentPos, dt);
        
        // Simulating gravity: 10 units of power are "lost" just holding the weight
        double effectivePower = power - 10.0; 
        currentPos += (effectivePower * 0.5);

        std::cout << "Tick " << i << " | Total Power: " << power 
                  << " | Position: " << currentPos << std::endl;
    }

    return 0;
}