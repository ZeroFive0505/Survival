#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GSAT_WaitDelayOneFrame.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaitDelayOneFrameDelegate);

/**
 * 
 */
UCLASS()
class GASSURVIVAL_API UGSAT_WaitDelayOneFrame : public UAbilityTask
{
	GENERATED_BODY()

public:
	UGSAT_WaitDelayOneFrame(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable)
	FWaitDelayOneFrameDelegate OnFinish;

	virtual void Activate() override;
	
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGSAT_WaitDelayOneFrame* WaitDelayOneFrame(UGameplayAbility* OwningAbility);

private:
	void OnDelayFinish();
};
