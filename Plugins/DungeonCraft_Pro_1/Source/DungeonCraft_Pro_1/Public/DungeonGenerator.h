// DungeonGenerator.h
// BR_BETRAYAL_2025 - Enhanced dungeon generator with replication support

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Net/UnrealNetwork.h"
#include "DungeonGenerator.generated.h"

/** Log category for all DungeonCraft Pro output. Filter with "DungeonGenerator" in the Output Log. */
DECLARE_LOG_CATEGORY_EXTERN(DungeonGenerator, Log, All);

/**
 * Multicast delegate broadcast after all dungeon meshes and actors have been spawned.
 *
 * Bind in Blueprint via the OnDungeonSpawned event on the ADungeonGenerator actor,
 * or in C++ with Generator->OnDungeonSpawned.AddDynamic(this, &AMyClass::HandleDungeonSpawned).
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDungeonSpawned);

class AStaticMeshActor;
class UStaticMesh;
class UMaterialInterface;
class UDecalComponent;
class UDungeonConfigDataAsset;
class URoomThemeDataAsset; // Forward declaration for room theme system

/**
 * Parameters for scattering a static mesh prop across dungeon floor tiles.
 *
 * Each entry in ADungeonGenerator::StaticMeshesToSpawn is evaluated independently
 * for every floor tile. The actual spawn position is a uniformly random point
 * between LocationOffsetMin and LocationOffsetMax, added to the tile center.
 *
 * Performance note: each entry multiplies spawning work by total floor tile count.
 * Keep SpawnChance low (0.02–0.10) and limit the array to 5–10 entries for
 * acceptable performance on mid-range hardware.
 */
USTRUCT(BlueprintType)
struct FStaticMeshSpawnParams
{
    GENERATED_BODY()

    /** Static mesh asset to spawn. Entry is silently skipped when null. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Meshes")
    UStaticMesh* StaticMesh;

    /** Optional material applied to slot 0 of the spawned mesh. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Meshes")
    UMaterialInterface* MaterialOverride;

    /** Per-tile spawn probability. 0.0 = never, 1.0 = always. Valid range: 0.0–1.0. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Meshes", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnChance = 0.1f;

    /** Minimum random world-space offset from tile center. Combined with LocationOffsetMax for a uniform random range. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Meshes")
    FVector LocationOffsetMin = FVector(0, 0, 0);

    /** Maximum random world-space offset from tile center. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Meshes")
    FVector LocationOffsetMax = FVector(0, 0, 0);

    /** Base rotation added to the spawned mesh instance. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Meshes")
    FRotator RotationOffset = FRotator(0, 0, 0);

    /** Scale applied to the spawned mesh. (1,1,1) = no change. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Meshes")
    FVector Scale = FVector(1, 1, 1);

    /** When true, this mesh is never placed on corridor tiles — only on room floor tiles. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Meshes")
    bool bOnlySpawnInRooms = false;
};

/**
 * Parameters for spawning a Blueprint actor class across dungeon floor tiles.
 *
 * Evaluated per floor tile during generation. Spawned actors are placed in the
 * world with a random offset in [LocationOffsetMin, LocationOffsetMax] and tagged
 * with "DungeonBP" so ClearDungeon() can find and destroy them.
 *
 * Cleanup requirement: The Blueprint class itself MUST add the tag "DungeonBP"
 * to its Tags array (Actor defaults), or ClearDungeon() will not remove it.
 *
 * Multiplayer note: Actors are spawned on the server only. Set bReplicates = true
 * on the actor class if clients need to see it.
 */
// Blueprint spawn settings
USTRUCT(BlueprintType)
struct FBlueprintSpawnParams
{
    GENERATED_BODY()

