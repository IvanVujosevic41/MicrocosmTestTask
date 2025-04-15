using UnrealBuildTool;

public class IlluviumTestTaskEditorTarget : TargetRules
{
	public IlluviumTestTaskEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("IlluviumTestTask");
	}
}
