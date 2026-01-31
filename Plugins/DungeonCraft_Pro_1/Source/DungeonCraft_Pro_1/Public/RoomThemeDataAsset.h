// RoomThemeDataAsset.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DungeonGenerator.h" // For spawn param structs
#include "RoomThemeDataAsset.generated.h"

/**
 * Structure for grid-based mesh placement (supports patterns like pits)
 */
USTRUCT(BlueprintType)
struct FGridPlacement
{
    GENERATED_BODY()

    // The mesh to place
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* Mesh;

    // Material override
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* MaterialOverride;

    // Flat grid pattern data (values should be 0 or 1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<uint8> GridPattern;

    // Grid width (for interpreting the flat array as 2D)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
    int32 GridWidth = 1;

    // Grid height (computed from total size and width)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 GridHeight = 1;

    // Location offset
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LocationOffset;

    // Rotation
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation;

    // Scale
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Scale = FVector(1, 1, 1);

    // Helper function to get grid value
    uint8 GetGridValue(int32 X, int32 Y) const
    {
        if (GridWidth <= 0)
            return 0;

        // Calculate index in the flat array
        int32 Index = Y * GridWidth + X;

        // Return value if index is valid
        if (GridPattern.IsValidIndex(Index))
            return GridPattern[Index];

        return 0;
    }

    // Calculate height from data
    void UpdateGridHeight()
    {
        if (GridWidth <= 0)
        {
            GridHeight = 0;
            return;
        }

        GridHeight = FMath::CeilToInt(static_cast<float>(GridPattern.Num()) / GridWidth);
    }
};

/**
 * Data asset for room themes with customizable properties
 */
UCLASS()
class DUNGEONCRAFT_PRO_1_API URoomThemeDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    // Constructor
    URoomThemeDataAsset();

    // Theme identification
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme Info")
    FString ThemeName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme Info")
    FText ThemeDescription;

    // Base meshes for this theme
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme Meshes")
    UStaticMesh* FloorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme Meshes")
    UMaterialInterface* FloorMaterialOverride;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme Meshes")
    FVector FloorPivotOffset;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme Meshes")
    UStaticMesh* WallMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme Meshes")
    UMaterialInterface* WallMaterialOverride;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme Meshes")
    FVector WallPivotOffset;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme Meshes")
    bool bWallFacingX = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme Meshes")
    UStaticMesh* CeilingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme Meshes")
    UMaterialInterface* CeilingMaterialOverride;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme Meshes")
    FVector CeilingPivotOffset;

    // Theme-specific static meshes
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme Props")
    TArray<FStaticMeshSpawnParams> ThemeSpecificMeshes;

    // Theme-specific blueprint actors
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme Props")
    TArray<FBlueprintSpawnParams> ThemeSpecificBlueprints;

    // Grid-based placement for pits or other pattern-based elements
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme Props")
    TArray<FGridPlacement> GridPlacedMeshes;

    // Selection weight (higher = more likely to be chosen)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme Info", meta = (ClampMin = "0.0"))
    float SelectionWeight = 1.0f;
};