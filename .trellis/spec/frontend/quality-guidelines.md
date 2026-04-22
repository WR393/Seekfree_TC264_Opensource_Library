# Frontend Quality Guidelines

> Not applicable. There is no frontend code to review against this document today.

---

## Current Reality

Current review effort should focus on firmware concerns:

- interrupt safety
- control-loop timing
- protocol correctness
- sensor and actuator sign conventions
- safe bounds and reset paths

---

## Rule

If frontend code appears later, create frontend quality checks from the real toolchain, build system, and accessibility requirements of that codebase. Until then, use `.trellis/spec/backend/quality-guidelines.md` for active work.
