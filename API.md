# API Reference — DungeonCraft Pro

This document covers every public class, struct, delegate, and function exposed by the plugin.

---

## Log Category

```cpp
DECLARE_LOG_CATEGORY_EXTERN(DungeonGenerator, Log, All);
```

All plugin log output uses the `DungeonGenerator` category. To filter in the Output Log:

```
Log DungeonGenerator All
```

To suppress verbose output in shipping builds, set the category to `Warning` in `DefaultEngine.ini`:

```ini
[Core.Log]
DungeonGenerator=Warning
```

---

## Delegates

### `FOnDungeonSpawned`

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDungeonSpawned);
```

Broadcast after all meshes and actors for a dungeon generation cycle have been spawned.

**Usage (Blueprint):** Bind to `OnDungeonSpawned` on the generator actor.

**Usage (C++):**
```cpp
Generator->OnDungeonSpawned.AddDynamic(this, &AMyClass::HandleDungeonSpawned);
```

---

## Structs

### `FStaticMeshSpawnParams`

Controls how a static mesh prop is scattered across dungeon floor tiles.

| Property | Type | Default | Description |
|---|---|---|---|
| `StaticMesh` | `UStaticMesh*` | `nullptr` | Mesh asset to spawn. Must be set; entry is skipped if null. |
| `MaterialOverride` | `UMaterialInterface*` | `nullptr` | Optional material override applied to slot 0. |
| `SpawnChance` | `float` | `0.1` | Probability per floor tile (0.0–1.0). |
| `LocationOffsetMin` | `FVector` | `(0,0,0)` | Minimum random location offset from tile center. |
| `LocationOffsetMax` | `FVector` | `(0,0,0)` | Maximum random location offset from tile center. |
| `RotationOffset` | `FRotator` | `(0,0,0)` | Additional rotation applied to the spawned mesh. |
| `Scale` | `FVector` | `(1,1,1)` | Uniform or non-uniform scale. |
| `bOnlySpawnInRooms` | `bool` | `false` | When `true`, this mesh never spawns in corridors. |

---

### `FBlueprintSpawnParams`

Controls how a Blueprint actor class is spawned inside the dungeon.

| Property | Type | Default | Description |
|---|---|---|---|
| `BlueprintClass` | `TSubclassOf<AActor>` | `nullptr` | Actor class to spawn. Must be set; entry is skipped if null. |
| `SpawnChance` | `float` | `0.1` | Probability per floor tile (0.0–1.0). |
| `LocationOffsetMin` | `FVector` | `(0,0,0)` | Minimum random location offset. |
| `LocationOffsetMax` | `FVector` | `(0,0,0)` | Maximum random location offset. |
| `RotationOffset` | `FRotator` | `(0,0,0)` | Base rotation applied after spawn. |
| `Scale` | `FVector` | `(1,1,1)` | Scale applied to the spawned actor's root. |
| `bOnlySpawnInRooms` | `bool` | `true` | When `true`, only spawns on room tiles (not corridors). |

> **Cleanup requirement:** Spawned actors must have the `DungeonBP` tag in their Tags array to be found and destroyed by `ClearDungeon()`.

---

### `FDecalSpawnParams`

Controls floor decal placement.

| Property | Type | Default | Description |
|---|---|---|---|
| `DecalMaterial` | `UMaterialInterface*` | `nullptr` | Decal material (must be a Decal domain material). Skipped if null. |
| `SpawnChance` | `float` | `0.25` | Probability per floor tile (0.0–1.0). |
| `DecalSize` | `FVector` | `(200,200,200)` | Size of the decal in world units. |
| `ZOffset` | `float` | `2.0` | Height above the floor tile to place the decal. |
| `RandomRotationRange` | `float` | `360.0` | Yaw rotation randomization range in degrees. |
| `bOnlySpawnInRooms` | `bool` | `false` | When `true`, decals only appear in rooms. |

---

### `FPillarSpawnParams`

Controls pillar mesh placement at room corners.

| Property | Type | Default | Description |
|---|---|---|---|
| `PillarMesh` | `UStaticMesh*` | `nullptr` | Pillar mesh asset. Pillar system is disabled when null. |
| `SpawnChance` | `float` | `0.75` | Probability per detected corner (0.0–1.0). |
| `LocationOffset` | `FVector` | `(0,0,0)` | Offset from corner world position. |
| `RotationOffset` | `FRotator` | `(0,0,0)` | Base rotation of the pillar. |
| `Scale` | `FVector` | `(1,1,1)` | Scale of the pillar. |
| `MaterialOverride` | `UMaterialInterface*` | `nullptr` | Optional material override. |

---

### `FRoomTemplate` (DataTable Row)

Used to drive room mesh selection from a `UDataTable`.

| Property | Type | Description |
|---|---|---|
| `RoomTileMesh` | `UStaticMesh*` | Floor tile mesh for this room type. |
| `RoomTileMeshMaterialOverride` | `UMaterialInterface*` | Floor material override. |
| `RoomTilePivotOffset` | `FVector` | Pivot offset for the floor mesh. |
| `WallMesh` | `UStaticMesh*` | Wall mesh for this room type. |
| `WallMeshMaterialOverride` | `UMaterialInterface*` | Wall material override. |
| `WallMeshPivotOffset` | `FVector` | Pivot offset for the wall mesh. |
| `bIsWallFacingX` | `bool` | Wall facing axis for this template. |
| `CeilingMesh` | `UStaticMesh*` | Ceiling mesh for this room type. |
| `CeilingMaterialOverride` | `UMaterialInterface*` | Ceiling material override. |
| `CeilingPivotOffset` | `FVector` | Ceiling pivot offset (default `(0,0,400)`). |

---

## Class: `ADungeonGenerator`

The main actor class. Place one per level. Inherits from `AActor` with `bReplicates = true`.

### Public Properties

#### Generator Properties

| Property | Type | Default | Clamp | Description |
|---|---|---|---|---|
| `DungeonConfig` | `UDungeonConfigDataAsset*` | `nullptr` | — | Optional data asset. When assigned with `bUseConfigDataAsset = true`, all generator values are overridden from the asset. |
| `bUseConfigDataAsset` | `bool` | `false` | — | Enable data-asset-driven configuration. |
| `TileMapRows` | `int32` | `50` | Min 10 | Number of tile rows (Y extent of the grid). |
| `TileMapColumns` | `int32` | `50` | Min 10 | Number of tile columns (X extent of the grid). |
| `MinRoomSize` | `int32` | `5` | Min 2 | Minimum room side length in tiles. Must be < `MaxRoomSize`. |
| `MaxRoomSize` | `int32` | `7` | Min 3 | Maximum room side length in tiles. |
| `RoomsToGenerate` | `int32` | `15` | Min 1 | Target number of rooms. Fewer may be placed if the grid is too small. |
| `MaxRandomAttemptsPerRoom` | `int32` | `1500` | Min 100 | Maximum placement attempts before a room is skipped. Increase for dense grids. |
| `bUseFixedSeed` | `bool` | `false` | — | Use `FixedSeed` instead of a random seed. |
| `FixedSeed` | `int32` | `12345` | — | Seed value when `bUseFixedSeed = true`. |
| `bGenerateOnBeginPlay` | `bool` | `true` | — | Call `GenerateDungeon()` automatically at `BeginPlay`. |

#### Floor Settings

| Property | Type | Default | Description |
|---|---|---|---|
| `FloorSM` | `UStaticMesh*` | `nullptr` | Floor tile mesh. Required for generation. |
| `FloorTileSize` | `float` | `400.0` | World-unit size of one floor tile. Overridden by auto-size. |
| `bAutoFloorTileSizeGeneration` | `bool` | `true` | Auto-calculate tile size from `FloorSM` bounds. |
| `FloorPivotOffset` | `FVector` | `(0,0,0)` | Pivot correction for off-center floor meshes. |

#### Wall Settings

| Property | Type | Default | Description |
|---|---|---|---|
| `WallSM` | `UStaticMesh*` | `nullptr` | Wall mesh. Walls are skipped when null. |
| `WallWidth` | `float` | `20.0` | Thickness of the wall in world units. Used for placement offset. |
| `WallSMPivotOffset` | `FVector` | `(0,0,0)` | Pivot correction for off-center wall meshes. |
| `bWallFacingX` | `bool` | `true` | `true` if the wall mesh faces the X-axis (common for forward-facing meshes). |

#### Ceiling Settings

| Property | Type | Default | Description |
|---|---|---|---|
| `CeilingSM` | `UStaticMesh*` | `nullptr` | Ceiling tile mesh. Ceiling is skipped when null. |
| `CeilingHeight` | `float` | `400.0` | Z offset above floor where ceiling tiles are placed. |
| `CeilingPivotOffset` | `FVector` | `(0,0,400)` | Pivot correction for ceiling meshes. |

#### Props

| Property | Type | Description |
|---|---|---|
| `PillarSettings` | `FPillarSpawnParams` | Configuration for pillar placement at room corners. |
| `BlueprintActorsToSpawn` | `TArray<FBlueprintSpawnParams>` | List of Blueprint actor definitions to scatter in the dungeon. |
| `StaticMeshesToSpawn` | `TArray<FStaticMeshSpawnParams>` | List of static mesh prop definitions. |
| `FloorDecalsToSpawn` | `TArray<FDecalSpawnParams>` | List of floor decal definitions. |

#### Theme System

| Property | Type | Default | Description |
|---|---|---|---|
| `RoomThemes` | `TArray<URoomThemeDataAsset*>` | `[]` | Pool of theme assets to randomly assign to rooms. |
| `DefaultThemeWeight` | `float` | `3.0` | Relative weight for unthemed (default) rooms in the selection pool. Higher values produce more plain rooms. |
| `MaxSpecialRooms` | `int32` | `2` | Maximum number of rooms that receive a theme asset. |

#### Data Table

| Property | Type | Description |
|---|---|---|
| `RoomTemplatesDataTable` | `UDataTable*` | Optional data table of `FRoomTemplate` rows for per-row mesh variation. |

#### Delegates

| Property | Type | Description |
|---|---|---|
| `OnDungeonSpawned` | `FOnDungeonSpawned` | Broadcast when all generation and spawning is complete. |

---

### Public Functions

#### `GenerateDungeon()`

```cpp
UFUNCTION(BlueprintCallable, CallInEditor, Category = "Dungeon Generation")
void GenerateDungeon();
```

Triggers a full dungeon generation cycle:
1. Validates configuration (logs errors and aborts if invalid).
2. Destroys any existing dungeon meshes via `ClearDungeon()`.
3. Initializes the tile matrix and places rooms.
4. Connects rooms with corridors.
5. Spawns all meshes, props, decals, and Blueprint actors.
6. Broadcasts `OnDungeonSpawned`.

**Thread safety:** Must be called on the game thread. Re-entrant calls are blocked with a guard flag.

**Performance:** O(R × A) where R = `RoomsToGenerate` and A = `MaxRandomAttemptsPerRoom`. For typical values (15 rooms, 1500 attempts), generation completes in < 50 ms on modern hardware.

**Example:**
```cpp
// Regenerate with new seed
Generator->bUseFixedSeed = false;
Generator->GenerateDungeon();
```

---

#### `ClearDungeon()`

```cpp
UFUNCTION(BlueprintCallable, CallInEditor, Category = "Dungeon Generator",
    meta = (DisplayName = "Clear Dungeon"))
