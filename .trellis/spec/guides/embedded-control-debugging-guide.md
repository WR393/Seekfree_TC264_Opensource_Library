# Embedded Control Debugging Guide

> Purpose: catch the common "module works alone, integrated control fails" mistakes before retuning firmware.

---

## Use This Guide When

- You are changing IMU, motor, servo, ISR, or `main` loop behavior together.
- A module test passed before, but the integrated bike behavior is now contradictory.
- The same control problem keeps reappearing after multiple parameter changes.

---

## Pre-Debug Checklist

- [ ] Verify the exact source lines in touched C files are structurally intact.
- [ ] Map the full chain: init -> ISR -> global state -> control loop -> actuator.
- [ ] Verify sensor interrupt pin, EXTI init, and ISR callback all match.
- [ ] Confirm standalone servo, motor, and IMU tests still pass before retuning PID.
- [ ] Confirm logs are in chronological order before drawing tuning conclusions.

---

## Tuning Checklist

- [ ] Lock servo center first. Do not compensate a bad center with larger PID gains.
- [ ] Tune only one loop at a time: gyro -> angle -> steering / heading.
- [ ] Add heading hold only after basic balance survives on the target surface.
- [ ] Treat startup, launch, and cruise as different phases.
- [ ] Re-test on the real competition surface before accepting a tuning result.

---

## Strong Triggers To Stop Tuning And Inspect Code

- IMU outputs suddenly become zero after unrelated edits.
- Motor worked in test mode but not in integrated mode.
- Servo behavior contradicts the sign logic you expect.
- A tiny patch causes multiple unrelated subsystems to fail at once.

When any of these happen, inspect file integrity and cross-layer contracts before touching gains again.

---

## Logging Rules

- Always keep timestamps.
- Always copy logs oldest to newest.
- Always note the test condition:
  - hand-held sway
  - ground launch
  - smooth floor
  - target track surface

Without those three constraints, PID conclusions are unreliable.
