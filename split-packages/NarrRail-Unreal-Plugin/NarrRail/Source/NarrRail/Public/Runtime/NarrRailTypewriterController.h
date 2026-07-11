#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NarrRailTypewriterController.generated.h"

UCLASS(BlueprintType)
class NARRRAIL_API UNarrRailTypewriterController : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="NarrRail|Typewriter")
    void StartTyping(const FString& InFullText);

    UFUNCTION(BlueprintCallable, Category="NarrRail|Typewriter")
    void TickTyping(float DeltaSeconds);

    UFUNCTION(BlueprintCallable, Category="NarrRail|Typewriter")
    void CompleteImmediately();

    UFUNCTION(BlueprintPure, Category="NarrRail|Typewriter")
    bool IsTyping() const { return bTyping; }

    UFUNCTION(BlueprintPure, Category="NarrRail|Typewriter")
    bool IsCompleted() const { return bCompleted; }

    UFUNCTION(BlueprintPure, Category="NarrRail|Typewriter")
    FText GetVisibleText() const { return FText::FromString(VisibleText); }

    UFUNCTION(BlueprintPure, Category="NarrRail|Typewriter")
    FString GetVisibleString() const { return VisibleText; }

private:
    static bool IsSentenceEndChar(TCHAR Ch);
    static bool IsPunctuationChar(TCHAR Ch);

    FString FullText;
    FString VisibleText;
    int32 CurrentIndex = 0;

    bool bTyping = false;
    bool bCompleted = true;

    float CarrySeconds = 0.0f;
    float PendingExtraDelay = 0.0f;
};
