#include <rclcpp/rclcpp.hpp>
#include <std_srvs/srv/trigger.hpp>
#include <chrono>

class MonitorClient : public rclcpp::Node
{
public:
    MonitorClient() : Node("monitor_client")
    {
        client_ = this->create_client<std_srvs::srv::Trigger>("reset_monitor");
    }

    bool sendResetRequest()
    {
        while(!client_->wait_for_service(std::chrono::seconds(1))){
            if(!rclcpp::ok()){
                RCLCPP_ERROR(this->get_logger(),"Interrupted while waiting for service.");
                return false;
            }
            RCLCPP_INFO(this->get_logger(), "Waiting for reset_monitor service...");
        }

        auto request = std::make_shared<std_srvs::srv::Trigger::Request>();
        auto future = client_->async_send_request(request);

        if(rclcpp::spin_until_future_complete(this->get_node_base_interface(), future) !=
                rclcpp::FutureReturnCode::SUCCESS)
        {
            RCLCPP_ERROR(this->get_logger(), "Service call failed.");
            return false;
        }

        auto response = future.get();
        RCLCPP_INFO(this->get_logger(), "Success: %s | Message: %s",
                response->success ? "true" : "false",
                response->message.c_str());

        return true;
    }

private:
    rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr client_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc,argv);
    auto node = std::make_shared<MonitorClient>();
    node->sendResetRequest();
    return 0;
}