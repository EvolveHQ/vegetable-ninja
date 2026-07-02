---
adr: 0003
title: Typed finite stage objectives
status: Implemented
date: 2026-07-02
owner: default-agent
supersedes:
superseded-by:
depends-on: ["0002"]
tags: [gameplay, objectives]
---

# ADR 0003 — Typed finite stage objectives

## Context

The endless run has no win condition — play continues until lives run
out. Saga stages (`adr/0002-saga-progression-structure.md`) must instead
be finite runs that end in success or failure against a defined goal, so
that completion can drive unlocks and star ratings.

## Capability statement

Every stage is a finite run judged against exactly one typed objective.
Five objective types exist and are mixed across the level set:
slice X vegetables, survive Y seconds, reach a score target, complete
without hitting a bomb, and achieve a combo of Z. Objective parameters
come from the level config; gameplay code contains no per-level logic.

## User stories / scenarios

- As a player, I see the stage's objective before/at the start of play
  and my live progress toward it, so that I know what I'm aiming for.
- As a player, the stage ends the moment the objective is decided
  (met, or made impossible), so that runs stay short and replayable.
- As a designer, I create a new stage by picking an objective type and
  parameters in the config, without touching gameplay code.

## Acceptance criteria

1. Exactly five objective types are supported: slice-count,
   survive-time, score-target, no-bomb, combo-target.
2. Each stage declares exactly one objective; its parameters (X, Y,
   score, Z, duration) come from the level config only.
3. Every stage terminates in bounded time with an explicit success or
   failure outcome — no endless stages.
4. The HUD shows the active objective and live progress toward it
   (count, timer, score, or combo as appropriate).
5. Failure occurs when the objective becomes unachievable: lives
   exhausted, a bomb is hit in a no-bomb stage, or the stage's time
   window expires without the target being met.
6. Success and failure route to the level-complete / level-failed
   screens defined in `adr/0002-saga-progression-structure.md`.

## Out of scope

- The concrete pairing of secondary termination rules per type (e.g.
  a no-bomb stage's duration) — expressed per level in the config
  schema (`adr/0101-data-driven-level-config-table.md`).
- Star thresholds — `adr/0004-star-ratings.md`.
- Multi-objective stages.

## Open questions

- None.

## References / cross-links

- adr/0002-saga-progression-structure.md
- adr/0101-data-driven-level-config-table.md

## Revision History

| Date | Revision | Author | Change |
|------|----------|--------|--------|
| 2026-07-02 | r1 | default-agent | Initial draft from approved brainstorm outline. |
| 2026-07-02 | r2 | default-agent | Status Accepted (no open questions). |
| 2026-07-02 | r3 | default-agent | Shipped: objective engine (e32d304), outcome routing (3429f46); status Implemented. |

## Approvals

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Maintainer | Eugenio Minardi | 2026-07-02 | — |
