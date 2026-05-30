# Configuration Reference — DungeonCraft Pro

This document is a complete reference for all configurable properties in DungeonCraft Pro, including valid ranges, defaults, and usage guidance.

---

## Overview

Configuration can be supplied in two ways:

1. **Actor Details Panel** — Set properties directly on the `ADungeonGenerator` actor in the level.
2. **Data Asset** — Create a `DungeonConfigDataAsset`, populate it, assign it to the generator, and enable `bUseConfigDataAsset = true`.

The data asset approach is recommended for games with multiple dungeon configurations or runtime switching.

---

## Generator Properties

### Grid Size

| Property | Default | Valid Range | Description |
|---|---|---|---|
| `TileMapRows` | `50` | ≥ 10 | Number of tile rows (Y axis extent). |
| `TileMapColumns` | `50` | ≥ 10 | Number of tile columns (X axis extent). |

**Notes:**
- Larger grids increase generation time quadratically for room placement.
- Memory usage scales with `TileMapRows × TileMapColumns × sizeof(FTile)`.
- For real-time regeneration, keep under 80×80 for smooth 60fps generation.

---

### Room Generation

| Property | Default | Valid Range | Description |
|---|---|---|---|
| `MinRoomSize` | `5` | ≥ 2 | Minimum room dimension in tiles (both width and height). |
| `MaxRoomSize` | `7` | ≥ 3 | Maximum room dimension in tiles. Must be > `MinRoomSize`. |
| `RoomsToGenerate` | `15` | ≥ 1 | Target number of rooms. May be fewer if the grid is too small. |
| `MaxRandomAttemptsPerRoom` | `1500` | ≥ 100 | Maximum placement retries per room before it is abandoned. |

**Notes:**
- `MinRoomSize` must be at least 2 tiles less than `MaxRoomSize` or validation will fail.
- Increasing `MaxRandomAttemptsPerRoom` on dense grids can significantly increase generation time. Profile with `RunGenerationCycles()`.
- If `RoomsToGenerate` is too high for the grid size, the generator will log a warning and place as many rooms as it can.

---

### Seed Control

| Property | Default | Description |
|---|---|---|
| `bUseFixedSeed` | `false` | Enable deterministic generation using `FixedSeed`. |
| `FixedSeed` | `12345` | Seed value when `bUseFixedSeed = true`. In multiplayer, the server seed is replicated to clients. |

**Notes:**
- In multiplayer, always leave `bUseFixedSeed = false` and allow the server to generate a random seed. The seed is replicated automatically.
- For level design testing, enable `bUseFixedSeed` to lock in a specific layout.

---

### Behavior

| Property | Default | Description |
|---|---|---|
| `bGenerateOnBeginPlay` | `true` | Automatically call `GenerateDungeon()` at `BeginPlay`. Disable if you want to trigger generation manually (e.g., after a loading screen). |
| `bUseConfigDataAsset` | `false` | Override all properties with the values from `DungeonConfig`. |
| `DungeonConfig` | `nullptr` | The `DungeonConfigDataAsset` to use when `bUseConfigDataAsset = true`. |

---

## Floor Settings

| Property | Default | Description |
|---|---|---|
| `FloorSM` | `nullptr` | **Required.** Static mesh used for every floor tile. |
| `FloorTileSize` | `400.0` | World-unit size of one tile. Used when `bAutoFloorTileSizeGeneration = false`. |
| `bAutoFloorTileSizeGeneration` | `true` | Derive tile size from `FloorSM`'s X-axis bounding box extent. |
| `FloorPivotOffset` | `(0,0,0)` | Applied to each floor tile spawn position. Use when the mesh pivot is not centered. |

**Pivot offset tips:**
- If your floor mesh pivot is at one corner, set `FloorPivotOffset = (TileSize/2, TileSize/2, 0)`.
- Check mesh pivot in the Static Mesh Editor → use the pivot tool if needed.

---

## Wall Settings

| Property | Default | Description |
|---|---|---|
| `WallSM` | `nullptr` | Wall mesh. Walls are skipped entirely if null. |
| `WallWidth` | `20.0` | Thickness of the wall in world units. Used to offset the wall mesh inward from the tile edge. |
| `WallSMPivotOffset` | `(0,0,0)` | Pivot correction for the wall mesh. |
| `bWallFacingX` | `true` | Set `true` if the wall faces the X-axis (the "front" of the mesh points in +X). Set `false` for Y-axis-facing meshes. |

**Wall axis guide:**
- Open the wall mesh in the Static Mesh Editor.
- The "front" of the wall (the visible face) should point in a positive axis direction.
- If it points in +X → `bWallFacingX = true`.
- If it points in +Y → `bWallFacingX = false`.

---

## Ceiling Settings

| Property | Default | Description |
|---|---|---|
| `CeilingSM` | `nullptr` | Ceiling mesh. Ceilings are skipped when null. |
| `CeilingHeight` | `400.0` | Z height above each floor tile where the ceiling mesh is placed. Should match your tile height. |
| `CeilingPivotOffset` | `(0,0,400)` | Pivot correction for the ceiling mesh. The default Z of 400 is typical for a 1:1 floor/ceiling tile pair. |

---

## Pillar Settings (`FPillarSpawnParams`)

Pillars are placed at detected room corners.

