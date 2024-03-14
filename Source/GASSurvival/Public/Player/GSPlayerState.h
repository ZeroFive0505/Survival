#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/PlayerState.h"
#include "GSPlayerState.generated.h"

class UGSAttributeSetBase;
class UGSAmmoAttributeSet;
class UGSAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGSOnGameplayAttributeValueChangedDelegate, FGameplayAttribute,
                                               Attribute, float, NewValue, float, OldValue);

UCLASS()
class GASSURVIVAL_API AGSPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGSPlayerState();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UGSAttributeSetBase* GetAttributeSetBase() const;

	UGSAmmoAttributeSet* GetAmmoAttributeSet() const;
	
	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|UI")
	void ShowAbilityConfirmPrompt(bool bShowPrompt);

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|UI")
	void ShowInteractionPrompt(float InteractionDuration);

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|UI")
	void SetInteractionDescription(FText Description);

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|UI")
	void HideInteractionPrompt();

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|UI")
	void StartInteractionTimer(float InteractionDuration);
	
	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|UI")
	void StopInteractionTimer();
	
	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	float GetHealthRegenRate() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	float GetMana() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	float GetMaxMana() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	float GetManaRegenRate() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	float GetStamina() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	float GetMaxStamina() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	float GetStaminaRegenRate() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	float GetShield() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	float GetMaxShield() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	float GetShieldRegenRate() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	float GetArmor() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	float GetMoveSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	int32 GetCharacterLevel() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	int32 GetXP() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	int32 GetXPBounty() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	int32 GetGold() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	int32 GetGoldBounty() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	int32 GetPrimaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSPlayerState|Attributes")
	int32 GetPrimaryReserveAmmo() const;

protected:
	FGameplayTag DeadTag;
	FGameplayTag KnockedDownTag;

	UPROPERTY()
	UGSAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	UGSAttributeSetBase* AttributeSetBase;

	UPROPERTY()
	UGSAmmoAttributeSet* AmmoAttributeSet;
	
	FDelegateHandle HealthChangedDelegateHandle;
	
	virtual void BeginPlay() override;
	
	virtual void HealthChanged(const FOnAttributeChangeData& Data);

	virtual void KnockDownTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
};
