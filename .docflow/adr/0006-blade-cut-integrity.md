---
adr: 0006
title: Blade cut integrity
status: Accepted
date: 2026-07-03
owner: default-agent
supersedes:
superseded-by:
depends-on: []
tags: [gameplay, input, integrity]
---

# ADR 0006 — Blade cut integrity

## Context

Slicing is the whole game, so the blade must cut only what the player
actually swiped through. An adversarial analysis of the shipped input
path found three ways a cut can occur along a path the player never
swiped:

- **Multi-touch phantom slice (web).** All touches share one pointer
  state; a second finger teleports the blade across the screen while
  the button state stays "down", slicing everything on the jump segment
  (including bombs). Alternating two-finger taps clears the screen with
  no swiping at all.
- **Off-window release (web).** Releasing the mouse button outside the
  page is never observed, so the blade stays held; re-entering the
  window jumps the pointer and produces the same phantom cut segment.
- **Hitch-inflated blade speed (both platforms).** The per-frame time
  step is clamped for physics, but blade displacement is real; a frame
  hitch while the button is held divides a large real displacement by
  the clamped step, inflating slow deliberate movement past the cut
  speed gate and slicing the entire hitch-spanning segment.

These are exploit-class defects: they trivialize objectives, detonate
bombs the player avoided, and break the fairness contract behind
objectives (`adr/0003-typed-finite-stage-objectives.md`) and star
grading (`adr/0004-star-ratings.md`).

## Capability statement

A cut happens only along a path the player actually swiped, in real
time. Exactly one pointer drives the blade; discontinuities in pointer
position or in frame timing never manufacture a cut segment the player
did not perform; and the blade is held only while that pointer is
genuinely pressed.

## User stories / scenarios

- As a player on a touch device, resting or tapping a second finger
  never slices anything, so that multi-touch cannot cheat or grief my
  run.
- As a player, dragging off the window and releasing the button ends my
  swipe, so that the blade is not stuck "held" when I return.
- As a player on a slow or stuttering machine, a frame hitch during a
  careful movement never turns it into a screen-wide cut, so that bombs
  I steered around are not detonated by lag.

## Acceptance criteria

1. Exactly one pointer drives the blade at any time; input from any
   additional concurrent pointer (extra touches, secondary buttons) has
   no effect on blade position or held state.
2. A fresh press never slices along the jump from the pointer's
   previous position: the cut segment starts at the press position.
3. A pointer-position discontinuity while held (second touch, window
   re-entry, focus regain) never produces a cut segment spanning the
   jump.
4. The blade-speed cut gate is computed from real elapsed time (not the
   physics-clamped time step), so a frame hitch cannot inflate the
   measured blade speed above the gate for movement that was actually
   slow.
5. Releasing the pointer anywhere — including outside the window or
   canvas — releases the blade within one frame of the release being
   observable, and the blade cannot remain held with no pointer pressed.

## Out of scope

- The concrete web input mechanism (event model, pointer capture) —
  `adr/0103-web-pointer-input-via-pointer-events.md`.
- Multi-blade / genuine multi-touch gameplay as a feature.
- Blade trail rendering aesthetics.

## Open questions

- None.

## References / cross-links

- adr/0003-typed-finite-stage-objectives.md
- adr/0004-star-ratings.md
- adr/0103-web-pointer-input-via-pointer-events.md

## Revision History

| Date | Revision | Author | Change |
|------|----------|--------|--------|
| 2026-07-03 | r1 | default-agent | Initial draft from approved brainstorm outline (adversarial-analysis findings). |
| 2026-07-03 | r2 | default-agent | Status Accepted (no open questions; outline approved by maintainer). |

## Approvals

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Maintainer | Eugenio Minardi | 2026-07-03 | — |
