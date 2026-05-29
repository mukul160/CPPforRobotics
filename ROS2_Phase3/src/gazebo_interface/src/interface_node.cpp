#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <tf2_ros/transform_listener.h>
#include <tf2_ros/buffer.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <algorithm>
#include <limits>

class GazeboInterface : public rclcpp::Node
{
public:
  GazeboInterface() : Node("gazebo_interface")
  {
    //Subscription
    scan_sub_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
      "/scan", 10,
      std::bind(&GazeboInterface::scanCallback, this, std::placeholders::_1)
    );

    odom_sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
      "/odom", 10,
      std::bind(&GazeboInterface::odomCallback, this, std::placeholders::_1)
    );

    //Publisher
    cmd_vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);

    //TF2
    tf_buffer_ = std::make_shared<tf2_ros::Buffer>(this->get_clock());
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_, this);

    //Drive timer at 10 Hz
    drive_timer_ = this->create_wall_timer(
      std::chrono::milliseconds(100),
      std::bind(&GazeboInterface::driveCallback, this)
    );

    RCLCPP_INFO(this->get_logger(), "GazeboInterface node started.");
  }

private:
  void scanCallback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
  {
    //Filter out invalid readings (inf, nan) before computing minimum
    float min_range = std::numeric_limits<float>::infinity();
    for(const float range : msg->ranges){
      if(std::isfinite(range) && range >= msg->range_min && range <= msg->range_max){
        min_range = std::min(min_range, range);
      }
    }

    if(std::isfinite(min_range)){
      RCLCPP_INFO(this->get_logger(), "Minimum laser range: %.3f m", min_range);
    }
  }

  void odomCallback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    //Exract position directly from odometry message
    double x = msg->pose.pose.position.x;
    double y = msg->pose.pose.position.y;
    RCLCPP_DEBUG(this->get_logger(), "Odom position | x: %.3f  y; %.3f", x, y);
  }

  void driveCallback()
  {
    //Publish a gentle circular motion
    geometry_msgs::msg::Twist cmd;
    cmd.linear.x = 0.15;
    cmd.angular.z = 0.4;
    cmd_vel_pub_->publish(cmd);

    //Query TF2 for the robot's current pose in the odom frame
    try{
      geometry_msgs::msg::TransformStamped t = tf_buffer_->lookupTransform(
      "odom", "base_footprint", tf2::TimePointZero);

      RCLCPP_INFO(this->get_logger(),
      "TF2 pose | x: %.3f  y: %.3f",
      t.transform.translation.x,
      t.transform.translation.y);
    }
    catch (const tf2::TransformException & ex){
      RCLCPP_WARN(this->get_logger(), "TF2 unavailable: %s", ex.what());
    }
  }

  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr scan_sub_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;
  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
  rclcpp::TimerBase::SharedPtr drive_timer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<GazeboInterface>());
  rclcpp::shutdown();
  return 0;
}