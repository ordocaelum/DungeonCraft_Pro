# DungeonCraft Pro

A procedural dungeon generation plugin for **Unreal Engine 5.5**, built with full multiplayer replication support, a flexible theme system, and Blueprint-friendly API.

---

## Features

- **Procedural Generation** — Randomized rooms and corridors using a tile-matrix BSP-inspired algorithm
- **Deterministic Seeds** — Fixed or random seeds ensure consistent dungeon layouts, including across networked clients
- **Theme System** — Assign `URoomThemeDataAsset` per room type with custom floor, wall, ceiling meshes and props
- **Blueprint Actor Spawning** — Spawn any Blueprint class inside dungeon rooms with configurable chance, offset, and scale
- **Static Mesh Props** — Scatter meshes across floors with per-entry spawn chances, rotations, and material overrides
- **Floor Decals** — Procedurally place decal actors with random rotation and controlled spawn density
- **Pillar System** — Auto-detect room corners and optionally spawn pillar meshes
- **Ceiling Meshes** — Place ceiling tiles above every floor tile, height-configurable
- **Data Asset Config** — Drive the entire generator from a `UDungeonConfigDataAsset` without touching C++
- **Multiplayer Ready** — Server-authoritative seed replication with `OnRep_DungeonSeed` client callback
- **Debug Visualization** — Editor-only draw-debug helpers for floor/wall/corner spawn points
- **Memory Profiling** — Built-in `RunGenerationCycles()` stress-test for leak detection
- **Comprehensive Logging** — All operations use a dedicated `DungeonGenerator` log category

---

## Requirements

| Requirement | Version |
|---|---|
| Unreal Engine | 5.5 or later |
| C++ | Basic familiarity (for config, not required for Blueprint use) |
| Visual Studio / Rider | Any UE5-compatible version |
| Target Platform | Windows, Mac, Linux (console untested) |

---

## Quick Installation

1. Copy the `Plugins/DungeonCraft_Pro_1` folder into your project's `Plugins/` directory.
2. Right-click your `.uproject` file → **Generate Visual Studio project files**.
3. Open the project in the editor; it will prompt you to compile the plugin — click **Yes**.
4. Enable the plugin via **Edit → Plugins → DungeonCraft Pro**.
5. Add an `ADungeonGenerator` actor to your level and configure it in the Details panel.

> For a step-by-step first dungeon walkthrough, see **[QUICKSTART.md](QUICKSTART.md)**.

---

## Basic Usage

### Blueprint

1. Drag a `DungeonGenerator` actor into your level.
2. Assign a `FloorSM` and `WallSM` in the Details panel.
3. Press **Play** — the dungeon generates automatically on `BeginPlay`.
4. Call `GenerateDungeon()` from any Blueprint to regenerate at runtime.

### C++

```cpp
// Spawn and configure via code
ADungeonGenerator* Gen = GetWorld()->SpawnActor<ADungeonGenerator>();
Gen->TileMapRows = 40;
Gen->TileMapColumns = 40;
Gen->RoomsToGenerate = 10;
Gen->FloorSM = MyFloorMesh;
Gen->WallSM = MyWallMesh;
Gen->GenerateDungeon();

// Listen for completion
Gen->OnDungeonSpawned.AddDynamic(this, &AMyClass::OnDungeonReady);
```

---

## Configuration Overview

All parameters can be set directly on the actor or via a `UDungeonConfigDataAsset`:

| Category | Key Parameters |
|---|---|
| Grid | `TileMapRows`, `TileMapColumns` |
| Rooms | `MinRoomSize`, `MaxRoomSize`, `RoomsToGenerate` |
| Meshes | `FloorSM`, `WallSM`, `CeilingSM` |
| Props | `BlueprintActorsToSpawn`, `StaticMeshesToSpawn` |
| Themes | `RoomThemes`, `DefaultThemeWeight`, `MaxSpecialRooms` |
| Seed | `bUseFixedSeed`, `FixedSeed` |

> Full configuration reference: **[CONFIGURATION.md](CONFIGURATION.md)**

---

## Documentation

| Document | Description |
|---|---|
| [QUICKSTART.md](QUICKSTART.md) | Get your first dungeon running in under 10 minutes |
| [API.md](API.md) | Complete public API reference |
| [CONFIGURATION.md](CONFIGURATION.md) | All configuration options with valid ranges and defaults |
| [TROUBLESHOOTING.md](TROUBLESHOOTING.md) | Diagnose and fix common issues |
| [PERFORMANCE.md](PERFORMANCE.md) | Profiling, tuning, and scaling guidelines |
| [MULTIPLAYER.md](MULTIPLAYER.md) | Server/client setup, replication, and best practices |
| [THEMES.md](THEMES.md) | Theme system architecture and custom theme creation |
| [VERSION.md](VERSION.md) | Changelog, known limitations, and roadmap |
| [CONTRIBUTING.md](CONTRIBUTING.md) | How to contribute code, docs, or bug reports |

---

## Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) before opening a pull request.

- Bug reports → open a GitHub Issue with the **bug** label
- Feature requests → open a GitHub Issue with the **enhancement** label
- Code changes → fork, branch, implement, and submit a PR targeting `main`

---

## License

This project is released under the **MIT License**. See `LICENSE` for details.

---

## Credits

- **BR_BETRAYAL_2025** — Original author and lead developer
- Unreal Engine 5 community — Inspiration and best-practice patterns
