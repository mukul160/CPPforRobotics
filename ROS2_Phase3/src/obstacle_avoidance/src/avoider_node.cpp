#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <algorithm>
#include <limits>

//Two Possible Robot States
enum class RobotState {DRIVING, AVOIDING};

class ObstacleAvoider : public rclcpp::Node
{
public:
    ObstacleAvoider() : Node("obstacle_avoider"), state_(RobotState::DRIVING)
    {
        //Declare parameters with safe defaults
        this->declare_parameter("obstacle_threshold", 0.5);
        this->declare_parameter("linear_speed", 0.15);
        this->declare_parameter("angular_speed", 0.5);

        obstacle_threshold_ = this->get_parameter("obstacle_threshold").as_double();
        linear_speed_ = this->get_parameter("linear_speed").as_double();
        angular_speed_ = this->get_parameter("angular_speed").as_double();

        //Subscribe to laser scan - fires scanCallback on every incoming scan
        scan_sub_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
            "/scan", 10,
            std::bind(&ObstacleAvoider::scanCallback, this, std::placeholders::_1)
        );

        //Publish velocity command
        cmd_vel_pub = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);

        //Control loop runs at 10 Hz independently of scan rate
        control_timer_ = this->create_wall_timer(
            std::chrono::milliseconds(100),
            std::bind(&ObstacleAvoider::controlLoop, this)
        );

        RCLCPP_INFO(this->get_logger(), "ObstacleAvoider started. Threshold: %.2f m",
                    obstacle_threshold_);
    }

private:
    //Sensor Callback - runs every time a laser scan arrives (~5 Hz)
    //Updates obstacle_detected_ flag but does not command the robot
    //Separating sensing from acting keeps each funtion focused

    void scanCallback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
    {
        int total = static_cast<int>(msg->ranges.size());

        //Number of readings spanning 30 degrees on either side.
        int window = static_cast<int>(M_PI / 6.0 / msg->angle_increment);

        float min_forward = std::numeric_limits<float>::infinity();

        //Check the forward arc -- indices near 0 and near total-1 (wrap around)
        for(int i = 0; i < window; i++) {
            float r1 = msg->ranges[i];
            float r2 = msg->ranges[total - 1 - i];

            if(std::isfinite(r1) && r1 >= msg->range_min && r1 <= msg->range_max)
                min_forward = std::min(min_forward, r1);
            
            if(std::isfinite(r2) && r2 >= msg->range_min && r2 <= msg->range_max)
                min_forward = std::min(min_forward, r2);
        }

        //Update shared state - control loop reads this on its own timer
        obstacle_detected_ = std::isfinite(min_forward) && min_forward < obstacle_threshold_;

        RCLCPP_DEBUG(this->get_logger(), "Min forward range: %.3f m | Obstacle: %s",
                min_forward, obstacle_detected_ ? "YES" : "NO");
    }

    //Control Loop - Runs at 10Hz on its own timer
    //Reads obstacle_detected_ and decides what velocity to publish
    //State transitions are logged so behavior is easy to trace

    void controlLoop()
    {
        geometry_msgs::msg::Twist cmd;

        if(obstacle_detected_){
            //Transition to AVOIDING if not already
            if(state_ != RobotState::AVOIDING) {
                state_ = RobotState::AVOIDING;
                RCLCPP_WARN(this->get_logger(), "Obstacle detected - switching to AVOIDING");
            }
            //Stop forward motion, rotate in place
            cmd.linear.x = 0.0;
            cmd.angular.z = angular_speed_;
        }
        else {
            //Transition to DRIVING if not already
            if(state_ != RobotState::DRIVING) {
                state_ = RobotState::DRIVING;
                RCLCPP_INFO(this->get_logger(), "Path clear - resuming DRIVING");
            }
            //Drive forward
            cmd.linear.x = linear_speed_;
            cmd.angular.z = 0.0;
        }

        cmd_vel_pub->publish(cmd);
    }

    //Subscriptions and Publishers
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr scan_sub_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub;
    rclcpp::TimerBase::SharedPtr control_timer_;

    //Parameters
    double obstacle_threshold_;
    double linear_speed_;
    double angular_speed_;

    //Shared state between scan callback and control loop
    RobotState state_;
    bool obstacle_detected_{false}; 
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ObstacleAvoider>());
    rclcpp::shutdown();
    return 0;
}
