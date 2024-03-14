#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Characters/Abilities/GSAbilitySystemComponent.h"
#include "Characters/Abilities/GSGameplayEffectTypes.h"
#include "Characters/Abilities/GSAbilityTypes.h"
#include "Characters/Abilities/GSGameplayAbility.h"
#include "GSBlueprintFunctionLibrary.generated.h"

UCLASS()
class GASSURVIVAL_API UGSBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static FString GetPlayerEditorWindowRole(UWorld* World);


	UFUNCTION(BlueprintCallable, Category = "Ability")
	static UGSGameplayAbility* GetPrimaryAbilityInstanceFromHandle(UAbilitySystemComponent* AbilitySystemComponent,
	                                                               FGameplayAbilitySpecHandle Handle);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	static UGSGameplayAbility* GetPrimaryAbilityInstanceFromClass(UAbilitySystemComponent* AbilitySystemComponent,
	                                                              TSubclassOf<UGameplayAbility> InAbilityClass);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	static bool IsPrimaryAbilityInstanceActive(UAbilitySystemComponent* AbilitySystemComponent,
	                                           FGameplayAbilitySpecHandle Handle);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	static FGameplayAbilitySpecHandle GiveAbilityWithSource(UAbilitySystemComponent* AbilitySystemComponent,
	                                                        TSubclassOf<UGameplayAbility> Ability, int32 Level,
	                                                        int32 InputID, UObject* SourceObject);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	static void GetActiveAbilitiesWithTags(UAbilitySystemComponent* AbilitySystemComponent,
	                                       FGameplayTagContainer AbilityTags,
	                                       TArray<UGSGameplayAbility*>& ActiveAbilities);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability")
	static bool IsAbilitySpecHandleValid(FGameplayAbilitySpecHandle Handle);

	UFUNCTION(BlueprintPure, Category = "Ability|Container")
	static bool DoesEffectContainerSpecHaveEffects(const FGSGameplayEffectContainerSpec& ContainerSpec);

	UFUNCTION(BlueprintPure, Category = "Ability|Container")
	static bool DoesEffectContainerSpecHaveTargets(const FGSGameplayEffectContainerSpec& ContainerSpec);

	UFUNCTION(BlueprintCallable, Category = "Ability|Container")
	static void ClearEffectContainerSpecTargets(UPARAM(ref) FGSGameplayEffectContainerSpec& ContainerSpec);

	UFUNCTION(BlueprintCallable, Category = "Ability|Container",
		Meta = (AutoCreateRefTerm = "TargetData, HitResults, TargetActors"))
	static void AddTargetsToEffectContainerSpec(UPARAM(ref) FGSGameplayEffectContainerSpec& ContainerSpec,
	                                            const TArray<FGameplayAbilityTargetDataHandle>& TargetData,
	                                            const TArray<FHitResult>& HitResults,
	                                            const TArray<AActor*>& TargetActors);
	
	UFUNCTION(BlueprintCallable, Category = "Ability|Container")
	static TArray<FActiveGameplayEffectHandle> ApplyExternalEffectContainerSpec(
		const FGSGameplayEffectContainerSpec& ContainerSpec);

	
	UFUNCTION(BlueprintCallable, Category = "Ability|EffectContext", Meta = (DisplayName = "GetTargetData"))
	static FGameplayAbilityTargetDataHandle
	EffectContextGetTargetData(FGameplayEffectContextHandle EffectContextHandle);
	
	UFUNCTION(BlueprintCallable, Category = "Ability|EffectContext", Meta = (DisplayName = "AddTargetData"))
	static void EffectContextAddTargetData(FGameplayEffectContextHandle EffectContextHandle,
	                                       const FGameplayAbilityTargetDataHandle& TargetData);
	
	UFUNCTION(BlueprintCallable, Category = "Ability|TargetData")
	static void ClearTargetData(UPARAM(ref) FGameplayAbilityTargetDataHandle& TargetData);
};
