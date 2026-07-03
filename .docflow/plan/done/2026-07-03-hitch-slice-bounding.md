# Hitch-slice bounding

Owning ADR: `../adr/0006-blade-cut-integrity.md`

## Scope

- Compute the blade-speed cut gate from real elapsed frame time (not
  the physics-clamped step), and/or bound the per-frame cut segment,
  so a frame hitch while the button is held cannot inflate slow
  movement past the gate and slice the hitch-spanning segment.
- Out: pointer identity / capture (item 0001); physics `dt` clamping
  itself (stays as is for simulation stability).

## Exit criteria

- ADR 0006 AC 4.
- Verify gate green: `just all && just selftest`.

## Dependencies

- Item 0001 (same blade-input code block; land sequentially to avoid
  conflicting edits).

---
Shipped at HEAD `0ad1b4f`. Gate green; selftest screenshot confirms
the scripted sweep still slices (real speed clears the gate). ADR 0006
AC 4 done — with item 0001's AC 1/2/3/5, ADR 0006 is fully implemented.
