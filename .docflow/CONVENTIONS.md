# Conventions

## Project

Project name: Vegetable Ninja.

Artefact root: `.docflow/` — `adr/`, `plan/`, `INDEX.md`, and this file
live under this root; `AGENTS.md` and `CLAUDE.md` always stay at the
repository root. Every lifecycle skill resolves paths against this root.

Language: en-US throughout (color, behavior, catalog-style spellings in
prose; existing file names are unaffected).

## ADR Files

ADR filenames use `NNNN-kebab-case-slug.md`, zero-padded to 4 digits,
with contiguous numbering and no reserved gaps within each shape's range.

The number is an **integer**; the four-digit zero-padding is a display
convention only — tools sort ADRs **numerically**, not lexically, so the
catalogue is not capped at `9999` (widen the padding to five digits if you
ever approach it; none do). Related ADRs may be **grouped** under
`domains/<slug>/README.md` without changing their numbers — grouping is a
curated view, not a separate namespace; the contiguous number stays the
single identity.

Each ADR describes one decision. If a decision splits, supersede the
original ADR and create new ADRs rather than expanding scope inside a
single document.

Status lifecycle: `Proposed → Accepted → Implemented →
(Superseded | Deprecated)`.

| Status | Meaning |
|---|---|
| Proposed | Draft. Decision authored but not yet approved. |
| Accepted | Decision approved; implementation authorised. Work item lives in `plan/todo/`. |
| Implemented | Code shipped per the completion event (§Plan Folder). Work item moved to `plan/done/`. ADR is the authoritative spec the running system matches. |
| Superseded | Replaced by another ADR. The successor is named in `superseded-by:` metadata. |
| Deprecated | Was real; the world moved on; no successor. Capability is not being rebuilt. |

Terminal states (Superseded / Deprecated) are reachable from any prior
state.

The first **persisted** status is `Proposed` — there is no separate `Draft`
state and no `brainstorming/`/`drafts/` folder. Work-in-progress lives in
the brainstorm conversation; only an approved decision is written, as a
numbered `Proposed` ADR.

Cross-references link by relative path to `adr/NNNN-*.md`.

## ADR Shapes

Capability ADRs (`0001`–`0099`) describe what the game must do. They
use `adr/0000-template.md` with sections: Context, Capability
statement, User stories / scenarios, Acceptance criteria, Out of
scope, Open questions, References, Revision History, Approvals.

Technology ADRs (`0100` onwards) describe how the game is built.
They use `adr/0100-template.md` with sections: Context, Decision,
Rationale, Consequences, Acceptance criteria, Out of scope, Open
questions, References, Revision History, Approvals.

Technology ADR Rationale must name alternatives considered and give
specific reasons they were rejected. Generic rationale such as
"simpler" or "more idiomatic" is not sufficient.

## ADR Privacy

ADRs are internal artefacts. ADR numbers, ADR titles, and the
existence of the ADR catalogue must never appear in any string the
game emits to players: UI copy, menu/HUD text, error messages, public
documentation, release notes, or store descriptions.

Allowed references:
- Inline code comments tying a non-obvious choice to its ADR
  (`// see .docflow/adr/0042-foo.md`).
- Commit messages and PR descriptions.
- Internal documents: `AGENTS.md`, `INDEX.md`, the `plan/` queue,
  `_agent/` files, internal runbooks.

Rule of thumb: if a non-builder could ever read the string, the ADR
reference comes out. Refer to the behaviour by its product-level name
instead.

## Multi-Agent Rules

A single agent owns this repo. The `_agent/` directory tracks live
state and history; no LOCKS discipline.

## Plan Folder

Pending and shipped work live in `plan/` under the artefact root:

- `plan/todo/NNNN-<slug>.md` — pending work, lower numbers run first.
  Each file names the owning ADR(s), scope, and exit criteria.
- `plan/done/<YYYY-MM-DD>-<slug>.md` — shipped work, chronological. A
  `git mv` from `todo/` to `done/` is the completion event.

The completion event is: **verify gate (`just all && just selftest`)
green + change fast-forwarded onto `master`** (and pushed, once a
remote exists).

When a `plan/todo/` item ships, the file moves to `plan/done/` AND the
owning ADR(s)' `status:` advances from `Accepted` to `Implemented`.
`INDEX.md` is regenerated to match.

## Audit Trail Policy

Every substantive change to an Accepted ADR appends a new row to its
Revision History table.

Editorial changes — typos, formatting, link fixes — are excluded from
Revision History but must be flagged "editorial" in the commit message.

The Approvals table is populated when an ADR transitions to Accepted
and updated on every subsequent substantive revision.

## Git Contract

Commit messages follow Conventional Commits with a mandatory
`Rationale:` footer for any commit that touches an ADR.

- Signed commits: yes (`commit.gpgsign=true` is set in this repo).
- ADR-revision tags `adr-NNNN-rN`: no.
- Co-Authored-By trailer on agent commits: no.

Changes are fast-forwarded onto `master`; no merge commits. The verify
gate runs locally and must pass before the change is considered
shipped. A change is "shipped" when it is on `master` (and pushed,
once a remote exists).
