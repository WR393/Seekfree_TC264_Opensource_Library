# Single-Track Bike Balance Bring-Up Spec

## Scenario: bring-up, debug, and tune the bike balance control chain

### 1. Scope / Trigger
- Trigger: any change touching `user/cpu0_main.c`, `user/isr.c`, `code/app/balance_app.c`, `code/app/imu_app.c`, `code/app/motor_app.c`, `code/app/servo_app.*`, or IMU driver interrupt wiring.
- Goal: keep module bring-up, closed-loop balance, and heading hold changes separated so failures can be localized quickly.

### 2. Signatures
- Main init order:
  - `clock_init()`
  - `debug_init()`
  - `pwm_init(ATOM1_CH1_P33_9, 330, mid)`
  - `balance_init()`
  - optional trim calibration / key init
  - `motor_init()`
  - `imu_all_init()`
  - `pit_ms_init(CCU60_CH0, 1)`
  - `pit_ms_init(CCU60_CH1, 2)`
  - `pit_ms_init(CCU61_CH0, 10)`
  - `pit_ms_init(CCU61_CH1, 20)`
- ISR rates:
  - `1ms`: `uwtick++`
  - `2ms`: `imu_proc()` then `balance_gyro_loop()`
  - `10ms`: `balance_angle_loop()`
  - `20ms`: `balance_steering_loop(...)`
- Key globals:
  - `pitch`, `roll`, `yaw`, `gyro_y_rate`
  - `motor_rpm`
  - `target_angle`, `target_gyro_rate`, `servo_output`
  - runtime trim via `balance_set_servo_trim()` / `balance_get_servo_trim()`

### 3. Contracts

#### IMU interrupt chain
- IMU driver EXTI init pin must match the ISR pin exactly.
- Expected chain:
  - IMU hardware INT pin
  - `exti_init(IMU660RC_INT2_PIN, ...)`
  - `ERU_CH3_REQ3_P10_3` ISR
  - `imu660rc_callback()`
  - `imu_proc()`
- If any link differs, IMU may initialize but runtime Euler / gyro outputs can stay zero.

#### Main loop responsibilities
- Main loop may refresh motor speed command continuously if the motor driver requires repeated setpoints.
- Main loop is allowed to gate balance enable until startup conditions are valid.
- Main loop key handling may adjust servo trim at runtime; trim is volatile unless written back to the default macro.

#### Closed-loop staging
- Bring-up must happen in this order:
  1. Servo standalone test
  2. Motor standalone test
  3. IMU standalone log
  4. Balance without heading hold
  5. Weak heading hold after balance is already stable
- Do not tune heading hold before balance is stable on the target surface.

#### Surface-specific tuning
- PID values validated on a smooth floor are not automatically valid on a rubber track.
- Treat a new surface as a retune event because rolling resistance, tire deformation, and steering damping change together.

### 4. Validation & Error Matrix

| Symptom | Likely Cause | First Check | Required Fix Pattern |
|---|---|---|---|
| IMU values are all zero | EXTI pin mismatch or callback not firing | Verify driver `exti_init(...)` pin and ISR pin | Fix pin contract before tuning |
| Servo moves only one side or jitters on power-up | servo trim / center is wrong, or control starts too early | Check `balance_servo_trim`, startup enable timing | Center servo first, then delay control enable |
| Motor worked in test mode but not in balance mode | setpoint refresh missing in main loop | Compare `motor_test()` vs normal path | Keep `motor_set_speed()` refresh in main loop if driver needs it |
| Car can balance only when wheel is manually centered | mechanical center and software trim disagree | Read runtime trim and center PWM | Save trim into default trim macro |
| Car keeps drifting further off heading | heading hold missing or too weak | Check steering loop input | Add weak delayed heading hold |
| Car follows a sinusoidal path | heading hold is too aggressive | Check steering gain / limit / deadband | Reduce heading gain and limit before touching inner balance loops |
| Works on floor, fails on track | tuning margin too small for real surface | Repeat same test on target surface | Retune startup and balance on target surface |
| Logs look contradictory to behavior | copied out of order or code file is formatting-corrupted | Verify chronological timestamps and inspect source lines | Use chronological logs and rewrite corrupted files cleanly |

### 5. Good / Base / Bad Cases

#### Good
- Power on, servo centers without violent swing.
- Motor reaches startup RPM, then balance loop arms.
- On target surface, bike survives launch and balances for multiple seconds.
- Heading hold keeps drift bounded without visible sinusoidal weaving.

#### Base
- Servo, motor, and IMU each pass their standalone tests.
- Runtime trim can be adjusted from board keys and printed over serial.
- Chronological logs show `pitch`, `gyro_y`, `servo`, `rpm`, `target_g` evolving consistently.

#### Bad
- Any tuning is attempted before standalone module checks pass.
- Multiple loops are retuned at once.
- Logs are pasted newest-to-oldest.
- Heading hold is used to mask a weak balance loop.

### 6. Tests Required
- Module smoke tests:
  - servo sweep / center test
  - motor loop / fixed-speed test
  - IMU live output test
- Integration tests:
  - power-on to armed transition
  - hand-held sway log in chronological order
  - launch on target surface
- Assertions to verify:
  - IMU interrupt path produces non-zero live outputs
  - balance enable occurs only after startup conditions
  - runtime trim changes move servo center immediately
  - heading hold correction remains weaker than balance correction

### 7. Wrong vs Correct

#### Wrong
- Enable full balance and heading hold immediately at power-on.
- Assume module tests guarantee the integrated path is correct.
- Tune angle, gyro, steering, startup speed, and trim in the same pass.
- Diagnose IMU zeros as a sensor issue before checking ISR / EXTI wiring.

#### Correct
- Gate control enable until startup timing, RPM, and pitch conditions are valid.
- Verify IMU pin-to-ISR contract before any PID tuning.
- Lock servo center with runtime trim first, then tune gyro loop, then angle loop, then heading hold.
- Re-run balance tuning on the real competition surface.

### Common Mistake: source corruption hides live code

**Symptom**: logic that should run does not run, or unrelated behavior changes after a tiny edit.

**Cause**: damaged formatting / encoding collapses multiple statements onto one line so `//` comments swallow real code.

**Fix**: inspect the exact file lines; if corruption is present, rewrite the touched file cleanly instead of patching around it.

**Prevention**: when symptoms contradict standalone module tests, inspect the real source lines before retuning hardware or PID.

### Design Decision: separate startup balance from cruise heading hold

**Context**: launch instability and later sinusoidal path are different problems.

**Decision**: keep startup arming conditions and heading hold as separate control stages.

**Why**:
- startup needs tolerance and low disturbance
- cruise needs weak directional correction
- mixing both into one always-on steering loop creates spin-at-launch and over-constrained weaving

### Tuning Order
- 1. Mechanical center and software trim
- 2. IMU data validity
- 3. Motor startup reliability
- 4. Gyro inner loop
- 5. Angle loop
- 6. Heading hold
- 7. Surface-specific retune
