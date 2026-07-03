---
adr: 0104
title: Git-derived build version
status: Implemented
date: 2026-07-03
owner: default-agent
supersedes:
superseded-by:
depends-on: ["0007"]
tags: [build, release, tooling]
---

# ADR 0104 — Git-derived build version

## Context

`adr/0007-visible-build-identity.md` requires a version string that
uniquely identifies the built commit on both targets. The repo already
has release tags (v1.0.0, v2.0.0) and a single build entry point
(`build.ps1`); the string must never be hand-maintained, because a
hand-bumped constant drifting from the actual commit is precisely the
failure the capability exists to prevent.

## Decision

The single source of truth is `git describe --tags --always --dirty`,
captured by `build.ps1` at build time and written into a generated,
gitignored `version.h` (`#define GAME_VERSION "..."`) consumed by both
the gcc and emcc compiles. When git is unavailable or the tree is not a
checkout, the generated value is `dev`. `just dist` stamps the same
string into the page footer of `dist/index.html` at assembly time, next
to the credit line. No version constant exists in committed source.

## Rationale

- Hand-maintained version constant in source — rejected: it drifts from
  the actual commit unless a human remembers every bump; a stale
  constant is worse than none because it asserts a false identity.
- Embedding only the raw commit SHA — rejected: not human-meaningful
  and carries no release semantics; `git describe` gives tag distance
  (`v2.0.0-12-gb821f85`) and collapses to the bare tag on releases.
- Runtime-fetched `version.json` beside the bundle — rejected: breaks
  the zero-external-assets posture, adds a fetch failure mode, and can
  disagree with the wasm actually loaded (the exact ambiguity to kill).
- `-DGAME_VERSION=...` compiler flag instead of a generated header —
  rejected: shell-quoting a quoted string through PowerShell into two
  different toolchains is fragile; a generated header is
  toolchain-neutral and diffable when debugging builds.

## Consequences

- Positive: version is correct by construction on every build; tagged
  releases display cleanly (`v2.1.0`); `-dirty` exposes uncommitted
  builds; deploy verification becomes a string comparison.
- Negative: builds outside git show only `dev`; `version.h` is a build
  artifact contributors must not commit (gitignored).
- Follow-up: none — CI builds, if ever added, inherit the mechanism.

## Acceptance criteria

1. `build.ps1` generates `version.h` from
   `git describe --tags --always --dirty` before every compile, for
   both targets.
2. With git unavailable, the build succeeds and `GAME_VERSION` is
   `"dev"`.
3. `version.h` is gitignored; grep finds no hand-written version
   constant in committed source.
4. `just dist` stamps the identical string into the footer of
   `dist/index.html`.

## Out of scope

- Where and how the string is displayed —
  `adr/0007-visible-build-identity.md`.
- Semantic-version policy for choosing tag numbers.

## Open questions

- None.

## References

- adr/0007-visible-build-identity.md

## Revision History

| Date | Revision | Author | Change |
|------|----------|--------|--------|
| 2026-07-03 | r1 | default-agent | Initial draft from approved brainstorm outline; status Accepted. |
| 2026-07-03 | r2 | default-agent | Shipped at 29a92a0 (plan 0001 version-identity); status Implemented. |

## Approvals

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Maintainer | Eugenio Minardi | 2026-07-03 | — |
