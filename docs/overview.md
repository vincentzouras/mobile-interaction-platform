# Overview

Remote robotic system that can physically interact with a smartphone using a gantry + stylus setup.

Distributed system using:

- A camera to view the phone screen (direct screen capture prevents some apps from running)
- Raspberry Pi as the main controller and for networking
- Arduino running GRBL for motor control of the stylus
- Laptop for remote control and debug/dev tooling

## Architecture

Perception &rarr; Interaction Planning &rarr; Motion Control &rarr; Physical Actuation

### 1. Perception (RPi)

Input: raw camera frames from Arducam  
Output: perceived states / vision functions

- Derives states from camera frames.
- Corrects perspective so the laptop can work with a flat screen coordinate space.
- Library of all useful functions separate from specific app logic, such as text OCR, pixel color detection, object tracking, etc.
  - Can be added to as new use cases arise for specific app logic.
- In the future, if more compute is needed, could run heavy CV logic on remote computer instead of RPi, and send the commands back.

### 2. Interaction Planning (RPi)

Input: perceived states / vision functions  
Output: high-level actions

- Interprets perceived states and uses vision functions to determine appropriate action sequences to complete desired tasks.
- Hierarchy of reusable functions to create complex behaviors.
  - i.e. PlayWordGame calls selectWord, which calls a sequence of motion controls like tap and swipe.

### 3. Motion Control (RPi)

Input: high-level actions  
Output: G-code motion commands sent via USB serial

- Convert user intent like tap, press, and swipe into reusable functions.
- Maps high-level touch gestures onto gantry motion and actuator behavior.
- Hides the raw motor movement details from the application layer.

### 4. Physical Actuation (Arduino)

Input: G-code motion commands sent via USB serial  
Output: stylus movement and touch

- Gantry system that moves the stylus across the phone screen.
- Uses X/Y belt driven stepper motors and a servo for Z-axis touch actuator.
- GRBL firmware interprets G-code and generates stepping/servo signals (pretty much done for me)

## Communication

Laptop needs to receive video stream from RPi.

Dev tooling requires laptop to be able to request the RGB values at a coordinate.

### UDP: Pi &rarr; Laptop

### TCP

Pi server listens for commands like "what color is x,y"

Laptop client sends color and waits for response.

## Gestures

### G-code Commands

### Primitives

- move(x, y)
- press(duration)
- tap(x, y)
- drag(x1, y1, x2, y2)
- home()

## Development Tooling

Can't trust the camera stream to

1. Stream video from RPi to laptop for human viewing.
2. Laptop debug tool lets you click on the displayed phone screen.
3. Laptop sends screen coordinate to RPi.
4. RPi samples its own uncompressed frame.
5. RPi returns color/features/state.
6. You write game logic against those RPi-side perception functions.

## Calibration Workflow

## Coordinate System Mapping
