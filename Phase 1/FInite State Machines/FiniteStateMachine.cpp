#include <iostream>
#include <string>

// The "states" of our Robot
enum class RobotState {
    IDLE,
    MOVING,
    CHARGING,
    ERROR
};

class HabitBot {
private:
    RobotState currentState = RobotState::IDLE;
    int batteryLevel = 5;

public:
    void update() {
        switch (currentState) {
            case RobotState::IDLE:
                std::cout << "[IDLE] Waiting for commands..." << std::endl;
                if(batteryLevel < 10) currentState = RobotState::CHARGING;
                else currentState = RobotState::MOVING;
                break;
                
            case RobotState::MOVING:
                std::cout << "[MOVING] Executing Task..." << std::endl;
                batteryLevel -= 5;
                if(batteryLevel <= 0) currentState = RobotState::ERROR;
                else currentState = RobotState::IDLE;
                break;

            case RobotState::CHARGING:
                std::cout << "[CHARING] Powering Up..." << std::endl;
                batteryLevel += 50;
                currentState = RobotState::IDLE;
                break;

            case RobotState::ERROR:
                std::cout << "[ERROR] System Failure! Reset Required." << std::endl;
                break;
        }
    }
};

int main() {
    HabitBot bot;

    //Simulate 5 "ticks" of the robot's life
    for(int i=0; i<5; ++i) {
        bot.update();
    }

    return 0;
}

