#pragma once

#include "CoreMinimal.h"
#include "NarrRailStoryTypes.generated.h"

UENUM(BlueprintType)
enum class ENarrRailNodeType : uint8
{
    Dialogue UMETA(DisplayName = "Dialogue"),
    Choice UMETA(DisplayName = "Choice"),
    Jump UMETA(DisplayName = "Jump"),
    SetVariable UMETA(DisplayName = "Set Variable"),
    EmitEvent UMETA(DisplayName = "Emit Event"),
    End UMETA(DisplayName = "End")
};

UENUM(BlueprintType)
enum class ENarrRailConditionLogic : uint8
{
    All UMETA(DisplayName = "All"),
    Any UMETA(DisplayName = "Any")
};

UENUM(BlueprintType)
enum class ENarrRailComparisonOp : uint8
{
    Equal UMETA(DisplayName = "=="),
    NotEqual UMETA(DisplayName = "!="),
    Greater UMETA(DisplayName = ">"),
    GreaterOrEqual UMETA(DisplayName = ">="),
    Less UMETA(DisplayName = "<"),
    LessOrEqual UMETA(DisplayName = "<=")
};

UENUM(BlueprintType)
enum class ENarrRailVariableType : uint8
{
    Bool UMETA(DisplayName = "Bool"),
    Int UMETA(DisplayName = "Int"),
    Float UMETA(DisplayName = "Float"),
    String UMETA(DisplayName = "String")
};

UENUM(BlueprintType)
enum class ENarrRailActionType : uint8
{
    Set UMETA(DisplayName = "Set"),
    Add UMETA(DisplayName = "Add"),
    Subtract UMETA(DisplayName = "Subtract"),
    EmitEvent UMETA(DisplayName = "Emit Event")
};

USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailDialoguePayload
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FName SpeakerId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FString TextKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    float SpeechRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    TSoftObjectPtr<class USoundBase> VoiceAsset;
};

// 变量引用定义：明确变量名、类型、作用域，用于静态校验和运行时绑定。
USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailVariableRef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FName VariableName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    ENarrRailVariableType VariableType = ENarrRailVariableType::String;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    bool bGlobalScope = false;
};

USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailConditionTerm
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FNarrRailVariableRef Variable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    ENarrRailComparisonOp Operator = ENarrRailComparisonOp::Equal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FString CompareValue;
};

USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailConditionExpression
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    ENarrRailConditionLogic Logic = ENarrRailConditionLogic::All;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    TArray<FNarrRailConditionTerm> Terms;
};

USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailChoiceOption
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FString TextKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FName TargetNodeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FNarrRailConditionExpression Availability;
};

// 节点动作定义：用于节点进入/退出时执行变量变更或事件派发。
USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailNodeAction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    ENarrRailActionType ActionType = ENarrRailActionType::Set;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FNarrRailVariableRef Variable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FString Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FName EventId = NAME_None;
};

USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FName NodeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    ENarrRailNodeType NodeType = ENarrRailNodeType::Dialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FNarrRailDialoguePayload Dialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    TArray<FNarrRailChoiceOption> Choices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FName JumpTargetNodeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    TArray<FNarrRailNodeAction> EnterActions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    TArray<FNarrRailNodeAction> ExitActions;
};

USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailNodeEdge
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FName SourceNodeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FName TargetNodeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    int32 Priority = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FNarrRailConditionExpression Condition;
};
