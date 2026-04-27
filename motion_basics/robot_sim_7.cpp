#include <iostream>
#include <cmath>
using namespace std;

class Robot {
public:
    double x,y,theta;

    double goalX,goalY;
    bool hasGoal;

    //Controller Gains
    double k_linear;
    double k_angular;

    Robot() {
        x=0;
        y=0;
        theta=0;

        hasGoal = false;

        k_linear=0.5; //how fast it moves
        k_angular=1.0; //how fast it turns

    }

    void setGoal(double gx, double gy){
        goalX = gx;
        goalY = gy;
        hasGoal = true;
    }

    double distanceToGoal(){
        double dx = goalX - x;
        double dy = goalY - y;
        return sqrt(dx*dx + dy*dy);
    }

    void update(){
        if(!hasGoal) return;

        double dx = goalX - x;
        double dy = goalY - y;

        double distance = sqrt(dx*dx + dy*dy);

        if(distance < 0.05){
            cout << "Reached Goal!\n";
            hasGoal = false;
            return;
        }

        //desired direction
        double targetAngle = atan2(dy,dx);

        //angle error
        double angleError = targetAngle - theta;

        //normalize angle
        while (angleError > M_PI) angleError -= 2*M_PI;
        while (angleError < -M_PI) angleError += 2*M_PI;

        //Proportional Control
        double angularVelocity = k_angular * angleError;
        double linearVelocity = k_linear * distance;

        //Update Orientation
        theta += angularVelocity;

        //Move forward
        x += linearVelocity * cos(theta);
        y += linearVelocity * sin(theta);

    }

    void printState(){
        cout << "Pos: (" << x << "," << y << ")";
        cout << "Theta: " << theta << endl;
    }
};

int main(){
    Robot robot;

    int command;

    while (true) {
        cout << "\n1: Set Goal | 2: Step | 3: Run\nEnter:";
        cin >> command;

        if(command == 1){
            double gx, gy;
            cout << "Enter Goal: ";
            cin >> gx >> gy;
            robot.setGoal(gx,gy);
        }

        else if (command == 2){
            robot.update();
            robot.printState();
        }

        else if (command == 3){
            for (int i=0; i<100; i++){
                robot.update();
                robot.printState();
            }
        }
    }
    return 0;
}