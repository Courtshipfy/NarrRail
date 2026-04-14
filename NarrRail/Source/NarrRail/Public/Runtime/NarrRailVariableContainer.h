#pragma once

#include "CoreMinimal.h"
#include "Runtime/NarrRailStoryTypes.h"
#include "NarrRailVariableContainer.generated.h"

// 变量变更委托：用于订阅变量变化事件
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNarrRailVariableChangedDelegate, FName, VariableName, ENarrRailVariableType, VariableType, const FString&, NewValue);

// 变量容器错误码
UENUM(BlueprintType)
enum class ENarrRailVariableError : uint8
{
	Success UMETA(DisplayName = "Success"),
	VariableNotFound UMETA(DisplayName = "Variable Not Found"),
	TypeMismatch UMETA(DisplayName = "Type Mismatch"),
	ParseError UMETA(DisplayName = "Parse Error"),
	InvalidOperation UMETA(DisplayName = "Invalid Operation")
};

// 变量容器操作结果
USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailVariableResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
	ENarrRailVariableError ErrorCode = ENarrRailVariableError::Success;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
	FString ErrorMessage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NarrRail")
	FString Value;

	static FNarrRailVariableResult MakeSuccess(const FString& InValue = TEXT(""))
	{
		FNarrRailVariableResult Result;
		Result.ErrorCode = ENarrRailVariableError::Success;
		Result.Value = InValue;
		return Result;
	}

	static FNarrRailVariableResult MakeError(ENarrRailVariableError InErrorCode, const FString& InMessage)
	{
		FNarrRailVariableResult Result;
		Result.ErrorCode = InErrorCode;
		Result.ErrorMessage = InMessage;
		return Result;
	}

	bool IsSuccess() const { return ErrorCode == ENarrRailVariableError::Success; }
};

// 变量定义：包含类型、作用域、默认值
USTRUCT(BlueprintType)
struct NARRRAIL_API FNarrRailVariableDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
	FName VariableName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
	ENarrRailVariableType VariableType = ENarrRailVariableType::String;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
	bool bGlobalScope = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NarrRail")
	FString DefaultValue;
};

// 变量容器：统一管理 Bool/Int/Float/String 四种类型变量
// 支持作用域隔离、变更通知、类型安全的读写操作
UCLASS(BlueprintType)
class NARRRAIL_API UNarrRailVariableContainer : public UObject
{
	GENERATED_BODY()

public:
	// 初始化容器，从变量定义列表加载默认值
	UFUNCTION(BlueprintCallable, Category = "NarrRail|Variables")
	void Initialize(const TArray<FNarrRailVariableDefinition>& VariableDefinitions);

	// 重置所有会话级变量到默认值（保留全局变量）
	UFUNCTION(BlueprintCallable, Category = "NarrRail|Variables")
	void ResetSessionVariables();

	// 清空所有变量
	UFUNCTION(BlueprintCallable, Category = "NarrRail|Variables")
	void Clear();

	// === 类型安全的读取接口 ===

	UFUNCTION(BlueprintPure, Category = "NarrRail|Variables")
	FNarrRailVariableResult GetBool(FName VariableName, bool& OutValue) const;

	UFUNCTION(BlueprintPure, Category = "NarrRail|Variables")
	FNarrRailVariableResult GetInt(FName VariableName, int32& OutValue) const;

	UFUNCTION(BlueprintPure, Category = "NarrRail|Variables")
	FNarrRailVariableResult GetFloat(FName VariableName, float& OutValue) const;

	UFUNCTION(BlueprintPure, Category = "NarrRail|Variables")
	FNarrRailVariableResult GetString(FName VariableName, FString& OutValue) const;

	// 通用读取（返回字符串表示）
	UFUNCTION(BlueprintPure, Category = "NarrRail|Variables")
	FNarrRailVariableResult GetVariable(FName VariableName, FString& OutValue) const;

	// === 类型安全的写入接口 ===

	UFUNCTION(BlueprintCallable, Category = "NarrRail|Variables")
	FNarrRailVariableResult SetBool(FName VariableName, bool Value);

	UFUNCTION(BlueprintCallable, Category = "NarrRail|Variables")
	FNarrRailVariableResult SetInt(FName VariableName, int32 Value);

