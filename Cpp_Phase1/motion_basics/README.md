# Motion Basics — From Geometry to Control

This folder tracks the evolution of a simple robot simulation built in C++.

The goal was not complexity, but **building intuition step-by-step**, similar to learning a game mechanic over time.

---

## Progression

### 1. Position Updates

* Directly updated `(x, y, theta)`
* No notion of control or dynamics
  → *pure geometry*

---

### 2. Heading-Based Movement

* Introduced `cos(theta), sin(theta)`
* Robot moves in the direction it faces
  → *orientation starts to matter*

---

### 3. Goal Seeking

* Robot moves toward a target point
* Introduced vector normalization
  → *basic navigation*

---

### 4. Obstacle Avoidance

* Added repulsive vector from obstacle
* Combined with goal direction
  → *environment interaction*

---

### 5. Blended Controller

* Used:

  * dot product → forward motion
  * cross product → turning direction
    → *control emerges from geometry*

---

### 6. Velocity-Based Motion (Current)

* Introduced:

  * linear velocity `v`
  * angular velocity `w`
  * timestep `dt`

```cpp
theta += w * dt;
x += v * cos(theta) * dt;
y += v * sin(theta) * dt;
```

* Added smoothing:

```cpp
v = 0.8*v + 0.2*target_v;
```

→ *motion becomes dynamic and realistic*

---

## Key Insight

The system evolved from:

> directly setting position
> → to
> computing motion through control signals

This mirrors how real robots operate.

---

## Next Direction

* visualization
* multiple obstacles
* better controllers (stability + convergence)
* transition toward ROS-style structure

---
