// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectFPS : ModuleRules
{
	public ProjectFPS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"SlateCore",
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
            "Networking",
			"Niagara"
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"ProjectFPS",
			"ProjectFPS/Variant_Horror",
			"ProjectFPS/Variant_Horror/UI",
			"ProjectFPS/Variant_Shooter",
			"ProjectFPS/Variant_Shooter/AI",
			"ProjectFPS/Variant_Shooter/UI",
			"ProjectFPS/Variant_Shooter/Weapons",
            "ProjectFPS/FPS",
            "ProjectFPS/FPS/AI",
            "ProjectFPS/FPS/Weapons",
            "ProjectFPS/FPS/UI"
        });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
