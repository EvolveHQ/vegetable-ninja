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
  items 0001–0006); 0001–0004 shipped, 0005 next.
- **Blockers:** none.
- **Uncommitted work:** ship bookkeeping for 0004 (own docs commit).

## Last shipped

Plan 0004 freeze results at outcome (`d28c49a`): post-decision play is
cosmetic; outcome screen score equals graded score. ADR 0003 AC 7 done
(AC 8 pending item 0006).

## Next item

`todo/0005-accuracy-at-resolution` (ADR 0004 AC 6), then 0006.
Known future work: persistence ADR behind the progress.h seam; human
balance playtest (config-only tuning per ADR 0005 AC 4).
