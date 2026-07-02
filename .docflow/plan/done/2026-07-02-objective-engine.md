# Objective engine

Owning ADR: `../adr/0003-typed-finite-stage-objectives.md`

## Scope

- Stage play consumes the active `LevelDef`: spawn rate, veg set, bomb
  frequency, time window from config.
- Track and decide the five objective types (slice-count, survive-time,
  score-target, no-bomb, combo-target); end the stage on
  success/failure.
- HUD shows objective + live progress.
- Out: the complete/failed screens themselves (item 0005) — this item
  may route to placeholder outcomes.

## Exit criteria

- ADR 0003 AC 1–5 (AC 6 lands fully with item 0005).

## Dependencies

- Item 0001 (level config).

---
Shipped at HEAD `e32d304`.
