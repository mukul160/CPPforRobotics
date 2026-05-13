#include <iostream>
#include <memory> //Required for smart pointers

class Sensor{
    public:
        Sensor() {std::cout<<"Sensor Online." << std::endl;}
        ~Sensor() {std::cout << "Sensor offline (Memory Cleaned)." << std::endl;}

        void read() {std::cout << "Reading Data..." << std::endl;}
};

int main(){
    std::cout << "---Starting Scope---" << std::endl;
    {
        //'std::make_unique' is the professional way to create an object
        std::unique_ptr<Sensor> mySensor = std::make_unique<Sensor>();
        mySensor->read();

        //Notice: We don't call 'delete'.
    }
    std::cout << "---Scope Ended---" << std::endl;
    return 0;
}

