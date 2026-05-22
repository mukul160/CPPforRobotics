#include <iostream>
class Motor {
    public:
        //'virtual' tells C++: "This can be refined by a child class."
        virtual void move(){
            std::cout << "Generic motor spinning..." << std::endl;
        }
};

class Servo:public Motor{
    public:
        //'override' proves we are intentionally changing the parent's behavior
        void move() override{
            std::cout << "Servo rotating to specific angle!" << std::endl;
        }
};

int main(){
    Motor basicMotor;
    Servo precisionServo;

    basicMotor.move();
    precisionServo.move();

    return 0;
}