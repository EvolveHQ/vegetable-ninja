# Level config schema + 20 level definitions

Owning ADRs: `../adr/0101-data-driven-level-config-table.md`,
`../adr/0005-difficulty-curve-and-boss-stages.md`

## Scope

- Define the `LevelDef` schema (C struct + objective/metric enums) in a
  new `levels.h` / `levels.c` module.
- Author all 20 level rows (2 worlds × 10 stages) implementing the
  difficulty curve and boss stages of ADR 0005, mixing the five
  objective types of ADR 0003.
- **The schema and the full 20-level table are presented to the
  maintainer for review before any gameplay code changes.**
- Out: consuming the config in gameplay (item 0003), map display
  (item 0004).

## Exit criteria

- ADR 0101 AC 1–2, 4: one module holds all definitions; schema covers
  id/world/stage/objective+params/spawn/veg set/bomb frequency/time
  window/star thresholds+metric; adding a stage = adding a row.
- ADR 0005 AC 1–4 expressed in the data: ramp within each world, boss
  stage 10s, W2S1 harder than W1S5, curve fully data-driven.
- Maintainer has reviewed and approved the schema + table.

## Dependencies

- None (first item; item 0002 can run in parallel).
