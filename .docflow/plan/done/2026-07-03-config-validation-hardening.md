# Config validation and hardening

Owning ADR: `../adr/0101-data-driven-level-config-table.md`
(also touches `progress.c` under `../adr/0102-in-memory-progression-module.md`,
no spec change there)

## Scope

- Startup validation of `LEVELS[]` per ADR 0101 AC 5: non-empty
  vegetable set, positive time window and spawn interval, wave size
  ≥ 1, ordered star thresholds; violating row stops the program with a
  diagnostic naming the row.
- Defensive guard in the spawn loop so an empty vegetable mask can
  never spin forever regardless of validation.
- Static assert tying `progress.c` `MAX_LEVELS` capacity to the level
  count (compile-time, no behavior change).
- Ride-along hygiene (no owning-ADR change): check `malloc` results in
  sound synthesis; guard the letterbox `scale` against a zero-sized
  window; fix the stale "F3 fps" header comment (key is F).

## Exit criteria

- ADR 0101 AC 5.
- Verify gate green: `just all && just selftest`.

## Dependencies

- None (parallelizable with items 0004–0006).

---
Shipped at HEAD `2f55fb5`. Gate green. ADR 0101 AC 5 implemented via
Levels_FirstInvalid() + fail-fast in main(); spawn-loop guard,
LEVELS_MAX_COUNT static assert, malloc checks, scale guard, and the
F-key comment fix rode along as scoped.
