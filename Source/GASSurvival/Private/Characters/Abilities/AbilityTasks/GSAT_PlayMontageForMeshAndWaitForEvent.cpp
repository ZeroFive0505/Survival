#include "Characters/Abilities/AbilityTasks/GSAT_PlayMontageForMeshAndWaitForEvent.h"

#include "AbilitySystemLog.h"
#include "Characters/Abilities/GSAbilitySystemComponent.h"
#include "Characters/Abilities/GSAbilitySystemGlobals.h"
#include "GameFramework/Character.h"

UGSAT_PlayMontageForMeshAndWaitForEvent::UGSAT_PlayMontageForMeshAndWaitForEvent(
	const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Rate = 1.0f;
	bStopWhenAbilityEnds = true;
}

void UGSAT_PlayMontageForMeshAndWaitForEvent::Activate()
{
	if (Ability == nullptr)
	{
		return;
	}

	if (Mesh == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s invalid Mesh"), *FString(__FUNCTION__));
		return;
	}

	bool bPlayedMontage = false;
	UGSAbilitySystemComponent* GSAbilitySystemComponent = GetTargetASC();

	if (GSAbilitySystemComponent)
	{
		const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
		UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			EventHandle = GSAbilitySystemComponent->AddGameplayEventTagContainerDelegate(
				EventTags, FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(
					this, &UGSAT_PlayMontageForMeshAndWaitForEvent::OnGameplayEvent));

			if (GSAbilitySystemComponent->PlayMontageForMesh(Ability, Mesh, Ability->GetCurrentActivationInfo(),
			                                                 MontageToPlay, Rate, StartSection,
			                                                 bReplicateMontage) > 0.f)
			{
				if (ShouldBroadcastAbilityTaskDelegates() == false)
				{
					return;
				}

				CancelledHandle = Ability->OnGameplayAbilityCancelled.AddUObject(
					this, &UGSAT_PlayMontageForMeshAndWaitForEvent::OnAbilityCancelled);

				BlendingOutDelegate.BindUObject(this, &UGSAT_PlayMontageForMeshAndWaitForEvent::OnMontageBlendingOut);
				AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

				MontageEndedDelegate.BindUObject(this, &UGSAT_PlayMontageForMeshAndWaitForEvent::OnMontageEnded);
				AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);

				ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
				if (Character && (Character->GetLocalRole() == ROLE_Authority ||
					(Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() ==
						EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
				{
					Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
				}

				bPlayedMontage = true;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("UGSAbilityTask_PlayMontageForMeshAndWaitForEvent call to PlayMontage failed!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("UGSAbilityTask_PlayMontageForMeshAndWaitForEvent called on invalid AbilitySystemComponent"));
	}

	if (!bPlayedMontage)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT(
			       "UGSAbilityTask_PlayMontageForMeshAndWaitForEvent called in Ability %s failed to play montage %s; Task Instance Name %s."
		       ), *Ability->GetName(), *GetNameSafe(MontageToPlay), *InstanceName.ToString());
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			ABILITY_LOG(Display, TEXT("%s: OnCancelled"), *GetName());
			OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}

	SetWaitingOnAvatar();
}

void UGSAT_PlayMontageForMeshAndWaitForEvent::ExternalCancel()
{
	check(AbilitySystemComponent.IsValid());

	OnAbilityCancelled();

	Super::ExternalCancel();
}

FString UGSAT_PlayMontageForMeshAndWaitForEvent::GetDebugString() const
{
	const UAnimMontage* PlayingMontage = nullptr;
	if (Ability && Mesh)
	{
		const UAnimInstance* AnimInstance = Mesh->GetAnimInstance();

		if (AnimInstance != nullptr)
		{
			PlayingMontage = AnimInstance->Montage_IsActive(MontageToPlay)
				                 ? MontageToPlay
				                 : AnimInstance->GetCurrentActiveMontage();
		}
	}

	return FString::Printf(
		TEXT("PlayMontageAndWaitForEvent. MontageToPlay: %s  (Currently Playing): %s"), *GetNameSafe(MontageToPlay),
		*GetNameSafe(PlayingMontage));
}

void UGSAT_PlayMontageForMeshAndWaitForEvent::OnDestroy(bool AbilityEnded)
{
	if (Ability)
	{
		Ability->OnGameplayAbilityCancelled.Remove(CancelledHandle);
		if (AbilityEnded && bStopWhenAbilityEnds)
		{
			StopPlayingMontage(OverrideBlendOutTimeForStopWhenEndAbility);
		}
	}

	UGSAbilitySystemComponent* GSAbilitySystemComponent = GetTargetASC();
	if (GSAbilitySystemComponent)
	{
		GSAbilitySystemComponent->RemoveGameplayEventTagContainerDelegate(EventTags, EventHandle);
	}

	Super::OnDestroy(AbilityEnded);
}

UGSAT_PlayMontageForMeshAndWaitForEvent* UGSAT_PlayMontageForMeshAndWaitForEvent::PlayMontageForMeshAndWaitForEvent(
	UGameplayAbility* OwningAbility, FName TaskInstanceName, USkeletalMeshComponent* InMesh,
	UAnimMontage* MontageToPlay,
	FGameplayTagContainer EventTags, float Rate, FName StartSection, bool bStopWhenAbilityEnds,
	float AnimRootMotionTranslationScale, bool bReplicateMontage, float OverrideBlendOutTimeForCancelAbility,
	float OverrideBlendOutTimeForStopWhenEndAbility)
{
	UGSAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(Rate);

	UGSAT_PlayMontageForMeshAndWaitForEvent* Task = NewAbilityTask<UGSAT_PlayMontageForMeshAndWaitForEvent>(
		OwningAbility, TaskInstanceName);
	Task->Mesh = InMesh;
	Task->MontageToPlay = MontageToPlay;
	Task->EventTags = EventTags;
	Task->Rate = Rate;
	Task->StartSection = StartSection;
	Task->AnimRootMotionTranslationScale = AnimRootMotionTranslationScale;
	Task->bStopWhenAbilityEnds = bStopWhenAbilityEnds;
	Task->bReplicateMontage = bReplicateMontage;
	Task->OverrideBlendOutTimeForCancelAbility = OverrideBlendOutTimeForCancelAbility;
	Task->OverrideBlendOutTimeForStopWhenEndAbility = OverrideBlendOutTimeForStopWhenEndAbility;

	return Task;
}

bool UGSAT_PlayMontageForMeshAndWaitForEvent::StopPlayingMontage(float OverrideBlendOutTime) const
{
	if (Mesh == nullptr)
	{
		return false;
	}

	UGSAbilitySystemComponent* GSAbilitySystemComponent = GetTargetASC();
	if (!GSAbilitySystemComponent)
	{
		return false;
	}

	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	if (!ActorInfo)
	{
		return false;
	}

	const UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return false;
	}
	
	if (GSAbilitySystemComponent && Ability)
	{
		if (GSAbilitySystemComponent->GetAnimatingAbilityFromAnyMesh() == Ability
			&& GSAbilitySystemComponent->GetCurrentMontageForMesh(Mesh) == MontageToPlay)
		{
			FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay);
			if (MontageInstance)
			{
				MontageInstance->OnMontageBlendingOutStarted.Unbind();
				MontageInstance->OnMontageEnded.Unbind();
			}

			GSAbilitySystemComponent->CurrentMontageStopForMesh(Mesh, OverrideBlendOutTime);
			return true;
		}
	}

	return false;
}

