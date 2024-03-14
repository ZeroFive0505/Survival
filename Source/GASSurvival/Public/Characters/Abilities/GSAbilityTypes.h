#pragma once

#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEffectRemoved.h"
#include "UObject/Object.h"
#include "GSAbilityTypes.generated.h"

class UGSAbilitySystemComponent;
class UGameplayEffect;
class UGSTargetType;

USTRUCT(BlueprintType)
struct FGSGameplayEffectContainer
{
	GENERATED_BODY()
	
	FGSGameplayEffectContainer() {}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	TSubclassOf<UGSTargetType> TargetType;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	TArray<TSubclassOf<UGameplayEffect>> TargetGameplayEffectClasses;
};

USTRUCT(BlueprintType)
struct FGSGameplayEffectContainerSpec
{
	GENERATED_BODY()
	
	FGSGameplayEffectContainerSpec() {}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	FGameplayAbilityTargetDataHandle TargetData;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	TArray<FGameplayEffectSpecHandle> TargetGameplayEffectSpecs;
	
	bool HasValidEffects() const;

	bool HasValidTargets() const;
	
	void AddTargets(const TArray<FGameplayAbilityTargetDataHandle>& InTargetData, const TArray<FHitResult>& HitResults, const TArray<AActor*>& TargetActors);
	
	void ClearTargets();
};