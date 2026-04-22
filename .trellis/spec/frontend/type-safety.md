# Frontend Type Safety

> Not applicable. The repository has no TypeScript or typed frontend layer today.

---

## Current Reality

Type discipline in the current project comes from C headers, structs, typedefs, and fixed-width integer types, for example:

- `PID_T` in `code/driver/pid/pid_driver.h`
- `uint8_t` and `int16_t` usage in `code/app/imu_app.c`
- shared typedefs from `libraries/zf_common/`

---

## Rule

If a TypeScript or other strongly typed frontend is added later, define its type organization separately. Do not map C header conventions directly onto a future frontend stack.
