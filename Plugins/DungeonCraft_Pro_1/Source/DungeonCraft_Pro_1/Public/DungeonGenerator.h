// DungeonGenerator.h
// BR_BETRAYAL_2025 - Enhanced dungeon generator with replication support

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Net/UnrealNetwork.h"
#include "DungeonGenerator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(DungeonGenerator, Log, All);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDungeonSpawned);

class AStaticMeshActor;
class UStaticMesh;
class UMaterialInterface;
class UDecalComponent;
class UDungeonConfigDataAsset;
class URoomThemeDataAsset; // Forward declaration for room theme system

USTRUCT(BlueprintType)
struct FStaticMeshSpawnParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Meshes")
    UStaticMesh* StaticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Meshes")
    UMaterialInterface* MaterialOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Meshes", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnChance = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Meshes")
    FVector LocationOffsetMin = FVector(0, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Meshes")
    FVector LocationOffsetMax = FVector(0, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Meshes")
    FRotator RotationOffset = FRotator(0, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Meshes")
    FVector Scale = FVector(1, 1, 1);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Meshes")
    bool bOnlySpawnInRooms = false;
};

// Blueprint spawn settings
USTRUCT(BlueprintType)
struct FBlueprintSpawnParams
{
    GENERATED_BODY()

    // The blueprint class to spawn
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint Spawning")
    TSubclassOf<AActor> BlueprintClass;

    // Chance to spawn (0.0 - 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint Spawning", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnChance = 0.1f;

    // Location offset from tile center
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint Spawning")
    FVector LocationOffsetMin = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint Spawning")
    FVector LocationOffsetMax = FVector::ZeroVector;

    // Rotation offset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint Spawning")
    FRotator RotationOffset = FRotator::ZeroRotator;

    // Scale to apply
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint Spawning")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    // Only spawn in rooms (not corridors)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint Spawning")
    bool bOnlySpawnInRooms = true;
};

// Decal spawn settings
USTRUCT(BlueprintType)
struct FDecalSpawnParams
{
    GENERATED_BODY()

    // The decal material to use
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal Spawning")
    UMaterialInterface* DecalMaterial;

    // Chance to spawn (0.0 - 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal Spawning", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnChance = 0.25f;

    // Decal size
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal Spawning")
    FVector DecalSize = FVector(200.0f, 200.0f, 200.0f);

    // Z offset from floor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal Spawning")
    float ZOffset = 2.0f;

    // Random rotation range (degrees)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal Spawning")
    float RandomRotationRange = 360.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decals")
    bool bOnlySpawnInRooms = false;
};

// Pillar spawn settings
USTRUCT(BlueprintType)
struct FPillarSpawnParams
{
    GENERATED_BODY()

    // The pillar mesh to use
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pillar Spawning")
    UStaticMesh* PillarMesh;

    // Chance to spawn (0.0 - 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pillar Spawning", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnChance = 0.75f;

    // Location offset from corner
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pillar Spawning")
    FVector LocationOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pillars")
    FRotator RotationOffset = FRotator(0, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pillars")
    FVector Scale = FVector(1, 1, 1);

    // Material override
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pillar Spawning")
    UMaterialInterface* MaterialOverride = nullptr;
};

// Room template with ceiling support
USTRUCT(BlueprintType)
struct FRoomTemplate : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
    UStaticMesh* RoomTileMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
    UMaterialInterface* RoomTileMeshMaterialOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
    FVector RoomTilePivotOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
    UStaticMesh* WallMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
    UMaterialInterface* WallMeshMaterialOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
    FVector WallMeshPivotOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
    bool bIsWallFacingX = true;

    // Ceiling mesh for this room type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
    UStaticMesh* CeilingMesh;

    // Ceiling material override
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RoomTemplate")
    UMaterialInterface* CeilingMaterialOverride;

    // Ceiling pivot offset
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties")
    UDungeonConfigDataAsset* DungeonConfig;

    // Should we use the config data asset?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties")
    bool bUseConfigDataAsset = false;

    // Sets default values for this actor's properties
    ADungeonGenerator();

    // Number of rows in tile map
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties", meta = (ClampMin = "10"))
    int32 TileMapRows = 50;

    // Number of columns in tile map
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties", meta = (ClampMin = "10"))
    int32 TileMapColumns = 50;

    // Minimum room size (in tiles)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties", meta = (ClampMin = "2"))
    int32 MinRoomSize = 5;

    // Maximum room size (in tiles)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties", meta = (ClampMin = "3"))
    int32 MaxRoomSize = 7;

    // Number of rooms to generate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties", meta = (ClampMin = "1"))
    int32 RoomsToGenerate = 15;

    // Maximum number of attempts to place each room
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties", meta = (ClampMin = "100"))
    int32 MaxRandomAttemptsPerRoom = 1500;

    // Use fixed seed for consistent generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties")
    bool bUseFixedSeed = false;

    // Fixed seed value (only used if bUseFixedSeed is true)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties", meta = (EditCondition = "bUseFixedSeed"))
    int32 FixedSeed = 12345;

    // Floor mesh settings
    UPROPERTY(EditAnywhere, Category = "Generator Properties - Floor Settings")
    UStaticMesh* FloorSM;

    // Floor tile size
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Floor Settings")
    float FloorTileSize = 400.0f;

    // Auto-calculate floor tile size from mesh
    UPROPERTY(EditAnywhere, Category = "Generator Properties - Floor Settings")
    bool bAutoFloorTileSizeGeneration = true;

    // Floor pivot offset
    UPROPERTY(EditAnywhere, Category = "Generator Properties - Floor Settings")
    FVector FloorPivotOffset = FVector::ZeroVector;

    // Wall mesh settings
    UPROPERTY(EditAnywhere, Category = "Generator Properties - Wall Settings")
    UStaticMesh* WallSM;

    // Wall width
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Wall Settings")
    float WallWidth = 20.0f;

    // Wall pivot offset
    UPROPERTY(EditAnywhere, Category = "Generator Properties - Wall Settings")
    FVector WallSMPivotOffset = FVector::ZeroVector;

    // Is wall facing X axis
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Wall Settings")
    bool bWallFacingX = true;

    // Ceiling mesh settings
    UPROPERTY(EditAnywhere, Category = "Generator Properties - Ceiling Settings")
    UStaticMesh* CeilingSM;

    // Ceiling height
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Ceiling Settings")
    float CeilingHeight = 400.0f;

    // Ceiling pivot offset
    UPROPERTY(EditAnywhere, Category = "Generator Properties - Ceiling Settings")
    FVector CeilingPivotOffset = FVector(0.0f, 0.0f, 400.0f);

    // Pillar settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Pillars")
    FPillarSpawnParams PillarSettings;

    // Blueprint spawning settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Props")
    TArray<FBlueprintSpawnParams> BlueprintActorsToSpawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Spawning")
    TArray<FStaticMeshSpawnParams> StaticMeshesToSpawn;

    // Floor decal settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Decals")
    TArray<FDecalSpawnParams> FloorDecalsToSpawn;

    // Room template data table
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties")
    UDataTable* RoomTemplatesDataTable;

    // Room theme system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Themes")
    TArray<URoomThemeDataAsset*> RoomThemes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Themes")
    float DefaultThemeWeight = 3.0f; // Standard rooms are more common

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Themes")
    int32 MaxSpecialRooms = 2; // Limit special themed rooms

    // Generate dungeon on Begin Play
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties")
    bool bGenerateOnBeginPlay = true;

    // Called when dungeon generator has finished spawning all meshes
    UPROPERTY(BlueprintAssignable, Category = "Dungeon Generation")
    FOnDungeonSpawned OnDungeonSpawned;

    // Generate the dungeon (can be called from Blueprint)
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Dungeon Generation")
    void GenerateDungeon();

    // Sets room size parameters
    UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
    void SetNewRoomSize(int32 NewMinRoomSize, int32 NewMaxRoomSize);

    // Sets new floor mesh properties
    UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
    void SetNewFloorMesh(UStaticMesh* NewFloorMesh, FVector NewFloorPivotOffset, bool bAutoFloorSizeGeneration = true, float OverrideFloorTileSize = 0.f);

    // Sets new wall mesh properties
    UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
    void SetNewWallMesh(UStaticMesh* NewWallMesh, FVector NewWallSMPivotOffset, bool bIsWallFacingX = true);

    // Sets new ceiling mesh properties
    UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
    void SetNewCeilingMesh(UStaticMesh* NewCeilingMesh, FVector NewCeilingPivotOffset, float NewCeilingHeight = 400.0f);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Dungeon Generator", meta = (DisplayName = "Clear Dungeon", Keywords = "dungeon clear delete reset", ButtonName = "Clear Dungeon"))
    void ClearDungeon();

#if WITH_EDITORONLY_DATA
    // Debug visualization settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Generation - Debug")
    bool bDebugActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Generation - Debug")
    FVector DebugVertexBoxExtents = FVector(10.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Generation - Debug")
    FLinearColor DefaultFloorSpawnLocationColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.5f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Generation - Debug")
    FLinearColor OffsetedFloorSpawnLocationColor = FLinearColor(0.0f, 0.5f, 0.0f, 0.5f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Generation - Debug")
    FLinearColor DefaultWallSpawnLocationColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.5f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Generation - Debug")
    FLinearColor OffsetedWallSpawnLocationColor = FLinearColor(0.5f, 0.0f, 0.0f, 0.5f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Generation - Debug")
    FLinearColor CornerPointColor = FLinearColor(0.0f, 0.0f, 1.0f, 0.5f);
#endif
};