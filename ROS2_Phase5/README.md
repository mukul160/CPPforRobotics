# Phase 5 — Lifecycle Nodes and the Multithreaded Executor

> ROS2 Fundamentals Drill · C++ / ROS2 Humble · Ubuntu 22.04

This phase covers the production patterns used in real ROS2 systems — managed lifecycle nodes for controlled startup and shutdown, and the multithreaded executor with callback groups for safe parallel execution. Nav2 uses both of these patterns internally. Phase 5 assumes Phase 1–4 fluency.

---

## Packages

| Package | What it does |
|---|---|
| `lifecycle_sensor` (sensor_node) | A lifecycle node simulating a hardware sensor. Transitions through all four states manually via `ros2 lifecycle set`. Introduces `LifecycleNode`, `LifecyclePublisher`, `CallbackReturn`, and the configure/activate separation. |
| `lifecycle_sensor` (multithreaded_node) | Extends the lifecycle node with a `MultiThreadedExecutor` and two callback groups. The timer callback calls a service without deadlocking. Introduces `MutuallyExclusive` and `Reentrant` callback groups, lambda captures, and `SharedFuture`. |

---

## The Lifecycle State Machine

```
                  configure()         activate()
  Unconfigured ────────────► Inactive ────────────► Active
       ▲                         ▲                     │
       │        cleanup()        │     deactivate()    │
       └─────────────────────────┘◄────────────────────┘
       
  Any state ──── shutdown() ────► Finalized
```

| State | What exists |
|---|---|
| Unconfigured | Node constructed, nothing allocated |
| Inactive | Resources allocated, hardware verified, not yet running |
| Active | Timers running, publishers publishing |
| Finalized | Resources released, node shutting down |

---

## ROS2 Concepts

### Why configure and activate are separate

Configure validates readiness and allocates resources. Activate starts execution. A system manager — like Nav2's lifecycle manager — can configure all nodes first, verify every node is ready, then activate them in dependency order. If any node fails to configure, nothing starts. This prevents a controller activating before its sensor publishers are ready, or a planner starting before its costmap is initialised.

### `CallbackReturn` — what the three values mean

| Return value | What happens |
|---|---|
| `SUCCESS` | Transition completes, node moves to new state |
| `FAILURE` | Transition rolls back, node stays in current state |
| `ERROR` | Node enters error processing state — needs explicit recovery |

Use `FAILURE` when a precondition isn't met — hardware not found, invalid parameter. Use `ERROR` only for genuinely unrecoverable situations.

### `LifecyclePublisher` — controlled data flow

A `LifecyclePublisher` has an internal active flag set by `on_activate()` and cleared by `on_deactivate()`. When inactive, `publish()` is silently a no-op. This ensures downstream subscribers never receive data from a node that isn't fully ready. `is_activated()` exposes the flag for manual guards inside callbacks.

### The single-threaded executor deadlock

The executor runs one callback at a time. If a callback calls a service and waits for the response, the executor is blocked — it cannot process the incoming response because it is occupied running the callback. Neither side can proceed. This is a deadlock.

The sequence:
1. `timerCallback` starts running on the executor thread
2. A service request is sent and the callback waits for a response
3. The response arrives as a middleware event
4. The executor cannot process it — it is blocked inside the callback
5. The callback cannot proceed — the response never arrives
6. Deadlock

### Callback groups — the solution

Callback groups tell the multithreaded executor which callbacks can run in parallel.

| Type | Behaviour | Use when |
|---|---|---|
| `MutuallyExclusive` | Only one callback in the group runs at a time | Callbacks share mutable state |
| `Reentrant` | Callbacks can run in parallel | Callbacks are stateless or thread-safe |

Assigning the service client to a `Reentrant` group means its response callback is processed on a separate thread while the timer callback is still running. The timer callback never blocks — it sends the request asynchronously and returns. The response fires independently on Thread 2.

### `MutuallyExclusive` does not mean globally thread-safe

Two callbacks in the same `MutuallyExclusive` group never run simultaneously — but they may run on different threads at different times. Shared mutable state accessed from callbacks in *different* groups still requires explicit protection — `std::mutex` or `std::atomic`. `LifecyclePublisher::publish()` is internally thread-safe, which is why accessing it from both groups in this node is safe. In general, audit any shared state accessed across group boundaries.

### The corrected main for lifecycle nodes

`rclcpp::spin()` accepts `rclcpp::Node::SharedPtr` directly. `LifecycleNode` exposes its base interface separately via `get_node_base_interface()`. The explicit executor pattern works for both:

```cpp
auto node = std::make_shared<MyLifecycleNode>();
rclcpp::executors::MultiThreadedExecutor executor(rclcpp::ExecutorOptions(), 2);
executor.add_node(node->get_node_base_interface());
executor.spin();
```

The integer argument to `MultiThreadedExecutor` sets the thread count. One thread per callback group is the typical starting point.

