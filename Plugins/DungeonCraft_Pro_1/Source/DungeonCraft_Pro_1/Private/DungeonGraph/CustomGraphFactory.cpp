#include "DungeonGraph/CustomGraphFactory.h"
#include "DungeonGraph/CustomGraph.h"

// Constructor
UCustomGraphFactory::UCustomGraphFactory()
{
    // The type of asset this factory creates
    SupportedClass = UCustomGraph::StaticClass();

    // Enable the creation of the asset in the Content Browser
    bCreateNew = true;

    // Open the asset for editing immediately after creation
    bEditAfterNew = true;
}

// Create a new Custom Graph asset
UObject* UCustomGraphFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    // Create a new instance of UCustomGraph
    return NewObject<UCustomGraph>(InParent, Class, Name, Flags | RF_Transactional);
}