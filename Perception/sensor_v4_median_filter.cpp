#include <iostream>
#include <vector>
#include <algorithm>

/**
 * VERSION 4: THE MEDIAN FILTER
 * Goal: Completely ignore "impossible" sensor spikes.
 *  */

class MedianFilter {
private:
    std::vector<double> window;
    size_t windowSize;

public:
    MedianFilter(size_t size) : windowSize(size) {
        //Window size should ideally be odd (e.g. 3,5,7)
    }

    double update(double rawValue) {
        window.push_back(rawValue);
        if(window.size() > windowSize) {
            window.erase(window.begin());
        }

        if (window.empty()) return 0.0;

        //Create a copy to sort, so we don't ruin the temporal order of 'window'
        std::vector<double> sortedWindow = window;
        size_t n = sortedWindow.size()/2;

        //nth element is faster than a full sort; it only finds the middle value
        std::nth_element(sortedWindow.begin(), sortedWindow.begin() + n, sortedWindow.end());

        return sortedWindow[n];
    }
};

int main() {
    MedianFilter mf(5);

    //Lidar data with a massive 0.0 drop and a 99.0 spike
    std::vector<double> sensorData = {5.0, 5.1, 0.0, 5.2, 99.0, 5.0, 5.1};

    std::cout << "---Sensor v4l Median Filter (Window=5)---" << std::endl;

    for(double val : sensorData) {
        std::cout << "Raw: " << val << " | Filtered: " << mf.update(val) << std::endl;
    }

    return 0;
}