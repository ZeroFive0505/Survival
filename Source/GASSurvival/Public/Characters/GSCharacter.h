#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "GSCharacter.generated.h"

class UDamageTextDisplayComponent;
class UGSAbilitySystemComponent;
class UGSGameplayAbility;
class UGSDamageTextWidgetComponent;
class USoundCue;
class UGameplayEffect;
class UGSAttributeSetBase;
class UGSFloatingStatusBarWidget;
class UWidgetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCharacterOnDiedDelegate, AGSCharacter*, Character);

UCLASS()
class GASSURVIVAL_API AGSCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGSCharacter(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable, Category = "GASSurvival|GSCharacter")
	FCharacterOnDiedDelegate OnCharacterDied;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSCharacter")
	virtual bool IsAlive() const;
	
	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSCharacter")
	virtual int32 GetAbilityLevel(EGSAbilityInputID AbilityID) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "GASSurvival|GSCharacter")
	FName GetHeadBoneName() const;

	virtual void PossessedBy(AController* NewController) override;
	
	virtual void RemoveCharacterAbilities();

	virtual void InitializeFloatingStatusBar();
	
	virtual void Die();

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSCharacter")
	virtual void FinishDying();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "GASSurvival|GSCharacter")
	void ApplyForceAtBone(const FName BoneName, float ForceAmount, FVector Direction);

	virtual void AddDamageNumber(float Damage, FGameplayTagContainer DamageNumberTags);
	
	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSCharacter|Attributes")
	int32 GetCharacterLevel() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSCharacter|Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSCharacter|Attributes")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSCharacter|Attributes")
	float GetMana() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSCharacter|Attributes")
	float GetMaxMana() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSCharacter|Attributes")
	float GetStamina() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSCharacter|Attributes")
	float GetMaxStamina() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSCharacter|Attributes")
	float GetShield() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSCharacter|Attributes")
	float GetMaxShield() const;
	
	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSCharacter|Attributes")
	float GetMoveSpeed() const;
	
	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSCharacter|Attributes")
	float GetMoveSpeedBaseValue() const;

protected:
	FGameplayTag DeadTag;
	FGameplayTag EffectRemoveOnDeathTag;
	
	UPROPERTY()
	UGSAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY()
	UGSAttributeSetBase* AttributeSetBase;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASSurvival|GSCharacter")
	FText CharacterName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASSurvival|Animation")
	UAnimMontage* DeathMontage;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASSurvival|Audio")
	USoundCue* DeathSound;

	UPROPERTY()
	UGSFloatingStatusBarWidget* UIFloatingStatusBar;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "GASSurvival|UI")
	UWidgetComponent* UIFloatingStatusBarComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GASSurvival|UI")
	TSubclassOf<UGSFloatingStatusBarWidget> UIFloatingStatusBarClass;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASSurvival|Abilities")
	TArray<TSubclassOf<UGSGameplayAbility>> CharacterAbilities;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASSurvival|Abilities")
	TSubclassOf<UGameplayEffect> DefaultAttributes;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASSurvival|Abilities")
	TArray<TSubclassOf<UGameplayEffect>> StartupEffects;

	UPROPERTY(EditDefaultsOnly)
	UDamageTextDisplayComponent* DamageTextDisplayComponent;
	
	virtual void BeginPlay() override;
	
	virtual void AddCharacterAbilities();
	
	virtual void InitializeAttributes();

	virtual void AddStartupEffects();
	
	virtual void SetHealth(float Health);
	virtual void SetMana(float Mana);
	virtual void SetStamina(float Stamina);
	virtual void SetShield(float Shield);
};
