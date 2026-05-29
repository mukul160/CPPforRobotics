#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/transform_listener.h>
#include <tf2_ros/buffer.h>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <cmath>

class CircleListener : public rclcpp::Node
{
public:
    CircleListener() : Node("circle_listener")
    {
        tf_buffer_= std::make_shared<tf2_ros::Buffer>(this->get_clock());
        tf_listerner_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_, this);

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(500),
            std::bind(&CircleListener::lookupTransform, this)
        );

        RCLCPP_INFO(this->get_logger(), "CircleListener started.");
    }

private:

    void lookupTransform()
    {
        try{
            geometry_msgs::msg::TransformStamped t = tf_buffer_->lookupTransform(
                "world", "base_link", tf2::TimePointZero
            );

            double x = t.transform.translation.x;
            double y = t.transform.translation.y;
            double distance = std::sqrt(x*x + y*y);

            RCLCPP_INFO(this->get_logger(),
                "base_link -> world | x: %.3f  y: %.3f  dist: %.3f", 
                    x, y, distance);
        }
        catch(const tf2::TransformException & ex){
            RCLCPP_WARN(this->get_logger(), "Transform not yet available: %s", ex.what());
        }
    }

    std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
    std::shared_ptr<tf2_ros::TransformListener> tf_listerner_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CircleListener>());
    rclcpp::shutdown();
    return 0;
}