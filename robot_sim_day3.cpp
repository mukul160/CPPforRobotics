#include <iostream>
#include <cmath>
#include <vector>

using namespace std;

class Robot {
public:
    double x,y;
    double theta;

    vector<pair<double,double>> path;

    Robot(){
        x=0;
        y=0;
        theta=0;
        path.push_back({x,y});
    }

    void move(double rotation, double forward){
        theta += rotation;

        double dx = forward * cos(theta);
        double dy = forward * sin(theta);

        x += dx;
        y += dy;

        path.push_back({x,y});
    }

    void moveToGoal(double goalX, double goalY){
        //Compute direction to goal
        double dx = goalX - x;
        double dy = goalY - y;

        double targetAngle = atan2(dy, dx);

        //Rotate toward goal
        double rotation = targetAngle - theta;

        //Distance to goal
        double distance = sqrt(dx*dx + dy*dy);

        //Move a Small Step
        double step = min(1.0,distance);

        move(rotation, step);
    }

    void printState(){
        cout << "Position: (" << x << ", " << y << ")\n";
        cout << "Orientation: " << theta << "\n";
    }

    void printPath(){
        cout << "Path:\n";
        for (int i=0; i<path.size(); i++){
            cout << "(" << path[i].first << "," << path[i].second << ")\n";
        }
    }
};

int main(){
    Robot robot;

    int command;

    while (true){
        cout << "\n1: Manual Move | 2: Show Path | 3: Go to Goal\nEnter Command: ";
        cin >> command;

        if(command == 1){
            double rotation, forward;

            cout << "Enter Rotation and Forward: ";
            if(!(cin >> rotation >> forward)){
                cin.clear();
                cin.ignore(1000,'\n');
                continue;
            }

            robot.move(rotation, forward);
            robot.printState();
        } else if (command == 2)
        {
            robot.printPath();
        } else if (command == 3)
        {
            double gx, gy;

            cout << "Enter goal (x,y): ";
            if(!(cin >> gx >> gy)){
                cin.clear();
                cin.ignore(1000,'\n');
                continue;
            }

            //move step-by-step toward goal:
            while(true){
                robot.moveToGoal(gx,gy);
                robot.printState();

                double dx = gx - robot.x;
                double dy = gy - robot.y;

                if((abs(dx) < 0.1) && (abs(dy) < 0.1)){
                    cout << "Goal Reached!";
                    break;
                }
            }
        } else {
            cout << "Unkown Command.\n";
        }
        
    }
    return 0;
}