// DungeonConfigDataAsset.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DungeonConfigDataAsset.generated.h"

struct FBlueprintSpawnParams;
struct FStaticMeshSpawnParams;
struct FDecalSpawnParams;


/**
 * Data asset that stores complete dungeon generation configuration
 */
UCLASS(BlueprintType)
class DUNGEONCRAFT_PRO_1_API UDungeonConfigDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    // Generation properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties")
    int32 TileMapRows = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties")
    int32 TileMapColumns = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties", meta = (ClampMin = "2"))
    int32 MinRoomSize = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties", meta = (ClampMin = "3"))
    int32 MaxRoomSize = 7;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties", meta = (ClampMin = "1"))
    int32 RoomsToGenerate = 15;

    // Floor settings
    UPROPERTY(EditAnywhere, Category = "Generator Properties - Floor Settings")
    UStaticMesh* FloorSM;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Floor Settings")
    float FloorTileSize = 400.0f;

    UPROPERTY(EditAnywhere, Category = "Generator Properties - Floor Settings")
    bool bAutoFloorTileSizeGeneration = true;

    UPROPERTY(EditAnywhere, Category = "Generator Properties - Floor Settings")
    FVector FloorPivotOffset = FVector::ZeroVector;

    // Wall settings
    UPROPERTY(EditAnywhere, Category = "Generator Properties - Wall Settings")
    UStaticMesh* WallSM;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Wall Settings")
    float WallWidth = 20.0f;

    UPROPERTY(EditAnywhere, Category = "Generator Properties - Wall Settings")
    FVector WallSMPivotOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Wall Settings")
    bool bWallFacingX = true;

    // Ceiling settings
    UPROPERTY(EditAnywhere, Category = "Generator Properties - Ceiling Settings")
    UStaticMesh* CeilingSM;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Ceiling Settings")
    float CeilingHeight = 400.0f;

    UPROPERTY(EditAnywhere, Category = "Generator Properties - Ceiling Settings")
    FVector CeilingPivotOffset = FVector(0.0f, 0.0f, 400.0f);

    // Use the EXISTING structs from DungeonGenerator.h
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Props")
    TArray<FBlueprintSpawnParams> BlueprintActorsToSpawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Spawning")
    TArray<FStaticMeshSpawnParams> StaticMeshesToSpawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator Properties - Decals")
    TArray<FDecalSpawnParams> FloorDecalsToSpawn;
};