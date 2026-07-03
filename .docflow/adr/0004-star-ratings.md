---
adr: 0004
title: Star ratings
status: Implemented
date: 2026-07-02
owner: default-agent
supersedes:
superseded-by:
depends-on: ["0002", "0003"]
tags: [gameplay, scoring]
---

# ADR 0004 — Star ratings

## Context

Linear unlocks (`adr/0002-saga-progression-structure.md`) give binary
progress. A per-stage quality grade rewards replaying stages and gives
the map a richness signal. The saga convention is 1–3 stars per
completed stage, judged against per-level thresholds.

## Capability statement

Completing a stage awards 1 to 3 stars based on thresholds defined in
that level's config entry, grading either score or accuracy — where
accuracy is defined as vegetables sliced ÷ vegetables resolved
(sliced + missed) during the run, bombs excluded from both counts; a
vegetable still in flight when the outcome is decided affects neither
term. The best star result per
stage is retained for the session and shown both on the level-complete
screen and beside the stage on the world map.

## User stories / scenarios

- As a player, finishing a stage shows me how many stars I earned, so
  that I know how well I did beyond pass/fail.
- As a player, the map shows my best stars per stage, so that I can
  spot stages worth replaying.
- As a player, replaying a stage can only improve my recorded stars,
  never lower them.

## Acceptance criteria

1. Every successful completion awards exactly 1, 2, or 3 stars,
   computed from the level's configured thresholds.
2. The level-complete screen displays the stars earned this run and
   the run's score.
3. The world map displays the best star count achieved per stage
   (0 for completed-never/locked stages is not shown as stars).
4. The recorded best per stage is monotonic: a worse replay never
   lowers it.
5. Star thresholds and the metric they grade live in the level config
   (`adr/0101-data-driven-level-config-table.md`), not in gameplay code.
6. Accuracy counts only resolved vegetables: a vegetable spawned but
   neither sliced nor missed when the outcome is decided appears in
   neither the numerator nor the denominator, so a run with no misses
   grades as 100% accuracy.

## Out of scope

- Star-gated unlocks (e.g. "need 15 stars to enter World 2") — unlocks
  are purely linear per `adr/0002-saga-progression-structure.md`.
- Persistence of stars across sessions —
  `adr/0102-in-memory-progression-module.md`.

## Open questions

- None. (Resolved 2026-07-02: accuracy = sliced ÷ spawned; revised
  2026-07-03 in r4 to sliced ÷ resolved so in-flight vegetables at the
  decision moment do not dock the grade.)

## References / cross-links

- adr/0002-saga-progression-structure.md
- adr/0003-typed-finite-stage-objectives.md
- adr/0101-data-driven-level-config-table.md

## Revision History

| Date | Revision | Author | Change |
|------|----------|--------|--------|
| 2026-07-02 | r1 | default-agent | Initial draft from approved brainstorm outline. |
| 2026-07-02 | r2 | default-agent | Defined accuracy metric (sliced ÷ spawned, bombs excluded); status Accepted. |
| 2026-07-02 | r3 | default-agent | Shipped: stars + screens + map display (3429f46); status Implemented. |
| 2026-07-03 | r4 | default-agent | Accuracy redefined to sliced ÷ resolved (sliced + missed): in-flight vegetables at decision time count in neither term. Returns to Accepted pending re-implementation. |
| 2026-07-03 | r5 | default-agent | Shipped: resolution-based miss counter (plan 0005, 25e0cf3); status Implemented. |

## Approvals

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Maintainer | Eugenio Minardi | 2026-07-02 | — |
| Maintainer | Eugenio Minardi | 2026-07-03 | — |
