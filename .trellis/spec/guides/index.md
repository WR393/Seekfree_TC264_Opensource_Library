# Thinking Guides

> Purpose: expand your thinking so common blind spots are caught before they become debugging sessions.

---

## Why Thinking Guides?

Most bugs and tech debt come from "didn't think of that", not from lack of coding ability:

- Did not think about layer boundaries -> cross-layer bugs
- Did not think about repeated patterns -> duplication and drift
- Did not think about edge cases -> runtime failures
- Did not think about maintainability -> unreadable fixes

These guides are short checklists that tell you what to consider before writing or changing code.

---

## Available Guides

| Guide | Purpose | When to Use |
|-------|---------|-------------|
| [Code Reuse Thinking Guide](./code-reuse-thinking-guide.md) | Identify patterns and reduce duplication | When you notice repeated patterns |
| [Cross-Layer Thinking Guide](./cross-layer-thinking-guide.md) | Think through data flow and boundary contracts | Features spanning multiple layers |
| [Embedded Control Debugging Guide](./embedded-control-debugging-guide.md) | Debug firmware control chains without mixing hardware, ISR, and PID causes | IMU / motor / servo / ISR / main-loop bring-up and tuning |

---

## Quick Reference: Thinking Triggers

### When to Think About Cross-Layer Issues

- [ ] Feature touches 3+ layers
- [ ] Data format changes between layers
- [ ] Multiple consumers need the same data
- [ ] You are not sure where a responsibility belongs
- [ ] Firmware behavior depends on `main`, ISR, driver pin mapping, and shared globals all matching

Read [Cross-Layer Thinking Guide](./cross-layer-thinking-guide.md)

### When to Think About Code Reuse

- [ ] You are writing code similar to something that already exists
- [ ] The same pattern appears 3+ times
- [ ] You are adding the same field or constant in multiple places
- [ ] You are modifying any constant or config
- [ ] You are creating a new helper or utility

Read [Code Reuse Thinking Guide](./code-reuse-thinking-guide.md)

### When to Think About Embedded Control Debugging

- [ ] Standalone module tests pass, but the integrated vehicle behavior is wrong
- [ ] IMU, motor, servo, startup timing, and ISR behavior changed together
- [ ] PID tuning keeps changing symptoms instead of fixing them
- [ ] Results change between smooth floor and target track surface

Read [Embedded Control Debugging Guide](./embedded-control-debugging-guide.md)

---

## Pre-Modification Rule

Before changing any value, search first.

```bash
rg "value_to_change" .
```

This avoids partial updates and hidden configuration drift.

---

## How to Use This Directory

1. Before coding: skim the relevant guide.
2. During coding: if something feels repetitive or cross-cutting, check the guides.
3. After debugging: add new insights so the same class of issue is less likely next time.

---

## Core Principle

30 minutes of thinking can save 3 hours of debugging.
