#include <iostream>
#include <vector>
#include <numeric>
#include <deque> //For double ended queue

/**
 * VERSION 2: THE MOVING AVERAGE FILTER
 * Goal: Smooth data by averaging the last N samples
 */

class MovingAverage {
    private:
        std::deque<double> window;
        size_t windowSize;
        double sum=0.0;

    public:
        MovingAverage(size_t size) : windowSize(size) {}

        double update(double rawValue){
            
            //Add new value to the window and sum
            window.push_back(rawValue);
            sum += rawValue;

            //If window is full, remove the oldest value
            if(window.size() > windowSize) {
                sum -= window.front();
                window.pop_front();
            }

            //Return the average
            return sum/window.size();
        }
};

int main() {
    //Window size of 5: Average the 5 most recent readings
    MovingAverage ma(5);

    //Simulated LIDAR data with a "spike" (noise)
    std::vector<double> lidarData = {5.0, 5.1, 5.0, 12.0, 5.2, 5.1, 4.9};

    std::cout << "---Sensor v2: Moving Average (Window = 5)---" << std::endl;

    for (double val : lidarData) {
        std::cout << "Raw: " << val << " | Filtered: " << ma.update(val) << std::endl;
    }
    return 0;
}



