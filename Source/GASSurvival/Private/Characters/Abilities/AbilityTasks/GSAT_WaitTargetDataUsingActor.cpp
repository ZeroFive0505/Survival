// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Abilities/AbilityTasks/GSAT_WaitTargetDataUsingActor.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Characters/Abilities/GSGATA_Trace.h"

UGSAT_WaitTargetDataUsingActor::UGSAT_WaitTargetDataUsingActor(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer)
{
}

UGSAT_WaitTargetDataUsingActor* UGSAT_WaitTargetDataUsingActor::WaitTargetDataWithReusableActor(
	UGameplayAbility* OwningAbility, FName TaskInstanceName,
	TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, AGameplayAbilityTargetActor* InTargetActor,
	bool bCreateKeyIfNotValidForMorePredicting)
{
	UGSAT_WaitTargetDataUsingActor* Task = NewAbilityTask<UGSAT_WaitTargetDataUsingActor>(
		OwningAbility, TaskInstanceName);
	Task->TargetActor = InTargetActor;
	Task->ConfirmationType = ConfirmationType;
	Task->bCreateKeyIfNotValidForMorePredicting = bCreateKeyIfNotValidForMorePredicting;
	return Task;
}

void UGSAT_WaitTargetDataUsingActor::Activate()
{
	if (!IsValid(this))
	{
		return;
	}

	if (Ability && TargetActor)
	{
		InitializeTargetActor();
		RegisterTargetDataCallbacks();
		FinalizeTargetActor();
	}
	else
	{
		EndTask();
	}
}

void UGSAT_WaitTargetDataUsingActor::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data,
                                                                    FGameplayTag ActivationTag)
{
	check(AbilitySystemComponent.IsValid());

	FGameplayAbilityTargetDataHandle MutableData = Data;
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());

	if (TargetActor && !TargetActor->OnReplicatedTargetDataReceived(MutableData))
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			Cancelled.Broadcast(MutableData);
		}
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			ValidData.Broadcast(MutableData);
		}
	}

	if (ConfirmationType != EGameplayTargetingConfirmation::CustomMulti)
	{
		EndTask();
	}
}

void UGSAT_WaitTargetDataUsingActor::OnTargetDataReplicatedCancelledCallback()
{
	check(AbilitySystemComponent.IsValid());
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		Cancelled.Broadcast(FGameplayAbilityTargetDataHandle());
	}

	EndTask();
}

void UGSAT_WaitTargetDataUsingActor::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& Data)
{
	check(AbilitySystemComponent.IsValid());
	if (!Ability)
	{
		return;
	}

	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get(),
	                                         ShouldReplicateDataToServer() && (bCreateKeyIfNotValidForMorePredicting &&
		                                         !AbilitySystemComponent->ScopedPredictionKey.
		                                                                  IsValidForMorePrediction()));

	const FGameplayAbilityActorInfo* Info = Ability->GetCurrentActorInfo();
	if (IsPredictingClient())
	{
		if (!TargetActor->ShouldProduceTargetDataOnServer)
		{
			const FGameplayTag ApplicationTag;
			AbilitySystemComponent->CallServerSetReplicatedTargetData(GetAbilitySpecHandle(),
			                                                          GetActivationPredictionKey(), Data,
			                                                          ApplicationTag,
			                                                          AbilitySystemComponent->ScopedPredictionKey);
		}
		else if (ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed)
		{
			AbilitySystemComponent->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericConfirm,
			                                                 GetAbilitySpecHandle(), GetActivationPredictionKey(),
			                                                 AbilitySystemComponent->ScopedPredictionKey);
		}
	}

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(Data);
	}

	if (ConfirmationType != EGameplayTargetingConfirmation::CustomMulti)
	{
		EndTask();
	}
}

void UGSAT_WaitTargetDataUsingActor::OnTargetDataCancelledCallback(const FGameplayAbilityTargetDataHandle& Data)
{
	check(AbilitySystemComponent.IsValid());

	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get(), IsPredictingClient());

	if (IsPredictingClient())
	{
		if (!TargetActor->ShouldProduceTargetDataOnServer)
		{
			AbilitySystemComponent->ServerSetReplicatedTargetDataCancelled(
				GetAbilitySpecHandle(), GetActivationPredictionKey(), AbilitySystemComponent->ScopedPredictionKey);
		}
		else
		{
			AbilitySystemComponent->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericCancel,
			                                                 GetAbilitySpecHandle(), GetActivationPredictionKey(),
			                                                 AbilitySystemComponent->ScopedPredictionKey);
		}
	}

	Cancelled.Broadcast(Data);
	EndTask();
}