    /** Actor class to spawn. Entry is silently skipped when null. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint Spawning")
    TSubclassOf<AActor> BlueprintClass;

    /** Per-tile spawn probability. 0.0 = never, 1.0 = always. Valid range: 0.0–1.0. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint Spawning", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnChance = 0.1f;

    /** Minimum random world-space offset from tile center. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint Spawning")
    FVector LocationOffsetMin = FVector::ZeroVector;

    /** Maximum random world-space offset from tile center. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint Spawning")
    FVector LocationOffsetMax = FVector::ZeroVector;

    /** Base rotation applied to the spawned actor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint Spawning")
    FRotator RotationOffset = FRotator::ZeroRotator;

    /** Scale applied to the spawned actor's root component. (1,1,1) = no change. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint Spawning")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    /** When true, this actor only spawns on room floor tiles, never on corridor tiles. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint Spawning")
    bool bOnlySpawnInRooms = true;
};

/**
 * Parameters for placing floor decal actors across dungeon floor tiles.
 *
 * The decal material must use the Deferred Decal material domain. Yaw rotation
 * is randomized within [0, RandomRotationRange] degrees for organic variation.
 */
// Decal spawn settings
USTRUCT(BlueprintType)
struct FDecalSpawnParams
{
    GENERATED_BODY()

    /** Decal material asset. Must use the Deferred Decal material domain. Entry is skipped when null. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal Spawning")
    UMaterialInterface* DecalMaterial;

    /** Per-tile spawn probability. 0.0 = never, 1.0 = always. Valid range: 0.0–1.0. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal Spawning", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnChance = 0.25f;

    /** Size of the decal projection box in world units. Default (200,200,200) covers a standard tile. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal Spawning")
    FVector DecalSize = FVector(200.0f, 200.0f, 200.0f);

    /** Z offset above the floor tile surface where the decal origin is placed. Increase to avoid z-fighting. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal Spawning")
    float ZOffset = 2.0f;

    /** Yaw randomization range in degrees. 360.0 = fully random rotation. 0.0 = no rotation. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal Spawning")
    float RandomRotationRange = 360.0f;

    /** When true, decals only appear on room floor tiles, not corridors. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decals")
    bool bOnlySpawnInRooms = false;
};

/**
 * Parameters for spawning pillar meshes at detected room corners.
 *
 * The generator detects inner corners of room boundaries. Each corner receives
 * an independent spawn chance roll. The entire pillar system is disabled when
 * PillarMesh is null.
 */
// Pillar spawn settings
USTRUCT(BlueprintType)
struct FPillarSpawnParams
{
    GENERATED_BODY()

    /** Pillar mesh asset. The entire pillar system is disabled when null. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pillar Spawning")
    UStaticMesh* PillarMesh;

    /** Per-corner spawn probability. 0.0 = never, 1.0 = always. Default 0.75. Valid range: 0.0–1.0. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pillar Spawning", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnChance = 0.75f;

    /** World-space offset applied to each pillar from the exact corner position. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pillar Spawning")
    FVector LocationOffset = FVector::ZeroVector;

    /** Rotation applied to each pillar. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pillars")
    FRotator RotationOffset = FRotator(0, 0, 0);

    /** Scale applied to each pillar. (1,1,1) = no change. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pillars")
    FVector Scale = FVector(1, 1, 1);

    /** Optional material override applied to slot 0 of the pillar mesh. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pillar Spawning")
    UMaterialInterface* MaterialOverride = nullptr;
};

/**
 * Data table row type for per-room-type mesh configuration.
 *
 * Create a UDataTable with this row type and assign it to
 * ADungeonGenerator::RoomTemplatesDataTable to enable room-type mesh variation.
 * Each row represents a distinct room visual variant with independent floor,
 * wall, and ceiling meshes and materials.
 *
 * @see ADungeonGenerator::RoomTemplatesDataTable
 */
// Room template with ceiling support
USTRUCT(BlueprintType)
struct FRoomTemplate : public FTableRowBase
{
    GENERATED_BODY()

