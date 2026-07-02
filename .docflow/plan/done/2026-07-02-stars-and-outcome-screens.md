# Stars + level-complete / level-failed screens

Owning ADR: `../adr/0004-star-ratings.md`

## Scope

- Compute 1–3 stars from the level's thresholds (score or accuracy =
  sliced ÷ spawned, bombs excluded).
- Level-complete screen (stars, score, Retry / Next / Map) and
  level-failed screen (Retry / Map); wire into the stage flow
  (closes ADR 0002 AC 6 and ADR 0003 AC 6).
- Show best stars per stage on the world map.
- Out: balance tuning (item 0006).

## Exit criteria

- ADR 0004 AC 1–5; ADR 0002 AC 6; ADR 0003 AC 6.

## Dependencies

- Items 0003 (objective outcomes) and 0004 (map + flow).

---
Shipped at HEAD `3429f46`.
