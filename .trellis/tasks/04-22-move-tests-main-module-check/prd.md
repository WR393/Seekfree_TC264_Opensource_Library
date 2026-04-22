# Move Tests To Main For Module Check

## Goal
Temporarily run module test routines from `core0_main()` so board-level module checks can happen before enabling balance control at fixed speed.

## Requirements
- Move existing module test execution from the scheduler path into `user/cpu0_main.c`
- Comment out balance-control startup/runtime logic during this test phase
- Keep changes scoped to startup/runtime flow without rewriting module implementations

## Acceptance Criteria
- [ ] `core0_main()` directly runs module test routines in its main loop
- [ ] Balance-control PIT callbacks are commented out
- [ ] Scheduler/PIT startup calls for the previous flow are disabled

## Technical Notes
- Keep IMU data refresh in the main loop so `imu_test()` still prints updated attitude data
- Preserve peripheral initialization for PWM, motor, and IMU modules
