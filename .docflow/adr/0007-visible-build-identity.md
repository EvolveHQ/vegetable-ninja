---
adr: 0007
title: Visible build identity
status: Implemented
date: 2026-07-03
owner: default-agent
supersedes:
superseded-by:
depends-on: ["0002"]
tags: [ui, operations, release]
---

# ADR 0007 — Visible build identity

## Context

Verifying which build is live currently means comparing artifact byte
sizes over HTTP against the local build — the deployed game carries no
identity a human can read. With a public site (vegetable.ninja) and a
native exe shipping from the same source, "is this the proper version?"
must be answerable at a glance, without tooling.

## Capability statement

The running game displays its build identity as a small, unobtrusive
version string in a corner of the title screen, on both targets. The
web page's footer credit line carries the same string. Opening the site
or the exe and glancing at the corner answers which build it is; the
string uniquely identifies the built commit.

## User stories / scenarios

- As the operator, after publishing I open the site, read the corner of
  the title screen, and compare it with `git describe` locally — match
  means the deploy landed.
- As a player reporting a bug, I can quote the version string so the
  report pins to an exact build.

## Acceptance criteria

1. The title screen shows the build identity in a corner, legible but
   visually subordinate to the menu content, on both native and web.
2. The displayed string equals the build identity embedded at compile
   time (`adr/0104-git-derived-build-version.md`) and uniquely
   identifies the built commit, including a dirty-tree marker.
3. The web page footer shows the same string as the in-game display.
4. A build made outside a git checkout displays `dev` rather than
   failing or showing an empty corner.

## Out of scope

- How the string is derived and embedded —
  `adr/0104-git-derived-build-version.md`.
- Update checks, changelogs, or version-gated behavior.

## Open questions

- None.

## References / cross-links

- adr/0002-saga-progression-structure.md
- adr/0104-git-derived-build-version.md

## Revision History

| Date | Revision | Author | Change |
|------|----------|--------|--------|
| 2026-07-03 | r1 | default-agent | Initial draft from approved brainstorm outline; status Accepted. |
| 2026-07-03 | r2 | default-agent | Shipped at 29a92a0 (plan 0001 version-identity); status Implemented. |

## Approvals

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Maintainer | Eugenio Minardi | 2026-07-03 | — |
