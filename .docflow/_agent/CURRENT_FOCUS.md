# Current Focus

This file is the LIVE SNAPSHOT of any in-flight session. It is short on
purpose — the durable record lives in git (`git log`),
`_agent/WORKLOG.md`, and `plan/done/`. The queued work lives in
`plan/todo/`.

If status files and git disagree, git is authoritative; correct this
file.

## Active state

- **Branch:** master
- **Active item:** none — saga conversion (items 0001–0006) complete.
- **Blockers:** none.
- **Uncommitted work:** none.

## Last shipped

Saga conversion complete: 20-stage level table, progression module,
objective engine, world map, star ratings, outcome screens. All ADRs
0001–0005, 0101–0102 Implemented. Verified: native selftest (title/
map/play/complete screenshots) + live web smoke test (map unlocks and
stars advancing through real play).

## Next item

`ls .docflow/plan/todo/` for the queue (currently empty). Known future
work: persistence ADR behind the progress.h seam; human balance
playtest (config-only tuning per ADR 0005 AC 4).
