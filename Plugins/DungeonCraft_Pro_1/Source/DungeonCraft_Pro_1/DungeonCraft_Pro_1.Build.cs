// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DungeonCraft_Pro_1 : ModuleRules
{
    public DungeonCraft_Pro_1(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
				// ... add public include paths required here ...
			}
            );


        PrivateIncludePaths.AddRange(
            new string[] {
				// ... add other private include paths required here ...
			}
            );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",  // Moved from private to public - needed for UCLASS/USTRUCT
				"Engine",       // Moved from private to public - needed for DataAsset
				// ... add other public dependencies that you statically link with here ...
			}
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
				// CoreUObject and Engine moved to public dependencies
				"Slate",
                "SlateCore",
                "NavigationSystem", // Added for potential navigation needs
				"AIModule",         // Added if using AI components
				// ... add private dependencies that you statically link with here ...	
			}
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );

        // Add the API definition for proper symbol export/import
        PublicDefinitions.Add("DUNGEONCRAFT_PRO_1_API=DLLEXPORT");
    }
}