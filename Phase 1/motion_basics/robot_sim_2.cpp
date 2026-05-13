#include <iostream>
#include <cmath>
#include <vector>
using namespace std;

class Robot{
public:
    double x,y;
    double theta;

    vector<pair<double,double>> path; // stores (x,y) history

    Robot(){
        x=0;
        y=0;
        theta=0;
        path.push_back({x,y}); //Start Position
    }

    void move(double rotation, double forward){
        theta += rotation;

        double dx = forward + cos(theta);
        double dy = forward + sin(theta);

        x += dx;
        y += dy;

        path.push_back({x,y}); // store new position
    }

    void printState(){
        cout << "Position: (" << x << ", " << y << ")" << endl;
        cout << "Orientation: " << theta << endl;
    }

    void printPath(){
        cout << "Path:\n";
        for(int i=0; i < path.size(); i++){
            cout << "(" << path[i].first << ", " << path[i].second << ")\n";
        }
    }
};

int main(){
    Robot robot;

    double rotation, forward;
    int command;

    while(true){
        cout << "\n1: Move | 2: Show Path\nEnter Command: ";
        cin >> command;

        if(command == 1){
            cout << "Enter rotation (rad) and forward movement: ";

            if(!(cin >> rotation >> forward)){
                cout << "Invalid Input!\n";
                cin.clear();
                cin.ignore(1000, '\n');
                continue;
            }

            robot.move(rotation,forward);
            robot.printState();

            if (abs(robot.x) < 0.0001 && abs(robot.y) < 0.0001){
                cout << "You're back at origin!\n";
            }

        } else if (command == 2){
            robot.printPath();
        } else {
            cout << "Unknown Command.\n";
        }   
    }
    return 0;
}