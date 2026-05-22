#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/int32.hpp>

class SystemMonitor : public rclcpp::Node
{

public:
    SystemMonitor() : Node("system_monitor"), tick_count_(0)
    {
        status_pub_ = this->create_publisher<std_msgs::msg::String>("system_status", 10);
        uptime_pub_ = this->create_publisher<std_msgs::msg::Int32>("system_uptime", 10);

        timer_ = this->create_wall_timer(
            std::chrono::seconds(1),
            std::bind(&SystemMonitor::timerCallback, this)
        );
        RCLCPP_INFO(this->get_logger(), "SystemMonitor node started.");
    }

private:

    void timerCallback()
    {
        tick_count_++;

        std_msgs::msg::Int32 uptime_msg;
        uptime_msg.data = tick_count_;
        uptime_pub_->publish(uptime_msg);

        std_msgs::msg::String status_msg;
        if(tick_count_ < 10){
            status_msg.data = "INITIALIZING";
        } else {
            status_msg.data = "NOMINAL";
        }
        status_pub_->publish(status_msg);

        RCLCPP_INFO(this->get_logger(), "Tick %d | Status: %s", tick_count_, status_msg.data.c_str());
    }

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr status_pub_;
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr uptime_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    int tick_count_;

};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SystemMonitor>());
    rclcpp::shutdown();
    return 0;
}