UGSAbilitySystemComponent* UGSAT_PlayMontageForMeshAndWaitForEvent::GetTargetASC() const
{
	return Cast<UGSAbilitySystemComponent>(AbilitySystemComponent);
}

void UGSAT_PlayMontageForMeshAndWaitForEvent::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (Ability && Ability->GetCurrentMontage() == MontageToPlay)
	{
		if (Montage == MontageToPlay)
		{
			AbilitySystemComponent->ClearAnimatingAbility(Ability);
			
			ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
			if (Character && (Character->GetLocalRole() == ROLE_Authority ||
				(Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() ==
					EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
			{
				Character->SetAnimRootMotionTranslationScale(1.f);
			}
		}
	}

	if (bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnInterrupted.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnBlendOut.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}
}

void UGSAT_PlayMontageForMeshAndWaitForEvent::OnAbilityCancelled()
{
	if (StopPlayingMontage(OverrideBlendOutTimeForCancelAbility))
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}
}

void UGSAT_PlayMontageForMeshAndWaitForEvent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCompleted.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}

	EndTask();
}

void UGSAT_PlayMontageForMeshAndWaitForEvent::OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload) const
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		FGameplayEventData TempData = *Payload;
		TempData.EventTag = EventTag;

		EventReceived.Broadcast(EventTag, TempData);
	}
}