    /** Floor tile mesh for this room variant. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
    UStaticMesh* RoomTileMesh;

    /** Optional material override for the floor tile mesh. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
    UMaterialInterface* RoomTileMeshMaterialOverride;

    /** Pivot correction offset for the floor tile mesh. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
    FVector RoomTilePivotOffset;

    /** Wall mesh for this room variant. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
    UStaticMesh* WallMesh;

    /** Optional material override for the wall mesh. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
    UMaterialInterface* WallMeshMaterialOverride;

    /** Pivot correction offset for the wall mesh. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
    FVector WallMeshPivotOffset;

    /** Whether the wall mesh faces the X axis (true) or Y axis (false). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
    bool bIsWallFacingX = true;

    /** Ceiling mesh for this room variant. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
    UStaticMesh* CeilingMesh;

    /** Optional material override for the ceiling mesh. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
    UMaterialInterface* CeilingMaterialOverride;

    /** Pivot correction offset for the ceiling mesh. Default Z of 400 suits a standard tile-height ceiling. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
    FVector CeilingPivotOffset = FVector(0.0f, 0.0f, 400.0f);
};

// Corner definition for pillar placement
struct FCornerPoint
{
    FVector WorldLocation;

    FCornerPoint() : WorldLocation(FVector::ZeroVector) {}
    FCornerPoint(FVector InWorldLocation) : WorldLocation(InWorldLocation) {}
};

// Tile definition with enhanced features
struct FTile
{
    TTuple<int32, int32> Coordinates;
    bool bIsRoom;  // Whether this tile belongs to a room or corridor
    int32 RoomID;  // -1 for corridors, room index for room tiles

    FTile() : Coordinates(TTuple<int32, int32>(0, 0)), bIsRoom(false), RoomID(-1) {}
    FTile(int32 Row, int32 Column, bool InIsRoom = false, int32 InRoomID = -1)
        : Coordinates(TTuple<int32, int32>(Row, Column)), bIsRoom(InIsRoom), RoomID(InRoomID) {
    }
};

// Wall spawn point definition
struct FWallSpawnPoint
{
    FVector WorldLocation;
    bool bFacingX;
    bool bIsCorner;

    FWallSpawnPoint() : WorldLocation(FVector::ZeroVector), bFacingX(true), bIsCorner(false) {}
    FWallSpawnPoint(FVector NewWorldLocation, bool IsFacingX = true)
        : WorldLocation(NewWorldLocation), bFacingX(IsFacingX), bIsCorner(false) {
    }
};

// Room definition
struct FRoom
{
    TArray<FVector> FloorTileWorldLocations;
    TArray<FWallSpawnPoint> WallSpawnPoints;
    TArray<FCornerPoint> CornerPoints;

    // Room Theme Data
    URoomThemeDataAsset* RoomTheme = nullptr;

    // Grid dimensions for organized prop placement
    int32 MinGridX = 0;
    int32 MinGridY = 0;
    int32 RoomWidth = 0;
    int32 RoomHeight = 0;
};

/**
 * Procedural dungeon generator actor for Unreal Engine 5.
 *
 * Place one instance per level. The generator creates a tile-matrix grid,
 * places randomly sized rooms, connects them with corridors, then spawns all
 * floor, wall, ceiling, prop, decal, and Blueprint actors.
 *
 * Configuration can be supplied directly via the Details panel or driven from
 * a UDungeonConfigDataAsset when bUseConfigDataAsset is true.
 *
 * Multiplayer: The actor replicates. Generation should only be triggered on the
 * server (HasAuthority()). The DungeonSeed is replicated to clients, which then
 * independently generate an identical dungeon from the same seed.
 *
 * @see UDungeonConfigDataAsset, URoomThemeDataAsset, FOnDungeonSpawned
 */
// Main dungeon generator class
UCLASS()
class DUNGEONCRAFT_PRO_1_API ADungeonGenerator : public AActor
{
    GENERATED_BODY()

private:
    void ApplyConfigFromDataAsset();

    // Validates all generator config values. Returns true if config is valid and generation may proceed.
    // Logs errors for critical issues (blocks generation) and warnings for auto-corrected values.
    bool ValidateConfig();

    bool WallExistsAnywhere(const FVector& WallPos, bool bFacingX) const;

    void AddWallsAroundTile(const FVector& FloorPos, TArray<FWallSpawnPoint>& OutWalls);

    bool HasFloorTileAt(const FVector& Location) const;
    void EnsureWallSpawnPoints();

    // Guard flag to prevent re-entrant dungeon generation
    bool bIsGenerating = false;
    bool bAllowClientGenerationFromReplication = false;

