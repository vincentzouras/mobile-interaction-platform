# Requirements

## Functional

- Stream the phone screen to the host computer in real time
- Derive screen state from camera frames for interaction planning
- Move the stylus to a targeted position on the screen
- Tap and swipe reliably
- Perform sets of actions in sequence
- Send motion commands to the controller and receive status or completion feedback

## Non-Functional

- Low enough latency for interactive use
- Repeatable positioning across the usable screen area
- Stable calibration workflow
- Simple recovery from errors or missed touches
- Modularity between perception, planning, motion control, and actuation

## Constraints

- Must operate on an iPhone SE (2nd generation)
- Must use Arduino Uno R3
