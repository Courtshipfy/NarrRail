#include "Runtime/NarrRailVariableContainer.h"

void UNarrRailVariableContainer::Initialize(const TArray<FNarrRailVariableDefinition>& VariableDefinitions)
{
	Variables.Empty();

	for (const FNarrRailVariableDefinition& Def : VariableDefinitions)
	{
		if (Def.VariableName == NAME_None)
		{
			continue;
		}

		FVariableEntry Entry;
		Entry.Type = Def.VariableType;
		Entry.bGlobalScope = Def.bGlobalScope;
		Entry.DefaultValue = Def.DefaultValue.IsEmpty() ? GetDefaultValueForType(Def.VariableType) : Def.DefaultValue;
		Entry.CurrentValue = Entry.DefaultValue;

		Variables.Add(Def.VariableName, Entry);
	}
}

void UNarrRailVariableContainer::ResetSessionVariables()
{
	for (auto& Pair : Variables)
	{
		if (!Pair.Value.bGlobalScope)
		{
			Pair.Value.CurrentValue = Pair.Value.DefaultValue;
		}
	}
}

void UNarrRailVariableContainer::Clear()
{
	Variables.Empty();
}

// === 读取接口实现 ===

FNarrRailVariableResult UNarrRailVariableContainer::GetBool(FName VariableName, bool& OutValue) const
{
	const FVariableEntry* Entry = FindVariable(VariableName);
	if (Entry == nullptr)
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound,
			FString::Printf(TEXT("Variable '%s' not found."), *VariableName.ToString()));
	}

	if (Entry->Type != ENarrRailVariableType::Bool)
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::TypeMismatch,
			FString::Printf(TEXT("Variable '%s' is not Bool type."), *VariableName.ToString()));
	}

	if (!TryParseBool(Entry->CurrentValue, OutValue))
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::ParseError,
			FString::Printf(TEXT("Failed to parse Bool value: '%s'"), *Entry->CurrentValue));
	}

	return FNarrRailVariableResult::MakeSuccess(Entry->CurrentValue);
}

FNarrRailVariableResult UNarrRailVariableContainer::GetInt(FName VariableName, int32& OutValue) const
{
	const FVariableEntry* Entry = FindVariable(VariableName);
	if (Entry == nullptr)
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound,
			FString::Printf(TEXT("Variable '%s' not found."), *VariableName.ToString()));
	}

	if (Entry->Type != ENarrRailVariableType::Int)
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::TypeMismatch,
			FString::Printf(TEXT("Variable '%s' is not Int type."), *VariableName.ToString()));
	}

	if (!TryParseInt(Entry->CurrentValue, OutValue))
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::ParseError,
			FString::Printf(TEXT("Failed to parse Int value: '%s'"), *Entry->CurrentValue));
	}

	return FNarrRailVariableResult::MakeSuccess(Entry->CurrentValue);
}

FNarrRailVariableResult UNarrRailVariableContainer::GetFloat(FName VariableName, float& OutValue) const
{
	const FVariableEntry* Entry = FindVariable(VariableName);
	if (Entry == nullptr)
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound,
			FString::Printf(TEXT("Variable '%s' not found."), *VariableName.ToString()));
	}

	if (Entry->Type != ENarrRailVariableType::Float)
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::TypeMismatch,
			FString::Printf(TEXT("Variable '%s' is not Float type."), *VariableName.ToString()));
	}

	if (!TryParseFloat(Entry->CurrentValue, OutValue))
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::ParseError,
			FString::Printf(TEXT("Failed to parse Float value: '%s'"), *Entry->CurrentValue));
	}

	return FNarrRailVariableResult::MakeSuccess(Entry->CurrentValue);
}

