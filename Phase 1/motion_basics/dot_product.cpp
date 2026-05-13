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
        return x * other.x + y * other.y;
    }
};

int main(){
    Vec2 a,b;

    cout << "Enter vector A (x,y): ";
    cin >> a.x >> a.y;

    cout << "Enter vector B (x,y): ";
    cin >> b.x >> b.y;

    cout << "\n---Raw---\n";
    cout << "A magnitude: " << a.magnitude() << endl;
    cout << "B magnitude: " << b.magnitude() << endl;

    Vec2 a_norm = a;
    Vec2 b_norm = b;

    a_norm.normalize();
    b_norm.normalize();

    cout << "\n--- Normalized ---\n";
    cout << "A: (" << a_norm.x << ", " << a_norm.y << ")\n";
    cout << "B: (" << b_norm.x << ", " << b_norm.y << ")\n";

    double dot = a_norm.dot(b_norm);

    cout << "\nDot Product (Direction Similarity): " << dot << endl;

    if(dot > 0.8)
        cout << "Almost Aligned\n";
    else if (dot > 0)
        cout << "Somewhat Aligned\n";
    else if (dot == 0)
        cout << "Perpendicular\n";
    else 
        cout << "Opposite Direction\n";

    return 0;
}