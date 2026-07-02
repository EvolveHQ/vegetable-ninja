---
adr: 0005
title: Difficulty curve and boss stages
status: Accepted
date: 2026-07-02
owner: default-agent
supersedes:
superseded-by:
depends-on: ["0002", "0003"]
tags: [gameplay, balance]
---

# ADR 0005 — Difficulty curve and boss stages

## Context

The endless mode ramps difficulty continuously with elapsed time. In a
level-based structure (`adr/0002-saga-progression-structure.md`) the
ramp must instead be expressed across the stage sequence, so that later
stages are harder and each world has a climax.

## Capability statement

Difficulty rises across each world's stage sequence — via spawn rate,
bomb frequency, and objective targets — and the final stage (stage 10)
of each world is a distinctly harder "boss" stage. World 2 starts
harder than World 1's midpoint and ramps further. The entire curve is
expressed in level config data, not in code.

## User stories / scenarios

- As a player, each stage feels slightly harder than the last, so that
  progress stays challenging without spikes.
- As a player, the last stage of a world is a noticeably tougher,
  climactic challenge.
- As a designer, I tune the whole curve by editing config values only.

## Acceptance criteria

1. Within each world, spawn rate, bomb frequency, and objective targets
   trend upward across stages 1→10 (monotonic within each parameter's
   role, allowing per-stage variety in which parameter carries the
   ramp).
2. Stage 10 of each world is a boss stage: its parameters are a clear
   step up from stage 9, and it is visually distinguished on the world
   map.
3. World 2 Stage 1 is harder than World 1 Stage 5 by config values.
4. The curve is fully data-driven: changing difficulty requires only
   level-config edits (`adr/0101-data-driven-level-config-table.md`).

## Out of scope

- Boss-specific mechanics (unique entities, patterns, boss health) —
  boss stages differ by intensity and objective, not by new mechanics.
- Dynamic difficulty adjustment based on player performance.

## Open questions

- None.

## References / cross-links

- adr/0002-saga-progression-structure.md
- adr/0003-typed-finite-stage-objectives.md
- adr/0101-data-driven-level-config-table.md

## Revision History

| Date | Revision | Author | Change |
|------|----------|--------|--------|
| 2026-07-02 | r1 | default-agent | Initial draft from approved brainstorm outline. |
| 2026-07-02 | r2 | default-agent | Status Accepted (no open questions). |

## Approvals

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Maintainer | Eugenio Minardi | 2026-07-02 | — |