FNarrRailVariableResult UNarrRailVariableContainer::GetString(FName VariableName, FString& OutValue) const
{
	const FVariableEntry* Entry = FindVariable(VariableName);
	if (Entry == nullptr)
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound,
			FString::Printf(TEXT("Variable '%s' not found."), *VariableName.ToString()));
	}

	if (Entry->Type != ENarrRailVariableType::String)
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::TypeMismatch,
			FString::Printf(TEXT("Variable '%s' is not String type."), *VariableName.ToString()));
	}

	OutValue = Entry->CurrentValue;
	return FNarrRailVariableResult::MakeSuccess(Entry->CurrentValue);
}

FNarrRailVariableResult UNarrRailVariableContainer::GetVariable(FName VariableName, FString& OutValue) const
{
	const FVariableEntry* Entry = FindVariable(VariableName);
	if (Entry == nullptr)
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound,
			FString::Printf(TEXT("Variable '%s' not found."), *VariableName.ToString()));
	}

	OutValue = Entry->CurrentValue;
	return FNarrRailVariableResult::MakeSuccess(Entry->CurrentValue);
}

// === 写入接口实现 ===

FNarrRailVariableResult UNarrRailVariableContainer::SetBool(FName VariableName, bool Value)
{
	FVariableEntry* Entry = FindVariable(VariableName);
	if (Entry == nullptr)
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound,
			FString::Printf(TEXT("Variable '%s' not found."), *VariableName.ToString()));
	}

	if (Entry->Type != ENarrRailVariableType::Bool)
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::TypeMismatch,
			FString::Printf(TEXT("Variable '%s' is not Bool type."), *VariableName.ToString()));
	}

	const FString NewValue = BoolToString(Value);
	Entry->CurrentValue = NewValue;
	NotifyVariableChanged(VariableName, Entry->Type, NewValue);

	return FNarrRailVariableResult::MakeSuccess(NewValue);
}

FNarrRailVariableResult UNarrRailVariableContainer::SetInt(FName VariableName, int32 Value)
{
	FVariableEntry* Entry = FindVariable(VariableName);
	if (Entry == nullptr)
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound,
			FString::Printf(TEXT("Variable '%s' not found."), *VariableName.ToString()));
	}

	if (Entry->Type != ENarrRailVariableType::Int)
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::TypeMismatch,
			FString::Printf(TEXT("Variable '%s' is not Int type."), *VariableName.ToString()));
	}

	const FString NewValue = IntToString(Value);
	Entry->CurrentValue = NewValue;
	NotifyVariableChanged(VariableName, Entry->Type, NewValue);

	return FNarrRailVariableResult::MakeSuccess(NewValue);
}

FNarrRailVariableResult UNarrRailVariableContainer::SetFloat(FName VariableName, float Value)
{
	FVariableEntry* Entry = FindVariable(VariableName);
	if (Entry == nullptr)
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound,
			FString::Printf(TEXT("Variable '%s' not found."), *VariableName.ToString()));
	}

	if (Entry->Type != ENarrRailVariableType::Float)
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::TypeMismatch,
			FString::Printf(TEXT("Variable '%s' is not Float type."), *VariableName.ToString()));
	}

	const FString NewValue = FloatToString(Value);
	Entry->CurrentValue = NewValue;
	NotifyVariableChanged(VariableName, Entry->Type, NewValue);

	return FNarrRailVariableResult::MakeSuccess(NewValue);
}

FNarrRailVariableResult UNarrRailVariableContainer::SetString(FName VariableName, const FString& Value)
{
	FVariableEntry* Entry = FindVariable(VariableName);
	if (Entry == nullptr)
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound,
			FString::Printf(TEXT("Variable '%s' not found."), *VariableName.ToString()));
	}

	if (Entry->Type != ENarrRailVariableType::String)
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::TypeMismatch,
			FString::Printf(TEXT("Variable '%s' is not String type."), *VariableName.ToString()));
	}

	Entry->CurrentValue = Value;
	NotifyVariableChanged(VariableName, Entry->Type, Value);

	return FNarrRailVariableResult::MakeSuccess(Value);
}

