#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2_ros/static_transform_broadcaster.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_ros/buffer.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <geometry_msgs/msg/point_stamped.hpp>
#include <cmath>

class ChainNode : public rclcpp::Node
{
public:
  ChainNode() : Node("chain_node"), angle_(0.0)
  {
    //Dynamic broadcaster: world -> base_link
    dynamic_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);

    //Static broadcaster: base_link -> laser_link
    static_broadcaster_ = std::make_shared<tf2_ros::StaticTransformBroadcaster>(this);
    publishStaticTransform();

    //TF2 buffer and listener
    tf_buffer_ = std::make_shared<tf2_ros::Buffer>(this->get_clock());
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_, this);
    
    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(50),
      std::bind(&ChainNode::update, this)
    );

    RCLCPP_INFO(this->get_logger(), "ChainNode started.");
  }
private:
  void publishStaticTransform()
  {
    geometry_msgs::msg::TransformStamped static_t;
    static_t.header.stamp = this->now();
    static_t.header.frame_id = "base_link";
    static_t.child_frame_id = "laser_link";

    //Laser is mounted 0.2m forward and 0.3m above base_link
    static_t.transform.translation.x = 0.2;
    static_t.transform.translation.y = 0.0;
    static_t.transform.translation.z = 0.3;

    //No rotation relative to base_link
    tf2::Quaternion q;
    q.setRPY(0.0, 0.0, 0.0);
    static_t.transform.rotation.x = q.x();
    static_t.transform.rotation.y = q.y();
    static_t.transform.rotation.z = q.z();
    static_t.transform.rotation.w = q.w();

    static_broadcaster_->sendTransform(static_t);
    RCLCPP_INFO(this->get_logger(), "Static transform published: base_link -> laser_link");
  }

  void broadcastDynamicTransform()
  {
    angle_ += 0.02;

    geometry_msgs::msg::TransformStamped t;
    t.header.stamp = this->now();
    t.header.frame_id = "world";
    t.child_frame_id = "base_link";

    t.transform.translation.x = std::cos(angle_) * 2.0;
    t.transform.translation.y = std::sin(angle_) * 2.0;
    t.transform.translation.z = 0.0;

    tf2::Quaternion q;
    q.setRPY(0.0, 0.0, angle_);
    t.transform.rotation.x = q.x();
    t.transform.rotation.y = q.y();
    t.transform.rotation.z = q.z();
    t.transform.rotation.w = q.w();

    dynamic_broadcaster_->sendTransform(t);
  }

  void lookupChain()
  {
    try{
      //Single lookup composing world->base_link and base_link->laser_link
      geometry_msgs::msg::TransformStamped t = tf_buffer_->lookupTransform(
        "world", "laser_link", tf2::TimePointZero
      );

      //Express a point in laser_link and transform it to world frame
      geometry_msgs::msg::PointStamped laser_point;
      laser_point.header.frame_id = "laser_link";
      laser_point.header.stamp = t.header.stamp;
      laser_point.point.x = 1.0;
      laser_point.point.y = 0.0;
      laser_point.point.z = 0.0;

      geometry_msgs::msg::PointStamped world_point;
      tf2::doTransform(laser_point, world_point, t);

      RCLCPP_INFO(this->get_logger(),
          "laser_link origin in world | x: %.3f y: %.3f z: %.3f",
            t.transform.translation.x,
            t.transform.translation.y,
            t.transform.translation.z);
    } catch (const tf2::TransformException & ex) {
      RCLCPP_WARN(this->get_logger(), "Transform unavailable: %s", ex.what());
    }
  }

  void update()
  {
    broadcastDynamicTransform();
    lookupChain();
  }

  std::shared_ptr<tf2_ros::TransformBroadcaster> dynamic_broadcaster_;
  std::shared_ptr<tf2_ros::StaticTransformBroadcaster> static_broadcaster_;
  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
  rclcpp::TimerBase::SharedPtr timer_;
  double angle_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ChainNode>());
  rclcpp::shutdown();
  return 0;
}