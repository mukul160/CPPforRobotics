#include <iostream>
#include <string>

/**
 * VERSION 3: THE EVENT DRIVEN BRAIN
 * Goal: Introduce "Sensors" (inputs) and internal logic to trigger transitions.
 * This makes the robot reactive rather than just repetitive.
 */

enum class State {
    IDLE,
    PROCESSING,
    EMERGENCY_STOP
};

class SmartBot {
private:
    State currentState = State::IDLE;

    //Internal "Sensors"
    bool sensor_data_detected = false;
    double system_temperature = 25.0; //Celsius

    //Action Methods: Professional robots isolate actions from logic
    void stopAllMotors() {
        std::cout << "!!! MOTORS DISENGAGED !!!" << std::endl;
    }

public:
    //This simulates an external sensor update (the "Input")
    void setSensorData(bool detected, double temp){
        sensor_data_detected = detected;
        system_temperature = temp;
    }

    void update() {
        //1. Safety Layer: Check for criical failures first (High Priority)
        if(system_temperature > 100.0) {
            currentState = State::EMERGENCY_STOP;
        }

        //2. Behavorial Logic
        switch (currentState) {
            case State::IDLE:
                std::cout << "[IDLE] Monitoring sensors..." << std::endl;
                if(sensor_data_detected){
                    std::cout << "-> Object detected! Starting process..." << std::endl;
                    currentState = State::PROCESSING;
                }
                break;

            case State::PROCESSING:
                std::cout << "[PROCESSING] Analyzing data..." << std::endl;
                // Once finished, we reset the sensor and go to IDLE
                sensor_data_detected = false;
                currentState = State::IDLE;
                break;

            case State::EMERGENCY_STOP:
                std::cout << "[FATAL] System Overhead! Standing by for technician." << std::endl;
                stopAllMotors();
                break;
        }   
    }
};

int main() {
    SmartBot bot;

    std::cout << "---Robot v3: Event-Driven Logic---" << std::endl;
    
    //Simulation Loop
    for (int i=0; i<5; ++i){
        std::cout << "\n--- Step " << i << " ---" << std::endl;

        if(i==2) bot.setSensorData(true, 30.0); //Trigger a detection
        if(i==4) bot.setSensorData(false,110.0); //Trigger a failure

        bot.update();
    }
    return 0;
}