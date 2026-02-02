#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CustomGraph.generated.h"

// Forward declaration of UCustomNode (to be implemented later)
class UCustomNode;

/** 
 * A Struct that represents an edge between two nodes in the graph.
 * This defines connections between rooms or corridors in a larger dungeon structure.
 */
USTRUCT(BlueprintType)
struct FCustomEdge
{
    GENERATED_BODY()

    // Pointer to the starting node of the edge
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom Edge")
    UCustomNode* StartNode = nullptr;

    // Pointer to the ending node of the edge
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom Edge")
    UCustomNode* EndNode = nullptr;
};

/**
 * The Graph (UObject) that contains all the nodes and edges. Represents the dungeon layout.
 * This object is the core data structure for the node-graph editor.
 */
UCLASS(BlueprintType)
class DUNGEONCRAFT_PRO_1_API UCustomGraph : public UObject
{
    GENERATED_BODY()

public:
    UCustomGraph();

    // List of all nodes in the graph
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom Graph")
    TArray<UCustomNode*> Nodes;

    // List of all edges between nodes in the graph
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom Graph")
    TArray<FCustomEdge> Edges;

    /** Adds a node to the graph */
    UFUNCTION(BlueprintCallable, Category = "Custom Graph")
    void AddNode(UCustomNode* Node);

    /** Removes a node from the graph (and its associated edges) */
    UFUNCTION(BlueprintCallable, Category = "Custom Graph")
    void RemoveNode(UCustomNode* Node);

    /** Adds an edge between two nodes in the graph */
    UFUNCTION(BlueprintCallable, Category = "Custom Graph")
    void AddEdge(UCustomNode* StartNode, UCustomNode* EndNode);
};