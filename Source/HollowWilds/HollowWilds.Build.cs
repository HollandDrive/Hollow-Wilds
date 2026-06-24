// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HollowWilds : ModuleRules
{
	public HollowWilds(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Code is organized into subfolders directly under the module root (no
		// Public/Private split), and includes are written module-root-relative
		// (e.g. #include "Abilities/HWElement.h"). Put the module root on the
		// include path so those resolve.
		PublicIncludePaths.Add(ModuleDirectory);

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
