# Phase 3 — Gazebo Integration

> ROS2 Fundamentals Drill · C++ / ROS2 Humble · Ubuntu 22.04

This phase connects ROS2 nodes to a physics simulation. The TurtleBot3 burger in Gazebo provides real sensor data and responds to real velocity commands — the same interfaces used with physical hardware. Phase 3 assumes Phase 1 and Phase 2 fluency.

---

## Packages

| Package | What it does |
|---|---|
| `gazebo_interface` | Subscribes to `/scan` and `/odom`, publishes velocity commands to drive the robot in a circle, and uses TF2 to track the robot's pose. Introduces `LaserScan`, `Odometry`, `Twist`, and the ROS2 logging level system. |
| `obstacle_avoidance` | Implements a two-state finite state machine — DRIVING and AVOIDING — driven by forward laser scan readings. Introduces the sense/act decoupling pattern, `enum class`, and in-class member initialisation. |

---

## How Gazebo and ROS2 Connect

Gazebo is a separate process with no native ROS2 knowledge. The connection is `ros_gz_bridge` — a plugin that translates between Gazebo's internal topic system and the ROS2 middleware in both directions. Gazebo sensor data becomes ROS2 messages on standard topics. ROS2 velocity commands become Gazebo actuator inputs. From your node's perspective, the simulator is invisible — you subscribe and publish to topics exactly as you would with a real robot.

Gazebo also publishes ground truth transforms to `/tf`. Combined with the `robot_state_publisher` broadcasting URDF joint transforms, the full REP-105 transform tree is available to your nodes exactly as it would be on hardware.

---

## ROS2 Concepts

### The `LaserScan` message

`sensor_msgs/msg/LaserScan` carries one complete rotation of lidar data. The key fields:

| Field | Meaning |
|---|---|
| `angle_min`, `angle_max` | Angular bounds of the scan in radians |
| `angle_increment` | Angular step between consecutive readings |
| `range_min`, `range_max` | Valid measurement bounds of the sensor |
| `ranges` | Array of distance readings in metres, in order of increasing angle |
| `scan_time` | Duration of one full rotation |
| `time_increment` | Time between consecutive beam firings — non-zero for spinning lidars |

The `ranges` array index corresponding to a given angle is `(angle - angle_min) / angle_increment`. For the TurtleBot3 LDS-01, `angle_min` is 0, making index 0 the forward direction. The forward arc therefore wraps across the beginning and end of the array — indices near 0 and near `total - 1` — because 359° and 0° are physically adjacent.

Every reading must be validated before use:
- `std::isfinite(r)` — rejects infinity (no return) and NaN (invalid signal)
- `r >= range_min` — rejects readings too close for the sensor, often the robot's own chassis
- `r <= range_max` — rejects readings beyond detection range

Skipping these checks is one of the most common sources of incorrect sensor behaviour in beginner code.

### The sense/act decoupling pattern

Sensor callbacks and control loops should run on separate timers at independent rates. The sensor callback updates a shared state variable. The control loop reads that variable and publishes commands at a fixed rate regardless of sensor arrival timing.

If the velocity command is placed inside the sensor callback, the robot only receives commands at the sensor rate. A dropped sensor message means no command for the full sensor period — potentially dangerous at speed. Decoupling ensures the control output is always timely even if sensor input is momentarily delayed or missing. This pattern appears in every production robotics codebase.

### QoS policies and silent subscription failures

ROS2 uses DDS Quality of Service profiles to let publishers and subscribers declare requirements for reliability and durability. A `RELIABLE` subscriber will not receive messages from a `BEST_EFFORT` publisher — the middleware silently declines to form the connection. No error is raised anywhere. The subscriber callback simply never fires.

Gazebo sensor topics are typically published as `BEST_EFFORT` because high-frequency sensor data tolerates dropped messages better than retransmission overhead. To match:

```cpp
auto qos = rclcpp::QoS(rclcpp::KeepLast(10)).best_effort();
scan_sub_ = this->create_subscription<sensor_msgs::msg::LaserScan>("/scan", qos, callback);
```

To diagnose a silent subscription failure:

```bash
ros2 topic info /scan --verbose
```

This prints the QoS profile of every publisher and subscriber on the topic, making mismatches immediately visible.

### The ROS2 logging level system

| Level | When to use it |
|---|---|
| `RCLCPP_DEBUG` | High-frequency diagnostic output — per-message logging in fast callbacks. Suppressed by default. |
| `RCLCPP_INFO` | Meaningful state transitions and confirmations — startup, mode changes. |
| `RCLCPP_WARN` | Recoverable anomalies — transform not yet available, sensor out of range. |
| `RCLCPP_ERROR` | Failures that impair functionality but don't require shutdown. |
| `RCLCPP_FATAL` | Unrecoverable failures preceding shutdown. |

The default level is INFO — DEBUG messages are compiled in but filtered at runtime. To enable:

```bash
ros2 run <package> <node> --ros-args --log-level debug
```

A node that logs INFO on every sensor message at 30 Hz produces 1800 lines per minute and buries meaningful information. Logging level discipline is immediately visible to a supervisor reading your code.

### `ros2 bag` and logging level independence

