using UnrealBuildTool;

public class DungeonCraft_Pro_1 : ModuleRules
{
    public DungeonCraft_Pro_1(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        // Add the public include paths
        PublicIncludePaths.AddRange(
            new string[] {
                "Plugins/DungeonCraft_Pro_1/Source/DungeonCraft_Pro_1/Public",
                "Plugins/DungeonCraft_Pro_1/Source/DungeonCraft_Pro_1/Public/DungeonGraph"
            }
        );

        // Add the private include paths
        PrivateIncludePaths.AddRange(
            new string[] {
                "Plugins/DungeonCraft_Pro_1/Source/DungeonCraft_Pro_1/Private",
                "Plugins/DungeonCraft_Pro_1/Source/DungeonCraft_Pro_1/Private/DungeonGraph"
            }
        );

        // For runtime modules
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine"
            }
        );

        // For editor functionality
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate",
                "SlateCore",
                "UnrealEd", // Required for UFactory
                "NavigationSystem",
                "AIModule"
            }
        );

        // Define API for DLL export/import
        PublicDefinitions.Add("DUNGEONCRAFT_PRO_1_API=DLLEXPORT");
    }
}