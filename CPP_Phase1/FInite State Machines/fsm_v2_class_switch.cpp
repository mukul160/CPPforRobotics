#include <iostream>

/**
 * VERSION 2: THE SWITCH UPGRADE
 * Goal: Use a class and a switch statement for cleaner "branching" logic.
 * This is the standard "engine" for most hobbyist and industrial robots.
 */

enum class State {
    IDLE,
    SCANNING,
    MOVING
 };

class DiscoveryBot {
private:
    State currentState = State::IDLE;

public:
    void update() {
        switch (currentState) {
            case State::IDLE:
                std::cout << "[IDLE] System ready. Initiating SCAN..." << std::endl;
                currentState = State::SCANNING;
                break;

            case State::SCANNING:
                std::cout << "[SCANNING] Environment clear. Initiating MOVE..." << std::endl;
                currentState = State::MOVING;
                break;

            case State::MOVING:
                std::cout << "[MOVING] Target reached. Returning to IDLE..." << std::endl;
                currentState = State::IDLE;
                break;
            
            default:
                std::cout << "[ERROR] Unknown State!" << std::endl;
                break;
        }
    }
 };


int main() {
    DiscoveryBot myBot;

    std::cout << "---Robot v2: Class & Switch Starting---" << std::endl;

    for(int i=0; i<6; ++i) {
        std::cout << "Cycle " << i << " -> ";
        myBot.update();
    }

    return 0;
}