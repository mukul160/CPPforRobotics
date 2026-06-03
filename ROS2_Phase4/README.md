# Phase 4 — The Nav2 Navigation Stack and ROS2 Actions

> ROS2 Fundamentals Drill · C++ / ROS2 Humble · Ubuntu 22.04

This phase covers ROS2 actions — the third communication primitive after topics and services — and the Nav2 navigation stack. Actions are the correct pattern for any long-running task that needs progress reporting and cancellation. Nav2 is the standard ROS2 navigation framework used in virtually every mobile robotics lab. Phase 4 assumes Phase 1, 2, and 3 fluency.

---

## Packages

| Package | What it does |
|---|---|
| `nav_client` (nav_client_node) | Sends a single navigation goal to Nav2, monitors feedback while the robot moves, and handles the result on arrival or failure. Introduces the action client pattern, `SendGoalOptions`, and the three-phase action structure. |
| `nav_client` (waypoint_node) | Sends a sequence of three waypoints, chaining each goal from the previous result callback. Cancels navigation mid-way through the second waypoint. Introduces goal chaining, `async_cancel_goal`, `std::vector`, `std::pair`, and the `cancelled_` flag pattern. |

---

## Nav2 Stack — Launch Sequence

Every Phase 4 session requires this startup order. Each step must complete before the next begins.

**Terminal 1 — Gazebo:**
```bash
export TURTLEBOT3_MODEL=burger
ros2 launch turtlebot3_gazebo turtlebot3_world.launch.py
```

**Terminal 2 — Nav2:**
```bash
export TURTLEBOT3_MODEL=burger
ros2 launch turtlebot3_navigation2 navigation2.launch.py \
  use_sim_time:=true \
  map:=/opt/ros/humble/share/nav2_bringup/maps/turtlebot3_world.yaml
```

**Terminal 3 — Initial pose:**
```bash
ros2 topic pub --once /initialpose geometry_msgs/msg/PoseWithCovarianceStamped \
  "{header: {frame_id: 'map'}, pose: {pose: {position: {x: 0.0, y: 0.0, z: 0.0}, orientation: {w: 1.0}}}}"
```

Verify the map frame exists before running any node:
```bash
ros2 run tf2_ros tf2_echo map base_link
```

**Terminal 4 — RViz (optional):**
```bash
ros2 launch nav2_bringup rviz_launch.py
```

**Force kill when Ctrl+C is slow:**
```bash
pkill -9 -f nav2 && pkill -9 -f gazebo && pkill -9 -f gzserver && pkill -9 -f gzclient
```

---

## ROS2 Concepts

### Why actions exist — the problem services cannot solve

A service has exactly two moments: request and response. Navigation is fundamentally incompatible with this model — a goal might take 30 seconds, during which you need progress visibility, cancellation capability, and a final outcome. Actions solve this with three distinct communication channels built into a single interface:

- **Goal channel** — sends the initial request to the server
- **Feedback channel** — streams progress updates from server to client while the goal is active
- **Result channel** — delivers the final outcome when the goal terminates (success, failure, or cancellation)

A fourth capability — cancellation — has no equivalent in services. `async_cancel_goal` sends a cancellation request through the action protocol, allowing the server to stop cleanly rather than being killed mid-execution.

### The three callbacks and what triggers each

| Callback | When it fires | What it receives |
|---|---|---|
| `goal_response_callback` | Once, when the server accepts or rejects the goal | `GoalHandle::SharedPtr` — null if rejected |
| `feedback_callback` | Repeatedly while the goal is active | `shared_ptr<const Nav2Action::Feedback>` |
| `result_callback` | Once, when the goal terminates for any reason | `GoalHandle::WrappedResult` containing `result.code` |

The result callback fires regardless of how the goal ended — success, failure, or cancellation. The `result.code` field distinguishes between them using `rclcpp_action::ResultCode::SUCCEEDED`, `CANCELED`, and `ABORTED`.

### Goal chaining — event loop sequencing, not recursion

Calling `sendNextWaypoint()` from inside the result callback is not recursion. The call stack fully unwinds after each callback returns. The executor regains control, then dispatches the next callback when the new goal's response arrives. The stack depth never exceeds one callback regardless of how many goals are chained. True recursion accumulates stack frames — the executor pattern does not. Between the result callback returning and the next goal's response callback firing, the executor is free to service any other pending events on the node.

