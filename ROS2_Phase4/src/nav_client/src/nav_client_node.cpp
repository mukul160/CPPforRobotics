#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <nav2_msgs/action/navigate_to_pose.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>

using Nav2Action = nav2_msgs::action::NavigateToPose;
using GoalHandle = rclcpp_action::ClientGoalHandle<Nav2Action>;

class NavClient : public rclcpp::Node
{
public:
    NavClient() : Node("nav_client")
    {
        action_client_ = rclcpp_action::create_client<Nav2Action>(
            this, "navigate_to_pose"
        );

        timer_ = this->create_wall_timer(
        std::chrono::seconds(2),
        std::bind(&NavClient::sendGoal, this)
            );

        RCLCPP_INFO(this->get_logger(), "NavClient started. Waiting 2s before sending goal.");
    }

private:
    void sendGoal()
    {
        timer_->cancel();

        if(!action_client_->wait_for_action_server(std::chrono::seconds(5))){
            RCLCPP_ERROR(this->get_logger(), "Nav2 action server not available.");
            rclcpp::shutdown();
            return;
        }

        Nav2Action::Goal goal;
        goal.pose.header.frame_id = "map";
        goal.pose.header.stamp = this->now();
        goal.pose.pose.position.x = 1.5;
        goal.pose.pose.position.y = 0.5;
        goal.pose.pose.orientation.w = 1.0;

        RCLCPP_INFO(this->get_logger(), "Sending goal: x=1.0, y=0.0");

        rclcpp_action::Client<Nav2Action>::SendGoalOptions options;

        options.goal_response_callback = 
        [this](const GoalHandle::SharedPtr & goal_handle){
            if(!goal_handle)
                RCLCPP_ERROR(this->get_logger(), "Goal rejected by Nav2.");
            else
                RCLCPP_INFO(this->get_logger(), "Goal acccepted. Robot is navigating.");
        };

        options.feedback_callback = 
        [this](GoalHandle::SharedPtr,
                    const std::shared_ptr<const Nav2Action::Feedback> feedback){
                        RCLCPP_INFO(this->get_logger(), "Distance remaining: %.2f m",
                            feedback->distance_remaining);
                    };

        options.result_callback = 
        [this](const GoalHandle::WrappedResult & result){
            if(result.code == rclcpp_action::ResultCode::SUCCEEDED)
                RCLCPP_INFO(this->get_logger(), "Goal reached successfully.");
            else
                RCLCPP_WARN(this->get_logger(), "Navigation failed or was cancelled.");
            
            rclcpp::shutdown();
        };

        action_client_->async_send_goal(goal,options);

    }

    rclcpp_action::Client<Nav2Action>::SharedPtr action_client_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<NavClient>());
    rclcpp::shutdown();
    return 0;
}