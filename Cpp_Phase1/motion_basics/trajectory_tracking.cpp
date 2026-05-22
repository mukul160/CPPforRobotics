#include <iostream>
#include <cmath>
#include <vector>
using namespace std;

struct Vec2 {
    double x,y;

    double mag() {
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

    vector<Vec2> path;
    int currentTarget = 0;

    double obsX=3, obsY=3;

    double v=0,w=0;

    void setPath(){
        path = {
            {1,1},{3,2},{5,5},{7,5}
        };
        currentTarget=0;
    }

    Vec2 heading(){
        return {cos(theta), sin(theta)};
    }

    void update(double dt){
        if(currentTarget >= path.size()){
            cout << "Path Complete!\n";
            return;
        }

        Vec2 target = path[currentTarget];

        Vec2 goal = {target.x - x, target.y - y};
        double dist = goal.mag();

        //Switch Waypoint
        if(dist < 0.5){
            currentTarget++;
            return;
        }

        goal.normalize();

        //Obstacle Avoidance
        Vec2 avoid = {x - obsX, y - obsY};
        double distObs = avoid.mag();
        avoid.normalize();

        double weight = 0;
        if(distObs < 2.0){
            weight = (2.0 - distObs);
        }

        Vec2 finalDir = {
            goal.x + weight * avoid.x,
            goal.y + weight * avoid.y
        };
        finalDir.normalize();

        Vec2 h = heading();

        double forward = h.dot(finalDir);
        double turn = h.cross(finalDir);

        if(forward < 0) forward = 0;

        double target_v = forward * dist;
        double target_w = turn;

        if(target_v > 1.0) target_v = 1.0;

        //Smoothing
        v = 0.8*v + 0.2*target_v;
        w = 0.8*w + 0.2*target_w;

        //motion
        theta += w*dt;
        x += v * cos(theta) * dt;
        y += v * sin(theta) * dt;

        cout << "Target: (" << target.x << ", " << target.y << ")";
        cout << "Pos: (" << x << ", " << y << ")\n";
    }
};

int main(){
    Robot robot;
    robot.setPath();

    int cmd;
    double dt=0.1;

    while(true){
        cout << "\n1: Step | 2: Run\nEnter ";
        cin >> cmd;

        if(cmd == 1){
            robot.update(dt);
        } else if(cmd == 2){
            for(int i=0;i<200;i++);
        }
    }
}

