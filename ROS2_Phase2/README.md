# Phase 2 — The TF2 Transform System

> ROS2 Fundamentals Drill · C++ / ROS2 Humble · Ubuntu 22.04

This phase covers TF2, ROS2's system for tracking coordinate frame relationships over time. Every multi-sensor and mobile robotics system depends on TF2 — it is the layer that makes spatial reasoning across frames possible. Phase 2 assumes Phase 1 fluency.

---

## Packages

| Package | What it does |
|---|---|
| `tf2_broadcaster` | Simulates a robot driving in a circle by broadcasting a time-varying `world` → `base_link` transform at 20 Hz. Introduces `TransformBroadcaster`, `TransformStamped`, and quaternion construction via `setRPY`. |
| `tf2_listener` | Listens to the transform tree and queries the current pose of `base_link` relative to `world` at 2 Hz. Introduces `Buffer`, `TransformListener`, `lookupTransform`, `TimePointZero`, and `TransformException`. |
| `tf2_chain` | Combines a dynamic broadcaster, a static broadcaster, and a listener in a single node. Broadcasts a fixed `base_link` → `laser_link` offset via `StaticTransformBroadcaster`, then queries the composed `world` → `laser_link` transform and applies it to a point using `tf2::doTransform`. |

---

## ROS2 Concepts

### The TF2 architecture — no central server

TF2 has no server process. The transform tree is reconstructed independently by every node that needs it. A `TransformBroadcaster` publishes `geometry_msgs/msg/TransformStamped` messages to the `/tf` topic. A `TransformListener` subscribes to `/tf` and `/tf_static` and feeds every incoming message into a local `Buffer`. Transform lookups are always local — they never involve a network round trip — which is why they are fast enough to call inside a high-frequency timer callback.

The buffer maintains a sliding time history of transforms — by default the last 10 seconds. Short broadcaster interruptions do not immediately break the system, but if a broadcaster stays down, the buffer's data ages out and lookups for recent timestamps begin failing.

### Static vs dynamic transforms

A dynamic transform, published to `/tf`, says the relationship between two frames changes over time. The buffer stores a time history so that lookups at specific timestamps are possible. A static transform, published to `/tf_static`, says the relationship never changes. The `/tf_static` topic is latched — the middleware delivers the last published value immediately to any new subscriber, even if the broadcaster has long since stopped. Static transforms never age out in the buffer. Publishing a time-varying relationship to `/tf_static` will cause the system to use a stale value forever with no error or warning.

### Transform composition

When you request a transform between two frames that are not directly connected — such as `world` → `laser_link` when only `world` → `base_link` and `base_link` → `laser_link` exist — TF2 walks the tree and composes the chain by multiplying homogeneous transformation matrices. If T_wb is world→base_link and T_bl is base_link→laser_link, the composed result is T_wl = T_wb × T_bl. This composition is exact floating point matrix multiplication. You never need to manually compose transforms — requesting any pair of frames in the tree gives you the full chain automatically.

### The transform tree must be a tree

TF2 requires frame relationships to form a strict tree with no loops. Every frame has exactly one parent. This constraint exists because a loop would create two different paths between the same pair of frames, producing two different composed transforms due to floating point errors in each path. A tree guarantees exactly one path and therefore one unambiguous answer. In practice this means each transform segment must have exactly one broadcaster — two nodes broadcasting the same frame pair with conflicting data produces undefined behaviour.

### `tf2::TimePointZero` and temporal semantics

`TimePointZero` is a sentinel value meaning "give me the latest available transform regardless of when it was recorded." It is the most robust option for development because it never fails due to timing issues as long as any transform exists in the buffer. Passing `this->now()` instead requests the transform at the current moment, which requires the buffer to have data at or beyond that timestamp — since broadcasted transforms take a non-zero middleware round trip to appear in the buffer, this will throw an `ExtrapolationException` on nearly every tick. A common production pattern is `this->now() - rclcpp::Duration::from_seconds(0.1)` — requesting a timestamp slightly in the past to stay safely within the buffered window.

