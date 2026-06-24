// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HollowWilds : ModuleRules
{
	public HollowWilds(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"AIModule",
			"NavigationSystem"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Niagara",
			"UMG",
			"Slate",
			"SlateCore"
		});
	}
}
