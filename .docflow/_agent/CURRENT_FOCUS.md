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
  items 0001–0006); 0001 shipped, 0002 next.
- **Blockers:** none.
- **Uncommitted work:** ship bookkeeping for 0001 (commits with item
  0002's flow or its own docs commit).

## Last shipped

Plan 0001 web pointer contract (`774fac8`): shell.html Pointer Events
with capture, single-pointerId blade. ADR 0103 Implemented. Verified
by gate + 10/10 in-browser synthetic pointer checks.

## Next item

`todo/0002-hitch-slice-bounding` (ADR 0006 AC 4), then 0003–0006.
Known future work: persistence ADR behind the progress.h seam; human
balance playtest (config-only tuning per ADR 0005 AC 4).
