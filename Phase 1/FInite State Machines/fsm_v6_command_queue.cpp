#include <iostream>
#include <queue>
#include <string>

/**
 * VERSION 6: The Command Queue
 * Goal: Decouple "What to do" from "How to do it."
 * This mimics how a robot receives a mission plan from a user or a high-level AI
 */

enum class Command {
    MOVE_FORWARD,
    PICK_UP,
    ROTATE_90,
    SHUTDOWN
};

class QueueBot {
    private:
        std::queue<Command> missionPlan;

        void execute(Command cmd){
            switch(cmd){
                case Command::MOVE_FORWARD: std::cout << "[EXEC] Moving Forward..." << std::endl; break;
                case Command::PICK_UP: std::cout << "[EXEC] Engaging Gripper..." << std::endl; break;
                case Command::ROTATE_90: std::cout << "[EXEC] Rotating Chassis..." << std::endl; break;
                case Command::SHUTDOWN: std::cout << "[EXEC] Powering Down." << std::endl; break;
            }
        }

        public:
            void addCommand(Command cmd){
                missionPlan.push(cmd);
            }

            void processNext(){
                if(missionPlan.empty()){
                    std::cout << "[IDLE] No commands in queue." << std::endl;
                    return;
                }

                Command currentTask = missionPlan.front();
                execute(currentTask);
                missionPlan.pop(); //Remove the task once "finished"
            }

            bool hasWork() const{return !missionPlan.empty();}
};

int main() {
    QueueBot bot;

    //"Programming" the robot via the queue
    bot.addCommand(Command::MOVE_FORWARD);
    bot.addCommand(Command::ROTATE_90);
    bot.addCommand(Command::PICK_UP);
    bot.addCommand(Command::SHUTDOWN);

    std::cout << "---Robot v6: Command Queue Architecture---" << std::endl;

    while(bot.hasWork()){
        bot.processNext();
    }
    return 0;
}