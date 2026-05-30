# Version History — DungeonCraft Pro

This document follows [Semantic Versioning](https://semver.org/) (MAJOR.MINOR.PATCH).

---

## v1.0.0 — 2026-05-30

**First stable release.**

### New Features

- **Procedural dungeon generation** — Tile-matrix based room and corridor generation with configurable grid size, room counts, and min/max room dimensions.
- **Deterministic seed system** — `bUseFixedSeed` + `FixedSeed` for reproducible layouts. Seed is replicated to all multiplayer clients.
- **Static mesh props** — `FStaticMeshSpawnParams` array with per-entry spawn chance, offsets, rotation, scale, and room-only restriction.
- **Blueprint actor spawning** — `FBlueprintSpawnParams` array with full transform control and room-only restriction.
- **Floor decals** — `FDecalSpawnParams` with decal size, Z offset, and random yaw rotation.
- **Pillar system** — Corner-detection and pillar mesh placement via `FPillarSpawnParams`.
- **Ceiling tiles** — Optional ceiling mesh above every floor tile at configurable height.
- **Room theme system** — `URoomThemeDataAsset` with per-theme floor/wall/ceiling overrides, props, and grid-based pattern placement.
- **Data asset config** — `UDungeonConfigDataAsset` for data-driven configuration without code changes.
- **Data table support** — `FRoomTemplate` row type for per-room-type mesh variation via `UDataTable`.
- **Multiplayer replication** — `DungeonSeed` replication with `OnRep_DungeonSeed` client-side auto-generation.
- **`OnDungeonSpawned` delegate** — Broadcast after all spawning completes.
- **`ClearDungeon()`** — Destroys all dungeon actors by tag with name-based fallback.
- **`RunGenerationCycles()`** — Stress test utility for memory leak detection and timing benchmarks.
- **Comprehensive logging** — All operations use the `DungeonGenerator` log category with Error/Warning/Log/Verbose levels.
- **Config validation** — Pre-generation validation of all parameters with actionable error messages.
- **Null pointer guards** — All spawn functions protected against null mesh/class references.
- **Debug visualization** — `bDebugActive` draws floor, wall, and corner spawn point boxes in the editor.

### Documentation

- README.md — Project overview and installation
- QUICKSTART.md — Under-10-minute setup guide
- API.md — Complete public API reference
- CONFIGURATION.md — All configuration options with ranges and defaults
- TROUBLESHOOTING.md — Common issues, diagnosis, and solutions
- PERFORMANCE.md — Profiling, tuning, and scaling guidelines
- MULTIPLAYER.md — Multiplayer setup, replication, and best practices
- THEMES.md — Theme system architecture and custom theme creation
- CONTRIBUTING.md — Contribution guidelines

### Known Limitations

| Limitation | Workaround |
|---|---|
| Generation runs synchronously on the game thread | For very large dungeons, keep grid ≤ 80×80 to avoid frame stalls. Async support planned. |
| Floor/wall/ceiling actors are not replicated | By design — each client generates from the same seed. Blueprint actors must be set to replicate individually. |
| No World Partition integration | Not tested. Tag-based cleanup may behave unexpectedly with streaming. |
| No Instanced Static Mesh Components (ISMCs) | Each tile spawns an individual `AStaticMeshActor`. For 500+ tile dungeons, this may impact draw calls. ISMC support planned. |
| No incremental/chunk-based generation | The entire dungeon is generated in one call. |
| Console platform support unverified | Tested on Windows. Console submissions require platform-specific validation. |
| Room connectivity algorithm is nearest-neighbor only | Some room configurations may leave isolated rooms in very sparse grids. Reduce grid or increase room count. |

---

## Future Roadmap

### v1.1.0 (Planned)
- [ ] Instanced Static Mesh Component (ISMC) tile spawning for improved performance
- [ ] Asynchronous generation (non-blocking generation on background thread)
- [ ] Late-join multiplayer recovery (re-send seed on new client connection)
- [ ] Per-corridor theme support

### v1.2.0 (Planned)
- [ ] World Partition integration for open-world dungeon streaming
- [ ] Minimum spanning tree corridor algorithm option (fewer dead-end corridors)
- [ ] Entrance/exit room designation system
- [ ] Custom room shape support (L-shaped, T-shaped rooms)

### v2.0.0 (Future)
- [ ] Full 3D dungeon generation (multi-floor)
- [ ] Navigation mesh auto-generation integration
- [ ] Prefab room injection (hand-crafted rooms mixed into procedural generation)

---

## Migration Guide

### From Pre-Release to v1.0.0

If you were using pre-release builds from the Phase 1–3 development branches:

1. **Tag system:** All Blueprint actor spawns now require the `DungeonBP` tag. Without it, `ClearDungeon()` will not clean up your actors.
2. **Validation:** `ValidateConfig()` now runs before generation. Previously invalid configs (e.g., `MinRoomSize >= MaxRoomSize`) that happened to work may now block generation. Fix the values to proceed.
3. **Log category:** All `LogTemp` calls have been replaced with the `DungeonGenerator` category. Update any log filters in your project.

---

## Contributors

- **ordocaelum** — Project owner and lead developer
- **GitHub Copilot** — Phase 1–4 implementation assistance
