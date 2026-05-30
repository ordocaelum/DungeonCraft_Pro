# Theme System Guide — DungeonCraft Pro

This guide explains the theme system architecture, how to create custom room themes, and advanced techniques for mixing themed and plain rooms.

---

## Architecture Overview

The theme system assigns a `URoomThemeDataAsset` to individual rooms during dungeon generation. Each asset defines:
- Override meshes for floor, walls, and ceiling
- Theme-specific static mesh props
- Theme-specific Blueprint actor spawns
- Grid-based mesh patterns (e.g., pit floors, mosaic tiles)

The system uses **weighted random selection** to assign themes: rooms are selected as "special" rooms up to `MaxSpecialRooms`, then a weighted random draw picks which theme asset to apply.

### Component diagram

```
ADungeonGenerator
  ├── RoomThemes[]            ← Pool of URoomThemeDataAsset
  ├── DefaultThemeWeight      ← Weight for unthemed rooms
  ├── MaxSpecialRooms         ← Cap on themed rooms
  │
  └── [per room at generation time]
        └── FRoom.RoomTheme   ← Assigned or null
              └── SpawnThemedRoom() ← Uses theme meshes & props
```

---

## Creating a Room Theme

### Step 1 — Create the Data Asset

1. Content Browser → right-click → **Miscellaneous → Data Asset**.
2. In the class picker, select **RoomThemeDataAsset**.
3. Name it descriptively, e.g., `DA_Theme_Crypt` or `DA_Theme_Library`.

### Step 2 — Configure Theme Info

Open the asset and fill in:

| Property | Example Value | Notes |
|---|---|---|
| `ThemeName` | `"Crypt"` | Used in log output for debugging. |
| `ThemeDescription` | `"Ancient burial chamber"` | Optional human-readable note. |
| `SelectionWeight` | `1.0` | Relative weight vs. other themes and `DefaultThemeWeight`. |

### Step 3 — Assign Meshes

Under **Theme Meshes**, assign override meshes for this theme. Any null mesh falls back to the generator's global mesh.

| Property | Required? | Description |
|---|---|---|
| `FloorMesh` | Recommended | Override floor tile for this theme. |
| `FloorMaterialOverride` | Optional | Material for the floor mesh. |
| `FloorPivotOffset` | As needed | Pivot correction for the floor mesh. |
| `WallMesh` | Recommended | Override wall mesh. |
| `WallMaterialOverride` | Optional | Wall material. |
| `WallPivotOffset` | As needed | Wall pivot correction. |
| `bWallFacingX` | Match mesh | Wall axis for this theme's wall mesh. |
| `CeilingMesh` | Optional | Override ceiling mesh. |
| `CeilingMaterialOverride` | Optional | Ceiling material. |
| `CeilingPivotOffset` | As needed | Ceiling pivot correction. |

### Step 4 — Add Theme Props

Under **Theme Props**, configure props that only appear in this themed room:

- `ThemeSpecificMeshes` — Array of `FStaticMeshSpawnParams` (same structure as global props).
- `ThemeSpecificBlueprints` — Array of `FBlueprintSpawnParams` for themed actor spawns.

**Example: Crypt theme**
```
ThemeSpecificMeshes[0]:
  StaticMesh = SM_Sarcophagus
  SpawnChance = 0.04
  bOnlySpawnInRooms = true

ThemeSpecificBlueprints[0]:
  BlueprintClass = BP_Skeleton_Enemy
  SpawnChance = 0.03
  bOnlySpawnInRooms = true
```

### Step 5 — Assign to the Generator

1. Select the `DungeonGenerator` actor in the level.
2. Under **Generator Properties - Themes**, expand `RoomThemes`.
3. Add an array entry and assign your new data asset.
4. Configure `DefaultThemeWeight` and `MaxSpecialRooms` as desired.

---

## Theme Selection Algorithm

The selection is performed once per room generation, up to `MaxSpecialRooms` themed rooms:

1. Build a weighted list: each `URoomThemeDataAsset` contributes its `SelectionWeight`, plus one entry for "no theme" with weight `DefaultThemeWeight`.
2. Compute total weight.
3. Draw a random float in `[0, totalWeight)`.
4. Walk the list to find the selected entry.
5. Assign the theme (or null for "no theme") to `FRoom.RoomTheme`.

**Example weights:**
```
DefaultThemeWeight = 3.0
Theme_Crypt.SelectionWeight = 1.0
Theme_Library.SelectionWeight = 1.0
Total weight = 5.0

Probability breakdown:
  No theme:  3.0 / 5.0 = 60%
  Crypt:     1.0 / 5.0 = 20%
  Library:   1.0 / 5.0 = 20%
```

