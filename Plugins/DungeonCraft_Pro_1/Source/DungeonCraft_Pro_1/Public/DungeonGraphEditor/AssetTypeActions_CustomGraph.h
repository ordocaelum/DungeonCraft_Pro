#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

/**
 * Defines custom asset type actions for UCustomGraph.
 * These actions register UCustomGraph in the Unreal Editor's "Add New" menu.
 */
class FAssetTypeActions_CustomGraph : public FAssetTypeActions_Base
{
public:
    /** Returns the display name of the asset type (shown in Add New menu) */
    virtual FText GetName() const override
    {
        return NSLOCTEXT("AssetTypeActions", "FAssetTypeActions_CustomGraph", "Custom Graph");
    }

    /** Returns the category this asset should appear under in the Add New menu */
    virtual uint32 GetCategories() override
    {
        return EAssetTypeCategories::Misc; // Category to display under "Miscellaneous"
    }

    /** Returns the class of the asset type this action supports */
    virtual UClass* GetSupportedClass() const override;

    /** Allows Custom Graph to have right-click menu options */
    virtual bool HasActions(const TArray<UObject*>& InObjects) const override
    {
        return true; // Enable context menu
    }

    /** Populates the right-click menu options for UCustomGraph */
    virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override
    {
        FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);
        // Additional custom actions can be added here if needed
    }

    /** Returns the color used to uniquely identify this asset type */
    virtual FColor GetTypeColor() const override
    {
        return FColor(255, 192, 0); // Example: Yellow-orange color
    }
};