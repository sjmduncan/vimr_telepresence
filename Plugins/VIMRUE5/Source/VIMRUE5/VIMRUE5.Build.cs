// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VIMRUE5 : ModuleRules
{
	public VIMRUE5(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicIncludePaths.Add("$(ProjectDir)/VIMR/lib/include/");
		PublicAdditionalLibraries.Add("$(PluginDir)/Binaries/Win64/vimr.lib");

		// Delay-load the DLL, so we can load it from the right place first
		PublicDelayLoadDLLs.Add("vimr.dll");

			// Ensure that the DLL is staged along with the executable
		RuntimeDependencies.Add("$(PluginDir)/Binaries/Win64/vimr.dll");
		
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
        "Engine",
				"Projects",
				"OnlineSubsystem",
				"OnlineSubsystemSteam",
				"HeadMountedDisplay",
				"Networking", 
				"Sockets"
			}
			);
	}
}
