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
        return x*o.y - y*o.y;
    }
};

class Robot {
public:
    double x=0, y=0, theta=0;

    double goalX, goalY;
    bool hasGoal = false;

    double obsX = 3, obsY = 3;

    //Velocity State
    double v=0; //Linear Velocity
    double w=0; //Angular Velocity

    void setGoal(double gx, double gy){
        goalX = gx;
        goalY = gy;
        hasGoal = true;
    }

    Vec2 heading(){
        return {cos(theta), sin(theta)};
    }

    void update(double dt){
        if(!hasGoal) return;

        //---Goal---
        Vec2 goal = {goalX - x, goalY - y};
        double distGoal = goal.mag();

        if(distGoal < 0.1){
            cout << "Reached Goal!\n";
            hasGoal = false;
            return;
        }

        goal.normalize();

        //---Obstacle---
        Vec2 avoid = {x - obsX, y - obsY};
        double distObs = avoid.mag();

        avoid.normalize();

        double weight = 0;
        if(distObs < 2.0){
            weight = (2.0 - distObs);
            cout << "Near Obstacle!\n";
        }

        //---Blend---
        Vec2 finalDir = {
            goal.x + weight * avoid.x,
            goal.y + weight * avoid.y
        };
        finalDir.normalize();

        //---Control---
        Vec2 h = heading();

        double forward = h.dot(finalDir);
        double turn = h.cross(finalDir);

        if(forward < 0) forward = 0;

        //Convert to Velocities
        double target_v = forward * distGoal;
        double target_w = turn;

        //Clamp
        if(target_v > 1.0) target_v = 1.0;

        //Smoothing (Inertia- like Behavior)
        v = 0.8*v + 0.2*target_v;
        w = 0.8*w + 0.2*target_w;

        //---motion update---
        theta += w*dt;
        x += v*cos(theta)*dt;
        y += v*sin(theta)*dt;

        cout << "Pos: (" << x << ", " << y <<")";
        cout << "Theta: " << theta << endl;
    }
};

int main(){
    Robot robot;

    int cmd;

    double dt=0.1; //time step

    while(true){
        cout << "\n1: Set Goal | 2: Step | 3: Run\nEnter: ";
        cin >> cmd;

        if(cmd == 1){
            double gx, gy;
            cout << "Enter goal: ";
            cin >> gx >> gy;
            robot.setGoal(gx,gy);
        }

        else if(cmd == 2){
            robot.update(dt);
        }

        else if(cmd == 3){
            for(int i=0;i<200;i++)
                robot.update(dt);
        }
    }
}