#include "Runtime/NarrRailGlobalStateSubsystem.h"

void UNarrRailGlobalStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (GlobalVariables == nullptr)
	{
		GlobalVariables = NewObject<UNarrRailVariableContainer>(this);
	}
}

bool UNarrRailGlobalStateSubsystem::ApplyGlobalConfig(const UNarrRailGlobalConfigAsset* GlobalConfig, FString& OutErrorMessage)
{
	if (GlobalConfig == nullptr)
	{
		return true;
	}

	const FSoftObjectPath ConfigPath(GlobalConfig);
	if (AppliedConfigPaths.Contains(ConfigPath))
	{
		return true;
	}

	if (GlobalVariables == nullptr)
	{
		GlobalVariables = NewObject<UNarrRailVariableContainer>(this);
	}

	TArray<FNarrRailVariableDefinition> GlobalDefinitions;
	GlobalDefinitions.Reserve(GlobalConfig->Variables.Num());
	for (FNarrRailVariableDefinition Definition : GlobalConfig->Variables)
	{
		Definition.bGlobalScope = true;
		GlobalDefinitions.Add(Definition);
	}

	if (!GlobalVariables->AddDefinitions(GlobalDefinitions, true, OutErrorMessage))
	{
		return false;
	}

	for (const FNarrRailPresetSpeaker& Speaker : GlobalConfig->PresetSpeakers)
	{
		if (Speaker.SpeakerId == NAME_None)
		{
			continue;
		}

		if (const FNarrRailPresetSpeaker* Existing = PresetSpeakersById.Find(Speaker.SpeakerId))
		{
			if (Existing->DisplayName != Speaker.DisplayName || Existing->Color != Speaker.Color)
			{
				OutErrorMessage = FString::Printf(TEXT("Preset speaker '%s' is already defined with different data."), *Speaker.SpeakerId.ToString());
				return false;
			}
			continue;
		}

		PresetSpeakersById.Add(Speaker.SpeakerId, Speaker);
	}

	AppliedConfigPaths.Add(ConfigPath);
	return true;
}

bool UNarrRailGlobalStateSubsystem::GetPresetSpeaker(const FName SpeakerId, FNarrRailPresetSpeaker& OutSpeaker) const
{
	if (const FNarrRailPresetSpeaker* Speaker = PresetSpeakersById.Find(SpeakerId))
	{
		OutSpeaker = *Speaker;
		return true;
	}

	return false;
}

FString UNarrRailGlobalStateSubsystem::ResolveSpeakerDisplayName(const FName SpeakerId) const
{
	FNarrRailPresetSpeaker Speaker;
	if (GetPresetSpeaker(SpeakerId, Speaker) && !Speaker.DisplayName.IsEmpty())
	{
		return Speaker.DisplayName;
	}

	return SpeakerId.ToString();
}

FNarrRailGlobalStateSnapshot UNarrRailGlobalStateSubsystem::GetGlobalStateSnapshot() const
{
	FNarrRailGlobalStateSnapshot Snapshot;
	Snapshot.AppliedGlobalConfigPaths = AppliedConfigPaths;
	if (GlobalVariables != nullptr)
	{
		Snapshot.GlobalVariableSnapshot = GlobalVariables->GetSnapshot();
	}
	return Snapshot;
}

bool UNarrRailGlobalStateSubsystem::RestoreGlobalStateSnapshot(const FNarrRailGlobalStateSnapshot& Snapshot, FString& OutErrorMessage)
{
	if (Snapshot.SnapshotVersion <= 0)
	{
		OutErrorMessage = TEXT("Invalid NarrRail global state snapshot version.");
		return false;
	}

	if (GlobalVariables == nullptr)
	{
		GlobalVariables = NewObject<UNarrRailVariableContainer>(this);
	}

	GlobalVariables->RestoreFromSnapshot(Snapshot.GlobalVariableSnapshot);
	AppliedConfigPaths = Snapshot.AppliedGlobalConfigPaths;
	return true;
}
