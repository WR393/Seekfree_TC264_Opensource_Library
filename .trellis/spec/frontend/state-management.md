# Frontend State Management

> Not applicable. There is no frontend state store in this repository.

---

## Current Reality

State in this project is firmware runtime state such as:

- `uwtick` in `code/app/schedule.c`
- controller instances in `code/app/balance_app.c`
- sensor values in `code/app/imu_app.c`

That state is governed by the firmware backend guidelines, not by frontend store patterns.

---

## Rule

If a frontend dashboard or tuning application is added later, define local state, shared state, and transport caching rules in a dedicated frontend spec based on the chosen framework.