    // Logs current memory usage to the output log with a contextual label
    void LogMemoryUsage(const FString& Context) const;

    // Room theme helpers
    void CalculateRoomGridDimensions(FRoom& Room);
    void SpawnThemedRoom(const FRoom& Room);

    // Corridor generation
    FVector FindClosestFloorTile(const FRoom& FromRoom, const FRoom& ToRoom);
    void GenerateCorridor(const FVector& Start, const FVector& End);
    void GenerateCorridorWalls();

    // The matrix of tiles representing the dungeon
    TArray<TArray<FTile>> TileMatrix;

    // Generated rooms storage
    TArray<FRoom> GeneratedRooms;

    // Corridor data
    TArray<FVector> CorridorFloorLocations;
    TArray<FWallSpawnPoint> CorridorWalls;
    TArray<FCornerPoint> CorridorCorners;

    // Mesh tags
    static const FName DUNGEON_MESH_TAG;
    static const FName CEILING_MESH_TAG;
    static const FName PILLAR_MESH_TAG;
    static const FName PROP_MESH_TAG;
    static const FName DUNGEON_BP_TAG;


    // Seed for deterministic generation
    UPROPERTY(ReplicatedUsing = OnRep_DungeonSeed)
    int32 DungeonSeed;

    // Method to spawn a dungeon mesh
    AStaticMeshActor* SpawnDungeonMesh(const FTransform& InTransform, UStaticMesh* SMToSpawn, UMaterialInterface* OverrideMaterial = nullptr);

    // Calculate floor tile size from mesh bounds
    float CalculateFloorTileSize(const UStaticMesh& Mesh) const;

    // Calculate wall rotation based on facing direction
    FRotator CalculateWallRotation(bool bWallFacingXProperty, const FWallSpawnPoint& WallSpawnPoint, const FVector& WallPivotOffsetOverride, FVector& LocationOffset) const;

    // Destroy existing dungeon meshes
    void DestroyDungeonMeshes();

    // Spawn dungeon from data table
    void SpawnDungeonFromDataTable();

    // Spawn generic dungeon (no data table)
    void SpawnGenericDungeon(const TArray<FVector>& FloorTileLocations, const TArray<FWallSpawnPoint>& WallSpawnPoints);

    // Detect corners for pillar placement
    void DetectCorners();

    // Spawn pillars at corners
    void SpawnPillars();

    // Spawn ceiling meshes
    void SpawnCeilingMeshes(const TArray<FVector>& FloorLocations);

    // Spawn decals on floors
    void SpawnFloorDecals(const TArray<FVector>& FloorLocations, bool bIsRoom);

    // Spawn blueprint actors
    void SpawnBlueprintActors(const TArray<FVector>& FloorLocations, bool bIsRoom);

    // Function to spawn static meshes
    void SpawnStaticMeshes(const TArray<FVector>& FloorLocations, bool bIsRoom);

    // Helper functions for tile matrix generation
    void InitTileMatrix();
    void GenerateRooms();
    void ConnectRooms();
    bool IsTileOccupied(int32 Row, int32 Column) const;
    bool CanPlaceRoom(int32 StartRow, int32 StartCol, int32 Width, int32 Height) const;
    void OccupyTiles(int32 StartRow, int32 StartCol, int32 Width, int32 Height, int32 RoomID);

    // Replication callback
    UFUNCTION()
    void OnRep_DungeonSeed();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Replication setup
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#if WITH_EDITOR
    // Called when a property is changed in the editor
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
    /** Optional data asset containing all generator settings. Applied when bUseConfigDataAsset is true. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties")
    UDungeonConfigDataAsset* DungeonConfig;

    /** When true, all generator properties are overridden by values from DungeonConfig. DungeonConfig must be non-null. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties")
    bool bUseConfigDataAsset = false;

    /** Default constructor. Initializes all properties to their documented defaults. */
    ADungeonGenerator();

