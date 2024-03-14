#pragma once

#include "CoreMinimal.h"
#include "GameplayCueManager.h"
#include "GSGameplayCueManager.generated.h"

UCLASS()
class GASSURVIVAL_API UGSGameplayCueManager : public UGameplayCueManager
{
	GENERATED_BODY()

public:
	virtual bool ShouldAsyncLoadRuntimeObjectLibraries() const override
	{
		//return true; // Default	
		return false;
	}
};
