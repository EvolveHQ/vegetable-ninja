# Balance pass + self-test coverage

Owning ADRs: `../adr/0005-difficulty-curve-and-boss-stages.md` (final
verification), `../adr/0002-saga-progression-structure.md`

## Scope

- Play-tune the 20-level curve end to end; adjust config values only
  (per ADR 0101 the tuning must not touch gameplay code).
- Extend `--selftest` to drive at least one stage through map → play →
  complete flow and screenshot the map and outcome screens.
- Out: new mechanics of any kind.

## Exit criteria

- ADR 0005 AC 1–3 verified by play; AC 4 verified by the config-only
  diff.
- Verify gate (`just all && just selftest`) green with the extended
  self-test.

## Dependencies

- Items 0001–0005 all shipped.

---
Shipped: curve kept as maintainer-reviewed; extended selftest (map + play + complete screenshots) landed with items 0004/0005; verified by native selftest + live web smoke test (stages 1-2 completed, stage 3 unlocked, stars displayed). Human playtest tuning remains config-only.
