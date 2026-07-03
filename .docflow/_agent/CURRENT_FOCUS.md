# Current Focus

This file is the LIVE SNAPSHOT of any in-flight session. It is short on
purpose — the durable record lives in git (`git log`),
`_agent/WORKLOG.md`, and `plan/done/`. The queued work lives in
`plan/todo/`.

If status files and git disagree, git is authoritative; correct this
file.

## Active state

- **Branch:** master
- **Active item:** none — adversarial-analysis queue (wave 2, items
  0001–0006) complete.
- **Blockers:** none.
- **Uncommitted work:** none.

## Last shipped

Wave 2 complete (items 0001–0006): web pointer contract (Pointer
Events + capture), hitch-slice bounding, startup config validation +
hardening, results freeze at outcome, resolution-based accuracy, combo
spec alignment. All ADRs (0001–0006, 0101–0103) Implemented. Every
item gate-verified; item 0001 additionally verified with in-browser
synthetic pointer checks (10/10).

## Next item

`ls .docflow/plan/todo/` for the queue (currently empty). Known future
work: persistence ADR behind the progress.h seam; human balance
playtest (config-only tuning per ADR 0005 AC 4); a manual two-finger
touch check of the web build on a real device would be a nice extra
confirmation of ADR 0103 AC 4.
Known future work: persistence ADR behind the progress.h seam; human
balance playtest (config-only tuning per ADR 0005 AC 4).
