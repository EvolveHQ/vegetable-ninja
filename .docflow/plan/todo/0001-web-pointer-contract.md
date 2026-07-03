# Web pointer contract

Owning ADRs: `../adr/0006-blade-cut-integrity.md`,
`../adr/0103-web-pointer-input-via-pointer-events.md`

## Scope

- Rewrite `shell.html` game-input handlers as Pointer Events on the
  canvas: `setPointerCapture` on primary `pointerdown`, blade tracked
  by a single `pointerId`, all other concurrent pointers ignored;
  `pointerup`/`pointercancel` of the blade pointer clears `down`.
  Remove the `mouse*`/`touch*` game-input listeners; keep the `blur`
  belt-and-braces handler and the `Module.pointer` `{x, y, down}`
  contract.
- `main.c` press/jump edge handling: the cut segment starts at the
  press position, and a held-pointer position discontinuity never
  yields a cut segment spanning the jump.
- Out: the frame-hitch speed gate (item 0002); native desktop input.

## Exit criteria

- ADR 0103 AC 1–5.
- ADR 0006 AC 1, 2, 3, 5.
- Verify gate green: `just all && just selftest`; manual two-finger
  and off-window-release checks on the web build per ADR 0103 AC 3–4.

## Dependencies

- None (runs first — exploit-class).
