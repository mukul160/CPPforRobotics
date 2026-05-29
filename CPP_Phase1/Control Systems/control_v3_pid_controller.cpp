#include <iostream>
#include <iomanip>

/**
 * VERSION 3: THE FULL PID Controller
 * Goal: Smooth, Precise Movement with no overshoot.
 */

class PID_Controller {
    private:
        double kp, ki, kd;
        double target;
        double integralSum = 0.0;
        double lastError = 0.0; //Required to calculate the slope

    public:
        PID_Controller(double p, double i, double d, double setpoint)
            : kp(p), ki(i), kd(d), target(setpoint) {}

        double calculate(double currentPosition, double dt) {
            double error = target - currentPosition;

            //P Term
            double p_term = kp * error;

            //I Term
            integralSum += error * dt;
            double i_term = ki * integralSum;

            //D Term: (Change in Error)/(Change in Time)
            double derivative = (error - lastError) / dt;
            double d_term = kd * derivative;

            //Save error for the next tick
            lastError = error;

            return p_term + i_term + d_term;
        }
};

int main() {
    // Kp=0.5, Ki=0.1, Kd=0.002, Target=100.0
    PID_Controller pid(0.5,0.1,0.002,100.0);
    double currentPos = 0.0;
    double dt = 0.1;

    std::cout << "---Control v3: The Full PID---" << std::endl;
    std::cout << std::fixed << std::setprecision(2);

    for (int i=0; i<100; ++i){
        double power = pid.calculate(currentPos,dt);

        //Simulating a system with momentum
        currentPos += (power * 0.5);

        std::cout << "Tick " << i << " | Power: " << std::setw(6) << power
                    << " | Position: " << currentPos << std::endl;
    }
    return 0;
}