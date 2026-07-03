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
  items 0001–0006); 0001–0002 shipped, 0003 next.
- **Blockers:** none.
- **Uncommitted work:** ship bookkeeping for 0002 (own docs commit).

## Last shipped

Plan 0002 hitch-slice bounding (`0ad1b4f`): blade speed gate computed
from real frame time. ADRs 0006 + 0103 Implemented — blade cut
integrity fully shipped.

## Next item

`todo/0003-config-validation-hardening` (ADR 0101 AC 5), then
0004–0006.
Known future work: persistence ADR behind the progress.h seam; human
balance playtest (config-only tuning per ADR 0005 AC 4).