### The `cancelled_` flag pattern

`ResultCode::CANCELED` fires when Nav2 cancels a goal for any reason — your explicit request, a new goal preempting the current one, or a failed recovery behaviour. Without an explicit `cancelled_` flag you cannot distinguish intentional cancellation from Nav2-initiated cancellation. The flag is set only by your own cancellation function, making it an unambiguous record of deliberate intent. This is the general pattern for any state machine where multiple paths lead to the same outcome.

### Nav2 internal architecture

When a navigation goal arrives, Nav2 hands it to a behaviour tree executor. The default `NavigateToPose` behaviour tree does the following in order:

The **global planner** (NavFn or Smac) computes a complete path from the robot's current position to the goal using the **global costmap** — a grid of the entire known map with obstacles inflated outward by the robot's footprint plus a safety margin. The planner runs once at goal acceptance.

The **controller** (Regulated Pure Pursuit by default) runs at high frequency, following the global path using the **local costmap** — a small rolling window around the robot updated continuously from live sensor data. The local costmap catches dynamic obstacles not present in the original map.

If the controller gets stuck, the behaviour tree triggers **recovery behaviours** — spinning in place, backing up, clearing the costmap — before replanning. This loop is what makes Nav2 robust to unexpected obstacles.

### Global vs local costmap

The distinction is data source and purpose, not resolution. The global costmap is built from the static map file — it represents the environment as it was when mapped and does not update in real time. Its purpose is path planning. The local costmap is built from live sensor data in a rolling window around the robot. Its purpose is immediate obstacle avoidance during path following. A goal that appears to be in open space may fall inside the inflation radius of a wall in the global costmap — making it unreachable to the planner even though no wall is directly at that coordinate.

### AMCL and the map frame

AMCL (Adaptive Monte Carlo Localisation) is responsible for publishing the `map` → `odom` transform — the segment of the REP-105 chain that connects the global frame to the odometry frame. It does this by maintaining a particle filter: a cloud of hypotheses about where the robot might be. Each particle is weighted by how well the current laser scan matches the known map at that hypothesised position. The cloud converges around the true position over time.

AMCL will not publish the `map` → `odom` transform until it receives an initial pose estimate via `/initialpose`. Without this estimate the `map` frame does not exist in the TF2 tree, and the global costmap cannot locate the robot — producing the "Timed out waiting for transform" error. The initial pose can be set from RViz via the 2D Pose Estimate button, or from the command line via `ros2 topic pub`.

### RViz vs Gazebo

Gazebo is a physics simulator — it has a physics engine, collision detection, and sensor simulation. It is the source of truth for what is physically happening. RViz is a visualisation tool with no physics engine. It subscribes to ROS2 topics and renders what it receives — laser scans, costmaps, planned paths, robot models. None of what RViz shows is being simulated there. The relationship is: Gazebo produces data → ROS2 topics carry it → RViz draws it.

---

## C++ Patterns

### `using` type aliases

```cpp
using Nav2Action = nav2_msgs::action::NavigateToPose;
using GoalHandle = rclcpp_action::ClientGoalHandle<Nav2Action>;
```

`using` creates a local alias for an existing type. Nothing new is compiled or allocated — it is purely a readability tool. Without aliases, every reference to the action client type would require writing the full nested namespace chain. With aliases, the code reads clearly while compiling to identical output.

### `std::vector` and `std::pair`

```cpp
std::vector<std::pair<double, double>> waypoints_;
```

Reading inside out using the `<>` rule: `std::pair<double, double>` holds exactly two doubles, accessed as `.first` and `.second`. `std::vector<...>` is a resizable array of those pairs. Brace initialisation constructs both inline:

```cpp
waypoints_ = { {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0} };
```

Each inner `{x, y}` constructs one pair. The outer `{}` constructs the vector from those pairs.

### `.` vs `->` — the complete rule

| Situation | Operator | Why |
|---|---|---|
| Regular object | `.` | Direct member access |
| Reference (`&`) | `.` | A reference is an alias for an object, not a pointer |
| Raw pointer (`*`) | `->` | Must dereference to reach the object |
| `shared_ptr` / `SharedPtr` | `->` | Smart pointer — must dereference to reach the object |

