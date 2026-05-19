#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "NarrRailDialogueSettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="NarrRail Dialogue"))
class NARRRAIL_API UNarrRailDialogueSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    /**
     * 基础打字速度：每秒显示的字符数（Characters Per Second）。
     * 数值越大，文本出现越快。
     */
    UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category="Typewriter", meta=(ClampMin="1.0", UIMin="1.0", UIMax="200.0"))
    float CharsPerSecond = 30.0f;

    /**
     * 普通标点的额外停顿时间（秒），用于增强阅读节奏。
     * 典型字符：, ， 、 ; ； : ：
     */
    UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category="Typewriter", meta=(ClampMin="0.0", UIMin="0.0", UIMax="1.0"))
    float PunctuationExtraDelay = 0.08f;

    /**
     * 句末标点的额外停顿时间（秒），通常比普通标点更长。
     * 典型字符：. ! ? 。 ！ ？
     */
    UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category="Typewriter", meta=(ClampMin="0.0", UIMin="0.0", UIMax="2.0"))
    float SentenceEndExtraDelay = 0.15f;

    /**
     * 当玩家按下“跳过/推进”输入时，是否直接补完当前句。
     * true：先补全文本；false：可由上层逻辑决定是否直接下一句。
     */
    UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category="Typewriter")
    bool bSkipCompletesCurrentLine = true;

    /**
     * 当前句打字完成后是否自动推进到下一句。
     * 该行为通常由展示层（UI/控制器）结合 Session 状态决定是否执行。
     */
    UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category="Typewriter")
    bool bAutoAdvance = false;

    /**
     * 自动推进前的等待时间（秒）。
     * 仅在 bAutoAdvance = true 时生效。
     */
    UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category="Typewriter", meta=(ClampMin="0.0", UIMin="0.0", UIMax="5.0"))
    float AutoAdvanceDelay = 0.40f;

    virtual FName GetCategoryName() const override { return FName(TEXT("Plugins")); }
};
