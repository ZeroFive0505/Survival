#include "Characters/Abilities/AbilityTasks/GSAT_ServerWaitForClientTargetData.h"

#include "AbilitySystemComponent.h"

UGSAT_ServerWaitForClientTargetData::UGSAT_ServerWaitForClientTargetData(
	const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

UGSAT_ServerWaitForClientTargetData* UGSAT_ServerWaitForClientTargetData::ServerWaitForClientTargetData(
	UGameplayAbility* OwningAbility, FName TaskInstanceName, bool TriggerOnce)
{
	UGSAT_ServerWaitForClientTargetData* Task = NewAbilityTask<UGSAT_ServerWaitForClientTargetData>(
		OwningAbility, TaskInstanceName);
	Task->bTriggerOnce = TriggerOnce;
	return Task;
}

void UGSAT_ServerWaitForClientTargetData::Activate()
{
	if (!Ability || !Ability->GetCurrentActorInfo()->IsNetAuthority())
	{
		return;
	}

	const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
	const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
	AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(
		this, &UGSAT_ServerWaitForClientTargetData::OnTargetDataReplicatedCallback);
}

void UGSAT_ServerWaitForClientTargetData::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data,
                                                                         FGameplayTag ActivationTag)
{
	const FGameplayAbilityTargetDataHandle MutableData = Data;
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(MutableData);
	}

	if (bTriggerOnce)
	{
		EndTask();
	}
}

void UGSAT_ServerWaitForClientTargetData::OnDestroy(bool AbilityEnded)
{
	if (AbilitySystemComponent.IsValid())
	{
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
		AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).RemoveAll(this);
	}

	Super::OnDestroy(AbilityEnded);
}
