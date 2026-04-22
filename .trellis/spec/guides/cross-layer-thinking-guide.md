# Cross-Layer Thinking Guide

> Purpose: think through data flow and boundary contracts before implementing or debugging.

---

## The Problem

Most bugs happen at boundaries, not inside isolated functions.

Common examples:
- API returns format A, consumer expects format B
- One layer changes a contract, another layer keeps the old assumption
- Firmware driver pin mapping, ISR callback, and runtime processing stop matching

---

## Before Implementing Cross-Layer Work

### Step 1: Map the flow

Write the real path end to end.

Examples:

```text
Source -> Transform -> Store -> Retrieve -> Display
```

```text
Sensor pin -> EXTI init -> ISR callback -> global state -> control loop -> actuator
```

For each arrow, ask:
- What is the exact input?
- What is the exact output?
- Who owns validation?
- What would failure look like?

### Step 2: Mark every boundary

| Boundary | Common Issues |
|----------|---------------|
| API -> Service | Missing fields, wrong shape |
| Service -> Database | Format conversion drift |
| Backend -> Frontend | Serialization mismatch |
| Driver -> ISR | Wrong pin, wrong channel, callback not connected |
| ISR -> Main / Control Loop | Stale globals, wrong timing, unexpected ordering |

### Step 3: Define contracts

For each boundary, write:
- exact trigger
- exact input
- exact output
- allowed errors
- fallback behavior

---

## Common Cross-Layer Mistakes

### Mistake 1: Implicit assumptions

Bad:
- assuming a format, unit, or pin mapping without checking

Good:
- writing the exact contract at the boundary

### Mistake 2: Tuning before contract verification

Bad:
- changing PID gains before confirming sensor data is actually flowing through the ISR path

Good:
- verifying the contract first, tuning second

### Mistake 3: Mixed-phase logic

Bad:
- using one always-on control path for startup, launch, and cruise

Good:
- defining separate startup and cruise contracts

---

## Checklist for Cross-Layer Features

Before implementation:
- [ ] Mapped the complete flow
- [ ] Identified all boundaries
- [ ] Defined input / output at each boundary
- [ ] Decided where validation happens

Before tuning firmware behavior:
- [ ] Verified the hardware pin to ISR contract
- [ ] Verified globals update at the expected rate
- [ ] Verified standalone module tests still pass

After implementation:
- [ ] Tested edge and failure cases
- [ ] Verified error handling at each boundary
- [ ] Confirmed data or control signals survive round-trip end to end
