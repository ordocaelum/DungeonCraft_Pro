# Troubleshooting Guide — DungeonCraft Pro

This guide covers the most common issues encountered when using DungeonCraft Pro, with step-by-step diagnosis and solutions.

---

## Enabling Debug Logging

Before diagnosing any issue, enable verbose logging in the editor's Output Log:

1. Open the **Output Log** panel (**Window → Developer Tools → Output Log**).
2. In the Output Log filter box, type `DungeonGenerator`.
3. Set verbosity to `Verbose` in `DefaultEngine.ini` for maximum detail:

```ini
[Core.Log]
DungeonGenerator=Verbose
```

Reset to `Log` (default) for shipping builds.

You can also enable `bDebugActive` on the generator actor to draw spawn-point visualization in the editor viewport.

---

## Issue 1 — Dungeon Not Spawning

**Symptoms:** Level loads, no floor tiles appear, Output Log may be silent.

### Diagnosis checklist

| Check | How to verify |
|---|---|
| `FloorSM` assigned | Details panel → Floor Settings → `FloorSM` not blank |
| Plugin enabled | Edit → Plugins → DungeonCraft Pro = enabled |
| `bGenerateOnBeginPlay` enabled | Details panel → Generator Properties → `bGenerateOnBeginPlay = true` |
| Actor is in the level | World Outliner lists a `DungeonGenerator` actor |
| `ValidateConfig` passing | Output Log should show no `Error` lines from `DungeonGenerator` |

### Common fixes

- **Missing `FloorSM`:** Assign any static mesh to `FloorSM`. Without it, no tiles can be placed.
- **Config validation failure:** Look for `[Error]` lines in the Output Log filtered to `DungeonGenerator`. Common messages:
  - `MinRoomSize must be < MaxRoomSize` — Correct the values.
  - `TileMapRows/Columns must be >= 10` — Set grid size to at least 10×10.
- **Data asset override:** If `bUseConfigDataAsset = true` but `DungeonConfig` is null, generation is blocked. Assign a valid asset or disable the flag.
- **Multiplayer client:** Clients do not generate the dungeon independently. The server replicates the seed and triggers client generation automatically. Ensure the server is running.

---

## Issue 2 — Actors Failing to Spawn

**Symptoms:** Floor tiles appear but no Blueprint actors, props, or decals are visible.

### Blueprint actors not spawning

| Check | Fix |
|---|---|
| `BlueprintClass` is set | Assign a valid `TSubclassOf<AActor>` to the entry |
| `SpawnChance` > 0 | A value of `0.0` means never spawn |
| `bOnlySpawnInRooms = true` but dungeon has no rooms | Check `RoomsToGenerate > 0` and that rooms are actually placed (see Issue 1) |
| Server-only spawn | Blueprint actors only spawn on the server in multiplayer. Configure replication on the actor itself. |

### Static mesh props not appearing

- Confirm `StaticMesh` is not null in each `FStaticMeshSpawnParams` entry.
- Confirm `SpawnChance` is above `0.0`.
- Check the Output Log for lines like `SpawnStaticMeshes: Skipped entry — StaticMesh is null`.

### Decals not appearing

- Decal materials must use the **Deferred Decal** material domain in the Material Editor.
- Ensure `DecalMaterial` is not null.
- Confirm `SpawnChance > 0`.
- Decals require a `DeferredDecals` rendering pass — verify **Project Settings → Rendering → Lighting → Support Global Clip Planes** is not causing issues.

---

## Issue 3 — Blueprint Actors Not Cleaning Up

**Symptoms:** After calling `ClearDungeon()` or regenerating, Blueprint actors from the previous generation remain in the level.

### Cause

`ClearDungeon()` finds Blueprint actors by the tag `DungeonBP`. If your Blueprint class does not add this tag, they will not be found.

### Fix

1. Open your Blueprint class in the Blueprint Editor.
2. Select the root **Actor** in the Components panel.
3. In the Details panel, scroll to **Actor → Tags**.
4. Add `DungeonBP` as a tag entry.

Alternatively, add the tag in C++:

```cpp
// In the Blueprint actor's constructor
Tags.Add(FName("DungeonBP"));
```

The generator has a name-based fallback that also searches for actors whose `GetName()` contains `"DungeonBP"`, but relying on the tag is more reliable.

---

## Issue 4 — Performance Issues

**Symptoms:** Generation takes several seconds, or the editor freezes during generation.

### Diagnosis

Use `RunGenerationCycles(1)` from the Details panel and note the timing in the Output Log:

```
DungeonGenerator: Cycle 1/1 completed in [X]ms
```

Common causes and fixes:

| Cause | Fix |
|---|---|
| Grid too large | Reduce `TileMapRows`/`TileMapColumns`. A 100×100 grid with 40 rooms can take 500+ ms. |
| `MaxRandomAttemptsPerRoom` too high | Reduce to 500–1000 for grids under 60×60. |
| Too many prop entries with high spawn chances | Reduce `SpawnChance` or number of entries. Each floor tile evaluates every entry. |
| Decals on every tile | Reduce decal `SpawnChance` to 0.1 or lower. |
| Blueprint actors spawning many times | Blueprint spawning triggers world context queries; many actors (> 200) may introduce frame spikes. |

