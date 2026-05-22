#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/int32.hpp>
#include <chrono>

class SystemMonitor : public rclcpp::Node
{
public:
    SystemMonitor() : Node("system_monitor"), tick_count_(0)
    {
        this->declare_parameter("timer_period_s", 1.0);
        this->declare_parameter("status_threshold", 10);

        double period = this->get_parameter("timer_period_s").as_double();
        status_threshold_ = this->get_parameter("status_threshold").as_int();

        status_pub_ = this->create_publisher<std_msgs::msg::String>("system_status", 10);
        uptime_pub_ = this->create_publisher<std_msgs::msg::Int32>("system_uptime", 10);

        timer_ = this->create_wall_timer(
            std::chrono::duration<double>(period),
            std::bind(&SystemMonitor::timerCallback, this)
        );

        RCLCPP_INFO(this->get_logger(),
            "SystemMonitor started. Period: %.1fs | Threshold: %ld ticks",
            period, status_threshold_);
    }


private:
    void timerCallback()
    {
        tick_count_++;

        std_msgs::msg::Int32 uptime_msg;
        uptime_msg.data = tick_count_;
        uptime_pub_->publish(uptime_msg);

        std_msgs::msg::String status_msg;
        status_msg.data = (tick_count_ < status_threshold_) ? "INITIALIZING" : "NOMINAL";
        status_pub_->publish(status_msg);

        RCLCPP_INFO(this->get_logger(), "Tick: %d | Status: %s",
                    tick_count_, status_msg.data.c_str());
    }

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr status_pub_;
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr uptime_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    int tick_count_;
    int64_t status_threshold_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SystemMonitor>());
    rclcpp::shutdown();
    return 0;
}