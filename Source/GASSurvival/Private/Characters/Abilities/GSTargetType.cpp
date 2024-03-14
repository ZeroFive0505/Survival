#include "GASSurvival/Public/Characters/Abilities/GSTargetType.h"
#include "Characters/GSCharacter.h"

#include "Abilities/GameplayAbilityTypes.h"

void UGSTargetType::GetTargets_Implementation(AGSCharacter* TargetingCharacter, AActor* TargetingActor,
                                              FGameplayEventData EventData,
                                              TArray<FGameplayAbilityTargetDataHandle>& OutTargetData,
                                              TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const
{
}

void UGSTargetType_UseOwner::GetTargets_Implementation(AGSCharacter* TargetingCharacter, AActor* TargetingActor,
                                                       FGameplayEventData EventData,
                                                       TArray<FGameplayAbilityTargetDataHandle>& OutTargetData,
                                                       TArray<FHitResult>& OutHitResults,
                                                       TArray<AActor*>& OutActors) const
{
	OutActors.Add(TargetingCharacter);
}

void UGSTargetType_UseEventData::GetTargets_Implementation(AGSCharacter* TargetingCharacter, AActor* TargetingActor,
                                                           FGameplayEventData EventData,
                                                           TArray<FGameplayAbilityTargetDataHandle>& OutTargetData,
                                                           TArray<FHitResult>& OutHitResults,
                                                           TArray<AActor*>& OutActors) const
{
	const FHitResult* FoundHitResult = EventData.ContextHandle.GetHitResult();
	
	if (FoundHitResult)
	{
		OutHitResults.Add(*FoundHitResult);
	}
	else if (EventData.Target)
	{
		OutActors.Add(const_cast<AActor*>(EventData.Target.Get()));
	}
}
