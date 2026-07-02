---
adr: 0101
title: Data-driven level config table
status: Implemented
date: 2026-07-02
owner: default-agent
supersedes:
superseded-by:
depends-on: ["0002", "0003", "0004", "0005"]
tags: [architecture, config]
---

# ADR 0101 — Data-driven level config table

## Context

The saga structure needs 20 level definitions covering objective type +
parameters, spawn behavior, vegetable mix, bomb frequency, and star
thresholds (`adr/0002` … `adr/0005`). The user requirement is a single
data-driven config with no level logic hardcoded in gameplay code. The
project has a standing zero-external-assets posture: the native build is
one statically linked exe with no files beside it, and the web build is
a self-contained bundle.

## Decision

All level definitions live in one static, compile-time C table — an
array of `LevelDef` structs in a dedicated module (e.g. `levels.h` /
`levels.c`), one row per stage. The schema covers: id, world, stage
index, display name, objective type + parameters, spawn interval /
ramp, allowed vegetable types (bitmask), bomb frequency, stage time
window where applicable, and 2-star / 3-star thresholds with the metric
they grade. Gameplay code consumes the active `LevelDef` generically;
no `switch` on level id and no per-level functions.

## Rationale

- External JSON file loaded at runtime — rejected because it breaks the
  zero-external-assets constraint on both targets (a file beside the
  exe; an extra fetch on web), adds a parser dependency, and introduces
  a runtime failure mode (missing/malformed file) for data that is
  fully known at compile time.
- Embedded JSON string + runtime parser — rejected because it keeps the
  parser dependency and runtime validation cost while losing the C
  compiler's field-level type checking; an array literal gives the same
  data-driven authoring with errors caught at build time.
- Per-level C functions or switch-on-id logic — rejected because it is
  exactly the hardcoding the requirement forbids: adding a level would
  mean writing code, and the difficulty curve could not be tuned by
  editing data alone.

## Consequences

- Positive: levels are reviewable as one table (the schema + 20 rows
  can be approved as a document before implementation); adding or
  tuning a level is a one-row diff; both build targets get it for free.
- Negative: changing a level requires recompiling (~seconds here); no
  runtime modding without a rebuild.
- Follow-up work implied: author the 20 `LevelDef` rows implementing
  the difficulty curve (`adr/0005`); if external level packs are ever
  wanted, a loader becomes a new ADR that supersedes this one.

## Acceptance criteria

1. One module contains all level definitions; no other compilation unit
   defines or special-cases level content.
2. The schema expresses at minimum: id, world, stage, objective type +
   parameters, spawn rate/ramp, allowed vegetable set, bomb frequency,
   time window, and star thresholds with their metric.
3. Gameplay code reads the active level only through the config struct;
   grep finds no branch on a concrete level id in gameplay logic.
4. Adding a playable new stage requires only appending a row (plus map
   capacity), with no gameplay-code change.

## Out of scope

- Runtime-loaded or downloadable level packs.
- Editor tooling for the table.

## Open questions

- None. (Resolved 2026-07-02: accuracy = vegetables sliced ÷ vegetables
  spawned, bombs excluded — per `adr/0004-star-ratings.md`; the schema
  names `score` and `accuracy` as gradable star metrics.)

## References

- adr/0002-saga-progression-structure.md
- adr/0003-typed-finite-stage-objectives.md
- adr/0004-star-ratings.md
- adr/0005-difficulty-curve-and-boss-stages.md

## Revision History

| Date | Revision | Author | Change |
|------|----------|--------|--------|
| 2026-07-02 | r1 | default-agent | Initial draft from approved brainstorm outline. |
| 2026-07-02 | r2 | default-agent | Resolved accuracy-metric question (sliced ÷ spawned, bombs excluded); status Accepted. |
| 2026-07-02 | r3 | default-agent | Shipped as levels.h/.c (1ab1422) + gameplay consumption (e32d304); status Implemented. |

## Approvals

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Maintainer | Eugenio Minardi | 2026-07-02 | — |
