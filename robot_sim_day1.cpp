#include <iostream>
#include <cmath>
using namespace std;

int main(){
    double x=0,y=0;
    double theta=0; //orientation (radians)
    double rotation, forward;

    while (true){
        cout << "\nEnter rotation (rad) and forward movement: ";

        if (!(cin >> rotation >> forward)){
            cout << "Invalid input! Enter two numbers.\n";
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }

        //Update Orientation
        theta += rotation;

        //Move forward in the current direction
        double dx = forward * cos(theta);
        double dy = forward * sin(theta);

        x += dx;
        y += dy;

        cout << "Position: (" << x << ", " << y << ")" << endl;
        cout << "Orientation (theta): " << theta << endl;

        //Bonus: Detect Near Origin
        if(abs(x) < 0.0001 && abs(y) < 0.0001){
            cout << "You're back at origin!\n";
        }
    }
    return 0;
}