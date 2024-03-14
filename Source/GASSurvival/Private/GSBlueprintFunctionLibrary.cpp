#include "GSBlueprintFunctionLibrary.h"

FString UGSBlueprintFunctionLibrary::GetPlayerEditorWindowRole(UWorld* World)
{
	FString Prefix;
	if (World)
	{
		if (World->WorldType == EWorldType::PIE)
		{
			switch (World->GetNetMode())
			{
			case NM_Client:
				Prefix = FString::Printf(TEXT("Client %d "), GPlayInEditorID - 1);
				break;
			case NM_DedicatedServer:
			case NM_ListenServer:
				Prefix = FString::Printf(TEXT("Server "));
				break;
			case NM_Standalone:
				break;
			default:
				break;
			}
		}
	}

	return Prefix;
}

UGSGameplayAbility* UGSBlueprintFunctionLibrary::GetPrimaryAbilityInstanceFromHandle(
	UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle)
{
	if (AbilitySystemComponent)
	{
		const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
		
		if (AbilitySpec)
		{
			return Cast<UGSGameplayAbility>(AbilitySpec->GetPrimaryInstance());
		}
	}

	return nullptr;
}

UGSGameplayAbility* UGSBlueprintFunctionLibrary::GetPrimaryAbilityInstanceFromClass(
	UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UGameplayAbility> InAbilityClass)
{
	if (AbilitySystemComponent)
	{
		const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(InAbilityClass);
		
		if (AbilitySpec)
		{
			return Cast<UGSGameplayAbility>(AbilitySpec->GetPrimaryInstance());
		}
	}

	return nullptr;
}

bool UGSBlueprintFunctionLibrary::IsPrimaryAbilityInstanceActive(UAbilitySystemComponent* AbilitySystemComponent,
	FGameplayAbilitySpecHandle Handle)
{
	if (AbilitySystemComponent)
	{
		const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
		
		if (AbilitySpec)
		{
			return Cast<UGSGameplayAbility>(AbilitySpec->GetPrimaryInstance())->IsActive();
		}
	}

	return false;
}

FGameplayAbilitySpecHandle UGSBlueprintFunctionLibrary::GiveAbilityWithSource(UAbilitySystemComponent* AbilitySystemComponent,
	TSubclassOf<UGameplayAbility> Ability, int32 Level, int32 InputID, UObject* SourceObject)
{
	return AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability, Level, InputID, SourceObject));
}

void UGSBlueprintFunctionLibrary::GetActiveAbilitiesWithTags(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTagContainer AbilityTags,
	TArray<UGSGameplayAbility*>& ActiveAbilities)
{
	if(AbilitySystemComponent)
	{
		TArray<FGameplayAbilitySpec*> AbilitiesToActive;
		AbilitySystemComponent->GetActivatableGameplayAbilitySpecsByAllMatchingTags(AbilityTags, AbilitiesToActive, false);

		for(const FGameplayAbilitySpec* Spec : AbilitiesToActive)
		{
			TArray<UGameplayAbility*> AbilityInstances = Spec->GetAbilityInstances();

			for(UGameplayAbility* ActiveAbility : AbilityInstances)
			{
				ActiveAbilities.Add(Cast<UGSGameplayAbility>(ActiveAbility));
			}
		}
	}
}

bool UGSBlueprintFunctionLibrary::IsAbilitySpecHandleValid(FGameplayAbilitySpecHandle Handle)
{
	return Handle.IsValid();
}

bool UGSBlueprintFunctionLibrary::DoesEffectContainerSpecHaveEffects(
	const FGSGameplayEffectContainerSpec& ContainerSpec)
{
	return ContainerSpec.HasValidEffects();
}

bool UGSBlueprintFunctionLibrary::DoesEffectContainerSpecHaveTargets(
	const FGSGameplayEffectContainerSpec& ContainerSpec)
{
	return ContainerSpec.HasValidTargets();
}

void UGSBlueprintFunctionLibrary::ClearEffectContainerSpecTargets(FGSGameplayEffectContainerSpec& ContainerSpec)
{
	ContainerSpec.ClearTargets();
}

void UGSBlueprintFunctionLibrary::AddTargetsToEffectContainerSpec(FGSGameplayEffectContainerSpec& ContainerSpec,
	const TArray<FGameplayAbilityTargetDataHandle>& TargetData, const TArray<FHitResult>& HitResults,
	const TArray<AActor*>& TargetActors)
{
	ContainerSpec.AddTargets(TargetData, HitResults, TargetActors);
}

TArray<FActiveGameplayEffectHandle> UGSBlueprintFunctionLibrary::ApplyExternalEffectContainerSpec(
	const FGSGameplayEffectContainerSpec& ContainerSpec)
{
	TArray<FActiveGameplayEffectHandle> AllEffects;
	
	for (const FGameplayEffectSpecHandle& SpecHandle : ContainerSpec.TargetGameplayEffectSpecs)
	{
		if (SpecHandle.IsValid())
		{
			for (const TSharedPtr<FGameplayAbilityTargetData> Data : ContainerSpec.TargetData.Data)
			{
				AllEffects.Append(Data->ApplyGameplayEffectSpec(*SpecHandle.Data.Get()));
			}
		}
	}
	
	return AllEffects;
}

FGameplayAbilityTargetDataHandle UGSBlueprintFunctionLibrary::EffectContextGetTargetData(
	FGameplayEffectContextHandle EffectContextHandle)
{
	FGSGameplayEffectContext* EffectContext = static_cast<FGSGameplayEffectContext*>(EffectContextHandle.Get());
	
	if (EffectContext)
	{
		return EffectContext->GetTargetData();
	}

	return FGameplayAbilityTargetDataHandle();
}

void UGSBlueprintFunctionLibrary::EffectContextAddTargetData(FGameplayEffectContextHandle EffectContextHandle,
                                                             const FGameplayAbilityTargetDataHandle& TargetData)
{
	FGSGameplayEffectContext* EffectContext = static_cast<FGSGameplayEffectContext*>(EffectContextHandle.Get());
	
	if (EffectContext)
	{
		EffectContext->AddTargetData(TargetData);
	}
}

void UGSBlueprintFunctionLibrary::ClearTargetData(FGameplayAbilityTargetDataHandle& TargetData)
{
	TargetData.Clear();
}