void ClearDungeon();
```

Destroys all actors spawned by the last generation cycle, identified by internal tags:
- `BR_BETRAYAL_DungeonTile` — floor and wall mesh actors
- `BR_BETRAYAL_CeilingTile` — ceiling mesh actors
- `BR_BETRAYAL_Prop` — static mesh prop actors
- `DungeonBP` — Blueprint actor spawns

Falls back to a name-based search for Blueprint actors when the tag search returns zero results.

---

#### `SetNewRoomSize()`

```cpp
UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
void SetNewRoomSize(int32 NewMinRoomSize, int32 NewMaxRoomSize);
```

Updates room size constraints at runtime. Values are clamped to valid ranges. Call `GenerateDungeon()` afterwards to apply.

| Parameter | Description |
|---|---|
| `NewMinRoomSize` | New minimum room size in tiles. Clamped to ≥ 2. |
| `NewMaxRoomSize` | New maximum room size in tiles. Clamped to ≥ `NewMinRoomSize + 1`. |

---

#### `SetNewFloorMesh()`

```cpp
UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
void SetNewFloorMesh(UStaticMesh* NewFloorMesh, FVector NewFloorPivotOffset,
                     bool bAutoFloorSizeGeneration = true, float OverrideFloorTileSize = 0.f);
```

Replaces the floor mesh reference and recalculates tile size if requested.

| Parameter | Description |
|---|---|
| `NewFloorMesh` | New floor static mesh asset. |
| `NewFloorPivotOffset` | Pivot correction offset for the new mesh. |
| `bAutoFloorSizeGeneration` | When `true`, tile size is derived from mesh bounds. |
| `OverrideFloorTileSize` | Manual tile size; only used when `bAutoFloorSizeGeneration = false`. |

---

#### `SetNewWallMesh()`

```cpp
UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
void SetNewWallMesh(UStaticMesh* NewWallMesh, FVector NewWallSMPivotOffset, bool bIsWallFacingX = true);
```

Replaces the wall mesh reference.

| Parameter | Description |
|---|---|
| `NewWallMesh` | New wall static mesh asset. |
| `NewWallSMPivotOffset` | Pivot correction for the new wall mesh. |
| `bIsWallFacingX` | `true` if the new mesh faces the X-axis. |

---

#### `SetNewCeilingMesh()`

```cpp
UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
void SetNewCeilingMesh(UStaticMesh* NewCeilingMesh, FVector NewCeilingPivotOffset,
                       float NewCeilingHeight = 400.0f);
