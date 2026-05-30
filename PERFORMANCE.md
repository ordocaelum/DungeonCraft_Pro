# Performance Tuning Guide — DungeonCraft Pro

This guide provides strategies for optimizing dungeon generation performance, memory efficiency, and runtime frame cost across different target platforms.

---

## Performance Budget Overview

DungeonCraft Pro generation has three cost phases:

| Phase | Cost Driver | Typical Time (medium config) |
|---|---|---|
| **1. Grid Initialization & Room Placement** | Grid size × room attempts | 5–30 ms |
| **2. Corridor Generation** | Room count × grid traversal | 1–10 ms |
| **3. Mesh & Actor Spawning** | Total tile count × spawn entries | 20–200 ms |

For a medium dungeon (40×40 grid, 10 rooms) on a modern desktop CPU, total generation takes approximately **30–80 ms**.

---

## Generation Speed Optimization

### Grid Size

Grid size has the largest impact on room-placement time:

| Grid | Approximate tile count | Room placement (10 rooms) |
|---|---|---|
| 20×20 | 400 | < 5 ms |
| 40×40 | 1,600 | 10–30 ms |
| 80×80 | 6,400 | 60–200 ms |
| 120×120 | 14,400 | 200–800 ms |

**Recommendation:** Use the smallest grid that achieves your desired dungeon density. A 40×40 grid with 10 rooms produces good results for most games.

### `MaxRandomAttemptsPerRoom`

This value caps how many times the generator retries placing each room. Default is 1500.

| Grid size | Recommended value |
|---|---|
| ≤ 30×30 | 200–500 |
| 30–60×60 | 500–1500 |
| > 60×60 | 1000–3000 |

Lower values speed up generation but may result in fewer rooms than `RoomsToGenerate`.

---

## Spawn Count Optimization

The spawning phase scales with:
`TotalSpawnEvaluations = TotalFloorTiles × NumberOfSpawnEntries`

### Static Mesh Props

Each entry in `StaticMeshesToSpawn` is evaluated per floor tile. 10 entries on a 1000-tile dungeon = 10,000 evaluations.

**Recommendations:**
- Keep `StaticMeshesToSpawn` to 5–10 entries.
- Use low `SpawnChance` values (0.02–0.10) to keep actual spawned mesh count manageable.
- Use `bOnlySpawnInRooms = true` for large meshes to reduce total instances.

### Blueprint Actors

Blueprint spawning is more expensive than static mesh spawning due to UObject creation overhead.

**Recommendations:**
- Limit `BlueprintActorsToSpawn` to 3–5 entries.
- Use `SpawnChance` of 0.02–0.05 for actors.
- Consider using static mesh stand-ins for purely decorative actors.
- For AI enemies, delay spawning until the player enters a room (use overlap volumes).

### Floor Decals

Decals have minimal generation cost but can impact render performance.

**Recommendations:**
- Keep `SpawnChance` at 0.10–0.25 for decals.
- Set a reasonable `DecalSize` — very large decals cause overdraw.
- Consider disabling decals on low-end platforms via a quality scalability setting.

---

## Memory Optimization

### Tile Matrix

Memory per dungeon grid: `TileMapRows × TileMapColumns × ~24 bytes`

| Grid | Memory |
|---|---|
| 40×40 | ~38 KB |
| 80×80 | ~153 KB |
| 120×120 | ~345 KB |

### Spawned Actors

Each `AStaticMeshActor` has a base overhead of roughly 1–2 KB. A medium dungeon with 400 floor tiles, 300 walls, and 100 props = ~800 actors = ~1–2 MB actor overhead.

**Reducing instance count:**
- Use `bOnlySpawnInRooms = true` to halve prop instances on dungeons with many corridors.
- For floor and wall tiles, consider using **Instanced Static Mesh Components (ISMCs)** for future optimization (planned feature).

### Memory Leak Detection

Use the built-in stress test:

```
// Details panel → CallInEditor button
RunGenerationCycles(10)
```

Output log shows memory delta per cycle. Non-zero growing deltas indicate a leak:

```
DungeonGenerator: Cycle 5/10 completed in 45ms. Memory delta: +0 MB  ← OK
DungeonGenerator: Cycle 5/10 completed in 45ms. Memory delta: +2 MB  ← LEAK
```

If you detect a leak, check that:
1. All Blueprint actor spawns have the `DungeonBP` tag.
2. No external systems are holding references to destroyed actors.

