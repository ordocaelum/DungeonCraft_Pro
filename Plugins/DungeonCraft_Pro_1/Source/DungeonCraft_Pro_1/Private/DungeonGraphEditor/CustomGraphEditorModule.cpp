#include "DungeonGraphEditor/CustomGraphEditorModule.h" // This MUST be the first include
#include "Modules/ModuleManager.h"
#include "AssetToolsModule.h"
#include "DungeonGraphEditor/AssetTypeActions_CustomGraph.h"
#include "DungeonGraph/CustomGraphSchema.h" // Include the schema

// Implement the Custom Graph Editor Module
IMPLEMENT_MODULE(FCustomGraphEditorModule, CustomGraphEditor)

void FCustomGraphEditorModule::StartupModule()
{
    RegisterAssetTypeActions();

    // Register the Custom Graph Schema
    UCustomGraphSchema::StaticClass()->AddToRoot(); // Ensure schema is registered.
}

void FCustomGraphEditorModule::ShutdownModule()
{
    // Unregister asset type actions when shutting down
    if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
    {
        IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
        for (TSharedRef<IAssetTypeActions> Action : RegisteredAssetTypeActions)
        {
            AssetTools.UnregisterAssetTypeActions(Action);
        }
    }

    // Clear registered actions list
    RegisteredAssetTypeActions.Empty();
}

void FCustomGraphEditorModule::RegisterAssetTypeActions()
{
    // Get the Asset Tools module
    IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

    // Register the custom graph asset's type actions
    TSharedRef<IAssetTypeActions> CustomGraphAction = MakeShared<FAssetTypeActions_CustomGraph>();
    AssetTools.RegisterAssetTypeActions(CustomGraphAction);

    // Track registered actions for cleanup during shutdown
    RegisteredAssetTypeActions.Add(CustomGraphAction);
}