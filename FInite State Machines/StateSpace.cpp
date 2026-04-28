#include <iostream>
#include <iomanip>

class RobotSystem {
public:
    //Physical Constraints you can Tweak
    double mass = 1.0;
    double friction = 0.1;

    double velocity = 0.0;
    double position = 0.0;

    void update(double motor_force, double dt) {
        //Physics -> a=F/m
        double acceleration = (motor_force - (velocity * friction))/mass;
        velocity += acceleration * dt;
        position += velocity * dt;
    }
};

int main() {
    RobotSystem myRobot;

    //---TWEAK THESE INPUTS---
    double kp = 1.0;
    double kd = 10.0;
    double target = 10.0;
    double dt = 0.1;
    //---

    double last_error = target - myRobot.position;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Starting Simulation...\n";

    for(int t=0; t<30; ++t) {
        double error = target - myRobot.position;

        //PD Logic
        double p_term = kp * error;
        double d_term = kd * ((error - last_error)/dt);
        double force = p_term + d_term;

        myRobot.update(force,dt);
        last_error = error;

        //Visualizing the "Game State"
        std::cout << "Time " << (t*dt) << "s | Pos: " << myRobot.position;

        //Print a simple bar to see the movement visually
        int visual = static_cast<int>(myRobot.position * 2);
        std::cout << " [" << std::string(visual,'#') << "]" << std::endl;
    }

    return 0;
}