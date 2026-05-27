#pragma once

#include "CoreMinimal.h"
#include "NarrRailStoryTypes.generated.h"

UENUM(BlueprintType)
enum class ENarrRailNodeType : uint8
{
    Dialogue UMETA(DisplayName = "Dialogue"),
    MultiDialogue UMETA(DisplayName = "Multi Dialogue"),
    Choice UMETA(DisplayName = "Choice"),
    Jump UMETA(DisplayName = "Jump"),
    SetVariable UMETA(DisplayName = "Set Variable"),
    EmitEvent UMETA(DisplayName = "Emit Event"),
    Condition UMETA(DisplayName = "Condition"),
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

UENUM(BlueprintType)
enum class ENarrRailChoiceMode : uint8
{
    SinglePass UMETA(DisplayName = "Single Pass"),
    ExhaustiveUntilComplete UMETA(DisplayName = "Exhaustive Until Complete")
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

USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailDialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FString TextKey;
};

USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailMultiDialoguePayload
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FName SpeakerId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    TArray<FNarrRailDialogueLine> Lines;
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

    // 运行时标记：该选项是否在当前会话中被选择过（主要用于 ExhaustiveUntilComplete UI 呈现）
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "NarrRail|Runtime")
    bool bHasBeenSelected = false;
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
    FNarrRailMultiDialoguePayload MultiDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    TArray<FNarrRailChoiceOption> Choices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    ENarrRailChoiceMode ChoiceMode = ENarrRailChoiceMode::SinglePass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FName ChoiceCompletionTargetNodeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FName JumpTargetNodeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    TArray<FNarrRailNodeAction> EnterActions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    TArray<FNarrRailNodeAction> ExitActions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FNarrRailConditionExpression Condition;
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
    FName SourceHandle = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    int32 Priority = 0;
};

// 最后一次选择的信息
USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailLastChoiceInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    FName ChoiceNodeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    int32 ChoiceIndex = -1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    FName TargetNodeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    FString ChoiceTextKey;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    bool bValid = false;
};
