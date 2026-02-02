#include "DungeonGraphEditor/AssetTypeActions_CustomGraph.h"
#include "DungeonGraph/CustomGraph.h"

UClass* FAssetTypeActions_CustomGraph::GetSupportedClass() const
{
    return UCustomGraph::StaticClass();
}