	UFUNCTION(BlueprintCallable, Category = "NarrRail|Variables")
	FNarrRailVariableResult SetFloat(FName VariableName, float Value);

	UFUNCTION(BlueprintCallable, Category = "NarrRail|Variables")
	FNarrRailVariableResult SetString(FName VariableName, const FString& Value);

	// 通用写入（从字符串解析）
	UFUNCTION(BlueprintCallable, Category = "NarrRail|Variables")
	FNarrRailVariableResult SetVariable(FName VariableName, const FString& Value);

	// === 算术操作（仅支持 Int/Float）===

	UFUNCTION(BlueprintCallable, Category = "NarrRail|Variables")
	FNarrRailVariableResult AddInt(FName VariableName, int32 Delta);

	UFUNCTION(BlueprintCallable, Category = "NarrRail|Variables")
	FNarrRailVariableResult AddFloat(FName VariableName, float Delta);

	UFUNCTION(BlueprintCallable, Category = "NarrRail|Variables")
	FNarrRailVariableResult SubtractInt(FName VariableName, int32 Delta);

	UFUNCTION(BlueprintCallable, Category = "NarrRail|Variables")
	FNarrRailVariableResult SubtractFloat(FName VariableName, float Delta);

	// === 比较操作 ===

	UFUNCTION(BlueprintPure, Category = "NarrRail|Variables")
	FNarrRailVariableResult CompareBool(FName VariableName, ENarrRailComparisonOp Operator, bool CompareValue, bool& OutResult) const;

	UFUNCTION(BlueprintPure, Category = "NarrRail|Variables")
	FNarrRailVariableResult CompareInt(FName VariableName, ENarrRailComparisonOp Operator, int32 CompareValue, bool& OutResult) const;

	UFUNCTION(BlueprintPure, Category = "NarrRail|Variables")
	FNarrRailVariableResult CompareFloat(FName VariableName, ENarrRailComparisonOp Operator, float CompareValue, bool& OutResult) const;

	UFUNCTION(BlueprintPure, Category = "NarrRail|Variables")
	FNarrRailVariableResult CompareString(FName VariableName, ENarrRailComparisonOp Operator, const FString& CompareValue, bool& OutResult) const;

	// === 查询接口 ===

	UFUNCTION(BlueprintPure, Category = "NarrRail|Variables")
	bool HasVariable(FName VariableName) const;

	UFUNCTION(BlueprintPure, Category = "NarrRail|Variables")
	ENarrRailVariableType GetVariableType(FName VariableName) const;

	UFUNCTION(BlueprintPure, Category = "NarrRail|Variables")
	bool IsGlobalVariable(FName VariableName) const;

	UFUNCTION(BlueprintPure, Category = "NarrRail|Variables")
	TArray<FName> GetAllVariableNames() const;

	// === 快照与恢复（用于存档）===

	UFUNCTION(BlueprintPure, Category = "NarrRail|Variables")
	TMap<FName, FString> GetSnapshot() const;

	UFUNCTION(BlueprintCallable, Category = "NarrRail|Variables")
	void RestoreFromSnapshot(const TMap<FName, FString>& Snapshot);

	// === 变更通知 ===

	UPROPERTY(BlueprintAssignable, Category = "NarrRail|Variables")
	FNarrRailVariableChangedDelegate OnVariableChanged;

private:
	// 内部存储结构
	struct FVariableEntry
	{
		ENarrRailVariableType Type = ENarrRailVariableType::String;
		bool bGlobalScope = false;
		FString DefaultValue;
		FString CurrentValue;
	};

	// 变量存储表
	TMap<FName, FVariableEntry> Variables;

	// 内部辅助函数
	FVariableEntry* FindVariable(FName VariableName);
	const FVariableEntry* FindVariable(FName VariableName) const;
	void NotifyVariableChanged(FName VariableName, ENarrRailVariableType Type, const FString& NewValue);

	// 类型转换辅助
	static bool TryParseBool(const FString& InValue, bool& OutValue);
	static bool TryParseInt(const FString& InValue, int32& OutValue);
	static bool TryParseFloat(const FString& InValue, float& OutValue);
	static FString BoolToString(bool Value);
	static FString IntToString(int32 Value);
	static FString FloatToString(float Value);
	static FString GetDefaultValueForType(ENarrRailVariableType Type);
};
