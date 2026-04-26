#include <iostream>
#include <cmath>
using namespace std;

struct Vec2{
    double x,y;

    double magnitude(){
        return sqrt(x*x + y*y);
    }

    void normalize(){
        double mag = magnitude();
        if(mag > 1e-6){
            x /= mag;
            y /= mag;
        }
    }

    double dot(Vec2 other){
        return x*other.x + y*other.y;
    }

    double cross(Vec2 other){
        return x*other.y - y*other.x;
    }
};

int main() {
        Vec2 robotDir, goalDir;

        cout << "Enter robot direction (x y): ";
        cin >> robotDir.x >> robotDir.y;

        cout << "Enter goal direction (x y): ";
        cin >> goalDir.x >> goalDir.y;

        robotDir.normalize();
        goalDir.normalize();

        double dot = robotDir.dot(goalDir);
        double cross = robotDir.cross(goalDir);

        cout << "\nDot (Alignment): " << dot << endl;
        cout << "cross (turn direction): " << cross << endl;
        
        //Interpret Turning
        if(cross > 0)
            cout << "Goal is to your LEFT\n";
        else if (cross < 0)
            cout << "Goal is to your RIGHT\n";
        else
            cout << "Goal is straight ahead or directly behind.\n";

        return 0;
}