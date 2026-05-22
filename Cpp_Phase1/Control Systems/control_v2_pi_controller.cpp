#include <iostream>
#include <vector>

/**
 * VERSION 2: THE PI-CONTROLLER
 * Goal: Eliminiate steady state error using an "Accumulator".
 */

 class PI_Controller {
    private:
        double kp;
        double ki;
        double target;
        double integralSum = 0.0; //The memory of past errors

    public:
        PI_Controller(double p_gain, double i_gain, double setpoint)
            : kp(p_gain), ki(i_gain), target(setpoint) {}

        double calculate(double currentPosition, double dt) {
            double error = target - currentPosition;

            //The Integral Step: Accumulate the error over time
            integralSum += error * dt;

            double p_term = kp * error;
            double i_term = ki * integralSum;

            return p_term + i_term;
        }
 };

 int main() {
    //Imagine a motor fighting heavy friction
    PI_Controller motorControl(0.5, 0.2, 100.0);
    double currentPos = 0.0;
    double dt = 0.1; //100ms time step

    std::cout << "--- Control v2: The PI-Controller ---" << std::endl;

    for (int i=0; i<15; ++i) {
        double power = motorControl.calculate(currentPos, dt);

        //Simulating friction: the motor is less efficient than the math says
        currentPos += (power * 0.8);

        std::cout << "Tick" << i << " | Integral: " << (power - (0.5 * (100-currentPos)))
                    << " | Position: " << currentPos << std::endl;
    }
    return 0;
 }