FNarrRailVariableResult UNarrRailVariableContainer::SetVariable(FName VariableName, const FString& Value)
{
	FVariableEntry* Entry = FindVariable(VariableName);
	if (Entry == nullptr)
	{
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::VariableNotFound,
			FString::Printf(TEXT("Variable '%s' not found."), *VariableName.ToString()));
	}

	// 验证值是否可以解析为目标类型
	switch (Entry->Type)
	{
	case ENarrRailVariableType::Bool:
	{
		bool Dummy;
		if (!TryParseBool(Value, Dummy))
		{
			return FNarrRailVariableResult::MakeError(ENarrRailVariableError::ParseError,
				FString::Printf(TEXT("Cannot parse '%s' as Bool."), *Value));
		}
		break;
	}
	case ENarrRailVariableType::Int:
	{
		int32 Dummy;
		if (!TryParseInt(Value, Dummy))
		{
			return FNarrRailVariableResult::MakeError(ENarrRailVariableError::ParseError,
				FString::Printf(TEXT("Cannot parse '%s' as Int."), *Value));
		}
		break;
	}
	case ENarrRailVariableType::Float:
	{
		float Dummy;
		if (!TryParseFloat(Value, Dummy))
		{
			return FNarrRailVariableResult::MakeError(ENarrRailVariableError::ParseError,
				FString::Printf(TEXT("Cannot parse '%s' as Float."), *Value));
		}
		break;
	}
	case ENarrRailVariableType::String:
	default:
		break;
	}

	Entry->CurrentValue = Value;
	NotifyVariableChanged(VariableName, Entry->Type, Value);

	return FNarrRailVariableResult::MakeSuccess(Value);
}

// === 算术操作实现 ===

FNarrRailVariableResult UNarrRailVariableContainer::AddInt(FName VariableName, int32 Delta)
{
	int32 CurrentValue = 0;
	FNarrRailVariableResult GetResult = GetInt(VariableName, CurrentValue);
	if (!GetResult.IsSuccess())
	{
		return GetResult;
	}

	return SetInt(VariableName, CurrentValue + Delta);
}

FNarrRailVariableResult UNarrRailVariableContainer::AddFloat(FName VariableName, float Delta)
{
	float CurrentValue = 0.f;
	FNarrRailVariableResult GetResult = GetFloat(VariableName, CurrentValue);
	if (!GetResult.IsSuccess())
	{
		return GetResult;
	}

	return SetFloat(VariableName, CurrentValue + Delta);
}

FNarrRailVariableResult UNarrRailVariableContainer::SubtractInt(FName VariableName, int32 Delta)
{
	int32 CurrentValue = 0;
	FNarrRailVariableResult GetResult = GetInt(VariableName, CurrentValue);
	if (!GetResult.IsSuccess())
	{
		return GetResult;
	}

	return SetInt(VariableName, CurrentValue - Delta);
}

FNarrRailVariableResult UNarrRailVariableContainer::SubtractFloat(FName VariableName, float Delta)
{
	float CurrentValue = 0.f;
	FNarrRailVariableResult GetResult = GetFloat(VariableName, CurrentValue);
	if (!GetResult.IsSuccess())
	{
		return GetResult;
	}

	return SetFloat(VariableName, CurrentValue - Delta);
}

// === 比较操作实现 ===

FNarrRailVariableResult UNarrRailVariableContainer::CompareBool(FName VariableName, ENarrRailComparisonOp Operator, bool CompareValue, bool& OutResult) const
{
	bool CurrentValue = false;
	FNarrRailVariableResult GetResult = GetBool(VariableName, CurrentValue);
	if (!GetResult.IsSuccess())
	{
		return GetResult;
	}

	switch (Operator)
	{
	case ENarrRailComparisonOp::Equal:
		OutResult = (CurrentValue == CompareValue);
		break;
	case ENarrRailComparisonOp::NotEqual:
		OutResult = (CurrentValue != CompareValue);
		break;
	default:
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::InvalidOperation,
			TEXT("Bool only supports == and != operators."));
	}

	return FNarrRailVariableResult::MakeSuccess();
}

