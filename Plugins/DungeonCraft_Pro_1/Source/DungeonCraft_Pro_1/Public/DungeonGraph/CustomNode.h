#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CustomNode.generated.h"

/**
 * Represents an individual node in the Custom Graph.
 * Nodes contain metadata (e.g., NodeName) and their position for visualization.
 */
UCLASS(BlueprintType)
class DUNGEONCRAFT_PRO_1_API UCustomNode : public UObject
{
    GENERATED_BODY()

public:
    // Default Constructor
    UCustomNode();

    /** The display name or unique identifier for this node */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Node")
    FName NodeName;

    /** The position of the node in the graph (for editor visualization) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Node")
    FVector2D Position;

    /** Allows customization of node behavior or metadata */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Node")
    FString ExtraData;
};