#pragma once

#include "Modules/ModuleManager.h"
#include "CoreMinimal.h"

/**
 * Custom module to register asset-related actions in Unreal Editor for Custom Graphs.
 */
class FCustomGraphEditorModule : public IModuleInterface
{
public:
    /** Called when the module is loaded */
    virtual void StartupModule() override;

    /** Called when the module is unloaded */
    virtual void ShutdownModule() override;

private:
    /** Register asset type actions for UCustomGraph */
    void RegisterAssetTypeActions();

    TArray<TSharedRef<class IAssetTypeActions>> RegisteredAssetTypeActions;
};