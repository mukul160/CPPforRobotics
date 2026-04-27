#include <iostream>
#include <cmath>
#include <vector>
using namespace std;

class Robot{
public:
    double x,y;
    double theta;

    double speed;
    double totalDistance;

    vector<pair<double,double>> path;

    Robot(){
        x=0;
        y=0;
        theta=0;

        speed = 1.0;
        totalDistance = 0.0;

        path.push_back({x,y});
    }

    void move(double rotation, double forward){
        theta += rotation;
        
        double dx = forward * cos(theta);
        double dy = forward * sin(theta);

        x += dx;
        y += dy;

        totalDistance += sqrt(dx*dx + dy*dy);

        path.push_back({x,y});
    }

    void moveToGoal(double goalX, double goalY){
        double dx = goalX - x;
        double dy = goalY - y;

        double targetAngle = atan2(dy,dx);

        //Limited Turning
        double maxTurn = 0.2;
        double diff = targetAngle - theta;

        if(diff > maxTurn) diff = maxTurn;
        if(diff < -maxTurn) diff = -maxTurn;

        double rotation = diff;

        double distance = sqrt(dx*dx + dy*dy);

        double step = min(speed,distance);

        move(rotation,step);
    }

    void printState(){
        cout << "Position: (" << x << ", " << y << ")\n";
        cout << "Orientation: " << theta << "\n";
        cout << "Total Distance: " << totalDistance << "\n";
    }

    void printPath(){
        cout << "Recent Path (Last 5):\n";

        int start = max(0,(int)path.size() - 5);

        for (int i=start; i<path.size(); i++){
            cout << "(" << path[i].first << ", " << path[i].second << ")\n";
        }
    }

    void reset(){
        x=0;
        y=0;
        theta=0;
        totalDistance=0;

        path.clear();
        path.push_back({x,y});
    }
};

int main(){
    Robot robot;

    int command;

    while(true){
        cout << "\n1: Manual Move | 2: Show Path | 3: Go to Goal | 4: Reset\nEnter Command ";
        cin >> command;

        if(command == 1){
            double rotation, forward;

            cout << "Enter rotation and forward: ";
            if(!(cin >> rotation >> forward)){
                cin.clear();
                cin.ignore(1000,'\n');
                continue;
            }

            robot.move(rotation,forward);
            robot.printState();
        } else if(command == 2){
            robot.printPath();
        } else if (command == 3){
            double gx, gy;

            cout << "Enter goal (x,y): ";
            if(!(cin >> gx >> gy)){
                cin.clear();
                cin.ignore(1000, '\n');
                continue;
            }

            while(sqrt((robot.x - gx)*(robot.x - gx) + (robot.y - gy)*(robot.y - gy))>0.1){
                robot.moveToGoal(gx,gy);
                robot.printState();
            }

            cout << "Reached Goal!\n";
        } else if (command == 4){
            robot.reset();
            cout << "Robot reset to origin.\n";
        } else {
            cout << "Unknown Command\n";
        }
    }

    return 0;
}