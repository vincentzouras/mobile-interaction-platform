# Architecture Overview

Camera &rarr; Perception &rarr; Interaction Planning &rarr; Motion Control &rarr; Physical Actuation

## Subsystems

### 1. Camera (Onboard Hardware)

Input: captures raw camera frames
Output: sends frames to the laptop for processing

- Camera mounted to see the phone screen clearly from a fixed angle.
- Captures video feed and constantly sends frames to the laptop for processing.

### 2. Perception (External Laptop)

Input: raw camera frames
Output: perceived states / vision functions

- Derives states from camera frames.
- Corrects perspective so the laptop can work with a flat screen coordinate space.
- Library of all useful functions separate from specific app logic, such as text OCR, pixel color detection, object tracking, etc.
  - Can be added to as new use cases arise for specific app logic.

### 3. Interaction Planning (External Laptop)

Input: perceived states / vision functions
Output: high-level actions

- Interprets perceived states and uses vision functions to determine appropriate action sequences to complete desired tasks.
- Hierarchy of reusable functions to create complex behaviors.
  - i.e. PlayWordGame calls selectWord, which calls a sequence of motion controls like tap and swipe.

### 4. Motion Control (External Laptop)

Input: high-level actions
Output: low-level motor and servo signals

- Convert user intent like tap, press, and swipe into reusable functions.
- Maps high-level touch gestures onto gantry motion and actuator behavior.
- Hides the raw motor movement details from the application layer.

### 5. Physical Actuation (Onboard Hardware)

Input: low-level motor and servo signals
Output: stylus movement and touch

- Gantry system that moves the stylus across the phone screen.
- Uses X/Y belt driven stepper motors and a servo for Z-axis touch actuator.
