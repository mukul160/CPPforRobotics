#include <iostream>
#include <algorithm>

double calculateMotorPower(double targetPos, double currentPos){
    //Error
    double error = targetPos - currentPos;

    //Gain (Kp)
    const double Kp = 0.5;

    //3. The Output: Power is proportional to the error.
    double output = error * Kp;

    //4. Clamping: Robot have physical limits
    return std::clamp(output,-1.0,1.0);
}

int main(){
    double target = 100.0; //Target Distance in cm
    double current = 0.0; //Starting Position

    std::cout << "Starting Motor Approach..." << std::endl;

    for(int i=0; i<10; i++){
        double power = calculateMotorPower(target,current);

        //Simulate movement: position increases based on power
        current += (power * 1.0);

        std::cout << "Step " << i << " | Position: " << current
            << " | Power Output: " << power << std::endl;
    }

    return 0;
}