# Freeze results at outcome

Owning ADR: `../adr/0003-typed-finite-stage-objectives.md`

## Scope

- From the moment the outcome is decided until the outcome screen is
  shown, recorded results are frozen: score, star-grading inputs,
  lives, and objective state cannot change. Slicing during the grace
  window stays visually alive (juice, halves, sound) but is cosmetic —
  no score, no combo bonuses, no bomb consequences.
- The outcome screen's displayed score must equal the score the stars
  were graded on.
- Out: the length or presentation of the grace window itself.

## Exit criteria

- ADR 0003 AC 7.
- Verify gate green: `just all && just selftest`.

## Dependencies

- None (parallelizable with items 0003, 0005, 0006).

---
Shipped at HEAD `d28c49a`. Gate green; the selftest exercises the
window (outcome decided mid-sweep, slicing continues to the complete
screen). ADR 0003 AC 7 done; the ADR stays Accepted until item 0006
closes AC 8.