---

## C++ Patterns

### `static` inside a function

```cpp
static std::mt19937 rng(std::random_device{}());
```

`static` inside a function means the variable is initialised once on the first call and persists for the program's lifetime. Without it, a new random generator would be constructed and seeded on every timer tick — expensive, and potentially producing identical sequences. With it, one generator lives for the node's lifetime.

### Lambda captures — by value vs by reference

```cpp
[this, reading](rclcpp::Client<std_srvs::srv::Trigger>::SharedFuture response) {
```

`reading` is captured by value — a copy is made when the lambda is created. By the time the lambda fires on Thread 2, the timer callback has returned and the original `reading` variable is destroyed. Capturing by value preserves it safely. Capturing by reference (`&reading`) would leave a dangling reference — undefined behaviour.

`this` is captured to give the lambda access to member variables and methods. Always capture `this` explicitly when a lambda needs to call methods or access members of the enclosing object.

### `SharedFuture` and chained `->` access

```cpp
response.get()->success
```

`response` is a `SharedFuture` — a shared version of `std::future` that multiple callbacks can hold simultaneously. `response.get()` blocks until the future is ready and returns the response `SharedPtr`. The `->` then accesses the `success` field through that pointer. Two operators in sequence: `.get()` on the future, then `->` on the pointer it returns.

### `shared_ptr::reset()`

```cpp
publisher_.reset();
timer_.reset();
```

`reset()` on a `shared_ptr` releases ownership — it decrements the reference count and sets the pointer to null. If this was the last owner, the object is destroyed. Used in `on_cleanup` and `on_shutdown` to explicitly release resources rather than waiting for the destructor. Explicit cleanup in lifecycle nodes is important because the node object may outlive the active state by a significant margin.

---

## Lifecycle Node CLI Commands

```bash
ros2 lifecycle get /node_name               # query current state
ros2 lifecycle set /node_name configure     # trigger configure transition
ros2 lifecycle set /node_name activate      # trigger activate transition
ros2 lifecycle set /node_name deactivate    # trigger deactivate transition
ros2 lifecycle set /node_name cleanup       # trigger cleanup transition
ros2 lifecycle set /node_name shutdown      # trigger shutdown transition
ros2 lifecycle list                         # list all lifecycle nodes on the graph
```

---

## Interview Questions

**Why are configure and activate separate transitions in a lifecycle node?**
Configure allocates resources and validates readiness. Activate starts execution. Separating them allows a system manager to verify all nodes are ready before starting any of them. If one node fails to configure, nothing activates. This prevents partial system startup where some nodes are running and others are not.

---

**What happens if `on_configure` returns `FAILURE`?**
The transition rolls back and the node stays in the Unconfigured state. No resources are allocated, no publishers exist. The node can be configured again once the underlying problem is fixed — a reconnected sensor, a corrected parameter file. `FAILURE` is the correct return for any recoverable precondition failure.

---

**Why does calling a service from inside a callback deadlock on the single-threaded executor?**
The executor runs one callback at a time. When a callback waits for a service response, the executor thread is blocked. The response arrives as a middleware event that requires the executor to process it — but the executor is occupied. Neither side can proceed. The solution is to put the service client in a separate callback group and use the multithreaded executor, so the response is processed on a different thread while the original callback runs.

---

**What is the difference between `MutuallyExclusive` and `Reentrant` callback groups?**
`MutuallyExclusive` allows only one callback in the group to run at a time — safe for callbacks that share mutable state. `Reentrant` allows callbacks to run in parallel with each other and with other groups — appropriate for stateless callbacks or those that only access thread-safe resources. Putting the timer in `MutuallyExclusive` and the service response in `Reentrant` allows the response to be processed on a separate thread without risking a data race on shared state.

---

**Does `MutuallyExclusive` make shared state thread-safe?**
No. It prevents two callbacks in the same group from running simultaneously, but it does not protect state accessed from callbacks in different groups. If a `MutuallyExclusive` callback and a `Reentrant` callback both access the same member variable, a data race is possible. Shared mutable state across group boundaries requires explicit protection — `std::mutex` or `std::atomic`.

---

**Why capture `reading` by value in the service response lambda?**
By the time the lambda fires on Thread 2, the timer callback that created it has already returned and its local `reading` variable has been destroyed. Capturing by value makes a copy at the moment the lambda is created, preserving the value safely. Capturing by reference would leave a dangling reference to a destroyed variable — undefined behaviour that may appear to work but will corrupt memory unpredictably.

---

**What does `publisher_->on_activate()` do and why is it needed?**
It sets the internal active flag on the `LifecyclePublisher`. Without this call, `publish()` is silently a no-op even though the publisher object exists. The flag ensures data only flows when the node is in the Active state. `on_deactivate()` clears the flag, stopping data flow without destroying the publisher — so it can be reactivated without reconstruction.
