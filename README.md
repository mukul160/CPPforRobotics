# Robotics Practice Log

## Day 1 - 2D Robot Simulator

- Built a simple robot simulator in C++
- Robot has position (x, y) and orientation (theta)
- Controlled using rotation + forward movement
- Learned about:
  - cin input handling
  - loops
  - basic trigonometry (cos, sin)

## Day 2 - Robot with Memory (Path Tracking)

- Converted robot logic into a `Robot` class
- Added path tracking using a vector of (x, y) positions
- Introduced command-based interaction:
  - Move robot
  - Print path history
- Learned about:
  - Classes and methods in C++
  - Vectors (`std::vector`)
  - Storing structured data (`pair<double, double>`)

### Bug I Encountered
- Initially used:
  dx = forward + cos(theta)
  dy = forward + sin(theta)

- This caused incorrect movement (robot moved even when forward = 0)

- Fixed to:
  dx = forward * cos(theta)
  dy = forward * sin(theta)

- Learned the importance of correct mathematical modeling in motion

## Day 3 - Goal-Based Movement (Basic Autonomy)

- Added ability for robot to move toward a target (x, y)
- Implemented `moveToGoal()`:
  - Computes direction using `atan2`
  - Computes distance to goal
  - Moves step-by-step toward target
- Introduced a new command system:
  - Manual movement
  - Path tracking
  - Autonomous goal-seeking

### Key Concepts Learned
- Converting goal position → direction (angle)
- Using `atan2(dy, dx)` for orientation
- Breaking motion into small steps
- Difference between:
  - Manual control
  - Goal-driven behavior

### Observations
- Robot instantly rotates to face the goal (not realistic yet)
- Movement is discrete (step-based), not smooth
- System now resembles basic robot navigation logic

## Day 4 - System Upgrade (Side Quests)

- Combined multiple improvements into one system:
  - Speed control
  - Total distance tracking
  - Limited turning (more realistic motion)
  - Path preview (last 5 positions)
  - Reset functionality

### Key Concepts Learned
- Incremental system design (adding features without breaking structure)
- Basic motion constraints (max turning rate)
- Tracking robot metrics (distance traveled)
- Managing growing state (path history)

### Observations
- Robot motion now feels more realistic (no instant turning)
- System is starting to resemble a simple simulation loop
- Small mathematical changes significantly affect behavior

### Reflection
Focused on reinforcing existing concepts rather than adding new ones.
This helped build intuition and confidence with the system.

## Day 5 - Autonomous Robot Loop (Control System)

- Converted the program into a continuous simulation loop
- Introduced `update()` function to represent robot behavior over time
- Added goal-setting mechanism:
  - Robot continuously moves toward an active goal
- Separated:
  - Goal assignment (`setGoal`)
  - Motion logic (`update`)

### Key Concepts Learned
- Sense → Decide → Act loop (core robotics pattern)
- Continuous vs discrete execution
- Gradual motion instead of instant execution
- State-driven behavior (`hasGoal`)

### Observations
- Robot now behaves more like a physical system
- Movement is smoother and more realistic (limited turning + step size)
- Architecture feels closer to real robotics software

### Reflection
This was a shift from writing functions to designing a system.
The robot is no longer directly controlled each step—instead, it operates autonomously toward a goal.

## Day 6 - Obstacle Avoidance (Reactive Behavior)

- Introduced a simple obstacle in the environment
- Added distance-based sensing using Euclidean distance
- Implemented reactive obstacle avoidance:
  - Robot moves away when within a threshold distance
  - Otherwise continues toward goal

### Key Concepts Learned
- Basic sensing (distance to obstacle)
- Reactive behavior vs planned behavior
- Angle normalization (handling wrap-around in rotation)
- Combining multiple behaviors (goal-seeking + avoidance)

### Observations
- Robot successfully avoids obstacle but does not reliably settle at goal
- Motion becomes oscillatory or spiral-like after avoidance
- Behavior emerges from simple rules but lacks stability

### Limitations Identified
- No proper goal convergence logic
- Competing behaviors (goal vs avoidance) are not blended
- No velocity control or damping near goal
- System is purely reactive, not predictive

