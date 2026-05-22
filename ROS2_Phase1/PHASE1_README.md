# Phase 1 — Core Communication Patterns

> ROS2 Fundamentals Drill · C++ / ROS2 Humble · Ubuntu 22.04

This phase establishes the four primitives that every ROS2 system is built on: topics, services, parameters, and custom interfaces. All subsequent phases assume fluency with these patterns at the level documented here.

---

## Packages

| Package | What it does |
|---|---|
| `system_monitor` | Timer-driven node publishing uptime and status on two topics. Introduces the node-as-class pattern, wall timers, the executor model, and `shared_ptr` ownership. |
| `monitor_service` | Two-node system where a client sends a reset request to a server over a service. Covers synchronous vs asynchronous service calls, futures, and `std::bind` with placeholders. |
| `system_monitor_params` | Extends the monitor node with declared parameters loaded from a YAML config file via a Python launch file. Covers the parameter lifecycle, type system, and the declare-before-get rule. |
| `report_node` + `robot_interfaces` | Custom `.msg` and `.srv` types in a dedicated interface package consumed by a separate node package. Covers `rosidl`, the two-package CMake dependency chain, and `std_msgs/Header`. |

---

## ROS2 Concepts

### The executor and `rclcpp::spin`

`rclcpp::spin(node)` hands the node to the default single-threaded executor, which runs an event loop asking the middleware whether any callbacks are ready — timers firing, messages arriving, service requests incoming. Callbacks run sequentially and must never block, because the executor cannot process any other event while a callback is executing. A callback that takes 900 ms inside a 1000 ms timer period will miss timer deadlines. For true parallelism, ROS2 provides `MultiThreadedExecutor` and callback groups.

### Clean shutdown and SIGINT handling

`rclcpp::init` installs a signal handler that intercepts `SIGINT` (Ctrl+C) and sets a stop flag inside the executor. The executor checks this flag each loop iteration and exits cleanly when it is set, allowing `spin` to return. `rclcpp::shutdown` then deregisters the node from the ROS2 graph and releases DDS resources. Omitting it leaves stale entries in the graph and can cause middleware cleanup issues.

### Services vs topics

Topics are for streaming data where acknowledgement is not required — sensor readings, odometry, status. Services are for transactional interactions with a confirmed response — reset commands, readiness checks, single queries. A service call that blocks inside a callback will deadlock the single-threaded executor because the executor cannot process the incoming response while it is stuck in the callback.

### The parameter system

Every node has a built-in parameter server wired up by the middleware automatically. Parameters must be declared before they can be read — this is a deliberate design choice that ensures introspection tools like `ros2 param list` always see a complete and accurate picture. Reading a parameter in the constructor is a one-time snapshot; it does not create a live binding. Runtime reactivity requires a parameter callback registered with `add_on_set_parameters_callback`. The priority order from lowest to highest is: default value in `declare_parameter`, YAML file, command-line arguments.

### The rosidl interface generation pipeline

Custom `.msg` and `.srv` files are language-agnostic interface definitions compiled by `rosidl` into C++ headers, type support libraries, and Python bindings. The generated headers are build artifacts — they live in the install space, not the source tree, which is why the install space must be sourced before any package that depends on them can be compiled. Interface packages should be kept separate from node packages so that the message contract can be shared without coupling node implementations to each other.

### `use_sim_time` and the `Header` field

The `std_msgs/Header` type carries a timestamp (`this->now()`) and a `frame_id` string. When `use_sim_time` is true, `now()` follows Gazebo's `/clock` topic rather than the wall clock, synchronising all nodes to simulation time. The `frame_id` names the coordinate frame the data is expressed in and is the bridge to the TF2 transform system. Every piece of sensor data with a header can be transformed into any other frame using TF2.

---

## C++ Patterns

### `std::shared_ptr` and `std::make_shared`

A `shared_ptr` wraps a heap pointer and tracks reference count. When the count reaches zero the object is destroyed automatically. ROS2 uses `shared_ptr` pervasively because ownership in a robotics system is genuinely shared — a publisher may be held by the node, the executor, and a callback simultaneously. `std::make_shared<T>()` performs a single heap allocation for both the object and the reference-counting block, whereas `shared_ptr<T>(new T())` allocates twice.

### The member initializer list

Members listed after the colon in a constructor are constructed before the constructor body runs. For parent classes like `rclcpp::Node`, the initializer list is the only way to pass constructor arguments — the parent cannot be constructed from inside the body. For plain members, the initializer list constructs them correctly in one step rather than default-constructing and then overwriting.

### `std::bind` and placeholders

