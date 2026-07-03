# Accuracy at resolution

Owning ADR: `../adr/0004-star-ratings.md`

## Scope

- Change the accuracy computation to sliced ÷ resolved
  (sliced + missed), bombs excluded from both terms: count a vegetable
  when it resolves (sliced, or fell off screen as a miss), not at
  launch. A vegetable still in flight when the outcome is decided
  affects neither term; a run with no misses grades as 100%.
- Out: star thresholds in the level table (unchanged; they were tuned
  as accuracy fractions and remain valid).

## Exit criteria

- ADR 0004 AC 6 (and AC 1–5 still hold).
- Verify gate green: `just all && just selftest`.

## Dependencies

- None (parallelizable with items 0003, 0004, 0006).

---
Shipped at HEAD `25e0cf3`. Gate green. ADR 0004 AC 6 implemented
(gMissedVeg counter at resolution, frozen after the outcome decision);
AC 1–5 unchanged and still hold.
