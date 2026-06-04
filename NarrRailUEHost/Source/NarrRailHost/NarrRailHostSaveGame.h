#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Runtime/NarrRailGlobalStateSubsystem.h"
#include "Runtime/NarrRailStorySession.h"
#include "NarrRailHostSaveGame.generated.h"

UCLASS()
class NARRRAILHOST_API UNarrRailHostSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail|Save")
    FNarrRailStorySessionSnapshot SessionSnapshot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail|Save")
    FNarrRailGlobalStateSnapshot GlobalStateSnapshot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail|Save")
    FString SavedAtUtc;
};
