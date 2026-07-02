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
