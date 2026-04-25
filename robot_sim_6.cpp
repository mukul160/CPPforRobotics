#include <iostream>
#include <cmath>
using namespace std;

class Robot{
public:
    double x,y,theta;
    double speed;

    double goalX, goalY;
    bool hasGoal;

    //obstacle position
    double obsX, obsY;

    Robot(){
        x=0;y=0;theta=0;
        speed=0.5;
        hasGoal=false;

        obsX=3;
        obsY=3;
    }

    void setGoal(double gx, double gy){
        goalX = gx;
        goalY = gy;
        hasGoal = true;
    }

    double distanceTo(double tx, double ty){
        double dx = tx - x;
        double dy = ty - y;
        return sqrt(dx*dx + dy*dy);
    }

    void update(){
        if(!hasGoal) return;

        double distGoal = distanceTo(goalX,goalY);

        if(distGoal < 0.1){
            cout << "Reached Goal!\n";
            hasGoal = false;
            return;
        }

        //distance to obstacle
        double distObs = distanceTo(obsX, obsY);

        double targetAngle;

        if(distObs < 1.5){
            //Avoid Obstacle
            cout << "Avoiding Obstacle!\n";
            double step = speed;

            //move away from obstacle
            double dx = x - obsX;
            double dy = y - obsY;

            targetAngle = atan2(dy,dx);
        }

        else {
            //normal goal-seeking
            double dx = goalX - x;
            double dy = goalY - y;
            
            targetAngle = atan2(dy,dx);
        }

        //lmited turning
        double maxTurn = 0.1;
        double diff = targetAngle - theta;

        //normalize to [-pi,pi]
        while(diff > M_PI) diff -= 2*M_PI;
        while(diff < -M_PI) diff += 2*M_PI;

        if(diff > maxTurn) diff = maxTurn;
        if(diff < maxTurn) diff = -maxTurn;

        theta += diff;

        double step = min(speed, distGoal);

        x += step * cos(theta);
        y += step * sin(theta);
    }

    void printState(){
        cout << "Pos: (" << x << ", " << y << ") ";
        cout << "Theta: " << theta << endl;
    }
};

int main(){
    Robot robot;

    int command;

    while(true){
        cout << "\n1: Set Goal | 2: Step | 3: Run\nEnter: ";
        cin >> command;

        if(command == 1){

            double gx,gy;

            cout << "Enter the x and y coordinate of the goal!\n";
            if(!(cin >> gx >> gy)){
                cin.clear();
                cin.ignore(1000,'\n');
                cout << "Invalid values!\n";
                continue;
            }

            robot.setGoal(gx,gy);
            cout << "Goal set, press 2 or 3 to proceed!\n";
        }
        else if(command == 2){
            robot.update();
            robot.printState();
        }
        else if(command == 3){
            for(int i=0; i<200; i++){
                robot.update();
                robot.printState();
            }
        }
    }
    return 0;
}