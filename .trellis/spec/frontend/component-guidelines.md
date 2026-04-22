# Frontend Component Guidelines

> Not applicable. The repository has no frontend component model today.

---

## Current Reality

There are no React, Vue, Qt, LVGL, or other UI component files in the current tree. The closest reusable units are firmware modules such as:

- `code/app/balance_app.c`
- `code/app/motor_app.c`
- `code/driver/pid/pid_driver.c`

Those are firmware modules, not UI components.

---

## Rule

When a real UI exists, write component rules from that codebase's actual framework and naming scheme. Do not reuse firmware module conventions as fake component guidance.
