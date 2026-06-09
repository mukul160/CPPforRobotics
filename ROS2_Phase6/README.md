# Phase 6 — Tool Fluency and Diagnostic Workflow

> ROS2 Fundamentals Drill · C++ / ROS2 Humble · Ubuntu 22.04

This phase covers the diagnostic tools and workflows used daily in a robotics lab. No new node patterns — the goal is the ability to sit down at an unfamiliar ROS2 system, understand what's running, and find what's broken within minutes.

---

## The Diagnostic Decision Tree

Work through this in order. Most problems surface at step 1, 2, or 3.

```
1. Is the node running?
   └── ros2 node list

2. Are the topics connected?
   └── rqt_graph  /  ros2 topic info /topic --verbose

3. Is data flowing?
   └── ros2 topic hz /topic
   └── ros2 topic echo /topic --once

4. Is the transform tree complete?
   └── ros2 run tf2_tools view_frames
   └── ros2 run tf2_ros tf2_monitor

5. Is the system healthy overall?
   └── ros2 doctor --report

6. Is it a lifecycle issue?
   └── ros2 lifecycle list
   └── ros2 lifecycle get /node_name

7. Need to reproduce a past failure?
   └── ros2 bag play session_name
```

---

## Tool Reference

### `ros2 doctor`

System health check. Run first on any unfamiliar system.

```bash
ros2 doctor              # quick summary
ros2 doctor --report     # full verbose output — middleware config, env vars, network
```

Surfaces middleware configuration issues, missing environment variables, and network problems that would otherwise take significant time to find manually. Run with nothing active to establish a baseline, then again with nodes running to see what changes.

---

### `rqt_graph`

Node and topic topology. Change the dropdown to **Nodes/Topics (all)** for the full picture.

| What you see | What it means |
|---|---|
| Node with no incoming edges on a subscription | No publisher exists for that topic |
| Topic with publisher but no subscribers | Data produced but nothing consuming it |
| Completely isolated node | Wrong namespace, or failed to connect |
| Node disappears from graph | Node crashed or was killed |

---

### `ros2 topic` — deep inspection

```bash
ros2 topic list                          # all active topics
ros2 topic info /topic --verbose         # QoS profile of every pub and sub
ros2 topic hz /topic                     # actual publish rate
ros2 topic delay /topic                  # latency between stamp and receipt
ros2 topic echo /topic --once            # single message — inspect field values
```

**QoS mismatch** is the most common cause of a silent subscription. A `RELIABLE` subscriber will not receive from a `BEST_EFFORT` publisher — the middleware silently declines to connect. `ros2 topic info --verbose` makes mismatches immediately visible.

**High delay** on a sensor topic means the pipeline is backed up — callbacks taking too long or the executor overloaded.

---

### `ros2 node` inspection

```bash
ros2 node list                   # all active nodes
ros2 node info /node_name        # all subscriptions, publishers, services, actions
```

Use `ros2 node info` when a node is present on the graph but not behaving correctly. If a subscription is listed but no publisher exists for that topic, the node is waiting for data that will never arrive.

---

### TF2 diagnostics

```bash
ros2 run tf2_tools view_frames               # generate frames.pdf — full tree
ros2 run tf2_ros tf2_echo <parent> <child>   # live transform stream
ros2 run tf2_ros tf2_monitor                 # publication rates, staleness, gaps
```

**`tf2_monitor` columns to watch:**

| Column | What to look for |
|---|---|
| Most recent transform | Growing old while system runs → broadcaster died |
| Frames with no parent | Multiple disconnected roots → tree is split |
| Publication rate | Below expected → broadcaster stalling or dropping |

A split transform tree — two disconnected root frames — means lookups between the two halves will always fail. The missing segment is the diagnosis.

---

### `ros2 bag`

Record and replay topic data. The primary tool for intermittent failures.

```bash
# Record
ros2 bag record -a -o session_name                        # all topics
ros2 bag record /scan /odom /tf /tf_static -o session     # specific topics

# Inspect
ros2 bag info session_name                                 # message counts, duration, topics

# Replay
ros2 bag play session_name                                 # normal speed
ros2 bag play session_name --rate 0.5                     # half speed
ros2 bag play session_name --remap /scan:=/scan_replay    # remap topic on replay
```

**Workflow for intermittent failures:**
1. Record a session that includes the failure
2. Inspect the bag to confirm the relevant topics were captured
3. Replay at reduced speed with debug logging enabled
4. Echo the relevant topics during replay to isolate whether the failure was in sensing or control

The logging level does not affect what gets recorded — bags capture the raw message stream regardless. You can record at normal verbosity and replay with `--ros-args --log-level debug` to get full diagnostic output without needing to reproduce the failure live.

---

### Reading a Nav2 YAML config

Nav2 behaviour is entirely driven by its parameter YAML. Knowing where to look is a daily skill.

