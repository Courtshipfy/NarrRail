#pragma once

#include "CoreMinimal.h"
#include "Runtime/NarrRailStoryAsset.h"
#include "NarrRailStorySession.generated.h"

UENUM(BlueprintType)
enum class ENarrRailRuntimeResultCode : uint8
{
    Success UMETA(DisplayName = "Success"),
    InvalidState UMETA(DisplayName = "Invalid State"),
    InvalidInput UMETA(DisplayName = "Invalid Input"),
    MissingNode UMETA(DisplayName = "Missing Node"),
    Completed UMETA(DisplayName = "Completed")
};

UENUM(BlueprintType)
enum class ENarrRailSessionState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Running UMETA(DisplayName = "Running"),
    WaitingForChoice UMETA(DisplayName = "Waiting For Choice"),
    Completed UMETA(DisplayName = "Completed"),
    Error UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailRuntimeResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    ENarrRailRuntimeResultCode Code = ENarrRailRuntimeResultCode::Success;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    FString Message;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    FName NodeId = NAME_None;

    static FNarrRailRuntimeResult Make(const ENarrRailRuntimeResultCode InCode, const TCHAR* InMessage, const FName InNodeId = NAME_None)
    {
        FNarrRailRuntimeResult Result;
        Result.Code = InCode;
        Result.Message = InMessage;
        Result.NodeId = InNodeId;
        return Result;
    }
};

// 剧情会话执行器：负责运行时 Start/Next/Choose/Stop 流程推进。
UCLASS(BlueprintType)
class NARRRAIL_API UNarrRailStorySession : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Runtime")
    FNarrRailRuntimeResult Initialize(const UNarrRailStoryAsset* InStoryAsset);

    UFUNCTION(BlueprintCallable, Category = "NarrRail|Runtime")
    FNarrRailRuntimeResult Start(FName OverrideEntryNodeId = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "NarrRail|Runtime")
    FNarrRailRuntimeResult Next();

    UFUNCTION(BlueprintCallable, Category = "NarrRail|Runtime")
    FNarrRailRuntimeResult Choose(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "NarrRail|Runtime")
    FNarrRailRuntimeResult Stop();

    UFUNCTION(BlueprintPure, Category = "NarrRail|Runtime")
    ENarrRailSessionState GetSessionState() const
    {
        return SessionState;
    }

    UFUNCTION(BlueprintPure, Category = "NarrRail|Runtime")
    FName GetCurrentNodeId() const
    {
        return CurrentNodeId;
    }

    UFUNCTION(BlueprintPure, Category = "NarrRail|Runtime")
    bool GetCurrentNode(FNarrRailNode& OutNode) const;

    UFUNCTION(BlueprintPure, Category = "NarrRail|Runtime")
    TArray<FNarrRailChoiceOption> GetCurrentChoices() const;

    UFUNCTION(BlueprintPure, Category = "NarrRail|Runtime")
    const TArray<FName>& GetHistory() const
    {
        return NodeHistory;
    }

private:
    FNarrRailRuntimeResult AdvanceToNode(FName TargetNodeId);
    FNarrRailRuntimeResult ResolveNextByEdge(const FNarrRailNode& FromNode, FName& OutNextNodeId) const;
    const FNarrRailNode* FindNode(FName NodeId) const;

private:
    UPROPERTY(Transient)
    TObjectPtr<const UNarrRailStoryAsset> StoryAsset;

    UPROPERTY(Transient)
    ENarrRailSessionState SessionState = ENarrRailSessionState::Idle;

    UPROPERTY(Transient)
    FName CurrentNodeId = NAME_None;

    UPROPERTY(Transient)
    TArray<FName> NodeHistory;
};
