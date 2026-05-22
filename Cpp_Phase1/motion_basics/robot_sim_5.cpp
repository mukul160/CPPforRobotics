#include <iostream>
#include <cmath>
#include <vector>
using namespace std;

class Robot {
public:
    double x,y;
    double theta;

    double speed;
    double totalDistance;

    double goalX, goalY;
    bool hasGoal;

    vector<pair<double,double>> path;

    Robot(){
        x=0;
        y=0;
        theta=0;

        speed=0.5;
        totalDistance=0;

        hasGoal = false;
        path.push_back({x,y});
    }

    void setGoal(double gx, double gy){
        goalX = gx;
        goalY = gy;
        hasGoal = true;
    }

    void update(){
        if(!hasGoal) return;

        double dx = goalX - x;
        double dy = goalY - y;

        double distace = sqrt(dx*dx + dy*dy);

        if(distace < 0.1){
            cout << "Reached Goal!\n";
            hasGoal = false;
            return;
        }

        double targetAngle = atan2(dy,dx);

        //Limited Turning
        double maxTurn = 0.1;
        double diff = targetAngle - theta;

        if (diff > maxTurn) diff = maxTurn;
        if (diff < -maxTurn) diff = -maxTurn;

        theta += diff;

        double step = min(speed,distace);

        double moveX = step * cos(theta);
        double moveY = step * sin(theta);

        x += moveX;
        y += moveY;

        totalDistance += sqrt(moveX*moveX + moveY*moveY);

        path.push_back({x,y});
    }

    void printState(){
        cout << "Position: (" << x << "," << y << ") | ";
        cout << "Theta: " << theta << " | ";
        cout << "Distance: " << totalDistance << endl;
    }
};

int main(){
    Robot robot;

    int command;

    while(true){
        cout << "\n1: Set Goal | 2: Step Simulation | 3: Run Continuous\nEnter Command: ";
        cin >> command;

        if (command == 1){
            double gx, gy;
            cout << "Enter Goal (x,y): ";
            cin >> gx >> gy;
            robot.setGoal(gx,gy);
        }

        else if (command == 2){
            robot.update();
            robot.printState();
        }

        else if (command == 3){
            // continuous loop 
            for (int i=0; i<50; i++){
                robot.update();
                robot.printState();
            }
        }

        else {
            cout << "Unknown Command\n";
        }
    }

    return 0;
}