| Property | Default | Valid Range | Description |
|---|---|---|---|
| `PillarMesh` | `nullptr` | — | Pillar mesh. Entire pillar system disabled if null. |
| `SpawnChance` | `0.75` | 0.0–1.0 | Probability that a pillar is placed at each corner. |
| `LocationOffset` | `(0,0,0)` | — | Offset from the exact corner world position. |
| `RotationOffset` | `(0,0,0)` | — | Rotation added to each pillar. |
| `Scale` | `(1,1,1)` | — | Scale of each pillar mesh. |
| `MaterialOverride` | `nullptr` | — | Optional material applied to slot 0. |

---

## Blueprint Actor Spawning (`FBlueprintSpawnParams`)

Each entry in `BlueprintActorsToSpawn` is evaluated per floor tile.

| Property | Default | Valid Range | Description |
|---|---|---|---|
| `BlueprintClass` | `nullptr` | — | Actor class to spawn. Entry is skipped if null. |
| `SpawnChance` | `0.1` | 0.0–1.0 | Per-tile spawn probability. |
| `LocationOffsetMin` | `(0,0,0)` | — | Minimum random offset from tile center. |
| `LocationOffsetMax` | `(0,0,0)` | — | Maximum random offset. The actual offset is a random point between Min and Max. |
| `RotationOffset` | `(0,0,0)` | — | Base rotation added to the spawned actor. |
| `Scale` | `(1,1,1)` | — | Scale applied to the spawned actor's root component. |
| `bOnlySpawnInRooms` | `true` | — | Restricts spawning to room tiles only (not corridors). |

**Best practices:**
- Keep `SpawnChance` low (0.02–0.10) to avoid over-population.
- Use `LocationOffsetMin/Max` to add organic variation.
- Ensure the Blueprint class adds `DungeonBP` to its Tags for proper cleanup.

---

## Static Mesh Props (`FStaticMeshSpawnParams`)

Each entry in `StaticMeshesToSpawn` is evaluated per floor tile.

| Property | Default | Valid Range | Description |
|---|---|---|---|
| `StaticMesh` | `nullptr` | — | Mesh to scatter. Skipped if null. |
| `MaterialOverride` | `nullptr` | — | Material override for slot 0. |
| `SpawnChance` | `0.1` | 0.0–1.0 | Per-tile spawn probability. |
| `LocationOffsetMin` | `(0,0,0)` | — | Minimum positional offset from tile center. |
| `LocationOffsetMax` | `(0,0,0)` | — | Maximum positional offset. |
| `RotationOffset` | `(0,0,0)` | — | Rotation to apply. |
| `Scale` | `(1,1,1)` | — | Scale of each instance. |
| `bOnlySpawnInRooms` | `false` | — | Restrict to room tiles only. |

---

## Floor Decals (`FDecalSpawnParams`)

Each entry in `FloorDecalsToSpawn` is evaluated per floor tile.

| Property | Default | Valid Range | Description |
|---|---|---|---|
| `DecalMaterial` | `nullptr` | — | Decal material (must use Deferred Decal material domain). Skipped if null. |
| `SpawnChance` | `0.25` | 0.0–1.0 | Per-tile spawn probability. |
| `DecalSize` | `(200,200,200)` | — | Size of the decal projection box. |
| `ZOffset` | `2.0` | — | Height above the floor to place the decal origin. |
| `RandomRotationRange` | `360.0` | 0.0–360.0 | Yaw randomization range. `360.0` = fully random rotation. |
| `bOnlySpawnInRooms` | `false` | — | Restrict to room tiles only. |

---

## Theme System

| Property | Default | Valid Range | Description |
|---|---|---|---|
| `RoomThemes` | `[]` | — | Array of `URoomThemeDataAsset` assets. Rooms are assigned themes from this pool. |
| `DefaultThemeWeight` | `3.0` | ≥ 0.0 | Weight of the "no theme" option in the weighted random selection. Higher values mean more plain rooms. |
| `MaxSpecialRooms` | `2` | ≥ 0 | Maximum rooms that receive a themed variant. Others use global floor/wall/ceiling meshes. |

**Theme selection algorithm:**
1. Total weight = sum of all `SelectionWeight` values + `DefaultThemeWeight`.
2. For each room (up to `MaxSpecialRooms`), a weighted random pick determines the theme.
3. Remaining rooms use global meshes.

---

## Data Table Configuration (`FRoomTemplate`)

If `RoomTemplatesDataTable` is set (non-null), each room is assigned a row from the table for mesh variation.

**Creating a data table:**
1. Content Browser → right-click → **Miscellaneous → Data Table**.
2. Select `FRoomTemplate` as the row type.
3. Add rows for each room variant.
4. Assign the asset to `RoomTemplatesDataTable` on the generator.

---

## Configuration Presets

### Small Dungeon
```
TileMapRows = 20, TileMapColumns = 20
MinRoomSize = 3, MaxRoomSize = 5
RoomsToGenerate = 4, MaxRandomAttemptsPerRoom = 500
```

### Medium Dungeon (recommended default)
```
TileMapRows = 40, TileMapColumns = 40
MinRoomSize = 4, MaxRoomSize = 8
RoomsToGenerate = 10, MaxRandomAttemptsPerRoom = 1000
```

### Large Dungeon
```
TileMapRows = 70, TileMapColumns = 70
MinRoomSize = 5, MaxRoomSize = 10
RoomsToGenerate = 20, MaxRandomAttemptsPerRoom = 2000
```

### Extreme (high-end hardware only)
```
TileMapRows = 120, TileMapColumns = 120
MinRoomSize = 6, MaxRoomSize = 14
RoomsToGenerate = 40, MaxRandomAttemptsPerRoom = 3000
```
