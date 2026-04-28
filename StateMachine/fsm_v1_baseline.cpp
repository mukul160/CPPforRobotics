#include <iostream>

/**
 * VERSION 1: THE CORE LOOP
 * Goal: Establish the state switching mechanic.
 */

 enum class State {
    IDLE,
    PATROL
 };

 int main() {
    State currentState = State::IDLE;
    int ticks = 0;

    std::cout << "---Robot Baseline FSM Starting---" << std::endl;

    while (ticks < 6) {
        std::cout << "Tick " << ticks << ": ";

        if (currentState == State::IDLE) {
            std::cout << "System is IDLE. Switching to PATROL..." << std::endl;
            currentState = State::PATROL;
        }

        else if (currentState == State::PATROL) {
            std::cout << "System is PATROLLING. Switching to IDLE..." << std::endl;
            currentState = State::IDLE;
        }
        ticks++;
    }
    return 0;
 }