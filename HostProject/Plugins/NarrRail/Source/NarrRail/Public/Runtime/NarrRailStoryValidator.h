#pragma once

#include "CoreMinimal.h"
#include "Runtime/NarrRailStoryAsset.h"
#include "NarrRailStoryValidator.generated.h"

// 校验问题严重级别。
UENUM(BlueprintType)
enum class ENarrRailValidationSeverity : uint8
{
    Info UMETA(DisplayName = "Info"),
    Warning UMETA(DisplayName = "Warning"),
    Error UMETA(DisplayName = "Error")
};

// 单条校验结果。
USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailValidationIssue
{
    GENERATED_BODY()

    // 问题级别。
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    ENarrRailValidationSeverity Severity = ENarrRailValidationSeverity::Error;

    // 关联节点 ID（无节点上下文时可能为空）。
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    FName NodeId = NAME_None;

    // 可读错误信息。
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
    FString Message;
};

// 剧情资产结构校验器。
UCLASS()
class NARRRAIL_API UNarrRailStoryValidator : public UObject
{
    GENERATED_BODY()

public:
    // 对剧情资产执行基础结构校验，返回问题列表。
    UFUNCTION(BlueprintCallable, Category = "NarrRail|Validation")
    static TArray<FNarrRailValidationIssue> ValidateStoryAsset(const UNarrRailStoryAsset* StoryAsset);
};
