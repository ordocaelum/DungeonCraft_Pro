#include "DungeonGraph/CustomNode.h"

UCustomNode::UCustomNode()
{
    // Default Initialization
    NodeName = NAME_None; // Default unnamed node
    Position = FVector2D::ZeroVector; // Default position at origin
    ExtraData = TEXT(""); // Empty metadata by default
}