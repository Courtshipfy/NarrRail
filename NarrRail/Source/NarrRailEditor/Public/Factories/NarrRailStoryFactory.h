#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "NarrRailStoryFactory.generated.h"

/**
 * Factory for importing .narrrail.yaml files as UNarrRailStoryAsset
 */
UCLASS()
class NARRRAILEDITOR_API UNarrRailStoryFactory : public UFactory
{
	GENERATED_BODY()

public:
	UNarrRailStoryFactory();

	// UFactory interface
	virtual bool DoesSupportClass(UClass* Class) override;
	virtual UClass* ResolveSupportedClass() override;
	virtual FText GetDisplayName() const override;
	virtual bool FactoryCanImport(const FString& Filename) override;

	virtual UObject* FactoryCreateFile(
		UClass* InClass,
		UObject* InParent,
		FName InName,
		EObjectFlags Flags,
		const FString& Filename,
		const TCHAR* Parms,
		FFeedbackContext* Warn,
		bool& bOutOperationCancelled
	) override;
};