```

Replaces the ceiling mesh reference.

| Parameter | Description |
|---|---|
| `NewCeilingMesh` | New ceiling static mesh asset. |
| `NewCeilingPivotOffset` | Pivot correction for the new ceiling mesh. |
| `NewCeilingHeight` | Z offset for ceiling tile placement. |

---

#### `RunGenerationCycles()`

```cpp
UFUNCTION(BlueprintCallable, CallInEditor, Category = "Dungeon Generation|Debug")
void RunGenerationCycles(int32 NumCycles = 10);
```

Stress-test function that runs `NumCycles` full generate-then-destroy loops. Logs timing and memory stats after each cycle. Use this to detect memory leaks during development.

**Example output:**
```
DungeonGenerator: Cycle 1/10 completed in 42ms. Memory delta: +0 MB
DungeonGenerator: Cycle 10/10 completed in 44ms. Memory delta: +0 MB
```

---

## Class: `UDungeonConfigDataAsset`

A `UDataAsset` subclass that mirrors all generator properties for data-driven configuration.

Assign the asset to `ADungeonGenerator::DungeonConfig` and set `bUseConfigDataAsset = true` to activate it.

All properties match the generator actor properties. See [CONFIGURATION.md](CONFIGURATION.md) for full details.

---

## Class: `URoomThemeDataAsset`

A `UDataAsset` subclass defining a themed room variant.

| Property | Type | Description |
|---|---|---|
| `ThemeName` | `FString` | Display name for the theme (used in logs). |
| `ThemeDescription` | `FText` | Human-readable description. |
| `FloorMesh` | `UStaticMesh*` | Override floor mesh for this theme. |
| `FloorMaterialOverride` | `UMaterialInterface*` | Floor material override. |
| `FloorPivotOffset` | `FVector` | Floor pivot correction. |
| `WallMesh` | `UStaticMesh*` | Override wall mesh. |
| `WallMaterialOverride` | `UMaterialInterface*` | Wall material override. |
| `WallPivotOffset` | `FVector` | Wall pivot correction. |
| `bWallFacingX` | `bool` | Wall facing for this theme. |
| `CeilingMesh` | `UStaticMesh*` | Override ceiling mesh. |
| `CeilingMaterialOverride` | `UMaterialInterface*` | Ceiling material override. |
| `CeilingPivotOffset` | `FVector` | Ceiling pivot correction. |
| `ThemeSpecificMeshes` | `TArray<FStaticMeshSpawnParams>` | Static mesh props for this theme only. |
| `ThemeSpecificBlueprints` | `TArray<FBlueprintSpawnParams>` | Blueprint actors for this theme only. |
| `GridPlacedMeshes` | `TArray<FGridPlacement>` | Pattern-based mesh placements (e.g., pit floors). |
| `SelectionWeight` | `float` | Relative probability weight for theme selection (≥ 0). |

---

## Struct: `FGridPlacement`

Defines a pattern-based mesh placement within a themed room.

| Property | Type | Description |
|---|---|---|
| `Mesh` | `UStaticMesh*` | Mesh to place at each matching grid cell. |
| `MaterialOverride` | `UMaterialInterface*` | Optional material override. |
| `GridPattern` | `TArray<uint8>` | Flat array of 0/1 values; 1 = place mesh at this cell. |
| `GridWidth` | `int32` | Width of the pattern in cells. |
| `GridHeight` | `int32` | Computed from `GridPattern.Num() / GridWidth`. |
| `LocationOffset` | `FVector` | Offset applied to each placed mesh. |
| `Rotation` | `FRotator` | Rotation of each placed mesh. |
| `Scale` | `FVector` | Scale of each placed mesh. |

**Example 3×3 center pattern:**
```
GridPattern = [0,0,0, 0,1,0, 0,0,0]
GridWidth   = 3
```

---

## Internal Tags

These `FName` tags are used to identify and clean up dungeon actors. They are `const` static members of `ADungeonGenerator`.

| Tag | Actors Tagged |
|---|---|
| `BR_BETRAYAL_DungeonTile` | Floor and wall `AStaticMeshActor` instances |
| `BR_BETRAYAL_CeilingTile` | Ceiling `AStaticMeshActor` instances |
| `BR_BETRAYAL_Prop` | Static mesh prop instances |
| `DungeonBP` | Blueprint-spawned actors |

> Blueprint actors you spawn via `FBlueprintSpawnParams` **must** add the `DungeonBP` tag to their own Tags array, or `ClearDungeon()` will not find them.
