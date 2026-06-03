#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <nav2_msgs/action/navigate_to_pose.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <vector>

using Nav2Action = nav2_msgs::action::NavigateToPose;
using GoalHandle = rclcpp_action::ClientGoalHandle<Nav2Action>;

class WaypointNavigator : public rclcpp::Node
{
public:
  WaypointNavigator() : Node("waypoint_navigator"), current_waypoint_(0), cancelled_(false)
  {
    action_client_ = rclcpp_action::create_client<Nav2Action>(this, "navigate_to_pose");

    // Define three waypoints as {x, y} pairs.
    // std::vector<std::pair<double,double>> — a resizable array where
    // each element is a pair of doubles. std::pair holds exactly two
    // values, accessed as .first and .second.
    waypoints_ = {
      {1.0,  0.0},
      {1.0,  1.0},
      {0.0,  1.0}
    };

    // Timer gives Nav2 time to initialise before first goal
    timer_ = this->create_wall_timer(
      std::chrono::seconds(2),
      std::bind(&WaypointNavigator::start, this));

    RCLCPP_INFO(this->get_logger(), "WaypointNavigator ready. %zu waypoints queued.",
      waypoints_.size());
  }

private:
  void start()
  {
    timer_->cancel();

    if (!action_client_->wait_for_action_server(std::chrono::seconds(5))) {
      RCLCPP_ERROR(this->get_logger(), "Action server not available.");
      rclcpp::shutdown();
      return;
    }

    sendNextWaypoint();
  }

  void sendNextWaypoint()
  {
    // Base case — all waypoints completed
    if (current_waypoint_ >= static_cast<int>(waypoints_.size())) {
      RCLCPP_INFO(this->get_logger(), "All waypoints completed.");
      rclcpp::shutdown();
      return;
    }

    // .first and .second access the two values inside each std::pair
    double x = waypoints_[current_waypoint_].first;
    double y = waypoints_[current_waypoint_].second;

    RCLCPP_INFO(this->get_logger(), "Sending waypoint %d: x=%.1f y=%.1f",
      current_waypoint_ + 1, x, y);

    Nav2Action::Goal goal;
    goal.pose.header.frame_id = "map";
    goal.pose.header.stamp    = this->now();
    goal.pose.pose.position.x = x;
    goal.pose.pose.position.y = y;
    goal.pose.pose.orientation.w = 1.0;

    rclcpp_action::Client<Nav2Action>::SendGoalOptions options;

    options.goal_response_callback =
      [this](const GoalHandle::SharedPtr & goal_handle) {
        if (!goal_handle) {
          RCLCPP_ERROR(this->get_logger(), "Waypoint %d rejected.", current_waypoint_ + 1);
          rclcpp::shutdown();
          return;
        }
        // Store the goal handle so we can cancel it later
        current_goal_handle_ = goal_handle;
        RCLCPP_INFO(this->get_logger(), "Waypoint %d accepted.", current_waypoint_ + 1);

        // Schedule a cancellation 8 seconds into the second waypoint.
        // This demonstrates mid-navigation cancellation.
        if (current_waypoint_ == 1) {
          cancel_timer_ = this->create_wall_timer(
            std::chrono::seconds(8),
            std::bind(&WaypointNavigator::cancelCurrentGoal, this));
          RCLCPP_WARN(this->get_logger(), "Cancellation scheduled in 8s for waypoint 2.");
        }
      };

    // feedback->distance_remaining — feedback is a SharedPtr so -> accesses its field
    options.feedback_callback =
      [this](GoalHandle::SharedPtr,
             const std::shared_ptr<const Nav2Action::Feedback> feedback) {
        RCLCPP_INFO(this->get_logger(), "Waypoint %d | Distance remaining: %.2f m",
          current_waypoint_ + 1, feedback->distance_remaining);
      };

    // result.code — result is a WrappedResult struct, accessed with . not ->
    // because it is a regular object, not a pointer
    options.result_callback =
      [this](const GoalHandle::WrappedResult & result) {
        if (cancelled_) {
          RCLCPP_WARN(this->get_logger(), "Navigation cancelled. Shutting down.");
          rclcpp::shutdown();
          return;
        }

        if (result.code == rclcpp_action::ResultCode::SUCCEEDED) {
          RCLCPP_INFO(this->get_logger(), "Waypoint %d reached.", current_waypoint_ + 1);
          current_waypoint_++;
          // Send the next waypoint from within the result callback.
          // This chains goals sequentially — each goal triggers the next.
          sendNextWaypoint();
        } else {
          RCLCPP_WARN(this->get_logger(), "Waypoint %d failed. Stopping.", current_waypoint_ + 1);
          rclcpp::shutdown();
        }
      };

    action_client_->async_send_goal(goal, options);
  }

  void cancelCurrentGoal()
  {
    // cancel_timer_ fires once — cancel it immediately to prevent repeat calls
    cancel_timer_->cancel();
    cancelled_ = true;

    if (current_goal_handle_) {
      RCLCPP_WARN(this->get_logger(), "Cancelling current navigation goal.");
      // async_cancel_goal sends a cancellation request to Nav2.
      // The result_callback will still fire — with ResultCode::CANCELED —
      // which is where we detect the cancellation and shut down.
      action_client_->async_cancel_goal(current_goal_handle_);
    }
  }

  rclcpp_action::Client<Nav2Action>::SharedPtr action_client_;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::TimerBase::SharedPtr cancel_timer_;

  // GoalHandle::SharedPtr — SharedPtr means -> to access members.
  // Stored so cancelCurrentGoal() can reference the active goal.
  GoalHandle::SharedPtr current_goal_handle_;

  // std::vector<std::pair<double,double>> — vector of coordinate pairs
  std::vector<std::pair<double, double>> waypoints_;

  int current_waypoint_;
  bool cancelled_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<WaypointNavigator>());
  rclcpp::shutdown();
  return 0;
}