### The startup race condition and try-catch

When a broadcaster and listener share the same node, transforms published in a callback travel through the DDS middleware and back before appearing in the buffer. On the first tick the buffer is empty, so `lookupTransform` throws a `TransformException`. `TimePointZero` reduces subsequent failures by accepting any available transform regardless of age, but the try-catch is the safety net that prevents the first-tick failure from crashing the node. In production you can additionally use `tf_buffer_->canTransform()` to check availability before attempting a lookup.

`tf2::TransformException` is the base class for more specific exceptions — `LookupException` when a frame does not exist, `ConnectivityException` when no path connects the requested frames, and `ExtrapolationException` when the requested timestamp falls outside the buffered window.

### REP-105 — the standard mobile robot frame chain

REP-105 is the ROS standard for coordinate frame conventions. The canonical chain for a mobile robot is:

`map` → `odom` → `base_link` → sensor frames

`map` is the global fixed frame representing ground truth. The localisation system (AMCL, Cartographer) publishes `map` → `odom`. This transform can jump discontinuously when the localisation corrects its estimate.

`odom` is a continuous, smooth local frame that drifts over time. Odometry (wheel encoders, VIO, IMU integration) publishes `odom` → `base_link`. It never jumps, which is important for local motion planning and control.

`base_link` is the robot body frame — by REP-103 convention, centred at the robot base with X forward, Y left, Z up. All sensor frames are children of `base_link` or of URDF links. Being able to read a `view_frames` output and identify which node is responsible for each segment, and what kind of data drives it, is a practical skill that is immediately visible to a supervisor on day one.

---

## C++ Patterns

### `tf2::Quaternion` and `setRPY`

Quaternions represent 3D rotations without the singularities of Euler angles. A quaternion q = (x, y, z, w) encodes a rotation of angle θ around unit axis (ax, ay, az) as x = ax·sin(θ/2), y = ay·sin(θ/2), z = az·sin(θ/2), w = cos(θ/2). The identity rotation has w = 1 and x = y = z = 0. A quaternion of (0, 0, 0, 0) is invalid and will cause TF2 to behave incorrectly. `setRPY(roll, pitch, yaw)` constructs a quaternion from Euler angles — the most convenient form for simple rotations. For a pure yaw, only z and w are non-zero.

Euler angles suffer from two problems that quaternions avoid. Gimbal lock occurs when two rotation axes align at pitch = ±90°, causing loss of a degree of freedom. Convention ambiguity — ZYX vs XYZ vs ZYZ rotation order — produces different results for the same numbers across different libraries. Quaternions have neither problem. Their only unintuitive property is that q and -q represent the same rotation, which occasionally causes sign-flip issues when interpolating.

### `tf2::doTransform`

`doTransform(input, output, transform)` applies a `TransformStamped` to a stamped geometric type, expressing it in the target frame. It is a template function — the same call works for `PointStamped`, `PoseStamped`, `Vector3Stamped`, and others. Points and vectors transform differently: a point is affected by both rotation and translation, a vector (direction or velocity) is affected by rotation only. This distinction is encoded in the homogeneous w component — w=1 for points, w=0 for vectors — and the matrix multiplication handles it correctly.

The stamp of the input geometry should match the stamp of the transform used. A mismatch creates temporal inconsistency — the data is from one moment but the transform is from another. In a real sensor pipeline, always take the timestamp from the incoming message header and use it consistently for both the lookup and the stamped geometry.

### Shared pointer construction order

In `tf2_chain`, the buffer is constructed before the listener:

```cpp
tf_buffer_ = std::make_shared<tf2_ros::Buffer>(this->get_clock());
tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_, this);
```

