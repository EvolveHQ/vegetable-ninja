---
adr: 0002
title: Saga progression structure
status: Implemented
date: 2026-07-02
owner: default-agent
supersedes:
superseded-by:
depends-on: []
tags: [gameplay, progression, ui-flow]
---

# ADR 0002 — Saga progression structure

## Context

Vegetable Ninja v1.0.0 is a single endless run: menu → play until lives
run out → game over. The product direction is a saga-style, level-based
game where players advance through a map of discrete stages. The
existing slicing mechanics, physics, and procedural assets are reused;
what changes is how a play session is entered, scoped, and exited.

## Capability statement

The game is structured as worlds containing stages — initially 2 worlds
of 10 stages each — presented on a world map screen. Stages unlock
linearly and failure is penalty-free: any unlocked stage can be
retried freely, and failing never re-locks anything. The endless mode
is removed: the saga replaces it, and the entry flow goes directly from
the title screen to the world map.

## User stories / scenarios

- As a player, I see a world map of all stages and which ones I have
  unlocked, so that I understand where I am in the game.
- As a player, completing a stage unlocks the next one, so that I feel
  steady progress.
- As a player, when I fail a stage I can immediately retry it or return
  to the map, so that failure costs nothing but the attempt.
- As a player, I can replay any stage I have already unlocked.

## Acceptance criteria

1. A world map screen shows 2 worlds × 10 stages, each stage visibly
   locked or unlocked.
2. At session start exactly one stage is unlocked: World 1, Stage 1.
3. Completing stage N of a world unlocks stage N+1 of that world.
4. Completing World 1 Stage 10 unlocks World 2 Stage 1.
5. A locked stage cannot be entered from the map.
6. Screen flow: world map → stage play → level-complete screen
   (Retry / Next / Map buttons) on success, or level-failed screen
   (Retry / Map buttons) on failure.
7. Failing a stage changes no unlock state; the failed stage can be
   retried immediately and indefinitely.
8. The endless mode is not reachable from any screen; the title screen
   leads to the world map.

## Out of scope

- Objective types and stage termination rules — `adr/0003-typed-finite-stage-objectives.md`.
- Star ratings and their display — `adr/0004-star-ratings.md`.
- Where unlock state lives and how it persists — `adr/0102-in-memory-progression-module.md`.
- More than 2 worlds / non-linear unlock graphs.

## Open questions

- None. (Resolved 2026-07-02: the endless mode is removed entirely.)

## References / cross-links

- adr/0003-typed-finite-stage-objectives.md
- adr/0004-star-ratings.md
- adr/0102-in-memory-progression-module.md

## Revision History

| Date | Revision | Author | Change |
|------|----------|--------|--------|
| 2026-07-02 | r1 | default-agent | Initial draft from approved brainstorm outline. |
| 2026-07-02 | r2 | default-agent | Resolved endless-mode question (removed); added AC 8; status Accepted. |
| 2026-07-02 | r3 | default-agent | Shipped: map + flow (fe59fa2), outcome-screen buttons (3429f46); status Implemented. |

## Approvals

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Maintainer | Eugenio Minardi | 2026-07-02 | — |
