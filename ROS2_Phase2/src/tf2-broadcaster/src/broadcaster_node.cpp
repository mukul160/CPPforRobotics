#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2/LinearMath/Quaternion.h>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <cmath>

class CircularBroadcaster : public rclcpp::Node
{
public:
    CircularBroadcaster() : Node("circle_broadcaster"), angle_(0.0)
    {
        broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(50),
            std::bind(&CircularBroadcaster::broadcastTransform, this)
        );

        RCLCPP_INFO(this->get_logger(), "CircleBroadcaster Started.");
    }   

private:
    void broadcastTransform()
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
        q.setRPY(0.0,0.0,angle_);
        t.transform.rotation.x = q.x();
        t.transform.rotation.y = q.y();
        t.transform.rotation.z = q.z();
        t.transform.rotation.w = q.w();

        broadcaster_->sendTransform(t);
    }

    std::shared_ptr<tf2_ros::TransformBroadcaster> broadcaster_;
    rclcpp::TimerBase::SharedPtr timer_;
    double angle_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc,argv);
    rclcpp::spin(std::make_shared<CircularBroadcaster>());
    rclcpp::shutdown();
    return 0;
}