# Visible build identity

Owning ADRs: `../adr/0007-visible-build-identity.md`,
`../adr/0104-git-derived-build-version.md`

## Scope

- `build.ps1` generates gitignored `version.h` from
  `git describe --tags --always --dirty` (fallback `dev`) before both
  compiles.
- Title screen displays `GAME_VERSION` in a corner, both targets.
- `just dist` stamps the same string into the page footer.
- Out: any version-gated behavior; CI.

## Exit criteria

- ADR 0007 AC 1–4 and ADR 0104 AC 1–4.
- Verify gate green; self-test menu screenshot shows the version.

## Dependencies

- None.

---
Shipped at HEAD `29a92a0`. Verified: selftest menu screenshot shows the version corner.