    /** Number of tile rows in the generation grid (Y extent). Minimum: 10. Large values increase generation time quadratically. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties", meta = (ClampMin = "10"))
    int32 TileMapRows = 50;

    /** Number of tile columns in the generation grid (X extent). Minimum: 10. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties", meta = (ClampMin = "10"))
    int32 TileMapColumns = 50;

    /** Minimum room dimension in tiles (both width and height). Minimum: 2. Must be < MaxRoomSize. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties", meta = (ClampMin = "2"))
    int32 MinRoomSize = 5;

    /** Maximum room dimension in tiles. Minimum: 3. Must be > MinRoomSize. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties", meta = (ClampMin = "3"))
    int32 MaxRoomSize = 7;

    /** Target number of rooms to generate. The actual count may be lower if the grid is too small. Minimum: 1. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties", meta = (ClampMin = "1"))
    int32 RoomsToGenerate = 15;

    /** Maximum placement retries for each room before it is abandoned. Increase for denser grids. Minimum: 100. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties", meta = (ClampMin = "100"))
    int32 MaxRandomAttemptsPerRoom = 1500;

    /** When true, FixedSeed is used for generation instead of a random seed. Enables deterministic/reproducible layouts. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties")
    bool bUseFixedSeed = false;

    /** Seed value used when bUseFixedSeed is true. In multiplayer, always use server-generated seeds (leave bUseFixedSeed false). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties", meta = (EditCondition = "bUseFixedSeed"))
    int32 FixedSeed = 12345;

    /** Floor tile static mesh. Required — generation is blocked when null. */
    UPROPERTY(EditAnywhere, Category = "Generator Properties - Floor Settings")
    UStaticMesh* FloorSM;

    /**
     * World-unit size of one floor tile side. Used for grid layout calculations.
     * Automatically derived from FloorSM bounds when bAutoFloorTileSizeGeneration is true.
     * Typical values: 100, 200, 400 (matching Unreal's default grid). Default: 400.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Floor Settings")
    float FloorTileSize = 400.0f;

    /** When true, FloorTileSize is computed from FloorSM's X-axis bounding box extent. Recommended for most setups. */
    UPROPERTY(EditAnywhere, Category = "Generator Properties - Floor Settings")
    bool bAutoFloorTileSizeGeneration = true;

    /** Pivot correction offset applied to each floor tile spawn. Use when the mesh pivot is not at the tile center. */
    UPROPERTY(EditAnywhere, Category = "Generator Properties - Floor Settings")
    FVector FloorPivotOffset = FVector::ZeroVector;

    /** Wall static mesh. Walls are skipped entirely when null. */
    UPROPERTY(EditAnywhere, Category = "Generator Properties - Wall Settings")
    UStaticMesh* WallSM;

    /** Thickness of the wall mesh in world units. Used to offset walls inward from tile edges. Default: 20. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Wall Settings")
    float WallWidth = 20.0f;

    /** Pivot correction offset for the wall mesh. */
    UPROPERTY(EditAnywhere, Category = "Generator Properties - Wall Settings")
    FVector WallSMPivotOffset = FVector::ZeroVector;

    /** True if the wall mesh's outward-facing surface points in the +X direction. False for +Y-facing meshes. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Wall Settings")
    bool bWallFacingX = true;

    /** Ceiling tile static mesh. Ceiling tiles are skipped when null. */
    UPROPERTY(EditAnywhere, Category = "Generator Properties - Ceiling Settings")
    UStaticMesh* CeilingSM;

    /** Z offset above each floor tile where ceiling tiles are placed. Should match FloorTileSize for standard rooms. Default: 400. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Ceiling Settings")
    float CeilingHeight = 400.0f;

    /** Pivot correction offset for ceiling tile meshes. Default Z of 400 suits a 1:1 floor/ceiling tile pair. */
    UPROPERTY(EditAnywhere, Category = "Generator Properties - Ceiling Settings")
    FVector CeilingPivotOffset = FVector(0.0f, 0.0f, 400.0f);

