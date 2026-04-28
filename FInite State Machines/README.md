# Robotics Habit: C++ Finite State Machines (FSM)

This module documents my progress in mastering robotics software architecture through consistent, "casual-repetition" C++ practice. The focus is on moving from basic procedural logic to professional, scalable, and memory-safe design patterns.

## Learning Path: From Toggles to State Patterns

### Phase 1: Logic Fundamentals
- v1: The Toggle (fsm_v1_baseline.cpp) - Basic if/else state switching.
- v2: The Switch Upgrade (fsm_v2_class_switch.cpp) - Moving logic into classes and using switch statements for deterministic behavior.
- v3: Event-Driven Logic (fsm_v3_event_driven.cpp) - Integrating sensor inputs and safety overrides (High-Temperature E-Stop).

### Phase 2: Real-World Constraints
- v4: Temporal Logic (fsm_v4_temporal_logic.cpp) - Using <chrono> for non-blocking time-based transitions (e.g., "Drill for 5 seconds").
- v5: Hierarchical States (HSM) (fsm_v5_hierarchical.cpp) - Nesting "Sub-states" within "Top-level" mission states to manage complexity.

### Phase 3: Professional Architecture

- v6: The Command Queue (fsm_v6_command_queue.cpp) - Decoupling the "Mission Plan" from "Execution" using std::queue.
- v7: The State Design Pattern (fsm_v7_state_pattern.cpp) - The "Boss Fight." Using Polymorphism and Smart Pointers to turn states into independent, memory-managed classes.

| Feature | Robotics Application |
| enum class | Strongly-typed states to prevent logic collisions. |
| std::chrono | Managing hardware timing without freezing the CPU. |
| std::unique_ptr | Preventing memory leaks in long-running robotic systems. |
| virtual / override | Creating generic interfaces for different hardware components (Sensors/Motors). |
| Polymorphism | Allowing a single controller to manage diverse hardware types. |

**Current Status: Completed Chapter 1 (Logic & Flow).**

**Next Objective: Fundamental Math for Robotics (Linear Algebra & 2D Geometry).**

---