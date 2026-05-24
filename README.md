# Mobile Interaction Platform

Software automation platform that uses computer vision to plan smartphone interactions and generate G-code commands for GRBL-compatible XY-gantry systems.

## Docs

- [Overview](docs/overview.md)
- [Requirements](docs/requirements.md)

## Hardware

- [Pen plotter XY gantry](https://www.amazon.com/dp/B0D1GK9TV8?ref=ppx_yo2ov_dt_b_fed_asin_title&tag=knoa-20)
  - Runs GRBL 0.9 on Arduino Uno (A4988 driver module)
  - NEMA 42 stepper motors for XY
  - TS90 Servo for pen Z actuation
  - Max speed: 3000mm/min recommended
  - Capacitive mesh tip on thin rod
- Raspberry Pi 4 Model B
- [Arducam OV9782](https://www.amazon.com/dp/B0CLXZ29F9?ref=ppx_yo2ov_dt_b_fed_asin_title&tag=knoa-20)
- [iPhone SE (2nd generation)](https://support.apple.com/en-us/111882)
