---
adr: 0103
title: Web pointer input via Pointer Events
status: Implemented
date: 2026-07-03
owner: default-agent
supersedes:
superseded-by:
depends-on: ["0006"]
tags: [web, input, shell]
---

# ADR 0103 — Web pointer input via Pointer Events

## Context

On the web build, blade input bypasses raylib's backend: `shell.html`
feeds a `Module.pointer = { x, y, down }` object that `main.c` reads
each frame (the raylib web backend reports mouse coordinates in a stale
coordinate system after its canvas auto-resize, per the comment at the
read site in `main.c`). The current shell implements this with separate
`mousemove`/`mousedown`/`mouseup` and `touchstart`/`touchmove`/
`touchend`/`touchcancel` window listeners that all write the same
shared pointer state.

That shared state is the mechanism behind two exploit-class defects
found by adversarial analysis and specified away by
`adr/0006-blade-cut-integrity.md`: any second touch teleports the
pointer while `down` stays 1 (multi-touch phantom slice, and a
lifted second finger releases a swipe the first finger is still
making), and a mouse released outside the window is never observed
(stuck blade, then a phantom cut segment on re-entry).

## Decision

Replace the shell's mouse/touch listeners with Pointer Events on the
canvas: on `pointerdown` (primary button/contact), call
`setPointerCapture` and record that `pointerId` as the blade pointer;
only events matching the blade `pointerId` update `Module.pointer.x/y`;
`pointerup`/`pointercancel` of that pointer (delivered even off-window
thanks to capture) clears `down` and the tracked id. All other
concurrent pointers are ignored. The `Module.pointer` `{ x, y, down }`
contract with `main.c` is unchanged, and the `blur` → `down = 0`
handler remains as belt-and-braces.

## Rationale

- Current shared-state mouse/touch listeners — rejected: a single
  shared pointer state cannot distinguish concurrent pointers, which is
  exactly the multi-touch phantom-slice and off-window stuck-blade
  defect class (violates ADR 0006 criteria 1, 3, 5); patching them
  would re-implement pointer identity tracking that Pointer Events
  provide natively.
- raylib web backend input (`GetMousePosition`/touch API) — rejected:
  reports coordinates in a stale coordinate system after the canvas
  auto-resize (documented at the `Module.pointer` read in `main.c`),
  and offers no pointer-capture semantics, so off-window release would
  remain unobservable.

## Consequences

- Positive: mouse, touch, and pen unify into one code path with
  per-pointer identity; capture delivers release events from outside
  the window, closing the stuck-blade hole; less shell code than the
  current two listener families.
- Negative: requires Pointer Events support — universal in the target
  browsers (all evergreen browsers since ~2019); `touch-action: none`
  on the canvas remains load-bearing to suppress scroll/zoom gestures.
- Follow-up work implied: the `main.c` fresh-press reset and real-time
  speed gate work is tracked under ADR 0006's plan item, not here.

## Acceptance criteria

1. `shell.html` registers `pointerdown`/`pointermove`/`pointerup`/
   `pointercancel` handlers and no `mouse*`/`touch*` game-input
   listeners.
2. The blade is driven only by events whose `pointerId` matches the one
   captured on `pointerdown`; a second concurrent pointer changes
   neither `Module.pointer.x/y` nor `down` (ADR 0006 criterion 1).
3. With the pointer captured, pressing in-canvas, dragging outside the
   window, and releasing there sets `Module.pointer.down = 0` without
   requiring re-entry (ADR 0006 criterion 5).
4. On a touch device, a two-finger alternating tap slices nothing
   (manual check on the web build; ADR 0006 criterion 3).
5. `Module.pointer` keeps the `{ x, y, down }` shape; `main.c` needs no
   change to its read site for this decision.

## Out of scope

- The platform-neutral cut-integrity rules (fresh-press reset,
  real-time speed gate) — `adr/0006-blade-cut-integrity.md`.
- Desktop (native) input, which uses raylib/GLFW directly and already
  captures the mouse while a button is held.
- Gamepad or keyboard-driven blade input.

## References

- adr/0006-blade-cut-integrity.md
- https://developer.mozilla.org/en-US/docs/Web/API/Pointer_events

## Revision History

| Date | Revision | Author | Change |
|------|----------|--------|--------|
| 2026-07-03 | r1 | default-agent | Initial draft from approved brainstorm outline (adversarial-analysis findings). |
| 2026-07-03 | r2 | default-agent | Status Accepted (no open questions; outline approved by maintainer). |
| 2026-07-03 | r3 | default-agent | Shipped: shell.html Pointer Events rewrite (plan 0001); AC 1–5 verified (gate + in-browser synthetic pointer checks, 10/10 pass); status Implemented. |

## Approvals

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Maintainer | Eugenio Minardi | 2026-07-03 | — |
