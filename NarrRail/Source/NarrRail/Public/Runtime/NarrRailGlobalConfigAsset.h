#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Runtime/NarrRailVariableContainer.h"
#include "NarrRailGlobalConfigAsset.generated.h"

class UAssetImportData;

USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailPresetSpeaker
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FName SpeakerId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FString DisplayName;
};

UCLASS(BlueprintType)
class NARRRAIL_API UNarrRailGlobalConfigAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    int32 SchemaVersion = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    TArray<FNarrRailVariableDefinition> Variables;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    TArray<FNarrRailPresetSpeaker> PresetSpeakers;

#if WITH_EDITORONLY_DATA
    UPROPERTY(VisibleAnywhere, Instanced, Category = "ImportSettings")
    TObjectPtr<UAssetImportData> AssetImportData;
#endif

#if WITH_EDITOR
    UAssetImportData* EnsureAssetImportData();
    UAssetImportData* GetAssetImportData() const;
#endif
};