### Profiling

Use Unreal's built-in Unreal Insights or `stat unit` to profile generation:

1. Open the Output Log console.
2. Type `stat unit` during PIE.
3. Call `GenerateDungeon()` and observe frame time spikes.
4. Use **Unreal Insights** for deeper call-stack profiling.

---

## Issue 5 — Multiplayer Desynchronization

**Symptoms:** Different clients see different dungeons, or clients see no dungeon.

### Cause

The server generates the dungeon with a random seed, which is replicated to clients via `OnRep_DungeonSeed`. If the seed arrives after the client has already attempted generation, or if the client generates independently, layouts diverge.

### Fix

- **Never call `GenerateDungeon()` directly on clients.** Only call it on the server (authority).
- Ensure `HasAuthority()` is checked before calling `GenerateDungeon()` from Blueprint.
- The generator handles seed replication automatically: when `DungeonSeed` is replicated, clients re-generate using the same seed.

```cpp
// Always gate on authority
if (HasAuthority())
{
    DungeonGenerator->GenerateDungeon();
}
```

- **Blueprint actors in multiplayer:** Spawned actors must also be replicated. Check the actor's **Replicates** flag.

---

## Issue 6 — Memory Usage High / Memory Leaks

**Symptoms:** Memory grows with each call to `GenerateDungeon()` and does not return to baseline.

### Diagnosis

Run `RunGenerationCycles(10)` and observe the "Memory delta" lines in the Output Log. A non-zero delta accumulating across cycles indicates a leak.

### Common causes

| Cause | Fix |
|---|---|
| Blueprint actors not tagged with `DungeonBP` | See Issue 3 above |
| Actors spawned outside the generator | Actors spawned by Blueprint events that fire during generation are not tracked by `ClearDungeon()`. Manage these manually. |
| Decal components not destroyed | `ClearDungeon()` destroys actor-based decals. Ensure decals are spawned as actors (default behavior). |

---

## Issue 7 — Theme System Not Applying

**Symptoms:** Rooms use the global floor/wall/ceiling meshes even with `RoomThemes` populated.

### Checklist

| Check | Fix |
|---|---|
| `RoomThemes` array has valid entries | Assign at least one `URoomThemeDataAsset` asset |
| `MaxSpecialRooms` > 0 | Set `MaxSpecialRooms` to at least 1 |
| Theme asset has `FloorMesh` set | Open the data asset and assign a `FloorMesh` |
| `SelectionWeight > 0` on the theme asset | Weight of 0 means never selected |
| `DefaultThemeWeight` is very high | Reduce it to give themes a fair chance |

### Verification

Enable verbose logging and look for lines matching `SpawnThemedRoom` or `RoomTheme` in the Output Log. These lines indicate which rooms received a theme.

---

## Issue 8 — Walls Facing Wrong Direction

**Symptoms:** Walls appear rotated 90°, facing inward or outward incorrectly.

### Fix

Toggle `bWallFacingX` on the generator. The correct value depends on your wall mesh's facing direction:
- Open the wall mesh in the Static Mesh Editor.
- The "outward" face should point in +X → set `bWallFacingX = true`.
- If it points in +Y → set `bWallFacingX = false`.

If the pivot is off-center, adjust `WallSMPivotOffset`.

---

## Issue 9 — Ceiling Tiles at Wrong Height

**Symptoms:** Ceilings are at floor level, or floating far above.

### Fix

- `CeilingHeight` should match your `FloorTileSize`. If tiles are 400 units wide, set `CeilingHeight = 400`.
- If your ceiling mesh pivot is not at the bottom face, adjust `CeilingPivotOffset`.

---

## FAQ

**Q: Can I call `GenerateDungeon()` multiple times without calling `ClearDungeon()` first?**
A: Yes. `GenerateDungeon()` calls `ClearDungeon()` internally before each new generation.

**Q: Can I change parameters between generations?**
A: Yes. Modify any property and call `GenerateDungeon()` to regenerate with the new settings.

**Q: Is the generator thread-safe?**
A: No. Always call `GenerateDungeon()` and `ClearDungeon()` on the game thread. Re-entrant calls are blocked by an internal guard flag.

**Q: Why are some rooms missing corridors?**
A: Room connectivity uses a simple nearest-neighbor algorithm. For very sparse grids (few rooms on a large grid), some rooms may be isolated. Reduce grid size or increase `RoomsToGenerate`.

**Q: Can I use this with World Partition?**
A: Not tested. World Partition streaming may conflict with the tag-based cleanup system. Report any issues via GitHub.

**Q: How do I make decals only appear on specific room types?**
A: Use per-theme `FDecalSpawnParams` in a `URoomThemeDataAsset` instead of the global `FloorDecalsToSpawn`.
