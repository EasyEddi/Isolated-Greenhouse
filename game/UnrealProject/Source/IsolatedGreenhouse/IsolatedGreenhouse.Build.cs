using UnrealBuildTool;

public class IsolatedGreenhouse : ModuleRules
{
	public IsolatedGreenhouse(ReadOnlyTargetRules Target) : base(Target)
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
