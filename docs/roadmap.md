# Roadmap

## Phase 1: Hardware & Firmware

- [ ] Assemble XY gantry (Openbuilds or equivalent)
- [ ] Mount NEMA steppers, servo, phone fixture
- [ ] Assemble A4988 drivers and CNC Shield
- [ ] Flash GRBL firmware to Arduino Uno
- [ ] Basic G-code validation (move to coordinates, servo control)

## Phase 2: Vision & Calibration

- [ ] Camera setup and mounting (high-framerate, fixed angle)
- [ ] Frame capture pipeline (camera → laptop via USB/network)
- [ ] Perspective calibration (map camera frame to flat screen coordinates)
- [ ] Screen state detection (OCR, color detection, UI element tracking)

## Phase 3: Motion Control & Primitives

- [ ] Serial communication with Arduino (USB, baud rate negotiation)
- [ ] G-code generation from high-level commands
- [ ] Gesture primitives: tap, swipe, press, hold
- [ ] Z-axis servo sequencing (lift → move → lower → wait → lift)
- [ ] Coordinate transformation (screen coords → gantry XY)

## Phase 4: Interaction Planning & Logic

- [ ] Perception function library (reusable vision functions)
- [ ] Action sequencing and state management
- [ ] Integration planning (decision trees based on perceived state)
- [ ] Feedback loop (detect success/failure of actions)

## Phase 5: Integration & Refinement

- [ ] End-to-end workflow validation
- [ ] Error handling and recovery mechanisms
- [ ] Calibration workflow refinement
- [ ] Performance optimization (latency, repeatability)
