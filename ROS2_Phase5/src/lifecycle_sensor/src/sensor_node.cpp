#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <rclcpp_lifecycle/lifecycle_publisher.hpp>
#include <std_msgs/msg/float32.hpp>
#include <chrono>
#include <random>

using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class LifecycleSensor : public rclcpp_lifecycle::LifecycleNode
{
public:
    LifecycleSensor() : LifecycleNode("lifecycle_sensor")
    {
        RCLCPP_INFO(get_logger(), "Node constructed. State: Unconfigured.");
    }

    CallbackReturn on_configure(const rclcpp_lifecycle::State &)
    {
        RCLCPP_INFO(get_logger(), "Configuring...");

        this->declare_parameter("publish_rate", 1.0);
        double rate = this->get_parameter("publish_rate").as_double();

        bool hardware_ok = SimulateHardwareCheck();
        if(!hardware_ok){
            RCLCPP_ERROR(get_logger(), "Hardware check failed. Staying unconfigured.");
            return CallbackReturn::FAILURE;
        }

        publisher_ = this->create_publisher<std_msgs::msg::Float32>("sensor_data", 10);

        period_ms_ = static_cast<int>(1000.0/rate);

        RCLCPP_INFO(get_logger(), "Configured. Rate: %.1f Hz", rate);
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_activate(const rclcpp_lifecycle::State &)
    {
        RCLCPP_INFO(get_logger(), "Activating...");

        publisher_->on_activate();

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(period_ms_),
            std::bind(&LifecycleSensor::publishReading, this)
        );

        RCLCPP_INFO(get_logger(), "Active. Publishing sensor data.");
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_deactivate(const rclcpp_lifecycle::State &)
    {
        RCLCPP_INFO(get_logger(), "Deactivating...");

        timer_->cancel();
        timer_->reset();

        publisher_->on_deactivate();

        RCLCPP_INFO(get_logger(), "Inactive. Publishing stopped.");
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_cleanup(const rclcpp_lifecycle::State &)
    {
        RCLCPP_INFO(get_logger(), "Cleaning up...");

        publisher_.reset();

        RCLCPP_INFO(get_logger(), "Cleaned up. Ready to reconfigure.");
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_shutdown(const rclcpp_lifecycle::State &)
    {
        RCLCPP_INFO(get_logger(), "Shutting down.");
        timer_.reset();
        publisher_.reset();
        return CallbackReturn::SUCCESS;
    }


private:
    void publishReading()
    {
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_real_distribution<float> dist(20.0f, 25.0f);

        if(!publisher_->is_activated()){
            return;
        }

        std_msgs::msg::Float32 msg;
        msg.data = dist(rng);
        publisher_->publish(msg);

        RCLCPP_INFO(get_logger(), "Published: %.2f", msg.data);
    }

    bool SimulateHardwareCheck()
    {
        RCLCPP_INFO(get_logger(), "Hardware check passed.");
        return true;
    }

    rclcpp_lifecycle::LifecyclePublisher<std_msgs::msg::Float32>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    int period_ms_{1000};
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<LifecycleSensor>();
  
  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node->get_node_base_interface());
  executor.spin();
  
  rclcpp::shutdown();
  return 0;
}