FNarrRailVariableResult UNarrRailVariableContainer::CompareInt(FName VariableName, ENarrRailComparisonOp Operator, int32 CompareValue, bool& OutResult) const
{
	int32 CurrentValue = 0;
	FNarrRailVariableResult GetResult = GetInt(VariableName, CurrentValue);
	if (!GetResult.IsSuccess())
	{
		return GetResult;
	}

	switch (Operator)
	{
	case ENarrRailComparisonOp::Equal: OutResult = (CurrentValue == CompareValue); break;
	case ENarrRailComparisonOp::NotEqual: OutResult = (CurrentValue != CompareValue); break;
	case ENarrRailComparisonOp::Greater: OutResult = (CurrentValue > CompareValue); break;
	case ENarrRailComparisonOp::GreaterOrEqual: OutResult = (CurrentValue >= CompareValue); break;
	case ENarrRailComparisonOp::Less: OutResult = (CurrentValue < CompareValue); break;
	case ENarrRailComparisonOp::LessOrEqual: OutResult = (CurrentValue <= CompareValue); break;
	default:
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::InvalidOperation,
			TEXT("Unknown comparison operator."));
	}

	return FNarrRailVariableResult::MakeSuccess();
}

FNarrRailVariableResult UNarrRailVariableContainer::CompareFloat(FName VariableName, ENarrRailComparisonOp Operator, float CompareValue, bool& OutResult) const
{
	float CurrentValue = 0.f;
	FNarrRailVariableResult GetResult = GetFloat(VariableName, CurrentValue);
	if (!GetResult.IsSuccess())
	{
		return GetResult;
	}

	switch (Operator)
	{
	case ENarrRailComparisonOp::Equal: OutResult = FMath::IsNearlyEqual(CurrentValue, CompareValue); break;
	case ENarrRailComparisonOp::NotEqual: OutResult = !FMath::IsNearlyEqual(CurrentValue, CompareValue); break;
	case ENarrRailComparisonOp::Greater: OutResult = (CurrentValue > CompareValue); break;
	case ENarrRailComparisonOp::GreaterOrEqual: OutResult = (CurrentValue >= CompareValue); break;
	case ENarrRailComparisonOp::Less: OutResult = (CurrentValue < CompareValue); break;
	case ENarrRailComparisonOp::LessOrEqual: OutResult = (CurrentValue <= CompareValue); break;
	default:
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::InvalidOperation,
			TEXT("Unknown comparison operator."));
	}

	return FNarrRailVariableResult::MakeSuccess();
}

FNarrRailVariableResult UNarrRailVariableContainer::CompareString(FName VariableName, ENarrRailComparisonOp Operator, const FString& CompareValue, bool& OutResult) const
{
	FString CurrentValue;
	FNarrRailVariableResult GetResult = GetString(VariableName, CurrentValue);
	if (!GetResult.IsSuccess())
	{
		return GetResult;
	}

	const int32 CompareResult = CurrentValue.Compare(CompareValue, ESearchCase::CaseSensitive);

	switch (Operator)
	{
	case ENarrRailComparisonOp::Equal: OutResult = (CompareResult == 0); break;
	case ENarrRailComparisonOp::NotEqual: OutResult = (CompareResult != 0); break;
	case ENarrRailComparisonOp::Greater: OutResult = (CompareResult > 0); break;
	case ENarrRailComparisonOp::GreaterOrEqual: OutResult = (CompareResult >= 0); break;
	case ENarrRailComparisonOp::Less: OutResult = (CompareResult < 0); break;
	case ENarrRailComparisonOp::LessOrEqual: OutResult = (CompareResult <= 0); break;
	default:
		return FNarrRailVariableResult::MakeError(ENarrRailVariableError::InvalidOperation,
			TEXT("Unknown comparison operator."));
	}

	return FNarrRailVariableResult::MakeSuccess();
}

