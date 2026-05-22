#include <rclcpp/rclcpp.hpp>
#include <robot_interfaces/msg/system_report.hpp>
#include <robot_interfaces/srv/get_report.hpp>
#include <chrono>

class ReportNode : public rclcpp::Node
{
public:
  ReportNode() : Node("report_node"), tick_count_(0)
  {
    this->declare_parameter("timer_period_s", 1.0);
    double period = this->get_parameter("timer_period_s").as_double();

    publisher_ = this->create_publisher<robot_interfaces::msg::SystemReport>(
      "system_report", 10);

    service_ = this->create_service<robot_interfaces::srv::GetReport>(
      "get_report",
      std::bind(&ReportNode::reportCallback, this,
        std::placeholders::_1, std::placeholders::_2)
    );

    timer_ = this->create_wall_timer(
      std::chrono::duration<double>(period),
      std::bind(&ReportNode::timerCallback, this)
    );

    RCLCPP_INFO(this->get_logger(), "ReportNode started.");
  }

private:
  void timerCallback()
  {
    tick_count_++;
    auto msg = robot_interfaces::msg::SystemReport();
    msg.header.stamp = this->now();
    msg.header.frame_id = "base_link";
    msg.tick_count = tick_count_;
    msg.status = (tick_count_ < 10) ? "INITIALIZING" : "NOMINAL";
    msg.uptime_seconds = static_cast<double>(tick_count_);
    publisher_->publish(msg);

    RCLCPP_INFO(this->get_logger(), "Published report. Tick: %d | Status: %s",
      tick_count_, msg.status.c_str());
  }

  void reportCallback(
    const std::shared_ptr<robot_interfaces::srv::GetReport::Request> request,
    std::shared_ptr<robot_interfaces::srv::GetReport::Response> response)
  {
    (void)request;
    response->header.stamp = this->now();
    response->header.frame_id = "base_link";
    response->tick_count = tick_count_;
    response->status = (tick_count_ < 10) ? "INITIALIZING" : "NOMINAL";
    response->uptime_seconds = static_cast<double>(tick_count_);
    response->success = true;

    RCLCPP_INFO(this->get_logger(), "Served report request. Tick: %d", tick_count_);
  }

  rclcpp::Publisher<robot_interfaces::msg::SystemReport>::SharedPtr publisher_;
  rclcpp::Service<robot_interfaces::srv::GetReport>::SharedPtr service_;
  rclcpp::TimerBase::SharedPtr timer_;
  int tick_count_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ReportNode>());
  rclcpp::shutdown();
  return 0;
}