```bash
# Open the default config
cat /opt/ros/humble/share/nav2_bringup/params/nav2_params.yaml

# Find specific parameters
grep -A 2 "inflation_radius" /opt/ros/humble/share/nav2_bringup/params/nav2_params.yaml
grep -A 2 "robot_radius" /opt/ros/humble/share/nav2_bringup/params/nav2_params.yaml
grep -A 2 "max_vel_x" /opt/ros/humble/share/nav2_bringup/params/nav2_params.yaml
```

**Key parameters and their effects:**

| Parameter | Effect |
|---|---|
| `inflation_radius` | How far obstacles are inflated outward in the costmap. Increase for more wall clearance. Too large → planner can't find paths in narrow spaces. |
| `robot_radius` | Robot footprint radius for collision checking. If larger than the actual robot, paths through narrow gaps will be rejected. |
| `max_vel_x` | Maximum forward speed the controller will command. |
| `xy_goal_tolerance` | How close the robot must get to the goal position before declaring success. |
| `transform_tolerance` | How old a transform can be before Nav2 considers it stale. Increase if you see transform timeout warnings on a slow system. |

The YAML structure is always:
```yaml
node_name:
  ros__parameters:
    parameter_name: value
```

The top-level key must match the node name exactly. A mismatch means the parameter is silently ignored and the node uses its default value — a common source of "I set this parameter but nothing changed" confusion.

---

### `ros2 lifecycle` — system state audit

```bash
ros2 lifecycle list                        # all lifecycle nodes on the graph
ros2 lifecycle get /node_name              # current state of a specific node
ros2 lifecycle set /node_name configure    # drive a transition manually
```

When Nav2 behaves unexpectedly, check whether all its nodes are in the `active [3]` state. A node stuck in `inactive [2]` will not publish costmaps, plan paths, or control the robot — and will produce no error, just silence.

---

## Common Failure Patterns and Diagnoses

| Symptom | First check | Likely cause |
|---|---|---|
| Subscription callback never fires | `ros2 topic info /topic --verbose` | QoS mismatch |
| Transform lookup always fails | `ros2 run tf2_tools view_frames` | Missing frame — broadcaster not running |
| Nav2 costmap not appearing | `ros2 lifecycle get /local_costmap` | Node not in active state |
| Robot not moving after goal sent | `ros2 topic hz /cmd_vel` | Controller not publishing — check lifecycle and transform tree |
| Node present but not responding | `ros2 node info /node_name` | Subscription topic has no publisher |
| Planning always fails | RViz global costmap | Goal inside inflation radius of a wall |
| System worked, now doesn't | `ros2 doctor --report` | Environment variable missing after new terminal |

---

## Interview Questions

**A subscription callback is never firing. The node is running and the publisher exists. What do you check?**
QoS compatibility. Run `ros2 topic info /topic --verbose` and compare the reliability and durability profiles of the publisher and subscriber. A `RELIABLE` subscriber will not receive from a `BEST_EFFORT` publisher — the middleware silently declines to connect with no error raised anywhere.

---

**A transform lookup is failing with "frame does not exist". How do you diagnose it?**
Run `ros2 run tf2_tools view_frames` to generate the full transform tree. Identify which frame is missing. Then identify which node should be broadcasting that frame — check whether it's running with `ros2 node list` and whether it's in the correct lifecycle state with `ros2 lifecycle get`. If the node is running, run `ros2 run tf2_ros tf2_monitor` to check whether the transform is being published at the expected rate or has gone stale.

---

**How do you debug a failure that only occurs occasionally and is hard to reproduce?**
Record a bag with `ros2 bag record` covering the relevant topics during a session that includes the failure. Inspect it with `ros2 bag info` to confirm the data was captured. Replay at reduced speed with `ros2 bag play --rate 0.5` and echo the relevant topics to isolate whether the failure originated in sensing, transforms, or control. The logging level does not affect what gets recorded — replay with debug logging enabled to get full diagnostic output without needing to reproduce the failure live.

---

**Nav2 is running but the robot doesn't move after a goal is sent. Walk through your diagnosis.**
First check `ros2 topic hz /cmd_vel` — if the controller is running, it should be publishing velocity commands. If nothing is publishing, check `ros2 lifecycle list` to verify all Nav2 nodes are active. If they are, check the transform tree with `view_frames` — the controller needs `map` → `odom` → `base_link` to be complete and fresh. If the tree is intact, check `ros2 topic info /cmd_vel --verbose` to verify the differential drive controller is subscribed and the QoS profiles match.

---

**A Nav2 parameter you set in the YAML has no effect. Why?**
The most common cause is a top-level node name mismatch — the YAML key must match the node name exactly as it appears in `ros2 node list`. A mismatch means the parameter server never receives the value and the node uses its compiled default silently. Verify with `ros2 param get /node_name parameter_name` — if it returns the default rather than your value, the YAML key is wrong.
