# Combo spec alignment

Owning ADR: `../adr/0003-typed-finite-stage-objectives.md`

## Scope

- Align the code-side documentation with ADR 0003 AC 8: the combo
  objective comment in `levels.h` ("in one swipe") becomes the adopted
  chain-window definition (each slice within a fixed short window of
  the previous; swipe continuity not required).
- No mechanic change — the shipped rolling-window implementation IS
  the adopted spec.
- Out: combo window length tuning; combo scoring changes.

## Exit criteria

- ADR 0003 AC 8 (comment matches the spec; mechanic already does).
- Verify gate green: `just all && just selftest`.

## Dependencies

- None (parallelizable with items 0003, 0004, 0005).

---
Shipped at HEAD `b471a0f`. Gate green. ADR 0003 AC 8: the mechanic
already matched the adopted spec; the stale "one swipe" comment is
gone. With AC 7 (item 0004), ADR 0003 is fully implemented again.
