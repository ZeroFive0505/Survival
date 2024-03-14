#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "GSAbilitySystemGlobals.generated.h"

UCLASS()
class GASSURVIVAL_API UGSAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()
	
public:
	UGSAbilitySystemGlobals();
	
	UPROPERTY()
	FGameplayTag DeadTag;

	UPROPERTY()
	FGameplayTag KnockedDownTag;

	UPROPERTY()
	FGameplayTag InteractingTag;

	UPROPERTY()
	FGameplayTag InteractingRemovalTag;

	static UGSAbilitySystemGlobals& GSGet()
	{
		return dynamic_cast<UGSAbilitySystemGlobals&>(Get());
	}
	
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;

	virtual void InitGlobalTags() override;
};
