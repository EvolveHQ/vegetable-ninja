---
adr: 0001
title: Record architecture decisions as ADRs
status: Implemented
date: 2026-07-02
owner: default-agent
supersedes:
superseded-by:
depends-on: []
tags: [process, conventions]
---

# ADR 0001 — Record architecture decisions as ADRs

## Context

Vegetable Ninja needs its significant decisions to be **discoverable,
traceable, and durable** — not held in chat logs, pull-request threads, or
one person's memory. The lightweight Architecture Decision Record practice
(Michael Nygard, 2011; the markdown-ADR convention) records each decision
as one small, numbered, immutable file stored beside the code, so the
reasons behind the system are part of the repository. This process
decision uses the technology-ADR shape and is grandfathered at `0001`,
ahead of the capability range.

## Decision

This repository is **documentation-led and ADR-driven**: every significant
decision is recorded as a numbered ADR under `adr/`; the catalogue is the
**source of truth** the running system is expected to match; and a status
lifecycle drives a `plan/` work queue. The authoring rules — ADR shapes,
status lifecycle, numbering, audit trail, and git contract — live in
`CONVENTIONS.md`. This ADR records the **decision to adopt the practice**,
not the rules themselves.

## Rationale

- No records at all (status quo) — rejected because decision context
  evaporates; the reasons behind hard-won build constraints (e.g. the
  wasm toolchain pitfalls already documented in the README) were nearly
  lost once and survive only as prose.
- Decisions in a wiki or external tool — rejected because records
  detached from the repo drift from the code and are invisible to
  coding agents working from the checkout.
- Heavyweight RFC process — rejected because a solo project needs
  minutes-per-decision overhead, not review boards.

## Consequences

- Positive: decisions, the work implementing them, and the commits
  shipping them are linked through one stable identifier; agents can
  orient from the catalogue alone.
- Negative: small ceremony cost per decision; INDEX regeneration and
  revision-history discipline must be maintained.
- Follow-up work implied by this decision: backfill ADRs for the
  decisions already embodied in v1.0.0.

## Acceptance criteria

1. Significant decisions are recorded as numbered ADRs under `adr/`,
   following `CONVENTIONS.md`.
2. The catalogue is the source of truth; code is expected to match the
   decisions it records.
3. ADR authoring, the status lifecycle, and the `plan/` queue follow
   `CONVENTIONS.md`.

## Out of scope

- The detailed authoring rules (ADR shapes, lifecycle, numbering, git
  contract) — these live in `CONVENTIONS.md`, not here.

## Open questions

- None.

## References

- ../CONVENTIONS.md (the operative authoring rules)
- Michael Nygard, "Documenting Architecture Decisions" — https://adr.github.io

## Revision History

| Date | Revision | Author | Change |
|------|----------|--------|--------|
| 2026-07-02 | r1 | default-agent | Adopted the documentation-led, ADR-driven method (seeded at bootstrap). |

## Approvals

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Maintainer | Eugenio Minardi | 2026-07-02 | — |
