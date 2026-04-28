#include <iostream>
#include <memory>
#include <vector>

class Component {
    public:
        virtual void test() = 0; //The '=0' makes this a "Pure Virtual" function
};

class Camera : public Component {
    public:
        void test() override {std::cout << "Camera: Capturing Frame..." << std::endl;}
};

class Lidar : public Component {
    public:
        void test() override {std::cout << "Lidar: Scanning Laser..." << std::endl;}
};

int main() {
    //A vector of pointers to the BASE class
    std::vector<std::unique_ptr<Component>> hardware;

    hardware.push_back(std::make_unique<Camera>());
    hardware.push_back(std::make_unique<Lidar>());

    for(const auto& item : hardware){
        item->test(); //C++ figures out which 'test()' to call at runtime!
    }

    return 0;
}