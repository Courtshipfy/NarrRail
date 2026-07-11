#pragma once

#include "CoreMinimal.h"
#include "EditorReimportHandler.h"

class FNarrRailStoryReimportHandler : public FReimportHandler
{
public:
    virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
    virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
    virtual EReimportResult::Type Reimport(UObject* Obj) override;
    virtual int32 GetPriority() const override;
};