// === 查询接口实现 ===

bool UNarrRailVariableContainer::HasVariable(FName VariableName) const
{
	return Variables.Contains(VariableName);
}

ENarrRailVariableType UNarrRailVariableContainer::GetVariableType(FName VariableName) const
{
	const FVariableEntry* Entry = FindVariable(VariableName);
	return Entry ? Entry->Type : ENarrRailVariableType::String;
}

bool UNarrRailVariableContainer::IsGlobalVariable(FName VariableName) const
{
	const FVariableEntry* Entry = FindVariable(VariableName);
	return Entry ? Entry->bGlobalScope : false;
}

TArray<FName> UNarrRailVariableContainer::GetAllVariableNames() const
{
	TArray<FName> Names;
	Variables.GetKeys(Names);
	return Names;
}

// === 快照与恢复实现 ===

TMap<FName, FString> UNarrRailVariableContainer::GetSnapshot() const
{
	TMap<FName, FString> Snapshot;
	for (const auto& Pair : Variables)
	{
		Snapshot.Add(Pair.Key, Pair.Value.CurrentValue);
	}
	return Snapshot;
}

void UNarrRailVariableContainer::RestoreFromSnapshot(const TMap<FName, FString>& Snapshot)
{
	for (const auto& Pair : Snapshot)
	{
		FVariableEntry* Entry = FindVariable(Pair.Key);
		if (Entry != nullptr)
		{
			Entry->CurrentValue = Pair.Value;
		}
	}
}

// === 内部辅助函数实现 ===

UNarrRailVariableContainer::FVariableEntry* UNarrRailVariableContainer::FindVariable(FName VariableName)
{
	return Variables.Find(VariableName);
}

const UNarrRailVariableContainer::FVariableEntry* UNarrRailVariableContainer::FindVariable(FName VariableName) const
{
	return Variables.Find(VariableName);
}

void UNarrRailVariableContainer::NotifyVariableChanged(FName VariableName, ENarrRailVariableType Type, const FString& NewValue)
{
	OnVariableChanged.Broadcast(VariableName, Type, NewValue);
}

// === 类型转换辅助实现 ===

bool UNarrRailVariableContainer::TryParseBool(const FString& InValue, bool& OutValue)
{
	if (InValue.Equals(TEXT("true"), ESearchCase::IgnoreCase) || InValue == TEXT("1"))
	{
		OutValue = true;
		return true;
	}

	if (InValue.Equals(TEXT("false"), ESearchCase::IgnoreCase) || InValue == TEXT("0"))
	{
		OutValue = false;
		return true;
	}

	return false;
}

bool UNarrRailVariableContainer::TryParseInt(const FString& InValue, int32& OutValue)
{
	return LexTryParseString(OutValue, *InValue);
}

bool UNarrRailVariableContainer::TryParseFloat(const FString& InValue, float& OutValue)
{
	return LexTryParseString(OutValue, *InValue);
}

FString UNarrRailVariableContainer::BoolToString(bool Value)
{
	return Value ? TEXT("true") : TEXT("false");
}

FString UNarrRailVariableContainer::IntToString(int32 Value)
{
	return FString::FromInt(Value);
}

FString UNarrRailVariableContainer::FloatToString(float Value)
{
	return FString::SanitizeFloat(Value);
}

FString UNarrRailVariableContainer::GetDefaultValueForType(ENarrRailVariableType Type)
{
	switch (Type)
	{
	case ENarrRailVariableType::Bool: return TEXT("false");
	case ENarrRailVariableType::Int: return TEXT("0");
	case ENarrRailVariableType::Float: return TEXT("0.0");
	case ENarrRailVariableType::String:
	default: return TEXT("");
	}
}
