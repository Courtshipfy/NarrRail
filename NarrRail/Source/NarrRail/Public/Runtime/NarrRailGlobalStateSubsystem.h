#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Runtime/NarrRailGlobalConfigAsset.h"
#include "Runtime/NarrRailVariableContainer.h"
#include "NarrRailGlobalStateSubsystem.generated.h"

USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailGlobalStateSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail|Save")
	int32 SnapshotVersion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail|Save")
	TArray<FSoftObjectPath> AppliedGlobalConfigPaths;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail|Save")
	TMap<FName, FString> GlobalVariableSnapshot;
};

UCLASS(BlueprintType)
class NARRRAIL_API UNarrRailGlobalStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "NarrRail|Global")
	bool ApplyGlobalConfig(const UNarrRailGlobalConfigAsset* GlobalConfig, FString& OutErrorMessage);

	UFUNCTION(BlueprintPure, Category = "NarrRail|Global")
	UNarrRailVariableContainer* GetGlobalVariableContainer() const
	{
		return GlobalVariables;
	}

	UFUNCTION(BlueprintPure, Category = "NarrRail|Global")
	bool GetPresetSpeaker(FName SpeakerId, FNarrRailPresetSpeaker& OutSpeaker) const;

	UFUNCTION(BlueprintPure, Category = "NarrRail|Global")
	FString ResolveSpeakerDisplayName(FName SpeakerId) const;

	UFUNCTION(BlueprintPure, Category = "NarrRail|Save")
	FNarrRailGlobalStateSnapshot GetGlobalStateSnapshot() const;

	UFUNCTION(BlueprintCallable, Category = "NarrRail|Save")
	bool RestoreGlobalStateSnapshot(const FNarrRailGlobalStateSnapshot& Snapshot, FString& OutErrorMessage);

private:
	bool ApplyGlobalConfigContent(const UNarrRailGlobalConfigAsset* GlobalConfig, FString& OutErrorMessage);

	UPROPERTY(Transient)
	TObjectPtr<UNarrRailVariableContainer> GlobalVariables;

	UPROPERTY(Transient)
	TArray<FSoftObjectPath> AppliedConfigPaths;

	UPROPERTY(Transient)
	TMap<FName, FNarrRailPresetSpeaker> PresetSpeakersById;
};
