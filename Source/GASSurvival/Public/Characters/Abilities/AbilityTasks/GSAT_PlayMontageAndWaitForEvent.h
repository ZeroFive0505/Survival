#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GSAT_PlayMontageAndWaitForEvent.generated.h"

class UGSAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGSPlayMontageAndWaitForEventDelegate, FGameplayTag, EventTag, FGameplayEventData, EventData);

UCLASS()
class GASSURVIVAL_API UGSAT_PlayMontageAndWaitForEvent : public UAbilityTask
{
	GENERATED_BODY()

public:
	UGSAT_PlayMontageAndWaitForEvent(const FObjectInitializer& ObjectInitializer);
	
	virtual void Activate() override;
	virtual void ExternalCancel() override;
	virtual FString GetDebugString() const override;
	virtual void OnDestroy(bool AbilityEnded) override;
	
	UPROPERTY(BlueprintAssignable)
	FGSPlayMontageAndWaitForEventDelegate OnCompleted;
	
	UPROPERTY(BlueprintAssignable)
	FGSPlayMontageAndWaitForEventDelegate OnBlendOut;
	
	UPROPERTY(BlueprintAssignable)
	FGSPlayMontageAndWaitForEventDelegate OnInterrupted;
	
	UPROPERTY(BlueprintAssignable)
	FGSPlayMontageAndWaitForEventDelegate OnCancelled;
	
	UPROPERTY(BlueprintAssignable)
	FGSPlayMontageAndWaitForEventDelegate EventReceived;
	
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGSAT_PlayMontageAndWaitForEvent* PlayMontageAndWaitForEvent(
			UGameplayAbility* OwningAbility,
			FName TaskInstanceName,
			UAnimMontage* MontageToPlay,
			FGameplayTagContainer EventTags,
			float Rate = 1.f,
			FName StartSection = NAME_None,
			bool bStopWhenAbilityEnds = true,
			float AnimRootMotionTranslationScale = 1.f);

private:
	UPROPERTY()
	UAnimMontage* MontageToPlay;
	
	UPROPERTY()
	FGameplayTagContainer EventTags;
	
	UPROPERTY()
	float Rate;
	
	UPROPERTY()
	FName StartSection;
	
	UPROPERTY()
	float AnimRootMotionTranslationScale;
	
	UPROPERTY()
	bool bStopWhenAbilityEnds;
	
	bool StopPlayingMontage() const;
	
	UGSAbilitySystemComponent* GetTargetASC() const;

	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	void OnAbilityCancelled() const;
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload) const;

	FOnMontageBlendingOutStarted BlendingOutDelegate;
	FOnMontageEnded MontageEndedDelegate;
	FDelegateHandle CancelledHandle;
	FDelegateHandle EventHandle;
};