### Reflection
This was the first step into environment interaction.
Small local rules produced complex motion, but also exposed the need for more structured control systems.

## Day 7 & 8 - Vector Blending + Control Instability

- Introduced vector-based control (instead of angle-only logic)
- Implemented blending of:
  - Goal direction (attractive vector)
  - Obstacle avoidance (repulsive vector)
- Added proportional control for:
  - Linear velocity (based on distance to goal)
  - Angular velocity (based on heading error)

### Key Concepts Learned
- Vector normalization and direction-based control
- Behavior blending instead of switching logic
- Feedback control (proportional controller)
- Angle wrapping and shortest rotation

### Observations
- Robot initially moves toward goal and avoids obstacle smoothly
- However, system becomes unstable over time
- Robot diverges and moves far away instead of settling

### Issues Encountered
- Linear velocity increases with distance → causes runaway behavior
- Misalignment between heading and goal leads to incorrect motion
- Positive feedback loop causes exponential divergence
- System lacks damping and stability guarantees

### Reflection
This was the first encounter with control instability.
Even though the logic was mathematically sound, the system behavior was not stable.

This highlights an important lesson:
A working algorithm is not the same as a stable system.

Further improvements needed:
- Better velocity control (clamping / damping)
- Alignment-aware movement
- Possibly more structured control systems (PID, potential fields, etc.)

## Stable Blended Controller (Dot + Cross Product)

### What Changed

Reimplemented the blended controller using **vector-based control (dot + cross products)** instead of angle-based error correction.

Previous versions relied on:
- Angle difference (`targetAngle - theta`)
- Direct rotation updates
- Linear velocity tied only to distance

This led to unstable behavior:
- Robot moved even when facing the wrong direction
- Speed increased as it moved away from the goal
- Positive feedback loop caused divergence (runaway motion)

---

### Key Improvement

The controller is now based on **geometric relationships between vectors**:

- **Dot product → controls forward motion**
- **Cross product → controls turning direction**

---

### Control Logic

Let:
- `heading` = robot’s current direction  
- `finalDir` = blended direction (goal + obstacle avoidance)

#### 1. Forward Motion (Dot Product)

```
forward = heading ⋅ finalDir
```

Interpretation:
- `≈ 1` → aligned → move forward
- `≈ 0` → sideways → slow down
- `< 0` → opposite → stop

This prevents the robot from moving in the wrong direction.

---

#### 2. Turning (Cross Product)

```
turn = heading × finalDir
```


Interpretation:
- `> 0` → turn left  
- `< 0` → turn right  
- `= 0` → already aligned  

This replaces angle difference calculations and avoids discontinuities (like ±π wrapping issues).

---

### Why This Version Works

This implementation introduces **negative feedback**, which stabilizes the system:

- Misalignment → reduces forward velocity (via dot product)
- Misalignment → increases turning (via cross product)
- Alignment → restores forward motion

As a result:
- The robot turns first, then moves
- Movement is always directionally valid
- No runaway acceleration occurs
- System converges instead of diverging

---

### Weighted Vector Blending (Goal + Obstacle)

The final movement direction is computed as:

```
finalDir = goalDir + weight * avoidDir
```


Where:
- `goalDir` → normalized vector toward goal  
- `avoidDir` → normalized vector away from obstacle  
- `weight` → depends on distance to obstacle  

#### Weight Calculation

```
if (distObs < threshold)
weight = threshold - distObs;
else
weight = 0;
```

Interpretation:
- Far from obstacle → ignore it  
- Close to obstacle → avoidance influence increases  
- Very close → strong repulsion  

After blending, the vector is normalized:

```
finalDir.normalize();
```

---

### Key Takeaways

- Stability is not just about correct math—it’s about correct **feedback structure**
- Dot product ensures movement is aligned with intent
- Cross product ensures correct turning direction
- Vector-based control is more robust than angle-based control

---

### Reflection

This iteration fixed a major issue from previous versions:
> A mathematically correct controller can still be behaviorally unstable.

The new approach produces:
- Smooth motion  
- Direction-aware movement  
- Stable convergence toward the goal  

This feels much closer to how real robotic systems behave.
