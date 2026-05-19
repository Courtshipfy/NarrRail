#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Runtime/NarrRailStorySession.h"
#include "NarrRailPlayerController.generated.h"

class UNarrRailTypewriterController;

UCLASS()
class NARRRAILHOST_API ANarrRailPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void SetupInputComponent() override;

    UFUNCTION(BlueprintCallable, Category="NarrRail|Runtime")
    void BindNarrRailSession(UNarrRailStorySession* InSession);

    UFUNCTION(BlueprintCallable, Category="NarrRail|UI")
    void BindTypewriterController(UNarrRailTypewriterController* InTypewriter);

    UFUNCTION(BlueprintPure, Category="NarrRail|Runtime")
    UNarrRailStorySession* GetNarrRailSession() const { return Session; }

    UFUNCTION(BlueprintPure, Category="NarrRail|UI")
    UNarrRailTypewriterController* GetTypewriterController() const { return Typewriter; }

protected:
    UFUNCTION()
    void HandleAdvancePressed();

private:
    UPROPERTY(Transient, BlueprintReadOnly, Category="NarrRail|Runtime", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UNarrRailStorySession> Session = nullptr;

    UPROPERTY(Transient, BlueprintReadOnly, Category="NarrRail|UI", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UNarrRailTypewriterController> Typewriter = nullptr;
};