void UGSAT_WaitTargetDataUsingActor::ExternalConfirm(bool bEndTask)
{
	check(AbilitySystemComponent.IsValid());
	if (TargetActor)
	{
		if (TargetActor->ShouldProduceTargetData())
		{
			TargetActor->ConfirmTargetingAndContinue();
		}
	}
	Super::ExternalConfirm(bEndTask);
}

void UGSAT_WaitTargetDataUsingActor::ExternalCancel()
{
	check(AbilitySystemComponent.IsValid());
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		Cancelled.Broadcast(FGameplayAbilityTargetDataHandle());
	}
	Super::ExternalCancel();
}

void UGSAT_WaitTargetDataUsingActor::InitializeTargetActor()
{
	check(TargetActor);
	check(Ability);

	TargetActor->PrimaryPC = Ability->GetCurrentActorInfo()->PlayerController.Get();

	TargetActor->TargetDataReadyDelegate.AddUObject(this,
	                                                &UGSAT_WaitTargetDataUsingActor::OnTargetDataReadyCallback);
	TargetActor->CanceledDelegate.AddUObject(this,
	                                         &UGSAT_WaitTargetDataUsingActor::OnTargetDataCancelledCallback);
}

void UGSAT_WaitTargetDataUsingActor::FinalizeTargetActor() const
{
	check(TargetActor);
	check(Ability);

	TargetActor->StartTargeting(Ability);

	if (TargetActor->ShouldProduceTargetData())
	{
		if (ConfirmationType == EGameplayTargetingConfirmation::Instant)
		{
			TargetActor->ConfirmTargeting();
		}
		else if (ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed)
		{
			TargetActor->BindToConfirmCancelInputs();
		}
	}
}

void UGSAT_WaitTargetDataUsingActor::RegisterTargetDataCallbacks()
{
	if (!ensure(!IsValid(this)))
	{
		return;
	}

	check(Ability);

	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	const bool bShouldProduceTargetDataOnServer = TargetActor->ShouldProduceTargetDataOnServer;
	
	if (!bIsLocallyControlled)
	{
		if (!bShouldProduceTargetDataOnServer)
		{
			const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
			const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
			
			AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(
				this, &UGSAT_WaitTargetDataUsingActor::OnTargetDataReplicatedCallback);
			AbilitySystemComponent->AbilityTargetDataCancelledDelegate(SpecHandle, ActivationPredictionKey).AddUObject(
				this, &UGSAT_WaitTargetDataUsingActor::OnTargetDataReplicatedCancelledCallback);

			AbilitySystemComponent->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);

			SetWaitingOnRemotePlayerData();
		}
	}
}

void UGSAT_WaitTargetDataUsingActor::OnDestroy(bool AbilityEnded)
{
	if (TargetActor)
	{
		AGSGATA_Trace* TraceTargetActor = Cast<AGSGATA_Trace>(TargetActor);
		if (TraceTargetActor)
		{
			TraceTargetActor->StopTargeting();
		}
		else
		{
			TargetActor->SetActorTickEnabled(false);
			
			TargetActor->TargetDataReadyDelegate.RemoveAll(this);
			TargetActor->CanceledDelegate.RemoveAll(this);

			AbilitySystemComponent->GenericLocalConfirmCallbacks.RemoveDynamic(
				TargetActor, &AGameplayAbilityTargetActor::ConfirmTargeting);
			AbilitySystemComponent->GenericLocalCancelCallbacks.RemoveDynamic(
				TargetActor, &AGameplayAbilityTargetActor::CancelTargeting);
			TargetActor->GenericDelegateBoundASC = nullptr;
		}
	}

	Super::OnDestroy(AbilityEnded);
}

bool UGSAT_WaitTargetDataUsingActor::ShouldReplicateDataToServer() const
{
	if (!Ability || !TargetActor)
	{
		return false;
	}
	
	const FGameplayAbilityActorInfo* Info = Ability->GetCurrentActorInfo();
	if (!Info->IsNetAuthority() && !TargetActor->ShouldProduceTargetDataOnServer)
	{
		return true;
	}

	return false;
}