---

## Profiling Instructions

### Quick Profile with `stat unit`

1. Start PIE.
2. Open the console (backtick key).
3. Type `stat unit`.
4. Call `GenerateDungeon()` from Blueprint.
5. Observe the frame time spike. A 50 ms generation stall shows as a single tall bar.

### Unreal Insights

For deep profiling:

1. Launch `UnrealInsights.exe` from the UE5 engine binary directory.
2. Start PIE with `-trace=cpu,gpu,frame` launch argument.
3. Stop trace after calling `GenerateDungeon()`.
4. Open the trace in Insights and filter for `DungeonGenerator` named events.

Key events to look for:
- `ADungeonGenerator::GenerateDungeon`
- `ADungeonGenerator::GenerateRooms`
- `ADungeonGenerator::SpawnStaticMeshes`
- `ADungeonGenerator::SpawnBlueprintActors`

---

## Platform-Specific Recommendations

### PC (High End)
```
TileMapRows/Columns: 60–80
RoomsToGenerate: 15–25
SpawnChance (props): 0.05–0.10
```

### PC (Low End / Laptop)
```
TileMapRows/Columns: 30–40
RoomsToGenerate: 8–12
SpawnChance (props): 0.02–0.05
MaxSpecialRooms: 1
```

### Console (Xbox Series / PS5)
```
TileMapRows/Columns: 50–60
RoomsToGenerate: 12–18
SpawnChance (props): 0.04–0.08
```

### Mobile (Unverified — not officially supported)
```
TileMapRows/Columns: 20–25
RoomsToGenerate: 4–6
SpawnChance (props): 0.01–0.03
Disable decals entirely
```

---

## Asynchronous Generation (Advanced)

The current implementation generates synchronously on the game thread. For large dungeons, consider offloading generation to a background task:

1. Move `InitTileMatrix()`, `GenerateRooms()`, and `ConnectRooms()` to an `AsyncTask` or `FRunnable`.
2. Return to the game thread for all spawning calls (which require the game thread).
3. Use a delegate or `FEvent` to signal completion.

This approach is not provided out of the box but is architecturally feasible given the separation between the matrix-generation phase and the spawning phase.

---

## Scaling Guidelines

| Target Metric | Recommended Configuration |
|---|---|
| < 100 actors total | 20×20 grid, 4 rooms, SpawnChance 0.05 |
| 100–500 actors | 40×40 grid, 10 rooms, SpawnChance 0.05 |
| 500–1000 actors | 60×60 grid, 15 rooms, SpawnChance 0.03 |
| > 1000 actors | Profile carefully; consider LODs and culling |

---

## Asset Optimization Recommendations

- **Floor/Wall Meshes:** Use meshes with ≤ 500 triangles. The tile mesh is instanced hundreds of times.
- **LODs:** Add LOD levels to all dungeon meshes. Use the built-in LOD generation tool in the Static Mesh Editor.
- **Nanite:** Enable Nanite on floor, wall, and ceiling meshes for modern platforms — it nearly eliminates mesh draw-call cost.
- **Collision:** Disable per-poly collision on dungeon meshes; use simple box/capsule collision instead.
- **Materials:** Share a single material instance across floor/wall/ceiling where possible to reduce draw calls.

---

## Multiplayer Performance

In multiplayer, generation runs on the server only. Clients receive the seed via replication and independently generate the same dungeon. This means:

- **No network bandwidth** is used for mesh data — only the seed integer is replicated.
- **Client generation time** is identical to server generation time.
- For 4 players, 5 total dungeon generations occur (1 server + 4 clients) per regeneration call.
- Stagger regeneration events where possible to avoid simultaneous CPU spikes across machines.

See [MULTIPLAYER.md](MULTIPLAYER.md) for full multiplayer guidance.

---

## Benchmarking Methodology

To establish a performance baseline:

1. Call `RunGenerationCycles(20)` with your target configuration.
2. Record the min/max/average cycle time from the Output Log.
3. Verify memory delta is consistently 0 across cycles.
4. Repeat on each target platform.
5. Document the baseline in your project's performance budget spreadsheet.

Example baseline record:
```
Config: 40x40 grid, 10 rooms, 5 prop entries @ SpawnChance 0.05
Platform: Windows, Intel Core i7-12700K
Average cycle time: 42 ms
Peak cycle time: 58 ms
Memory delta: 0 MB per cycle
```
