#include <rclcpp/rclcpp.hpp> //What does rclcpp stand for?
#include <std_srvs/srv/trigger.hpp> //What's the difference between .h and .hpp?

class MonitorServer : public rclcpp::Node
{

public:
    MonitorServer() : Node("monitor_server"), tick_count_(0)
    {
        timer_ = this->create_wall_timer(
            std::chrono::seconds(1),
            std::bind(&MonitorServer::timerCallback,this)
        );

        service_ = this->create_service<std_srvs::srv::Trigger>(
            "reset_monitor",
            std::bind(&MonitorServer::resetCallback,this,
                std::placeholders::_1,std::placeholders::_2) //Why these placeholders?
        );

        RCLCPP_INFO(this->get_logger(), "MonitorServer ready.");
    }

private:
    void timerCallback()
    {
        tick_count_++;
        RCLCPP_INFO(this->get_logger(), "Tick: %d", tick_count_);
    }

    void resetCallback(
        const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
        std::shared_ptr<std_srvs::srv::Trigger::Response> response //How does one know the chain of packages for accessing ROS functionality?
    )

    {
        (void)request; //What is the meaning of this syntax?
        RCLCPP_INFO(this->get_logger(), "Reset requested. Counter was: %d", tick_count_);
        response->success = true;
        response->message = "Counter reset from " + std::to_string(tick_count_);
        tick_count_ = 0;
    }

    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr service_; //What's the difference between ::, -> and <> syntax?
    int tick_count_;
};

int main(int argc, char * argv[]) //What are these arguments being passed into main?
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MonitorServer>());
    rclcpp::shutdown();
    return 0;
}