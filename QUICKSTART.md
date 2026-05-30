# Quickstart Guide — Your First Dungeon in Under 10 Minutes

This guide walks you from a blank UE5 project to a fully generated dungeon with floor tiles, walls, and corridors.

---

## Prerequisites

- Unreal Engine 5.5 installed
- A C++ Unreal project (Blueprint-only projects work, but the plugin requires a C++ module to compile)
- Plugin installed (see [README.md § Quick Installation](README.md#quick-installation))

---

## Step 1 — Verify the Plugin is Enabled (~1 min)

1. Open the editor.
2. Go to **Edit → Plugins**.
3. Search for **DungeonCraft Pro** and confirm it is enabled (checkbox ticked).
4. Restart the editor if prompted.

---

## Step 2 — Source Your Meshes (~2 min)

The generator needs at minimum:
- A **floor tile mesh** — any flat square mesh (e.g., `SM_Floor_Tile` from Starter Content)
- A **wall mesh** — any rectangular mesh (e.g., `SM_Wall` from Starter Content)

You can use Unreal's built-in Starter Content or any custom assets. The generator auto-calculates tile size from the floor mesh bounds if `bAutoFloorTileSizeGeneration` is enabled.

---

## Step 3 — Add the DungeonGenerator Actor (~1 min)

1. In the **Place Actors** panel, search for `DungeonGenerator`.
2. Drag it into your level (anywhere — position does not affect generation).
3. With it selected, open the **Details** panel.

---

## Step 4 — Assign Floor and Wall Meshes (~1 min)

In the Details panel under **Generator Properties - Floor Settings**:

| Property | Value |
|---|---|
| `FloorSM` | Your floor tile mesh |
| `bAutoFloorTileSizeGeneration` | `true` (recommended) |
| `FloorPivotOffset` | `(0, 0, 0)` unless your mesh pivot is off-center |

Under **Generator Properties - Wall Settings**:

| Property | Value |
|---|---|
| `WallSM` | Your wall mesh |
| `bWallFacingX` | `true` if the wall faces the X-axis, `false` for Y-axis |
| `WallSMPivotOffset` | `(0, 0, 0)` unless your mesh pivot is off-center |

---

## Step 5 — Configure Generation Parameters (~1 min)

Under **Generator Properties**:

| Property | Recommended Starter Value | Description |
|---|---|---|
| `TileMapRows` | `30` | Height of the tile grid |
| `TileMapColumns` | `30` | Width of the tile grid |
| `MinRoomSize` | `3` | Minimum room size in tiles |
| `MaxRoomSize` | `6` | Maximum room size in tiles |
| `RoomsToGenerate` | `5` | Number of rooms to place |
| `bGenerateOnBeginPlay` | `true` | Auto-generate when the level starts |

---

## Step 6 — Press Play (~30 sec)

Press **Play in Editor (PIE)**. The dungeon will generate automatically. You should see floor tiles, walls, and corridors appear.

To regenerate without restarting Play:
- Select the `DungeonGenerator` actor and click the **Generate Dungeon** button in the Details panel (available in editor via `CallInEditor`).
- Or call `GenerateDungeon()` from any Blueprint.

---

## Step 7 — Optional: Add Props (~2 min)

### Static Mesh Props

Under **Generator Properties - Props → Static Meshes to Spawn**, add an entry:

| Property | Value |
|---|---|
| `StaticMesh` | A barrel, crate, or any mesh |
| `SpawnChance` | `0.05` (5% per floor tile) |
| `bOnlySpawnInRooms` | `true` |

### Blueprint Actors

Under **Generator Properties - Props → Blueprint Actors to Spawn**, add an entry:

| Property | Value |
|---|---|
| `BlueprintClass` | Your enemy or chest Blueprint class |
| `SpawnChance` | `0.03` |
| `bOnlySpawnInRooms` | `true` |

> **Important:** Blueprint actors must have the tag `DungeonBP` added to their Tags array for `ClearDungeon()` to clean them up automatically.

---

## Step 8 — Add a Ceiling (Optional)

Under **Generator Properties - Ceiling Settings**:

| Property | Value |
|---|---|
| `CeilingSM` | A ceiling tile mesh (same as floor tile works fine) |
| `CeilingHeight` | `400.0` (match your floor tile size) |

---

## Configuration Presets

Copy any of these presets into the Details panel for common dungeon sizes:

### Small Dungeon (fast generation)
```
TileMapRows = 20, TileMapColumns = 20
MinRoomSize = 3, MaxRoomSize = 5
RoomsToGenerate = 4
```

### Medium Dungeon (recommended for most games)
```
TileMapRows = 40, TileMapColumns = 40
MinRoomSize = 4, MaxRoomSize = 8
RoomsToGenerate = 10
```

### Large Dungeon
```
TileMapRows = 70, TileMapColumns = 70
MinRoomSize = 5, MaxRoomSize = 10
RoomsToGenerate = 20
```

### Extreme (stress test — may be slow)
```
TileMapRows = 100, TileMapColumns = 100
MinRoomSize = 6, MaxRoomSize = 12
RoomsToGenerate = 40
```

---

## Common Pitfalls

| Problem | Cause | Fix |
|---|---|---|
| No floor tiles appear | `FloorSM` not assigned | Assign a mesh to `FloorSM` |
| Walls face wrong direction | `bWallFacingX` incorrect | Toggle `bWallFacingX` |
| Tiles are the wrong size | Auto-size picking up wrong bounds | Disable `bAutoFloorTileSizeGeneration` and set `FloorTileSize` manually |
| Blueprint actors not cleaning up | Missing `DungeonBP` tag | Add `DungeonBP` to the Blueprint actor's Tags array |
| Dungeon generates twice | Both `bGenerateOnBeginPlay` and a Blueprint call | Disable one source |

---

## Next Steps

- **[CONFIGURATION.md](CONFIGURATION.md)** — Complete reference for all spawn parameters
- **[THEMES.md](THEMES.md)** — Create themed rooms with custom meshes and props
- **[MULTIPLAYER.md](MULTIPLAYER.md)** — Set up server-authoritative dungeon generation
- **[TROUBLESHOOTING.md](TROUBLESHOOTING.md)** — Diagnose generation or spawn issues
