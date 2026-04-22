# Frontend Directory Structure

> Not applicable. No frontend source directories exist in this repository today.

---

## Current Reality

The repository is organized around firmware runtime layers, not UI layers:

- `user/` contains startup and interrupt entry files
- `code/` contains application logic and driver wrappers
- `libraries/` contains shared platform and device code

---

## Rule

If a real frontend is introduced later, do not place it under `user/`, `code/`, or `libraries/`. Add a separate package or clearly isolated tool directory first, then document its actual structure.
