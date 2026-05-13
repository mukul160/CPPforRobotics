#include <iostream>
#include <algorithm>

/**
 * VERSION 5: THE INDUSTRIAL PID
 * Goal: Prevent "Integral Windup" and handle physical motor limits.
 */

class IndustrialPID {
private:
    double kp, ki, kd;
    double target;
    double integralSum = 0.0;
    double lastError = 0.0;

    //Physical Limits
    double minOutput = -100.0;
    double maxOutput = 100.0;

public:
    IndustrialPID(double p, double i, double d, double setpoint)
        : kp(p), ki(i), kd(d), target(setpoint) {}

    double calculate(double currentPosition, double dt) {
        double error = target - currentPosition;

        //Proportional & Derivative
        double p_term = kp * error;
        double d_term = kd * (error - lastError) / dt;
        lastError = error;

        //Integral with Anti-Windup Logic
        //We only add to the integral if we haven't hit the power limit yet
        double potentialOutput = p_term + (ki * (integralSum + error * dt)) + d_term;

        if (potentialOutput > minOutput && potentialOutput < maxOutput) {
            integralSum += error * dt;
        }

        double i_term = ki * integralSum;
        double totalOutput = p_term + i_term + d_term;

        //3. Output Saturation (The Physical Cap)
        return std::clamp(totalOutput, minOutput, maxOutput);
    }
};

int main() {
    IndustrialPID safePID(2.0, 0.5, 0.1, 100.0);
    double currentPos = 0.0;
    double dt = 0.1;

    std::cout << "---Control v5: Safety & Saturation---" << std::endl;

    for (int i=0; i<101; ++i) {
        //Imagine the robot is stuck for the first 5 ticks
        bool isStuck = (i < 5);

        double power = safePID.calculate(currentPos, dt);

        if(!isStuck) {
            currentPos += (power * 0.1);
        }

        std::cout << "Tick " << i << (isStuck ? " [STUCK]" : " [MOVING]")
                    << " | Power: " << power << " | Position: " << currentPos << std::endl;
    }
    return 0;
}