A member function has an implicit `this` parameter and cannot be stored as a plain callable without binding it to an object. `std::bind(&Class::method, this, std::placeholders::_1, std::placeholders::_2)` produces a callable that forwards its arguments to the method on the bound object. The modern equivalent is a lambda with a `[this]` capture, which is more readable and generally preferred in new code.

### `static_cast`

`static_cast<double>(int_value)` makes an implicit numeric conversion explicit and visible. With `-Wall -Wextra`, some implicit conversions produce warnings; `static_cast` suppresses them by signalling intent. It is resolved at compile time — both types are known at the point of the cast. Contrast with `dynamic_cast` (runtime, polymorphic hierarchies) and `reinterpret_cast` (raw bit reinterpretation, almost never appropriate in robotics code).

### `std::chrono::duration<double>`

`std::chrono::seconds` is an alias for `duration<int64_t, ratio<1>>` and accepts only integers. `duration<double>` represents seconds as a floating-point value, enabling fractional periods like 0.5 s. The tradeoff is losing the precision guarantees of integer arithmetic, which is acceptable for a non-realtime wall timer.

---

## Build System

### The ament CMake pattern

| CMake call | What it does |
|---|---|
| `find_package(X REQUIRED)` | Locates package X in the install space and makes its headers and libraries available. `REQUIRED` aborts the build if not found rather than continuing silently. |
| `add_executable(target src/file.cpp)` | Compiles the source file into a named binary. |
| `ament_target_dependencies(target pkgs...)` | Sets include paths, linker flags, and build ordering for ament packages. Preferred over manual `target_link_libraries` for ROS2 dependencies. |
| `install(TARGETS ... DESTINATION lib/${PROJECT_NAME})` | Copies the binary to the install space where `ros2 run` looks for it. Without this, the build succeeds but the node cannot be found at runtime. |
| `install(DIRECTORY launch config ...)` | Copies non-code assets to the share space where `ros2 launch` and `get_package_share_directory` look for them. |

### The two-package interface dependency chain

In the interface package: `rosidl_generate_interfaces` generates headers and type support libraries. `DEPENDENCIES std_msgs` tells rosidl that generated headers need to include `std_msgs` headers. `ament_export_dependencies(rosidl_default_runtime)` makes the runtime serialisation libraries available to downstream packages — without it the downstream linker fails even if the compiler succeeds. In the node package: `find_package(robot_interfaces REQUIRED)` reads the exported dependency information and resolves both headers and libraries in one call.

---

## Interview Questions

**Why is a ROS2 node implemented as a class rather than a set of free functions?**
Callbacks need persistent state between invocations — publishers, timers, and data members must live somewhere that survives each call. A class instance on the heap provides this. Free functions have no persistent home for such state.

---

**What happens if you call `rclcpp::spin` from inside a callback?**
The single-threaded executor is already running — calling spin again from within a callback it is executing creates a re-entrant deadlock. The inner spin blocks waiting for events that the outer spin cannot process because it is stuck waiting for the callback to return.

---

**Why does `async_send_request` return a future rather than the response directly?**
A blocking synchronous call inside a callback would deadlock the single-threaded executor — it cannot process the incoming service response while stuck in the callback. The future separates the act of sending from the act of waiting, letting the executor remain live until the response arrives.

---

**What is the difference between declaring a parameter and getting a parameter?**
Declaring registers the parameter with the node's parameter server, establishes its type, and sets a default. Getting reads the current value. ROS2 requires declaration first so that introspection tools always see a complete picture. Calling get on an undeclared parameter throws a runtime exception.

---

**If you call `ros2 param set` on a running node, why might the node's behaviour not change?**
Reading a parameter in the constructor is a one-time snapshot. The parameter server is updated, but nothing in the node is watching it. Runtime reactivity requires a parameter callback registered with `add_on_set_parameters_callback`, which is invoked by the middleware whenever a set request arrives.

---

**Why must an interface package be sourced before a node package that depends on it can be built?**
The C++ headers for custom types are generated by rosidl at build time and placed in the install space — they are build artifacts, not source files. The compiler needs them to exist before it can compile any translation unit that includes them, and they only exist after the interface package has been built and installed.

---

**What does `ament_export_dependencies(rosidl_default_runtime)` do and what breaks without it?**
It records that any package depending on this interface package also needs the rosidl runtime serialisation libraries. Without it, the downstream package's compiler succeeds — the headers are found — but the linker fails because the type support libraries are not resolved.

---

**What does `use_sim_time` do and where does it need to be set?**
It switches the node's clock source from the system wall clock to the `/clock` topic published by Gazebo. It must be set on every node that needs to be time-synchronised with the simulator, typically via a YAML parameter file loaded in the launch file.
