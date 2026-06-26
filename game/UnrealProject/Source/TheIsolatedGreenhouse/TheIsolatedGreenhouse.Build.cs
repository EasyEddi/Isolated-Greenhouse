using UnrealBuildTool;

public class TheIsolatedGreenhouse : ModuleRules
{
	public TheIsolatedGreenhouse(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"EnhancedInput",
			"InputCore",
			"UMG",
			"Slate",
			"SlateCore"
		});
	}
}
