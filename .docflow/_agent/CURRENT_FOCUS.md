# Current Focus

This file is the LIVE SNAPSHOT of any in-flight session. It is short on
purpose — the durable record lives in git (`git log`),
`_agent/WORKLOG.md`, and `plan/done/`. The queued work lives in
`plan/todo/`.

If status files and git disagree, git is authoritative; correct this
file.

## Active state

- **Branch:** master
- **Active item:** working the adversarial-analysis queue (wave 2,
  items 0001–0006); 0001–0003 shipped, 0004 next.
- **Blockers:** none.
- **Uncommitted work:** ship bookkeeping for 0003 (own docs commit).

## Last shipped

Plan 0003 config validation + hardening (`2f55fb5`): startup table
validation, spawn guard, capacity static assert, malloc/scale guards.
ADR 0101 Implemented.

## Next item

`todo/0004-freeze-results-at-outcome` (ADR 0003 AC 7), then 0005–0006.
Known future work: persistence ADR behind the progress.h seam; human
balance playtest (config-only tuning per ADR 0005 AC 4).
