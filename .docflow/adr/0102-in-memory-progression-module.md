---
adr: 0102
title: In-memory progression module
status: Implemented
date: 2026-07-02
owner: default-agent
supersedes:
superseded-by:
depends-on: ["0002", "0004"]
tags: [architecture, state]
---

# ADR 0102 — In-memory progression module

## Context

Unlock state (`adr/0002-saga-progression-structure.md`) and best stars
(`adr/0004-star-ratings.md`) need a home. The user has explicitly
deferred persistence: progression must live in memory for the current
session only, with saving added later behind a stable seam — and no
storage API (file, localStorage, or otherwise) may be used for
progression now.

## Decision

Progression state is isolated in one module (e.g. `progress.h` /
`progress.c`) that owns unlock flags and best-star records, exposed
exclusively through a narrow interface: `getProgress()` (read-only
snapshot), `completeLevel(id, stars)` (records completion, applies
monotonic best-star update, computes unlocks), and `isUnlocked(id)`.
Storage is a static in-memory structure initialized at launch. No
storage API is called for progression; the module's interface is the
seam where a persistence backend gets swapped in later without touching
callers.

## Rationale

- Persist now via the existing high-score mechanism (file on native,
  localStorage on web) — rejected because the user explicitly forbade
  storage APIs for progression at this stage; building it now
  pre-empts the later persistence design instead of leaving a clean
  seam.
- Progression state as globals inside `main.c` alongside existing game
  state — rejected because scattered globals leave no single interface
  to put persistence behind later; every reader/writer would need
  finding and rewiring, which is precisely what the requirement's
  "isolate in one module" guards against.
- Full save system designed now but feature-flagged off — rejected as
  scope creep: it front-loads design decisions (format, versioning,
  per-platform backends) the user has deliberately deferred.

## Consequences

- Positive: persistence later becomes an internal change to one module;
  callers (map, complete screen, stage gate) are untouched. The module
  is unit-testable without any I/O.
- Negative: all progression is lost when the process exits — accepted
  and intended for this phase.
- Follow-up work implied: a future persistence ADR choosing format and
  per-platform backend behind this interface.

## Acceptance criteria

1. One module owns all progression state; it exposes `getProgress`,
   `completeLevel(id, stars)`, and `isUnlocked(id)` (naming adapted to
   C convention is acceptable, arity and semantics are not).
2. All other code reads/writes progression only through that interface
   — no direct access to its state from outside the module.
3. No storage API (fopen, localStorage, or any I/O) is invoked for
   progression state anywhere in the codebase.
3a. The legacy endless high-score persistence (file read/write on
   native, localStorage on web) is removed along with the endless mode.
4. Restarting the game yields a fresh progression state (only World 1
   Stage 1 unlocked, no stars).
5. `completeLevel` enforces the monotonic best-star rule and the unlock
   rules of `adr/0002-saga-progression-structure.md`.

## Out of scope

- The eventual persistence format, backend, and migration story.
- Multiple player profiles.

## Open questions

- None. (Resolved 2026-07-02: the endless mode is removed per
  `adr/0002-saga-progression-structure.md`, and the legacy high-score
  persistence — `vegninja_hiscore.txt` on native, localStorage on web —
  is removed with it. The codebase carries no persistence code until
  the future persistence ADR.)

## References

- adr/0002-saga-progression-structure.md
- adr/0004-star-ratings.md

## Revision History

| Date | Revision | Author | Change |
|------|----------|--------|--------|
| 2026-07-02 | r1 | default-agent | Initial draft from approved brainstorm outline. |
| 2026-07-02 | r2 | default-agent | Resolved hiscore-persistence question (legacy persistence removed); added AC 3a; status Accepted. |
| 2026-07-02 | r3 | default-agent | Shipped as progress.h/.c at 1ab1422; status Implemented. |

## Approvals

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Maintainer | Eugenio Minardi | 2026-07-02 | — |
