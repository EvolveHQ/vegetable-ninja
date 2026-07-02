# World map + screen flow, endless removal

Owning ADR: `../adr/0002-saga-progression-structure.md`

## Scope

- World map screen: 2 worlds × 10 stages, lock states, boss-stage
  marker, stage selection; entry from title screen.
- Remove the endless mode and its menu path.
- Wire unlock rules through the progression module.
- Out: star display on the map (item 0005 adds it once stars exist).

## Exit criteria

- ADR 0002 AC 1–5, 7, 8 (AC 6 completes with item 0005's screens).

## Dependencies

- Items 0001 (level list to display) and 0002 (unlock state).

---
Shipped at HEAD `fe59fa2`. ADR 0002 AC 6 (outcome-screen buttons) completes with the stars item.
