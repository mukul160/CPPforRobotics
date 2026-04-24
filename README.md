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