#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <std_msgs/msg/float32.hpp>
#include <std_srvs/srv/trigger.hpp>
#include <chrono>
#include <random>

using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class MultithreadedSensor : public rclcpp_lifecycle::LifecycleNode
{
public:
    MultithreadedSensor() : LifecycleNode("multithreaded_sensor")
    {
        sensor_group_ = this->create_callback_group(
            rclcpp::CallbackGroupType::MutuallyExclusive
        );

        service_group_ = this->create_callback_group(
            rclcpp::CallbackGroupType::MutuallyExclusive
        );

        RCLCPP_INFO(get_logger(), "MultithreadedSensor constructed.");
    }

    CallbackReturn on_configure(const rclcpp_lifecycle::State &)
    {
        RCLCPP_INFO(get_logger(), "Configuring...");

        publisher_ = this->create_publisher<std_msgs::msg::Float32>("sensor_data", 10);

        validate_client_ = this->create_client<std_srvs::srv::Trigger>(
            "validate_reading",
            rmw_qos_profile_services_default,
            service_group_
        );

        RCLCPP_INFO(get_logger(), "Configured.");
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_activate(const rclcpp_lifecycle::State &)
    {
        publisher_->on_activate();

        rclcpp::SubscriptionOptions options;
        options.callback_group = sensor_group_;

        timer_ = this->create_wall_timer(
            std::chrono::seconds(1),
            std::bind(&MultithreadedSensor::timerCallback, this),
            sensor_group_
        );

        RCLCPP_INFO(get_logger(), "Active.");
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_deactivate(const rclcpp_lifecycle::State &)
    {
        publisher_->on_deactivate();
        timer_->cancel();
        timer_.reset();
        RCLCPP_INFO(get_logger(), "Deactivated.");
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_cleanup(const rclcpp_lifecycle::State &)
    {
        publisher_.reset();
        validate_client_.reset();
        RCLCPP_INFO(get_logger(), "Cleaned up.");
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_shutdown(const rclcpp_lifecycle::State &)
    {
        timer_.reset();
        publisher_.reset();
        return CallbackReturn::SUCCESS;
    }

private:
    void timerCallback()
    {
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_real_distribution<float> dist(18.0f, 27.0f);
        float reading = dist(rng);

        RCLCPP_INFO(get_logger(), "Reading: %.2f", reading);

        if(!validate_client_->service_is_ready()){
            RCLCPP_WARN(get_logger(), "Validation service not available.");
            return;
        }

        auto request = std::make_shared<std_srvs::srv::Trigger::Request>();

    validate_client_->async_send_request(request,
      [this, reading](rclcpp::Client<std_srvs::srv::Trigger>::SharedFuture response) {
        if (response.get()->success) {
          std_msgs::msg::Float32 msg;
          msg.data = reading;
          publisher_->publish(msg);
          RCLCPP_INFO(get_logger(), "Validated and published: %.2f", reading);
        } else {
          RCLCPP_WARN(get_logger(), "Reading %.2f rejected by validator.", reading);
        }
      });
    }

    rclcpp::CallbackGroup::SharedPtr sensor_group_;
    rclcpp::CallbackGroup::SharedPtr service_group_;
    rclcpp_lifecycle::LifecyclePublisher<std_msgs::msg::Float32>::SharedPtr publisher_;
    rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr validate_client_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<MultithreadedSensor>();

    rclcpp::executors::MultiThreadedExecutor executor(
        rclcpp::ExecutorOptions(), 2
    );

    executor.add_node(node->get_node_base_interface());
    executor.spin();

    rclcpp::shutdown();
    return 0;
}