`ros2 bag record` captures the raw message stream to disk regardless of logging level. The bag records what flows on topics — not what gets printed. This means you can record a failure scenario at normal logging verbosity, then replay it with debug logging enabled to investigate without reproducing the failure live.

---

## C++ Patterns

### `enum class` for robot state

```cpp
enum class RobotState { DRIVING, AVOIDING };
```

`enum class` defines a named set of mutually exclusive states. The compiler enforces that `state_` can only hold `DRIVING` or `AVOIDING` — no integer casting, no string comparison, no invalid values. The `class` keyword scopes the values, requiring `RobotState::DRIVING` rather than just `DRIVING`, which prevents name collisions across multiple enums in a larger system.

### State transition guards

```cpp
if (state_ != RobotState::AVOIDING) {
    state_ = RobotState::AVOIDING;
    RCLCPP_WARN(this->get_logger(), "Obstacle detected — switching to AVOIDING");
}
```

The guard fires the log only when the state *changes*, not every control loop tick. Without it, a 10 Hz control loop produces ten identical log lines per second. With it, you get one line per transition — which is actually informative. This is the correct pattern for any state machine in ROS2.

### In-class member initialisation

```cpp
bool obstacle_detected_{false};
```

Initialises a member variable at the point of declaration. Without this, `obstacle_detected_` would be uninitialised between object construction and the first sensor callback, and the control loop could read garbage from it. Any member variable that needs a safe default and isn't covered by the constructor's initialiser list should be initialised this way.

### `std::numeric_limits` and `std::isfinite`

```cpp
float min_range = std::numeric_limits<float>::infinity();
```

Initialising a running minimum to positive infinity is idiomatic C++. Any finite value is less than infinity, so the first valid reading immediately becomes the minimum without special-case logic. If no valid reading is found, the result stays at infinity and `std::isfinite` correctly identifies it as invalid.

`std::numeric_limits<T>` provides compile-time constants for any numeric type — maximum, minimum, epsilon, infinity, NaN. Prefer it over magic numbers. `std::isfinite` returns true only if a value is neither infinite nor NaN — use it instead of checking each separately.

---

## `Twist` and Differential Drive

`geometry_msgs/msg/Twist` encodes velocity in 6 degrees of freedom — three linear (x, y, z) and three angular (roll, pitch, yaw rates). A differential drive robot uses only two:

- `linear.x` — forward speed in metres per second
- `angular.z` — yaw rate in radians per second

The radius of the resulting arc is `linear.x / angular.z`. Setting `linear.x = 0` with `angular.z > 0` rotates in place. The `/cmd_vel` topic name is a ROS convention followed by every standard robot driver and navigation stack.

---

## Diagnostic Tools

| Tool | What it tells you |
|---|---|
| `rqt_graph` | Full node and topic graph — which nodes exist, which topics connect them. First tool when a node isn't receiving data. |
| `ros2 topic info /topic --verbose` | QoS profile of every publisher and subscriber. Use when a subscription produces no data. |
| `ros2 topic echo /scan --once` | Prints one message in full — useful for inspecting `angle_min`, `angle_increment`, and `ranges.size()` to verify index arithmetic. |
| `ros2 param set /node param value` | Override a parameter at runtime. Only effective if the node has a parameter callback registered — otherwise the parameter server updates but behaviour does not change. |

---

## Interview Questions

**How does Gazebo communicate with ROS2 nodes?**
Through `ros_gz_bridge`, a plugin that translates between Gazebo's internal topic system and the ROS2 DDS middleware in both directions. From the node's perspective, the simulator is invisible — it subscribes and publishes to standard ROS2 topics.

---

**Why should sensing and acting be on separate timers?**
If the velocity command lives in the sensor callback, the robot only receives commands at the sensor rate. A dropped sensor message leaves the robot without a command for the full sensor period. A separate control timer ensures commands are published at a fixed rate regardless of sensor timing, which is the correct pattern for any real-time control loop.

---

**A subscription is not receiving any messages. The publisher exists on the graph. What do you check first?**
QoS compatibility. Run `ros2 topic info /topic --verbose` and compare the publisher and subscriber profiles. A `RELIABLE` subscriber will not receive from a `BEST_EFFORT` publisher — the middleware silently declines to form the connection with no error raised.

---

**You call `ros2 param set` on a running node and the behaviour doesn't change. Why?**
Parameters are read once in the constructor and stored as member variables. The parameter server is updated, but the member variable is never refreshed. Runtime reactivity requires a parameter callback registered with `add_on_set_parameters_callback`, which the middleware calls whenever a set request arrives.

---

**Why must every laser scan reading be validated before use?**
The lidar reports invalid readings as infinity (no return) or NaN (invalid signal). Readings below `range_min` often represent the robot's own chassis. Readings above `range_max` are beyond detection range. Infinity corrupts minimum calculations. NaN is contagious — any arithmetic involving NaN produces NaN silently, propagating incorrect values downstream.

---

**What is the forward arc index calculation doing in the obstacle avoidance node?**
The TurtleBot3 laser scan has `angle_min = 0`, making index 0 the forward direction. The forward arc wraps across the array boundary — indices near 0 and near `total - 1` are physically adjacent at 0° and 359°. The window size is computed as `(π/6) / angle_increment`, converting 30 degrees into a number of array indices. Sampling that many indices from each end of the array captures a 60-degree forward arc.
