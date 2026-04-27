#include <iostream>
#include <cmath>
using namespace std;

class Robot {
public:
    double x,y,theta;

    double goalX,goalY;
    bool hasGoal;

    double obsX, obsY;

    double k_linear;
    double k_angular;

    Robot(){
        x=0;y=0;theta=0;
        hasGoal = false;

        obsX = 3;
        obsY = 3;

        k_linear = 0.5;
        k_angular = 2.0;
    }

    void setGoal(double gx, double gy){
        goalX = gx;
        goalY = gy;
        hasGoal = true;
    }

    double magnitude(double dx, double dy){
        return sqrt(dx*dx + dy*dy);
    }

    void normalize(double &dx, double &dy){
        double mag = magnitude(dx,dy);
        if(mag > 1e-6){
            dx /= mag;
            dy /= mag;
        }
    }

    void update(){
        if (!hasGoal) return;

        // -- Goal Vector --
        double gx = goalX - x;
        double gy = goalY - y;

        double distGoal = magnitude(gx,gy);

        if (distGoal < 0.05){
            cout << "Reached Goal!\n";
            hasGoal = false;
            return;
        }

        normalize(gx, gy);

        // --- Obstacle Vector ---
        double ox = x - obsX;
        double oy = y - obsY;

        double distObs = magnitude(ox,oy);

        normalize(ox,oy);

        // --- Weight (Stronger When Closer) ---
        double avoidWeight = 0.0;

        if(distObs < 2.0){
            avoidWeight = (2.0 - distObs); // linear falloff
        }

        // ---Blend Vectors---
        double finalX = gy + avoidWeight * ox;
        double finalY = gy + avoidWeight * oy;

        normalize(finalX,finalY);

        //---Convert to Angle---
        double targetAngle = atan2(finalY,finalX);

        double angleError = targetAngle - theta;

        while(angleError > M_PI) angleError -= 2*M_PI;
        while(angleError < -M_PI) angleError += 2*M_PI;
        
        //---Controller---
        double angularVel = k_angular * angleError;
        double linearVel = k_linear * distGoal;

        theta += angularVel;

        x += linearVel * cos(theta);
        y += linearVel * sin(theta);
        
    }

    void printState(){
        cout << "Pos: (" << x << "," << y << ")";
        cout << "Theta: " << theta << endl;
    }
};

int main() {
    Robot robot;

    int command;

    while(true){
        cout << "\n1: Set Goal | 2: Step | 3: Run\nEnter: ";
        cin >> command;

        if(command == 1){
            double gx,gy;
            cout << "Enter Goal: ";
            cin >> gx >> gy;
            robot.setGoal(gx,gy);
        }

        else if (command == 2) {
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