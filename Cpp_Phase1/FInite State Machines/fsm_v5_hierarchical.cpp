#include <iostream>
#include <string>

/**
 * VERSION 5: HIERARCHICAL STATES (HSM)
 * Goal: Nest "Sub-states" within "Top-level" states.
 * This allows a robot to have a "Mission" while managing specific tasks.
 */

 enum class TopLevelState {
    STANDBY,
    MISSION_ACTIVE
 };

 enum class SubState {
    MOVING_TO_POINT,
    PERFORMING_TASK,
    NONE //Used when in STANDBY
 };

 class HeavyLifter {
    private:
        TopLevelState mainState = TopLevelState::STANDBY;
        SubState taskState = SubState::NONE;
        int progress = 0;

    public:
        void update() {
            switch (mainState) {
                case TopLevelState::STANDBY:
                    std::cout << "[MAIN: STANDBY] Awaiting Mission..." << std::endl;
                    //Transition trigger: Imagine a remote command received
                    mainState = TopLevelState::MISSION_ACTIVE;
                    taskState = SubState::MOVING_TO_POINT;
                    break;

                case TopLevelState::MISSION_ACTIVE:
                    handleMission();
                    break;

            }
        }

    private:
        void handleMission() {
            std::cout << "[MAIN MISSION] -> ";

            switch(taskState){
                case SubState::MOVING_TO_POINT:
                    std::cout << "Sub: NAVIGATING. Progress: " << progress << "%" << std::endl;
                    progress += 50;
                    if(progress >= 100){
                        progress=0;
                        taskState = SubState::PERFORMING_TASK;
                    }
                    break;

                case SubState::PERFORMING_TASK:
                    std::cout << "Sub: LIFTING CARGO..." << std::endl;
                    //Task complete, go back to standby
                    mainState = TopLevelState::STANDBY;
                    taskState = SubState::NONE;
                    break;

                case SubState::NONE:
                    break;
            }
        }
 };

 int main() {
    HeavyLifter robot;

    std::cout << "---Robot v5: Hierarchical State Machine---" << std::endl;

    for(int i=0; i<5; ++i){
        robot.update();
    }

    return 0;
 }