#pragma once

#include "CoreMinimal.h"
#include "NarrRailStoryTypes.generated.h"

// 剧情节点类型。运行时执行器将按该类型决定推进逻辑。
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

// 条件表达式内部多个条件项的组合关系。
UENUM(BlueprintType)
enum class ENarrRailConditionLogic : uint8
{
    All UMETA(DisplayName = "All"),
    Any UMETA(DisplayName = "Any")
};

// 单个条件项支持的比较运算符。
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

// 对白节点承载的主要内容。
USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailDialoguePayload
{
    GENERATED_BODY()

    // 角色 ID（后续可映射到角色配置或立绘资源）。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FName SpeakerId = NAME_None;

    // 文本键，用于本地化查表。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FString TextKey;

    // 播放语速系数，1.0 为默认值。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    float SpeechRate = 1.0f;

    // 可选语音资源。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    TSoftObjectPtr<class USoundBase> VoiceAsset;
};

// 条件表达式中的最小判断单元。
USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailConditionTerm
{
    GENERATED_BODY()

    // 被比较的变量名。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FName VariableName = NAME_None;

    // 比较运算符。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    ENarrRailComparisonOp Operator = ENarrRailComparisonOp::Equal;

    // 比较目标值（字符串形式，后续按变量类型解释）。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FString CompareValue;
};

// 由多个条件项组成的表达式。
USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailConditionExpression
{
    GENERATED_BODY()

    // 条件项组合逻辑（All/Any）。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    ENarrRailConditionLogic Logic = ENarrRailConditionLogic::All;

    // 条件项列表。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    TArray<FNarrRailConditionTerm> Terms;
};

// 分支节点中的一个可选项。
USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailChoiceOption
{
    GENERATED_BODY()

    // 选项文本键。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FString TextKey;

    // 选中后跳转到的目标节点。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FName TargetNodeId = NAME_None;

    // 该选项可见/可选的条件。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FNarrRailConditionExpression Availability;
};

// 剧情图中的节点定义。
USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailNode
{
    GENERATED_BODY()

    // 节点唯一 ID。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FName NodeId = NAME_None;

    // 节点类型。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    ENarrRailNodeType NodeType = ENarrRailNodeType::Dialogue;

    // 对白数据（在 Dialogue 节点上生效）。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FNarrRailDialoguePayload Dialogue;

    // 分支选项（在 Choice 节点上生效）。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    TArray<FNarrRailChoiceOption> Choices;

    // 跳转目标（在 Jump 节点上生效）。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FName JumpTargetNodeId = NAME_None;
};

// 显式边定义，用于构建剧情图拓扑。
USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailNodeEdge
{
    GENERATED_BODY()

    // 边起点。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FName SourceNodeId = NAME_None;

    // 边终点。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FName TargetNodeId = NAME_None;

    // 同源多条边时的优先级（数值越小优先级越高）。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    int32 Priority = 0;

    // 边生效条件。
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
    FNarrRailConditionExpression Condition;
};
