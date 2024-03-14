﻿#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GSAT_WaitInputPressWithTags.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInputPressWithTagsDelegate, float, TimeWaited);

UCLASS()
class GASSURVIVAL_API UGSAT_WaitInputPressWithTags : public UAbilityTask
{
	GENERATED_BODY()

public:
	UGSAT_WaitInputPressWithTags(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable)
	FInputPressWithTagsDelegate OnPress;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
		meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGSAT_WaitInputPressWithTags* WaitInputPressWithTags(UGameplayAbility* OwningAbility,
	                                                            FGameplayTagContainer RequiredTags,
	                                                            FGameplayTagContainer IgnoredTags,
	                                                            bool bTestAlreadyPressed = false);

	virtual void Activate() override;

	UFUNCTION()
	void OnPressCallback();

protected:
	float StartTime;

	bool bTestInitialState;

	FDelegateHandle DelegateHandle;

	FGameplayTagContainer RequiredTags;
	FGameplayTagContainer IgnoredTags;

	virtual void OnDestroy(bool AbilityEnded) override;
	
	virtual void Reset();
};
