using UnrealBuildTool;
using System.Collections.Generic;

public class GASSurvivalEditorTarget : TargetRules
{
	public GASSurvivalEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_1;
		ExtraModuleNames.Add("GASSurvival");
	}
}
