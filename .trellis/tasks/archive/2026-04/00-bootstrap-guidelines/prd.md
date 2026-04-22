# Bootstrap Guidelines For TC264 Firmware

## Goal

Adapt Trellis development guidelines to this TC264 embedded firmware repository so future AI sessions follow the real project structure, timing constraints, and control/driver patterns instead of generic web defaults.

## What I Already Know

- This repository is an embedded C firmware project. The active source tree is centered on `user/`, `code/`, and `libraries/`.
- The existing Trellis task is `.trellis/tasks/00-bootstrap-guidelines/`, currently `in_progress`.
- The current codebase has no real frontend package. `.trellis/spec/frontend/` is intentionally marked "not applicable" for the present repository state.
- `.trellis/spec/backend/` already contains firmware-oriented guideline drafts for directory structure, state/persistence, error handling, logging, and quality.
- `CLAUDE.md` captures project-specific conventions that should be reflected in Trellis guidance:
  - prefer the unified include entry `zf_common_headfile.h`
  - current vehicle model uses `pitch` semantics and explicitly avoids `roll`
- Real code patterns already visible in the repo:
  - `user/cpu0_main.c` owns startup ordering and the foreground `while (TRUE)` loop
  - `user/isr.c` owns PIT / EXTI / UART / DMA interrupt dispatch
  - `code/app/*.c` owns control orchestration and application-level logic
  - `code/driver/*` owns reusable algorithms and device/protocol wrappers

## Assumptions (Temporary)

- The main remaining work is to verify and refine existing spec files against real code, not invent a new architecture.
- For this repository, Trellis `backend` guidelines are the authoritative coding rules.
- `frontend` stays out of scope unless the repo later gains a host-side UI, telemetry tool, or configuration application.

## Open Questions

- None for current MVP scope.

## Requirements (Evolving)

- Keep guideline content grounded in the current repository layout and real source files.
- Treat firmware code as the active Trellis `backend` layer.
- Keep `frontend` guidelines explicitly marked not applicable until a real frontend codebase exists.
- Preserve project-specific conventions already found in repository instructions and source patterns.
- Verify that spec files match the actual layering used by `user/`, `code/app/`, `code/driver/`, and `libraries/`.
- Limit this bootstrap task to documenting the current firmware repository state only.
- Add explicit hard constraints for the most likely future-AI drift points:
  - prefer the unified include entry `zf_common_headfile.h` for new/updated module headers
  - keep startup, ISR, app, driver, and library responsibilities separated
  - keep high-frequency control loops out of foreground scheduler tasks
  - keep protocol receive buffers, counters, and frame recovery logic inside protocol/driver modules
  - keep `printf()` and heavy text logging out of 1/2/10/20ms ISR paths
  - treat `pitch` as the default balance-control attitude signal; do not introduce new control logic around `roll` unless there is an explicit requirement
- Identify any remaining gaps before closing the bootstrap task.

## Acceptance Criteria (Evolving)

- [ ] This PRD reflects the actual embedded-firmware scope instead of the default Trellis scaffold.
- [ ] The task scope clearly states that `backend` guidelines are authoritative for the current repo.
- [ ] The task scope clearly states that `frontend` guidelines are currently not applicable.
- [ ] Real code examples are identified for startup flow, ISR dispatch, app-layer logic, and driver-layer logic.
- [ ] The task scope clearly states that future host-side tooling is out of scope for this bootstrap pass.
- [ ] The bootstrap scope includes explicit anti-drift constraints for headers, control semantics, ISR logging, scheduler usage, and protocol-state ownership.

## Definition Of Done

- Existing Trellis spec files are reviewed against the current codebase.
- Any gaps or follow-up edits are identified.
- The task scope is explicitly confirmed before more guideline rewriting continues.

## Out Of Scope (Explicit)

- Designing conventions for a nonexistent frontend stack, host-side tuning UI, telemetry dashboard, or configuration tool.
- Refactoring firmware source code as part of this bootstrap task.
- Writing idealized standards that do not match the current repository reality.

## Technical Approach

Use the current task as a documentation-and-validation pass:

1. Inspect existing project instructions and code patterns.
2. Compare them against `.trellis/spec/backend/` and `.trellis/spec/frontend/`.
3. Converge on an MVP scope for this bootstrap task.
4. Add project-specific anti-drift rules where repo reality is clear and future AI drift is likely.
5. Only then continue with spec refinements or task completion.

## Decision (ADR-lite)

**Context**: The default Trellis scaffold assumes both backend and frontend conventions may need bootstrapping. This repository is firmware-only today, but future AI sessions could still drift by applying generic web patterns or by over-generalizing brief repository notes like "avoid roll."

**Decision**: Limit this bootstrap task to current firmware conventions only, and within that scope explicitly harden the rules most likely to drift:
- unified include entry through `zf_common_headfile.h`
- strict layering between startup / ISR / app / driver / library code
- no heavy logging inside high-frequency interrupt paths
- scheduler reserved for deferrable foreground tasks
- protocol parsing state stays with the protocol owner
- balance-control semantics stay `pitch`-centric unless a future task explicitly changes the control model

**Consequences**:
- The guidelines stay aligned with the current repository and are less likely to push future AI sessions toward generic patterns.
- Future host-side tooling remains intentionally undocumented here and will need its own package/spec work when it exists.
- The `roll` rule is captured as a control-path constraint rather than an absolute repository-wide ban, which matches current code reality in `code/app/imu_app.c`.

## Technical Notes

- Current task: `.trellis/tasks/00-bootstrap-guidelines/`
- Existing instruction sources: `AGENTS.md`, `CLAUDE.md`
- Existing guideline indexes:
  - `.trellis/spec/backend/index.md`
  - `.trellis/spec/frontend/index.md`
  - `.trellis/spec/guides/index.md`
- Primary code references:
  - `user/cpu0_main.c`
  - `user/isr.c`
  - `code/app/balance_app.c`
  - `code/app/imu_app.c`
  - `code/app/motor_app.c`
  - `code/app/schedule.c`
  - `code/driver/pid/pid_driver.c`
  - `code/driver/motor/small_driver_uart_control.c`
- Confirmed MVP decision: document current firmware conventions only; defer any future host-side package conventions until such code exists.
- Verified nuance: `roll` is still present in `code/app/imu_app.c` for IMU/telemetry state, while the balance-control path in `code/app/balance_app.c` consumes `pitch`.
