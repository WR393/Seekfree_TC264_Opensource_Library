# Frontend Hook Guidelines

> Not applicable. The repository has no hook-based frontend framework today.

---

## Current Reality

No source files use React hooks, composition hooks, or similar frontend lifecycle helpers. Runtime scheduling is currently done through:

- fixed-rate interrupts in `user/isr.c`
- cooperative tasks in `code/app/schedule.c`

These are timing mechanisms, not frontend hooks.

---

## Rule

If a hook-based frontend is added later, define hook naming, side-effect boundaries, and data-fetch rules only after real hook code exists.
