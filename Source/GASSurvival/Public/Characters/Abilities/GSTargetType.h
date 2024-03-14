#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GSTargetType.generated.h"

class AGSCharacter;

UCLASS(Blueprintable, meta = (ShowWorldContextPin))
class GASSURVIVAL_API UGSTargetType : public UObject
{
	GENERATED_BODY()

public:
	UGSTargetType()
	{
	}

	UFUNCTION(BlueprintNativeEvent)
	void GetTargets(AGSCharacter* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData,
	                TArray<FGameplayAbilityTargetDataHandle>& OutTargetData, TArray<FHitResult>& OutHitResults,
	                TArray<AActor*>& OutActors) const;
	virtual void GetTargets_Implementation(AGSCharacter* TargetingCharacter, AActor* TargetingActor,
	                                       FGameplayEventData EventData,
	                                       TArray<FGameplayAbilityTargetDataHandle>& OutTargetData,
	                                       TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const;
};

UCLASS(NotBlueprintable)
class GASSURVIVAL_API UGSTargetType_UseOwner : public UGSTargetType
{
	GENERATED_BODY()

public:
	UGSTargetType_UseOwner()
	{
	}
	
	virtual void GetTargets_Implementation(AGSCharacter* TargetingCharacter, AActor* TargetingActor,
	                                       FGameplayEventData EventData,
	                                       TArray<FGameplayAbilityTargetDataHandle>& OutTargetData,
	                                       TArray<FHitResult>& OutHitResults,
	                                       TArray<AActor*>& OutActors) const override;
};

UCLASS(NotBlueprintable)
class GASSURVIVAL_API UGSTargetType_UseEventData : public UGSTargetType
{
	GENERATED_BODY()

public:
	UGSTargetType_UseEventData()
	{
	}
	
	virtual void GetTargets_Implementation(AGSCharacter* TargetingCharacter, AActor* TargetingActor,
	                                       FGameplayEventData EventData,
	                                       TArray<FGameplayAbilityTargetDataHandle>& OutTargetData,
	                                       TArray<FHitResult>& OutHitResults,
	                                       TArray<AActor*>& OutActors) const override;
};
