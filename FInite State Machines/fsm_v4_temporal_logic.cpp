#include <iostream>
#include <chrono>
#include <thread>

/**
 * VERSION 4: The Temporal FSM
 * Goal: Use non-blocking timers to manage state duration.
 * This prevents the robot from "freezing" while waiting for a task to finish.
 */

 enum class State {
    IDLE,
    DRILLING,
    COOLING
 };

 class IndustrialBot {
    private:
        State currentState = State::IDLE;

        //Time Management using <chrono>
        std::chrono::steady_clock::time_point stateStartTime;

        void transitionTo(State newState) {
            std::cout << "-> Transitioning to " << getStateName(newState) << std::endl;
            currentState = newState;
            stateStartTime = std::chrono::steady_clock::now();
        }

        std::string getStateName(State s) {
            switch(s) {
                case State::IDLE: return "IDLE";
                case State::DRILLING: return "DRILLING";
                case State::COOLING: return "COOLING";
                default: return "UNKNOWN";
            }
        }

    public:
        IndustrialBot() { stateStartTime = std::chrono::steady_clock::now(); }
        
        void update() {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - stateStartTime).count();

            switch (currentState) {
                case State::IDLE:
                    if (elapsed >= 2) {
                        std::cout << "[IDLE] Wait complete. Starting Drill..." << std::endl;
                        transitionTo(State::DRILLING);
                    }
                    break;

                case State::DRILLING:
                    std::cout << "[DRILLING] Working... (Elapsed: " << elapsed << "s)" << std::endl;
                    if (elapsed >= 5) {
                        std::cout << "[DRILLING] Hole finished. Cooling down..." << std::endl;
                        transitionTo(State::COOLING);
                    }
                    break;

                case State::COOLING:
                    if (elapsed >= 1) {
                        std::cout << "[COOLING] Ready for next task." << std::endl;
                        transitionTo(State::IDLE);
                    }
                    break;
            }
        }
 };


 int main() {
    IndustrialBot bot;

    std::cout << "---Robot v4: Temporal Logic (Time-Based)---" << std::endl;

    //We run a loop for 10 seconds of simulated time
    for (int i=0; i<10; i++) {
        bot.update();
        //Standard sleep to simulate real-world clock ticks
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "System Heartbeat" << std::endl;
    }
    return 0;
 }