    /** Configuration for pillar mesh placement at detected room corners. Pillars are disabled when PillarSettings.PillarMesh is null. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Pillars")
    FPillarSpawnParams PillarSettings;

    /**
     * Blueprint actor classes to scatter across dungeon floor tiles.
     * Each entry is evaluated independently per tile. Keep SpawnChance low (0.02–0.05)
     * to avoid over-population. Actors must add the "DungeonBP" tag for cleanup.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Props")
    TArray<FBlueprintSpawnParams> BlueprintActorsToSpawn;

    /**
     * Static mesh props to scatter across dungeon floor tiles.
     * Each entry is evaluated per tile. Recommended: 5–10 entries with SpawnChance 0.02–0.10.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Spawning")
    TArray<FStaticMeshSpawnParams> StaticMeshesToSpawn;

    /**
     * Floor decal definitions to scatter across dungeon floor tiles.
     * DecalMaterial must use the Deferred Decal material domain.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Decals")
    TArray<FDecalSpawnParams> FloorDecalsToSpawn;

    /**
     * Optional data table of FRoomTemplate rows for per-room-type mesh variation.
     * When set, each room is assigned a random row from the table for mesh selection.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties")
    UDataTable* RoomTemplatesDataTable;

    /**
     * Pool of room theme data assets. Rooms are assigned themes from this pool up to MaxSpecialRooms.
     * Theme selection uses weighted random drawing. @see DefaultThemeWeight, MaxSpecialRooms.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Themes")
    TArray<URoomThemeDataAsset*> RoomThemes;

    /**
     * Relative weight for unthemed (plain) rooms in the theme selection pool.
     * Higher values produce more plain rooms relative to themed rooms.
     * Default: 3.0 (plain rooms are 3× more likely than any single theme at weight 1.0).
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Themes")
    float DefaultThemeWeight = 3.0f;

    /**
     * Maximum number of rooms that receive a URoomThemeDataAsset assignment.
     * Rooms beyond this cap always use global floor/wall/ceiling meshes.
     * Set to 0 to disable the theme system entirely.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Themes")
    int32 MaxSpecialRooms = 2;

    /** When true, GenerateDungeon() is called automatically in BeginPlay. Disable for manual or delayed generation. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties")
    bool bGenerateOnBeginPlay = true;

    /** Broadcast after all dungeon meshes, props, decals, and Blueprint actors have been spawned. */
    UPROPERTY(BlueprintAssignable, Category = "Dungeon Generation")
    FOnDungeonSpawned OnDungeonSpawned;

    /**
     * Triggers a full dungeon generation cycle.
     *
     * Execution order:
     *   1. ValidateConfig() — aborts with errors logged if invalid
     *   2. ClearDungeon() — destroys any previous dungeon
     *   3. InitTileMatrix() + GenerateRooms() + ConnectRooms()
     *   4. SpawnDungeonFromDataTable() or SpawnGenericDungeon()
     *   5. SpawnThemedRoom() for each themed room
     *   6. SpawnPillars(), SpawnCeilingMeshes(), SpawnBlueprintActors(), SpawnStaticMeshes(), SpawnFloorDecals()
     *   7. Broadcasts OnDungeonSpawned
     *
     * Must be called on the game thread. Re-entrant calls are silently dropped (guarded by bIsGenerating).
     * In multiplayer, call only on the server (HasAuthority()). Clients regenerate automatically via OnRep_DungeonSeed.
     *
     * @see ClearDungeon, OnDungeonSpawned, bUseFixedSeed
     */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Dungeon Generation")
    void GenerateDungeon();

    /**
     * Updates room size constraints at runtime. Call GenerateDungeon() afterwards to apply.
     *
     * Both values are clamped: NewMinRoomSize is clamped to [2, NewMaxRoomSize-1],
     * NewMaxRoomSize is clamped to [NewMinRoomSize+1, INT_MAX].
     *
     * @param NewMinRoomSize  New minimum room dimension in tiles. Clamped to >= 2.
     * @param NewMaxRoomSize  New maximum room dimension in tiles. Clamped to > NewMinRoomSize.
     */
    UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
    void SetNewRoomSize(int32 NewMinRoomSize, int32 NewMaxRoomSize);