The listener takes a reference to the buffer at construction — it needs somewhere to put incoming transforms immediately. Constructing the listener first would require passing a buffer that doesn't yet exist. Construction order of member variables in C++ follows the order they are declared in the class, not the order they appear in the initializer list — a subtle source of bugs if the declaration order and initializer order differ.

---

## Build System

### `tf2_geometry_msgs`

`tf2_geometry_msgs` provides the `doTransform` specialisations for `geometry_msgs` types. It must be listed as both a CMake dependency in `ament_target_dependencies` and included as a header in the source file. Without it, calls to `tf2::doTransform` with `geometry_msgs` types will fail to compile because the template specialisations are not visible to the compiler.

---

## Diagnostic Tools

| Tool | What it tells you |
|---|---|
| `ros2 run tf2_tools view_frames` | Generates `frames.pdf` — the full transform tree with frame names, broadcaster node names, and publish rates. First tool to reach for when debugging spatial issues. |
| `ros2 run tf2_ros tf2_echo <parent> <child>` | Streams the live transform between two frames — translation and quaternion — directly from the `/tf` topic. |
| `ros2 run tf2_ros tf2_monitor <parent> <child>` | Prints statistics about transform publication frequency, timestamp age, and gaps. Use when a transform is arriving irregularly or with stale timestamps. |

---

## Interview Questions

**What is the TF2 buffer and where does it live?**
The buffer is a local time-history of the transform tree, maintained inside each node that creates a `TransformListener`. There is no central TF2 server — every node reconstructs the tree independently from the `/tf` and `/tf_static` topics. Lookups are always local and never involve a network round trip.

---

**Why must the TF2 frame graph be a tree with no loops?**
A loop would create two different paths between the same frame pair, producing two different composed transforms due to floating point errors. A tree guarantees exactly one path and therefore one unambiguous answer for any transform lookup.

---

**What is the difference between `/tf` and `/tf_static`?**
`/tf` carries time-varying transforms published continuously. The buffer stores a sliding time history of these. `/tf_static` carries fixed transforms published once. The topic is latched — new subscribers receive the last value immediately — and the buffer treats static transforms as having infinite temporal validity. They never age out and never need refreshing.

---

**What does `TimePointZero` do and when would you use a real timestamp instead?**
`TimePointZero` requests the latest available transform regardless of age. It is robust during development but hides staleness. A real timestamp requests the transform at a specific moment, enabling TF2 to interpolate between buffered samples for temporal precision. Use real timestamps in sensor pipelines where the transform must correspond to the exact moment the data was captured.

---

**Why does TF2 store a time history of transforms rather than just the latest value?**
Sensor data arrives at different rates and with processing latency. A lidar scan timestamped 80 ms ago needs the transform from 80 ms ago to be correctly expressed in the world frame — the current transform is wrong. The time history allows TF2 to interpolate between buffered samples and return the transform at any requested timestamp within the window.

---

**What happens if you publish a time-varying transform to `/tf_static`?**
The system uses the first published value forever. The topic is latched and the buffer treats static transforms as infinitely valid. Subsequent publications are ignored by existing subscribers. The spatial reasoning downstream will silently produce wrong results with no error.

---

**What is the REP-105 frame chain and which node is responsible for each segment?**
The canonical chain is `map` → `odom` → `base_link` → sensor frames. The localisation system publishes `map` → `odom` — this can jump. Odometry publishes `odom` → `base_link` — this is always continuous. The robot state publisher broadcasts fixed and joint-driven URDF links as children of `base_link`. Sensor frames are static children of their mounting link.

---

**Why are points and vectors transformed differently, and how does `doTransform` handle this?**
A point has a position in space — it is affected by both rotation and translation. A vector represents a direction or magnitude — it is affected by rotation only. In homogeneous coordinates, points have w=1 and vectors have w=0. The 4×4 transformation matrix multiplication naturally applies translation only to w=1 components, so the correct behaviour follows from the representation without any special casing in the code.
