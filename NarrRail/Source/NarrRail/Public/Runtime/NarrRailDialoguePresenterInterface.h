#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Runtime/NarrRailStoryTypes.h"
#include "NarrRailDialoguePresenterInterface.generated.h"

/**
 * 对话显示模式枚举
 * 用于区分不同的 UI 显示风格
 */
UENUM(BlueprintType)
enum class ENarrRailDialogueMode : uint8
{
	/** ADV 模式：单行对话框，通常位于屏幕底部 */
	ADV UMETA(DisplayName = "ADV Mode (Adventure)"),

	/** NVL 模式：全屏文本，类似小说阅读 */
	NVL UMETA(DisplayName = "NVL Mode (Novel)")
};

/**
 * 对话显示请求结构
 * 包含 UI 显示对话所需的所有信息
 */
USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailDialogueRequest
{
	GENERATED_BODY()

	/** 节点 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
	FName NodeId = NAME_None;

	/** 说话人 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
	FName SpeakerId = NAME_None;

	/** 对话文本 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
	FString TextContent;

	/** 语速 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
	float SpeechRate = 1.0f;

	/** 语音资产 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
	TSoftObjectPtr<class USoundBase> VoiceAsset;

	/** 是否自动推进到下一句（用于 NVL 模式的连续显示） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
	bool bAutoAdvance = false;
};

/**
 * 选项显示请求结构
 */
USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailChoiceRequest
{
	GENERATED_BODY()

	/** 节点 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
	FName NodeId = NAME_None;

	/** 选项列表 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
	TArray<FNarrRailChoiceOption> Choices;

	/** Session 对象（用于 UI 调用 Choose 方法） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
	TObjectPtr<class UNarrRailStorySession> Session = nullptr;
};

// UInterface 声明（用于 Blueprint）
UINTERFACE(MinimalAPI, Blueprintable)
class UNarrRailDialoguePresenterInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 对话显示器接口
 *
 * 所有对话 UI 组件（ADV/NVL）都应该实现此接口
 *
 * 使用示例：
 * - WBP_Dialogue_ADV: 实现此接口，显示底部单行对话框
 * - WBP_Dialogue_NVL: 实现此接口，显示全屏文本
 */
class NARRRAIL_API INarrRailDialoguePresenterInterface
{
	GENERATED_BODY()

public:
	/**
	 * 显示对话
	 * @param Request 对话显示请求
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NarrRail|UI")
	void ShowDialogue(const FNarrRailDialogueRequest& Request);

	/**
	 * 显示选项
	 * @param Request 选项显示请求
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NarrRail|UI")
	void ShowChoices(const FNarrRailChoiceRequest& Request);

	/**
	 * 隐藏对话框
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NarrRail|UI")
	void HideDialogue();

	/**
	 * 清空对话历史（主要用于 NVL 模式）
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NarrRail|UI")
	void ClearDialogueHistory();

	/**
	 * 获取当前支持的显示模式
	 * @return 显示模式（ADV 或 NVL）
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NarrRail|UI")
	ENarrRailDialogueMode GetDialogueMode() const;

	/**
	 * 检查对话是否正在显示中（例如打字机效果未完成）
	 * @return 如果对话正在显示返回 true
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NarrRail|UI")
	bool IsDialogueActive() const;

	/**
	 * 跳过当前对话动画（例如立即显示完整文本）
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NarrRail|UI")
	void SkipDialogueAnimation();
};
