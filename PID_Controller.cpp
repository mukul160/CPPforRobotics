#include <iostream>
#include <vector>

class PIDController {
public:
    PIDController(double p, double i, double d):kp(p),ki(i),kd(d) {}

    double compute(double setpoint, double measured_value, double dt) {
        double error = setpoint - measured_value;

        //Proportional: Fix the error now.
        double Pout = kp * error;

        //Integral: Fix the error that has built up over time (overcomes friction).
        integral += error * dt;
        double Iout = ki * integral;

        //Derivative: Predict the future to prevent overshooting the target.
        double derivative = (error - last_error)/dt;
        double Dout = kd * derivative;

        last_error = error;

        return Pout + Iout + Dout;
    }

private:
    double kp, ki, kd;
    double integral = 0.0;
    double last_error = 0.0;

};

int main() {
    //Initializing with specific "tuning" gains
    PIDController pid(0.1,0.1,0.05);

    double robot_pos = 0.0;
    double target = 100;
    double time_step = 0.1; //100ms intervals

    for(int i=0; i<100; i++){
        double adjustment = pid.compute(target, robot_pos, time_step);
        robot_pos += adjustment;

        std::cout << "Cycle " << i << " | Position: " << robot_pos << std::endl;
    }

    return 0;
}