---

## Grid Placement System

`FGridPlacement` allows pattern-based mesh placement within a themed room — useful for pit floors, tiled mosaics, altar arrangements, and more.

### How it works

The room's grid dimensions (`RoomWidth × RoomHeight`) are computed from the tile positions. The `GridPattern` array is mapped onto the room's grid. For each cell where `GridPattern[index] == 1`, the `Mesh` is spawned.

### Creating a grid pattern

```
Example: 3×3 pit (center tile open, ring solid)

Pattern (9 values, width = 3):
[1,1,1,
 1,0,1,
 1,1,1]

→ GridPattern = [1,1,1,1,0,1,1,1,1]
→ GridWidth = 3
```

The pattern tiles/repeats across the full room dimensions if the room is larger than the pattern.

### Example: Altar room

```
GridPlacedMeshes[0]:
  Mesh = SM_Altar
  GridPattern = [0,0,0, 0,1,0, 0,0,0]
  GridWidth = 3
  LocationOffset = (0, 0, 0)
  Scale = (1, 1, 1)
```

This places one altar at the center of any room it is applied to.

---

## Theme Fallback Behavior

If a themed room's asset is missing a mesh (e.g., `FloorMesh = null`), the generator falls back to the generator's global mesh for that component:

| Theme property | Fallback |
|---|---|
| `FloorMesh = null` | `ADungeonGenerator::FloorSM` |
| `WallMesh = null` | `ADungeonGenerator::WallSM` |
| `CeilingMesh = null` | `ADungeonGenerator::CeilingSM` |

This allows themes to override only specific components (e.g., change only the floor material while keeping the same wall mesh).

---

## Theme Mixing Strategies

### Strategy 1 — Accent rooms (recommended)
Set `DefaultThemeWeight` high (3–5) and `MaxSpecialRooms` to 1–3. Most rooms are plain; 1–3 rooms are special.

```
DefaultThemeWeight = 4.0
MaxSpecialRooms = 2
```

### Strategy 2 — Themed majority
Lower `DefaultThemeWeight` so themed rooms are more common:

```
DefaultThemeWeight = 0.5
MaxSpecialRooms = 10
```

With multiple themes at `SelectionWeight = 1.0`, rooms are almost always themed.

### Strategy 3 — Single theme for all rooms
Set `DefaultThemeWeight = 0` and `MaxSpecialRooms` to a high number:

```
DefaultThemeWeight = 0.0
MaxSpecialRooms = 99
RoomThemes = [DA_Theme_Castle]
```

All rooms receive the Castle theme. Corridors always use global meshes.

### Strategy 4 — Multiple themes, equal distribution
```
DefaultThemeWeight = 1.0
MaxSpecialRooms = 8
RoomThemes = [DA_Theme_Crypt, DA_Theme_Library, DA_Theme_Armory]
```

Each theme has equal probability with the default.

---

## Data Asset Structure Reference

```
URoomThemeDataAsset
├── Theme Info
│   ├── ThemeName (FString)
│   ├── ThemeDescription (FText)
│   └── SelectionWeight (float, ≥ 0)
│
├── Theme Meshes
│   ├── FloorMesh + FloorMaterialOverride + FloorPivotOffset
│   ├── WallMesh + WallMaterialOverride + WallPivotOffset + bWallFacingX
│   └── CeilingMesh + CeilingMaterialOverride + CeilingPivotOffset
│
└── Theme Props
    ├── ThemeSpecificMeshes[]    (FStaticMeshSpawnParams)
    ├── ThemeSpecificBlueprints[] (FBlueprintSpawnParams)
    └── GridPlacedMeshes[]       (FGridPlacement)
```

---

## Advanced Techniques

### Dynamic Theme Switching

To change which theme pool is active at runtime (e.g., floor 1 uses caves, floor 2 uses crypts):

```cpp
// C++: swap theme array before generating
Generator->RoomThemes.Empty();
Generator->RoomThemes.Add(CryptThemeAsset);
Generator->MaxSpecialRooms = 5;
Generator->GenerateDungeon();
```

### Theme-Exclusive Corridors

Corridors always use global meshes. To give corridors a distinct look, assign different global `FloorSM`/`WallSM` values that contrast with your themed room overrides.

### Debugging Theme Assignment

Enable verbose logging and filter the Output Log for `SpawnThemedRoom`. Each themed room logs:

```
DungeonGenerator: SpawnThemedRoom: Applying theme 'Crypt' to room 3 (4 tiles)
```

If no such lines appear, check that `MaxSpecialRooms > 0` and themes have `SelectionWeight > 0`.