`result` in the result callback is typed as `const GoalHandle::WrappedResult &` — a const reference. Use `.`. `feedback` is typed as `std::shared_ptr<const Nav2Action::Feedback>` — a smart pointer. Use `->`. If the type contains `SharedPtr`, `shared_ptr`, or `*`, use `->`. Everything else uses `.`.

### `static_cast<int>` with container sizes

```cpp
if (current_waypoint_ >= static_cast<int>(waypoints_.size())) {
```

`size()` returns `size_t` — an unsigned integer. Comparing a signed `int` against an unsigned `size_t` produces a compiler warning because a negative signed value converts to a very large unsigned value, making the comparison behave unexpectedly. `static_cast<int>` converts explicitly to a signed type, making the comparison well-defined and silencing the warning. Always cast container sizes when comparing against signed counters.

### One-shot timers

```cpp
cancel_timer_ = this->create_wall_timer(
  std::chrono::seconds(8),
  std::bind(&WaypointNavigator::cancelCurrentGoal, this));
```

```cpp
void cancelCurrentGoal() {
  cancel_timer_->cancel();  // prevent repeat firing
  ...
}
```

A wall timer repeats indefinitely by default. Calling `timer->cancel()` inside the callback makes it fire exactly once. This is the correct pattern for any delayed one-shot event — schedule it with a timer, cancel it immediately on first execution.

---

## Diagnostic Commands

```bash
ros2 action list                          # verify Nav2 action servers are up
ros2 action info /navigate_to_pose        # inspect goal, feedback, result types
ros2 lifecycle get /amcl                  # verify AMCL is in active state
ros2 topic echo /amcl_pose --once         # confirm localisation is publishing
ros2 run tf2_ros tf2_echo map base_link   # confirm map frame exists
ros2 daemon stop && ros2 daemon start     # clear stale graph state after force kill
```

---

## Interview Questions

**Why are actions used for navigation instead of services?**
Services have one request and one response with no mechanism for intermediate communication. Navigation requires feedback during execution, cancellation capability, and a final result distinguishing success from failure. Actions provide all three through separate goal, feedback, and result channels built into a single interface.

---

**What is the difference between `ResultCode::CANCELED` and `ResultCode::ABORTED`?**
`CANCELED` means the goal was cancelled — either by the client via `async_cancel_goal` or by the server accepting a cancellation request. `ABORTED` means the server terminated the goal due to an internal failure — a planning failure, exhausted recovery attempts, or an unrecoverable error. Both fire the result callback but require different responses from the client.

---

**Is calling `sendNextWaypoint()` from inside the result callback recursion?**
No. The call stack fully unwinds after each callback returns. The executor regains control and dispatches the next callback when the new goal's events arrive. The stack depth never exceeds one callback regardless of how many goals are chained. True recursion accumulates stack frames on the same stack — the executor pattern does not.

---

**What causes the "failed to create plan with tolerance" error in Nav2?**
The global planner searched the global costmap and found no valid path to the goal coordinate. The most common cause is that the goal falls inside the inflation radius of a wall — the costmap inflates obstacles outward by the robot's footprint plus a safety margin, making areas near walls unreachable to the planner even if they appear to be open space. Diagnose by inspecting the global costmap in RViz — goals in red or yellow inflation zones will fail.

---

**What is AMCL doing and why does the map frame not exist without an initial pose?**
AMCL maintains a particle filter — a cloud of hypotheses about the robot's position, each weighted by how well the current laser scan matches the known map at that position. It publishes the `map` → `odom` transform only after receiving an initial pose estimate to seed the particle cloud. Without this seed, AMCL has no starting hypothesis to weight against the map, so it never converges and never publishes the transform. The `map` frame therefore does not exist in the TF2 tree until the initial pose is set.

---

**What is the difference between the global and local costmap?**
The global costmap is built from the static map file and represents the environment as it was when mapped. It does not update in real time. The global planner uses it once to compute a complete path at goal acceptance. The local costmap is built from live sensor data in a rolling window around the robot and updates continuously. The controller uses it to avoid dynamic obstacles while following the global path. The distinction is data source and temporal behaviour, not resolution.
