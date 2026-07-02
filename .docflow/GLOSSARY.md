# Glossary

Shared terms used across the code, ADRs, and plan items. Keep entries
short; link an ADR when a term is defined by a decision.

| Term | Meaning |
|---|---|
| Vegetable (veg) | A sliceable entity tossed from the bottom of the screen; 8 types, each with procedural art and a juice color. |
| Bomb | A non-sliceable entity; slicing it triggers an explosion and costs a life. |
| Half | One of the two sprite halves a vegetable splits into when sliced; rendered from a half source-rect of the baked texture. |
| Blade / trail | The player's swipe path while the pointer is held down; slicing requires blade speed above the threshold. |
| Splat | A fading juice decal left at the slice point. |
| Combo | 3+ slices within one swipe window; awards bonus score. |
| Wave | A batch of 1–4 entities launched together; spawn cadence ramps with play time. |
| Miss | A vegetable that falls off-screen unsliced; costs a life. |
| Design space | The fixed 1600x900 logical coordinate system all gameplay logic uses; letterboxed onto the actual window/canvas. |
| SSAA target | The 2x supersampled render texture (3200x1800) the scene is drawn into before post-processing. |
| Verify gate | `just all && just selftest` — both targets build and the scripted native self-test produces its screenshots. |
| Self-test | `VegetableNinja.exe --selftest`: a scripted 270-frame session that saves `selftest_menu.png` and `selftest.png`. |
