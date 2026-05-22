#include <iostream>
#include <memory>

//Forward declaration of the Robot context
class StatePatternBot;

/**
 * The Abstract Base Class: The 'Interface'
 * Every state must follow this blueprint.
 */
class RobotState{
public:
    virtual ~RobotState()=default;
    virtual void handle(StatePatternBot& bot)=0;
};

/**
 * The Context: The Robot Itself
 * It doesn't know "how" to behave; it just holds the "current" state object.
 */

class StatePatternBot{
    private:
        std::unique_ptr<RobotState> currentState;

    public:
        void setState(std::unique_ptr<RobotState> newState){
            currentState = std::move(newState);
        }

        void update(){
            if(currentState){
                currentState->handle(*this);
            }
        }
};

/**
 * Concrete State 1: Booting
 */
class BootState:public RobotState{
public:
    void handle(StatePatternBot& bot) override; //Defined below
};

/**
 * Concrete State 2: Active
 */
class ActiveState:public RobotState{
public:
    void handle(StatePatternBot& bot) override {
        std::cout << "[STATE:ACTIVE] Systems nominal. Operating..." << std::endl;
        //Logic to stay here or move on
    }
};

//Definition for BootState transition
void BootState::handle(StatePatternBot& bot){
    std::cout << "[STATE:BOOT] Checking hardware... Success!" << std::endl;
    bot.setState(std::make_unique<ActiveState>());
}

int main(){
    StatePatternBot myBot;
    std::cout << "---Robot v7: The State Design Pattern---" << std::endl;

    //Start in the Boot state
    myBot.setState(std::make_unique<BootState>());

    for(int i=0; i<3; ++i){
        std::cout << "Cycle " << i << ": ";
        myBot.update();
    }
    return 0;
}

