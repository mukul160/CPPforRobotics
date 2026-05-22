## Chapter 3: Control Systems (PID)

This module implements a professional-grade **PID (Proportional-Integral-Derivative)** control system. Control theory is the "reflex" system of a robot—it determines how to translate a high-level goal (like "move to 90 degrees") into specific motor voltages while accounting for physics, friction, and momentum.

### 🕹️ The Control Triad

*   **Proportional (The Muscle):** Applies power based on the current **Error** ($Target - Actual$). The further away the robot is, the harder it pushes.
*   **Integral (The Persistence):** Accumulates error over time. If the robot is stalled by friction, the Integral builds up "pressure" until the motor overcomes the obstacle. Includes **Anti-Windup** logic for safety.
*   **Derivative (The Brakes):** Responds to the **Rate of Change** of the error. It acts as a dampener to prevent overshoot and oscillation as the robot approaches the target.

### 🚀 Advanced Features

*   **Feed-Forward (FF):** A "proactive" term that provides base power to fight constant forces like gravity or static friction before the PID even starts.
*   **Output Saturation:** Hard-capping the command to match physical hardware limits (e.g., -100% to 100%) using `std::clamp`.
*   **Stability Logic:** Conditional integration ensures the robot remains predictable even if physically blocked or "stuck."

### 🛠️ Tuning Guide

| Parameter | If too Low... | If too High... |
| :--- | :--- | :--- |
| **Kp** | Robot never reaches target. | Robot oscillates (shakes) violently. |
| **Ki** | Robot stalls due to friction. | Robot overshoots and takes too long to settle. |
| **Kd** | Robot overshoots or "rings." | Robot becomes jerky and sensitive to sensor noise. |