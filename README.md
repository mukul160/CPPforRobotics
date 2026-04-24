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