#include <iostream>
#include <cmath>
using namespace std;

struct Vec2 {
    double x,y;

    double mag(){
        return sqrt(x*x + y*y);
    }

    void normalize(){
        double m = mag();
        if(m > 1e-6){
            x /= m;
            y /= m;
        }
    }

    double dot(Vec2 o){
        return x*o.x + y*o.y;
    }

    double cross(Vec2 o){
        return x*o.y - y*o.x;
    }
};

class Robot {
public:
    double x=0, y=0, theta=0;

    double goalX, goalY;
    bool hasGoal = false;

    double obsX = 30, obsY = 30;

    void setGoal(double gx, double gy){
        goalX = gx;
        goalY = gy;
        hasGoal = true;
    }

    Vec2 getHeading(){
        return {cos(theta),sin(theta)};
    }

    void update(){
        if (!hasGoal) return;

        //---Goal Vector---
        Vec2 goal = {goalX - x, goalY - y};
        double distGoal = goal.mag();

        if(distGoal < 0.1){
            cout << "Reached Goal!\n";
            hasGoal = false;
            return;
        }

        goal.normalize();

        //---Obstacle Vector---
        Vec2 avoid = {x-obsX, y-obsY};
        double distObs = avoid.mag();

        avoid.normalize();

        double weight = 0;
        if(distObs < 2.0){
            weight = (2.0 - distObs);
            cout << "Avoiding Obstacle!\n";
        }

        //---Blend---
        Vec2 finalDir = {
            goal.x + weight * avoid.x,
            goal.y + weight * avoid.y
        };
        finalDir.normalize();

        //---Robot Heading---
        Vec2 heading = getHeading();

        //---Control Using Dot & Cross---
        double turn = heading.cross(finalDir); //turning signal
        double forward = heading.dot(finalDir); //Alignment

        //Clamp Forward
        if(forward < 0) forward = 0;

        //apply motion
        theta += 2.0 * turn; //angular gain
        double speed = 0.5 * distGoal;

        if(speed > 1.0) speed = 1.0;

        x += speed * forward * cos(theta);
        y += speed * forward * sin(theta);

        cout << "Pos: (" << x << "," << y << ")";
        cout << "Theta: " << theta << endl;
    }
};

int main(){
    Robot robot;

    int cmd;
    
    while(true){
        cout << "\n1: Set Goal | 2: Step | 3: Run\nEnter: ";
        cin >> cmd;

        if(cmd == 1){
            double gx, gy;
            cout << "Enter Goal: ";
            cin >> gx >> gy;
            robot.setGoal(gx,gy);
        }

        else if (cmd == 2){
            robot.update();
        }

        else if(cmd == 3){
            for (int i=0; i<200; i++)
                robot.update();       
        }
    }
}