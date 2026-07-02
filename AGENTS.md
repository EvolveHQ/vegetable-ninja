# AGENTS.md

This file provides guidance to coding agents working in this repository.

## What this repository is

Vegetable Ninja — a Fruit Ninja-style slicing game with vegetables. A
single `main.c` (C, raylib 5.5) builds two targets: a native Windows exe
(MinGW gcc, statically linked) and a WebAssembly web build (emcc, raylib
compiled from source). All assets are procedural: vegetable art baked from
shape primitives, synthesized sound effects, GLSL shaders, embedded font.
Baseline: v1.0.0. Decision records and the work queue live under
`.docflow/` (the artefact root).

## Repository structure

- `.docflow/adr/0000-template.md` — capability-ADR template.
- `.docflow/adr/0100-template.md` — technology-ADR template
  (technology ADRs number from 0100).
- `.docflow/adr/NNNN-<kebab-slug>.md` — one ADR per decision, contiguous
  numbering, no gaps.
- `.docflow/INDEX.md` — table regenerated from every ADR's metadata block.
- `.docflow/CONVENTIONS.md` — authoring rules (read before editing anything).
- `.docflow/GLOSSARY.md` — shared terms.
- `.docflow/plan/todo/NNNN-<slug>.md` — pending work, lower numbers run first.
- `.docflow/plan/done/<YYYY-MM-DD>-<slug>.md` — shipped work, chronological.
- `.docflow/_agent/` — coordination: `ROLES.md`, `WORKLOG.md`,
  `CURRENT_FOCUS.md`, `HANDOFF.md`, `prompts/`.

## Hard rules when editing ADRs

These come from `.docflow/CONVENTIONS.md` and override default behaviour:

- **One decision per ADR.** Splits become new ADRs that supersede;
  never expand scope inside an existing one.
- **Status lifecycle:** `Proposed → Accepted → Implemented →
  (Superseded | Deprecated)`.
- **Capability ADR section order:** metadata → Context → Capability
  statement → User stories / scenarios → Acceptance criteria → Out of
  scope → Open questions → References → Revision History → Approvals.
- **Technology ADR section order:** metadata → Context → Decision →
  Rationale → Consequences → Acceptance criteria → Out of scope → Open
  questions → References → Revision History → Approvals. Rationale must
  name alternatives considered with specific rejection reasons.
- **Acceptance criteria are testable and numbered.**
- **ADRs are internal artefacts — never user-visible.** ADR numbers,
  ADR titles, and the existence of the ADR catalogue must NEVER appear
  in any string the game emits to players: UI copy, menu text, error
  messages, public documentation, release notes, or store descriptions.
  The catalogue is a builder's tool, not a user-facing surface.
  References ARE allowed in: code comments
  (`// see .docflow/adr/0042-foo.md`), commit messages, PR descriptions,
  internal docs, `AGENTS.md`, `CONVENTIONS.md`, `INDEX.md`, and the
  `plan/` queue. Rule of thumb: if a non-builder could ever see the
  string, the ADR reference comes out.

## Implementation work

- Start from the ADRs. Identify which ADRs a code change implements or
  affects before changing behaviour.
- If implementation reveals a capability gap or changed decision, update
  the relevant ADR rather than silently diverging.
- Add or update tests for implemented behaviour. Map tests back to ADR
  acceptance criteria where practical. The scripted self-test
  (`VegetableNinja.exe --selftest`) is the current automated check.
- **Do not leak ADR identifiers into user-visible surfaces.** When
  writing menu text, HUD strings, or public docs, refer to behaviour by
  its product-level name — never by ADR number or title. The ADR link
  belongs in the commit message and (optionally) an inline code comment.

## Audit trail and revision discipline

- Substantive ADR changes append a row to the Revision History table.
  Editorial changes (typos, formatting, link fixes) are excluded but
  flagged `editorial` in the commit message.
- Approvals table populates when an ADR is Accepted and updates on each
  later substantive revision.
- Regenerate `.docflow/INDEX.md` from ADR metadata after any ADR status
  change or new ADR.

## Multi-agent workflow

A single agent owns this repo. The `.docflow/_agent/` directory tracks
live state and history; LOCKS discipline is not in use.

## Plan folder

- A pending item gets a `.docflow/plan/todo/NNNN-<slug>.md` file BEFORE
  work starts, naming the owning ADR(s), scope, and exit criteria.
- The completion event is: **verify gate green + change fast-forwarded
  onto `master`** (and pushed, once a remote exists). On completion,
  `git mv` the file to `.docflow/plan/done/<YYYY-MM-DD>-<slug>.md` with
  a footer naming the HEAD SHA.
- The owning ADR(s) advance `Accepted → Implemented` on the same
  commit. Regenerate `.docflow/INDEX.md`.

## Git contract

- Commit messages follow **Conventional Commits**.
- Mandatory `Rationale:` footer on any commit touching an ADR.
- Signed commits: yes (repo has `commit.gpgsign=true`).
- ADR-revision tags `adr-NNNN-rN`: no.
- Co-Authored-By trailer: no.
- Cross-references between ADRs use relative paths (`adr/NNNN-*.md`
  from within `.docflow/`).
- **Integration:** direct-to-main, **fast-forward only**. No merge
  commits on `master`. The verify gate (`just all && just selftest`)
  runs locally and must pass before the change is considered shipped.
