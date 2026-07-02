# In-memory progression module

Owning ADR: `../adr/0102-in-memory-progression-module.md`

## Scope

- New `progress.h` / `progress.c` module owning unlock flags and best
  stars, exposed via `Progress_Get` / `Progress_CompleteLevel(id,
  stars)` / `Progress_IsUnlocked(id)` (C naming of the agreed
  interface).
- In-memory only; no storage API calls.
- Remove the legacy endless high-score persistence (file on native,
  localStorage on web) — the load/save helpers and their call sites.
- Out: UI that displays progression (item 0004).

## Exit criteria

- ADR 0102 AC 1–5, incl. 3a (legacy persistence removed) and the
  fresh-state-on-restart check.

## Dependencies

- None (parallel with item 0001).

---
Shipped at HEAD `1ab1422`.
