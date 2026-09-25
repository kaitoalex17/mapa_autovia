using UnrealBuildTool;
using System.Collections.Generic;

public class AutopistasEspanaEditorTarget : TargetRules
{
	public AutopistasEspanaEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("AutopistasEspana");
	}
}