    /**
     * Replaces the floor mesh and recalculates tile size.
     *
     * @param NewFloorMesh              New floor static mesh asset.
     * @param NewFloorPivotOffset       Pivot correction for the new mesh.
     * @param bAutoFloorSizeGeneration  When true, tile size is derived from mesh bounds.
     * @param OverrideFloorTileSize     Manual tile size used when bAutoFloorSizeGeneration is false.
     */
    UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
    void SetNewFloorMesh(UStaticMesh* NewFloorMesh, FVector NewFloorPivotOffset, bool bAutoFloorSizeGeneration = true, float OverrideFloorTileSize = 0.f);

    /**
     * Replaces the wall mesh reference.
     *
     * @param NewWallMesh          New wall static mesh asset.
     * @param NewWallSMPivotOffset Pivot correction for the new wall mesh.
     * @param bIsWallFacingX       True if the new mesh faces the +X axis.
     */
    UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
    void SetNewWallMesh(UStaticMesh* NewWallMesh, FVector NewWallSMPivotOffset, bool bIsWallFacingX = true);

    /**
     * Replaces the ceiling mesh reference.
     *
     * @param NewCeilingMesh          New ceiling static mesh asset.
     * @param NewCeilingPivotOffset   Pivot correction for the new ceiling mesh.
     * @param NewCeilingHeight        Z offset for ceiling tile placement.
     */
    UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
    void SetNewCeilingMesh(UStaticMesh* NewCeilingMesh, FVector NewCeilingPivotOffset, float NewCeilingHeight = 400.0f);

    /**
     * Destroys all actors spawned by the previous GenerateDungeon() call.
     *
     * Identifies actors by internal tags: BR_BETRAYAL_DungeonTile, BR_BETRAYAL_CeilingTile,
     * BR_BETRAYAL_Prop, and DungeonBP. Falls back to name-based search for Blueprint actors
     * if the tag search returns zero results.
     *
     * Safe to call even if no dungeon has been generated yet.
     */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Dungeon Generator", meta = (DisplayName = "Clear Dungeon", Keywords = "dungeon clear delete reset", ButtonName = "Clear Dungeon"))
    void ClearDungeon();

    /**
     * Stress test: runs NumCycles full generate-then-destroy cycles, logging timing and memory stats.
     *
     * Use this in editor (via the CallInEditor button) or in automated tests to:
     *   - Detect memory leaks (non-zero memory delta across cycles indicates a leak)
     *   - Benchmark generation time for the current configuration
     *   - Validate cleanup correctness
     *
     * Example output:
     *   DungeonGenerator: Cycle 5/10 completed in 43ms. Memory delta: +0 MB
     *
     * @param NumCycles  Number of generate/destroy cycles to run. Default: 10.
     */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Dungeon Generation|Debug")
    void RunGenerationCycles(int32 NumCycles = 10);

#if WITH_EDITORONLY_DATA
    /** When true, draws debug boxes in the editor viewport at floor, wall, and corner spawn points. Editor-only. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Generation - Debug")
    bool bDebugActive = false;

    /** Size of debug visualization boxes drawn at each spawn point. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Generation - Debug")
    FVector DebugVertexBoxExtents = FVector(10.0f);

    /** Color used to draw floor tile spawn points before offset is applied. Default: green. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Generation - Debug")
    FLinearColor DefaultFloorSpawnLocationColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.5f);

    /** Color used to draw floor tile spawn points after offset is applied. Default: dark green. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Generation - Debug")
    FLinearColor OffsetedFloorSpawnLocationColor = FLinearColor(0.0f, 0.5f, 0.0f, 0.5f);

    /** Color used to draw wall spawn points before offset is applied. Default: red. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Generation - Debug")
    FLinearColor DefaultWallSpawnLocationColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.5f);

    /** Color used to draw wall spawn points after offset is applied. Default: dark red. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Generation - Debug")
    FLinearColor OffsetedWallSpawnLocationColor = FLinearColor(0.5f, 0.0f, 0.0f, 0.5f);

    /** Color used to draw detected corner points (pillar spawn positions). Default: blue. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Generation - Debug")
    FLinearColor CornerPointColor = FLinearColor(0.0f, 0.0f, 1.0f, 0.5f);
#endif
};