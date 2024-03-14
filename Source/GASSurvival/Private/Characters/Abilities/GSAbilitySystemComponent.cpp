#include "Characters/Abilities/GSAbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "GameplayCueManager.h"
#include "Characters/Abilities/GSAbilitySystemGlobals.h"
#include "Characters/Abilities/GSGameplayAbility.h"
#include "Net/UnrealNetwork.h"

static TAutoConsoleVariable<float> CVarReplayMontageErrorThreshold(
	TEXT("GS.replay.MontageErrorThreshold"),
	0.5f,
	TEXT("Tolerance level for when montage playback position correction occurs in replays")
);

UGSAbilitySystemComponent::UGSAbilitySystemComponent()
{
}

void UGSAbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGSAbilitySystemComponent, RepAnimMontageInfoForMeshes);
}

bool UGSAbilitySystemComponent::GetShouldTick() const
{
	for (const FGameplayAbilityRepAnimMontageForMesh RepMontageInfo : RepAnimMontageInfoForMeshes)
	{
		const bool bHasReplicatedMontageInfoToUpdate = (IsOwnerActorAuthoritative() && RepMontageInfo.RepMontageInfo.
			IsStopped == false);

		if (bHasReplicatedMontageInfoToUpdate)
		{
			return true;
		}
	}

	return Super::GetShouldTick();
}

void UGSAbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	if (IsOwnerActorAuthoritative())
	{
		for (const FGameplayAbilityLocalAnimMontageForMesh& MontageInfo : LocalAnimMontageInfoForMeshes)
		{
			AnimMontage_UpdateReplicatedDataForMesh(MontageInfo.Mesh);
		}
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UGSAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	LocalAnimMontageInfoForMeshes = TArray<FGameplayAbilityLocalAnimMontageForMesh>();
	RepAnimMontageInfoForMeshes = TArray<FGameplayAbilityRepAnimMontageForMesh>();

	if (bPendingMontageRep)
	{
		OnRep_ReplicatedAnimMontageForMesh();
	}
}

void UGSAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability,
                                                   bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);
	
	ClearAnimatingAbilityForAllMeshes(Ability);
}

UGSAbilitySystemComponent* UGSAbilitySystemComponent::GetAbilitySystemComponentFromActor(const AActor* Actor,
	const bool LookForComponent)
{
	return Cast<UGSAbilitySystemComponent>(
		UGSAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, LookForComponent));
}

void UGSAbilitySystemComponent::AbilityLocalInputPressed(const int32 InputID)
{
	if (IsGenericConfirmInputBound(InputID))
	{
		LocalInputConfirm();
		return;
	}

	if (IsGenericCancelInputBound(InputID))
	{
		LocalInputCancel();
		return;
	}
	
	ABILITYLIST_SCOPE_LOCK();
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.InputID == InputID)
		{
			if (Spec.Ability)
			{
				Spec.InputPressed = true;
				if (Spec.IsActive())
				{
					if (Spec.Ability->bReplicateInputDirectly && IsOwnerActorAuthoritative() == false)
					{
						ServerSetInputPressed(Spec.Handle);
					}

					AbilitySpecInputPressed(Spec);
					
					InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle,
					                      Spec.ActivationInfo.GetActivationPredictionKey());
				}
				else
				{
					const UGSGameplayAbility* GA = Cast<UGSGameplayAbility>(Spec.Ability);
					if (GA && GA->bActivateOnInput)
					{
						TryActivateAbility(Spec.Handle);
					}
				}
			}
		}
	}
}

int32 UGSAbilitySystemComponent::BP_GetTagCount(const FGameplayTag TagToCheck) const
{
	return GetTagCount(TagToCheck);
}

FGameplayAbilitySpecHandle UGSAbilitySystemComponent::FindAbilitySpecHandleForClass(
	const TSubclassOf<UGameplayAbility> AbilityClass, UObject* OptionalSourceObject)
{
	ABILITYLIST_SCOPE_LOCK();
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		TSubclassOf<UGameplayAbility> SpecAbilityClass = Spec.Ability->GetClass();
		if (SpecAbilityClass == AbilityClass)
		{
			if (!OptionalSourceObject || (OptionalSourceObject && Spec.SourceObject == OptionalSourceObject))
			{
				return Spec.Handle;
			}
		}
	}

	return FGameplayAbilitySpecHandle();
}

void UGSAbilitySystemComponent::BP_AddLooseGameplayTag(const FGameplayTag& GameplayTag, int32 Count)
{
	AddLooseGameplayTag(GameplayTag, Count);
}

void UGSAbilitySystemComponent::BP_AddLooseGameplayTags(const FGameplayTagContainer& GameplayTags, int32 Count)
{
	AddLooseGameplayTags(GameplayTags, Count);
}

void UGSAbilitySystemComponent::BP_RemoveLooseGameplayTag(const FGameplayTag& GameplayTag, int32 Count)
{
	RemoveLooseGameplayTag(GameplayTag, Count);
}

void UGSAbilitySystemComponent::BP_RemoveLooseGameplayTags(const FGameplayTagContainer& GameplayTags, int32 Count)
{
	RemoveLooseGameplayTags(GameplayTags, Count);
}

bool UGSAbilitySystemComponent::BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle,
                                                           bool EndAbilityImmediately)
{
	bool AbilityActivated = false;
	if (InAbilityHandle.IsValid())
	{
		FScopedServerAbilityRPCBatcher GSAbilityRPCBatcher(this, InAbilityHandle);
		AbilityActivated = TryActivateAbility(InAbilityHandle, true);

		if (EndAbilityImmediately)
		{
			FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(InAbilityHandle);
			if (AbilitySpec)
			{
				UGSGameplayAbility* GSAbility = Cast<UGSGameplayAbility>(AbilitySpec->GetPrimaryInstance());
				GSAbility->ExternalEndAbility();
			}
		}

		return AbilityActivated;
	}

	return AbilityActivated;
}

void UGSAbilitySystemComponent::ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag,
                                                        const FGameplayCueParameters& GameplayCueParameters)
{
	UGSAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
		GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Executed, GameplayCueParameters);
}

void UGSAbilitySystemComponent::AddGameplayCueLocal(const FGameplayTag GameplayCueTag,
                                                    const FGameplayCueParameters& GameplayCueParameters)
{
	UGSAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
		GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::OnActive, GameplayCueParameters);
	UGSAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
		GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::WhileActive, GameplayCueParameters);
}

void UGSAbilitySystemComponent::RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag,
                                                       const FGameplayCueParameters& GameplayCueParameters)
{
	UGSAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
		GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Removed, GameplayCueParameters);
}

FString UGSAbilitySystemComponent::GetCurrentPredictionKeyStatus()
{
	return ScopedPredictionKey.ToString() + " is valid for more prediction: " + (
		ScopedPredictionKey.IsValidForMorePrediction() ? TEXT("true") : TEXT("false"));
}

FActiveGameplayEffectHandle UGSAbilitySystemComponent::BP_ApplyGameplayEffectToSelfWithPrediction(
	const TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level,
	FGameplayEffectContextHandle EffectContext)
{
	if (GameplayEffectClass)
	{
		if (!EffectContext.IsValid())
		{
			EffectContext = MakeEffectContext();
		}

		if (GameplayEffectClass == nullptr)
		{
			ABILITY_LOG(
				Error,
				TEXT(
					"UAbilitySystemComponent::BP_ApplyGameplayEffectToSelfWithPrediction called with null GameplayEffectClass. %s."
				), *GetFullName());
			return FActiveGameplayEffectHandle();
		}

		const UGameplayEffect* GameplayEffect = GameplayEffectClass->GetDefaultObject<UGameplayEffect>();

		if (CanPredict())
		{
			return ApplyGameplayEffectToSelf(GameplayEffect, Level, EffectContext, ScopedPredictionKey);
		}

		return ApplyGameplayEffectToSelf(GameplayEffect, Level, EffectContext);
	}

	return FActiveGameplayEffectHandle();
}

FActiveGameplayEffectHandle UGSAbilitySystemComponent::BP_ApplyGameplayEffectToTargetWithPrediction(
	const TSubclassOf<UGameplayEffect> GameplayEffectClass, UAbilitySystemComponent* Target, float Level,
	FGameplayEffectContextHandle Context)
{
	if (Target == nullptr)
	{
		ABILITY_LOG(
			Log,
			TEXT(
				"UAbilitySystemComponent::BP_ApplyGameplayEffectToTargetWithPrediction called with null Target. %s. Context: %s"
			), *GetFullName(), *Context.ToString());
		return FActiveGameplayEffectHandle();
	}

	if (GameplayEffectClass == nullptr)
	{
		ABILITY_LOG(
			Error,
			TEXT(
				"UAbilitySystemComponent::BP_ApplyGameplayEffectToTargetWithPrediction called with null GameplayEffectClass. %s. Context: %s"
			), *GetFullName(), *Context.ToString());
		return FActiveGameplayEffectHandle();
	}

	UGameplayEffect* GameplayEffect = GameplayEffectClass->GetDefaultObject<UGameplayEffect>();

	if (CanPredict())
	{
		return ApplyGameplayEffectToTarget(GameplayEffect, Target, Level, Context, ScopedPredictionKey);
	}

	return ApplyGameplayEffectToTarget(GameplayEffect, Target, Level, Context);
}

float UGSAbilitySystemComponent::PlayMontageForMesh(UGameplayAbility* InAnimatingAbility,
                                                    USkeletalMeshComponent* InMesh,
                                                    FGameplayAbilityActivationInfo ActivationInfo,
                                                    UAnimMontage* NewAnimMontage, float InPlayRate,
                                                    FName StartSectionName,
                                                    bool bReplicateMontage)
{
	UGSGameplayAbility* InAbility = Cast<UGSGameplayAbility>(InAnimatingAbility);

	float Duration = -1.0f;

	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	if (AnimInstance && NewAnimMontage)
	{
		Duration = AnimInstance->Montage_Play(NewAnimMontage, InPlayRate);
		if (Duration > 0.0f)
		{
			FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
			
			if (NewAnimMontage->HasRootMotion() && AnimInstance->GetOwningActor())
			{
				UE_LOG(LogRootMotion, Log, TEXT("UAbilitySystemComponent::PlayMontage %s, Role: %s")
				       , *GetNameSafe(NewAnimMontage)
				       , *UEnum::GetValueAsString(TEXT("Engine.ENetRole"), AnimInstance->GetOwningActor()->GetLocalRole(
				       ))
				);
			}

			AnimMontageInfo.LocalMontageInfo.AnimMontage = NewAnimMontage;
			AnimMontageInfo.LocalMontageInfo.AnimatingAbility = InAnimatingAbility;

			if (InAbility)
			{
				InAbility->SetCurrentMontageForMesh(InMesh, NewAnimMontage);
			}
			
			if (StartSectionName != NAME_None)
			{
				AnimInstance->Montage_JumpToSection(StartSectionName, NewAnimMontage);
			}
			
			if (IsOwnerActorAuthoritative())
			{
				if (bReplicateMontage)
				{
					FGameplayAbilityRepAnimMontageForMesh& AbilityRepMontageInfo =
						GetGameplayAbilityRepAnimMontageForMesh(InMesh);
					AbilityRepMontageInfo.RepMontageInfo.AnimMontage = NewAnimMontage;
					
					AnimMontage_UpdateReplicatedDataForMesh(InMesh);
					
					if (AbilityActorInfo->AvatarActor != nullptr)
					{
						AbilityActorInfo->AvatarActor->ForceNetUpdate();
					}
				}
			}
			else
			{
				FPredictionKey PredictionKey = GetPredictionKeyForNewAction();
				if (PredictionKey.IsValidKey())
				{
					PredictionKey.NewRejectedDelegate().BindUObject(
						this, &UGSAbilitySystemComponent::OnPredictiveMontageRejectedForMesh, InMesh, NewAnimMontage);
				}
			}
		}
	}

	return Duration;
}

float UGSAbilitySystemComponent::PlayMontageSimulatedForMesh(USkeletalMeshComponent* InMesh,
                                                             UAnimMontage* NewAnimMontage,
                                                             float InPlayRate, FName StartSectionName)
{
	float Duration = -1.0f;
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	if (AnimInstance && NewAnimMontage)
	{
		Duration = AnimInstance->Montage_Play(NewAnimMontage, InPlayRate);
		if (Duration > 0.0f)
		{
			FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
			AnimMontageInfo.LocalMontageInfo.AnimMontage = NewAnimMontage;
		}
	}

	return Duration;
}

void UGSAbilitySystemComponent::CurrentMontageStopForMesh(USkeletalMeshComponent* InMesh, float OverrideBlendOutTime)
{
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	const FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
	const UAnimMontage* MontageToStop = AnimMontageInfo.LocalMontageInfo.AnimMontage;
	bool bShouldStopMontage = AnimInstance && MontageToStop && !AnimInstance->Montage_GetIsStopped(MontageToStop);

	if (bShouldStopMontage)
	{
		const float BlendOutTime = (OverrideBlendOutTime >= 0.0f
			                            ? OverrideBlendOutTime
			                            : MontageToStop->BlendOut.GetBlendTime());

		AnimInstance->Montage_Stop(BlendOutTime, MontageToStop);

		if (IsOwnerActorAuthoritative())
		{
			AnimMontage_UpdateReplicatedDataForMesh(InMesh);
		}
	}
}

void UGSAbilitySystemComponent::StopAllCurrentMontages(float OverrideBlendOutTime)
{
	for (const FGameplayAbilityLocalAnimMontageForMesh& GameplayAbilityLocalAnimMontageForMesh :
	     LocalAnimMontageInfoForMeshes)
	{
		CurrentMontageStopForMesh(GameplayAbilityLocalAnimMontageForMesh.Mesh, OverrideBlendOutTime);
	}
}

void UGSAbilitySystemComponent::StopMontageIfCurrentForMesh(USkeletalMeshComponent* InMesh, const UAnimMontage& Montage,
                                                            float OverrideBlendOutTime)
{
	const FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
	if (&Montage == AnimMontageInfo.LocalMontageInfo.AnimMontage)
	{
		CurrentMontageStopForMesh(InMesh, OverrideBlendOutTime);
	}
}

void UGSAbilitySystemComponent::ClearAnimatingAbilityForAllMeshes(UGameplayAbility* Ability)
{
	UGSGameplayAbility* GSAbility = Cast<UGSGameplayAbility>(Ability);
	for (FGameplayAbilityLocalAnimMontageForMesh& GameplayAbilityLocalAnimMontageForMesh :
	     LocalAnimMontageInfoForMeshes)
	{
		if (GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimatingAbility == Ability)
		{
			GSAbility->SetCurrentMontageForMesh(GameplayAbilityLocalAnimMontageForMesh.Mesh, nullptr);
			GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimatingAbility = nullptr;
		}
	}
}

void UGSAbilitySystemComponent::CurrentMontageJumpToSectionForMesh(USkeletalMeshComponent* InMesh, FName SectionName)
{
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	const FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
	if ((SectionName != NAME_None) && AnimInstance && AnimMontageInfo.LocalMontageInfo.AnimMontage)
	{
		AnimInstance->Montage_JumpToSection(SectionName, AnimMontageInfo.LocalMontageInfo.AnimMontage);
		if (IsOwnerActorAuthoritative())
		{
			AnimMontage_UpdateReplicatedDataForMesh(InMesh);
		}
		else
		{
			ServerCurrentMontageJumpToSectionNameForMesh(InMesh, AnimMontageInfo.LocalMontageInfo.AnimMontage,
			                                             SectionName);
		}
	}
}

void UGSAbilitySystemComponent::CurrentMontageSetNextSectionNameForMesh(USkeletalMeshComponent* InMesh,
                                                                        FName FromSectionName, FName ToSectionName)
{
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	const FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
	if (AnimMontageInfo.LocalMontageInfo.AnimMontage && AnimInstance)
	{
	
		AnimInstance->Montage_SetNextSection(FromSectionName, ToSectionName,
		                                     AnimMontageInfo.LocalMontageInfo.AnimMontage);
	
		if (IsOwnerActorAuthoritative())
		{
			AnimMontage_UpdateReplicatedDataForMesh(InMesh);
		}
		else
		{
			float CurrentPosition = AnimInstance->Montage_GetPosition(AnimMontageInfo.LocalMontageInfo.AnimMontage);
			ServerCurrentMontageSetNextSectionNameForMesh(InMesh, AnimMontageInfo.LocalMontageInfo.AnimMontage,
			                                              CurrentPosition, FromSectionName, ToSectionName);
		}
	}
}

void UGSAbilitySystemComponent::CurrentMontageSetPlayRateForMesh(USkeletalMeshComponent* InMesh, float InPlayRate)
{
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	const FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
	if (AnimMontageInfo.LocalMontageInfo.AnimMontage && AnimInstance)
	{
		AnimInstance->Montage_SetPlayRate(AnimMontageInfo.LocalMontageInfo.AnimMontage, InPlayRate);
		
		if (IsOwnerActorAuthoritative())
		{
			AnimMontage_UpdateReplicatedDataForMesh(InMesh);
		}
		else
		{
			ServerCurrentMontageSetPlayRateForMesh(InMesh, AnimMontageInfo.LocalMontageInfo.AnimMontage, InPlayRate);
		}
	}
}

bool UGSAbilitySystemComponent::IsAnimatingAbilityForAnyMesh(const UGameplayAbility* InAbility) const
{
	for (FGameplayAbilityLocalAnimMontageForMesh GameplayAbilityLocalAnimMontageForMesh : LocalAnimMontageInfoForMeshes)
	{
		if (GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimatingAbility == InAbility)
		{
			return true;
		}
	}

	return false;
}

UGameplayAbility* UGSAbilitySystemComponent::GetAnimatingAbilityFromAnyMesh()
{
	for (FGameplayAbilityLocalAnimMontageForMesh& GameplayAbilityLocalAnimMontageForMesh :
	     LocalAnimMontageInfoForMeshes)
	{
		if (GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimatingAbility.IsValid())
		{
			return GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimatingAbility.Get();
		}
	}

	return nullptr;
}

TArray<UAnimMontage*> UGSAbilitySystemComponent::GetCurrentMontages() const
{
	TArray<UAnimMontage*> Montages;

	for (FGameplayAbilityLocalAnimMontageForMesh GameplayAbilityLocalAnimMontageForMesh : LocalAnimMontageInfoForMeshes)
	{
		const UAnimInstance* AnimInstance = IsValid(GameplayAbilityLocalAnimMontageForMesh.Mesh)
		                                    && GameplayAbilityLocalAnimMontageForMesh.Mesh->GetOwner() ==
		                                    AbilityActorInfo->AvatarActor
			                                    ? GameplayAbilityLocalAnimMontageForMesh.Mesh->GetAnimInstance()
			                                    : nullptr;

		if (GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimMontage && AnimInstance
			&& AnimInstance->Montage_IsActive(GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimMontage))
		{
			Montages.Add(GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimMontage);
		}
	}

	return Montages;
}

UAnimMontage* UGSAbilitySystemComponent::GetCurrentMontageForMesh(USkeletalMeshComponent* InMesh)
{
	const UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                                    ? InMesh->GetAnimInstance()
		                                    : nullptr;
	
	const FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);

	if (AnimMontageInfo.LocalMontageInfo.AnimMontage && AnimInstance
		&& AnimInstance->Montage_IsActive(AnimMontageInfo.LocalMontageInfo.AnimMontage))
	{
		return AnimMontageInfo.LocalMontageInfo.AnimMontage;
	}

	return nullptr;
}

int32 UGSAbilitySystemComponent::GetCurrentMontageSectionIDForMesh(USkeletalMeshComponent* InMesh)
{
	const UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                                    ? InMesh->GetAnimInstance()
		                                    : nullptr;
	const UAnimMontage* CurrentAnimMontage = GetCurrentMontageForMesh(InMesh);

	if (CurrentAnimMontage && AnimInstance)
	{
		float MontagePosition = AnimInstance->Montage_GetPosition(CurrentAnimMontage);
		return CurrentAnimMontage->GetSectionIndexFromPosition(MontagePosition);
	}

	return INDEX_NONE;
}

FName UGSAbilitySystemComponent::GetCurrentMontageSectionNameForMesh(USkeletalMeshComponent* InMesh)
{
	const UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                                    ? InMesh->GetAnimInstance()
		                                    : nullptr;
	const UAnimMontage* CurrentAnimMontage = GetCurrentMontageForMesh(InMesh);

	if (CurrentAnimMontage && AnimInstance)
	{
		float MontagePosition = AnimInstance->Montage_GetPosition(CurrentAnimMontage);
		int32 CurrentSectionID = CurrentAnimMontage->GetSectionIndexFromPosition(MontagePosition);

		return CurrentAnimMontage->GetSectionName(CurrentSectionID);
	}

	return NAME_None;
}

float UGSAbilitySystemComponent::GetCurrentMontageSectionLengthForMesh(USkeletalMeshComponent* InMesh)
{
	const UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                                    ? InMesh->GetAnimInstance()
		                                    : nullptr;
	UAnimMontage* CurrentAnimMontage = GetCurrentMontageForMesh(InMesh);

	if (CurrentAnimMontage && AnimInstance)
	{
		int32 CurrentSectionID = GetCurrentMontageSectionIDForMesh(InMesh);
		if (CurrentSectionID != INDEX_NONE)
		{
			TArray<FCompositeSection>& CompositeSections = CurrentAnimMontage->CompositeSections;
			
			if (CurrentSectionID < (CompositeSections.Num() - 1))
			{
				return (CompositeSections[CurrentSectionID + 1].GetTime() - CompositeSections[CurrentSectionID].
					GetTime());
			}
			
			return (CurrentAnimMontage->GetPlayLength() - CompositeSections[CurrentSectionID].GetTime());
		}
		
		return CurrentAnimMontage->GetPlayLength();
	}

	return 0.0f;
}

float UGSAbilitySystemComponent::GetCurrentMontageSectionTimeLeftForMesh(USkeletalMeshComponent* InMesh)
{
	const UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                                    ? InMesh->GetAnimInstance()
		                                    : nullptr;
	UAnimMontage* CurrentAnimMontage = GetCurrentMontageForMesh(InMesh);

	if (CurrentAnimMontage && AnimInstance && AnimInstance->Montage_IsActive(CurrentAnimMontage))
	{
		const float CurrentPosition = AnimInstance->Montage_GetPosition(CurrentAnimMontage);
		return CurrentAnimMontage->GetSectionTimeLeftFromPos(CurrentPosition);
	}

	return -1.0f;
}

FGameplayAbilityLocalAnimMontageForMesh& UGSAbilitySystemComponent::GetLocalAnimMontageInfoForMesh(
	USkeletalMeshComponent* InMesh)
{
	for (FGameplayAbilityLocalAnimMontageForMesh& MontageInfo : LocalAnimMontageInfoForMeshes)
	{
		if (MontageInfo.Mesh == InMesh)
		{
			return MontageInfo;
		}
	}

	const FGameplayAbilityLocalAnimMontageForMesh MontageInfo = FGameplayAbilityLocalAnimMontageForMesh(InMesh);
	LocalAnimMontageInfoForMeshes.Add(MontageInfo);
	return LocalAnimMontageInfoForMeshes.Last();
}

FGameplayAbilityRepAnimMontageForMesh& UGSAbilitySystemComponent::GetGameplayAbilityRepAnimMontageForMesh(
	USkeletalMeshComponent* InMesh)
{
	for (FGameplayAbilityRepAnimMontageForMesh& RepMontageInfo : RepAnimMontageInfoForMeshes)
	{
		if (RepMontageInfo.Mesh == InMesh)
		{
			return RepMontageInfo;
		}
	}

	const FGameplayAbilityRepAnimMontageForMesh RepMontageInfo = FGameplayAbilityRepAnimMontageForMesh(InMesh);
	RepAnimMontageInfoForMeshes.Add(RepMontageInfo);
	return RepAnimMontageInfoForMeshes.Last();
}

void UGSAbilitySystemComponent::OnPredictiveMontageRejectedForMesh(USkeletalMeshComponent* InMesh,
                                                                   UAnimMontage* PredictiveMontage)
{
	static const float MONTAGE_PREDICTION_REJECT_FADETIME = 0.25f;

	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	if (AnimInstance && PredictiveMontage)
	{
		if (AnimInstance->Montage_IsPlaying(PredictiveMontage))
		{
			AnimInstance->Montage_Stop(MONTAGE_PREDICTION_REJECT_FADETIME, PredictiveMontage);
		}
	}
}

void UGSAbilitySystemComponent::AnimMontage_UpdateReplicatedDataForMesh(USkeletalMeshComponent* InMesh)
{
	check(IsOwnerActorAuthoritative());

	AnimMontage_UpdateReplicatedDataForMesh(GetGameplayAbilityRepAnimMontageForMesh(InMesh));
}

void UGSAbilitySystemComponent::AnimMontage_UpdateReplicatedDataForMesh(
	FGameplayAbilityRepAnimMontageForMesh& OutRepAnimMontageInfo)
{
	const UAnimInstance* AnimInstance = IsValid(OutRepAnimMontageInfo.Mesh) && OutRepAnimMontageInfo.Mesh->GetOwner()
	                                    == AbilityActorInfo->AvatarActor
		                                    ? OutRepAnimMontageInfo.Mesh->GetAnimInstance()
		                                    : nullptr;
	
	const FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(
		OutRepAnimMontageInfo.Mesh);

	if (AnimInstance && AnimMontageInfo.LocalMontageInfo.AnimMontage)
	{
		OutRepAnimMontageInfo.RepMontageInfo.AnimMontage = AnimMontageInfo.LocalMontageInfo.AnimMontage;
		
		bool bIsStopped = AnimInstance->Montage_GetIsStopped(AnimMontageInfo.LocalMontageInfo.AnimMontage);

		if (!bIsStopped)
		{
			OutRepAnimMontageInfo.RepMontageInfo.PlayRate = AnimInstance->Montage_GetPlayRate(
				AnimMontageInfo.LocalMontageInfo.AnimMontage);
			OutRepAnimMontageInfo.RepMontageInfo.Position = AnimInstance->Montage_GetPosition(
				AnimMontageInfo.LocalMontageInfo.AnimMontage);
			OutRepAnimMontageInfo.RepMontageInfo.BlendTime = AnimInstance->Montage_GetBlendTime(
				AnimMontageInfo.LocalMontageInfo.AnimMontage);
		}

		if (OutRepAnimMontageInfo.RepMontageInfo.IsStopped != bIsStopped)
		{
			OutRepAnimMontageInfo.RepMontageInfo.IsStopped = bIsStopped;
			
			if (AbilityActorInfo->AvatarActor != nullptr)
			{
				AbilityActorInfo->AvatarActor->ForceNetUpdate();
			}
			
			UpdateShouldTick();
		}
		
		const int32 CurrentSectionID = AnimMontageInfo.LocalMontageInfo.AnimMontage->GetSectionIndexFromPosition(
			OutRepAnimMontageInfo.RepMontageInfo.Position);
		
		if (CurrentSectionID != INDEX_NONE)
		{
			const int32 NextSectionID = AnimInstance->Montage_GetNextSectionID(
				AnimMontageInfo.LocalMontageInfo.AnimMontage, CurrentSectionID);
			if (NextSectionID >= (256 - 1))
			{
				ABILITY_LOG(
					Error,
					TEXT(
						"AnimMontage_UpdateReplicatedData. NextSectionID = %d.  RepAnimMontageInfo.Position: %.2f, CurrentSectionID: %d. LocalAnimMontageInfo.AnimMontage %s"
					),
					NextSectionID, OutRepAnimMontageInfo.RepMontageInfo.Position, CurrentSectionID,
					*GetNameSafe(AnimMontageInfo.LocalMontageInfo.AnimMontage));
				ensure(NextSectionID < (256 - 1));
			}
			OutRepAnimMontageInfo.RepMontageInfo.NextSectionID = static_cast<uint8>(NextSectionID + 1);
		}
		else
		{
			OutRepAnimMontageInfo.RepMontageInfo.NextSectionID = 0;
		}
	}
}

void UGSAbilitySystemComponent::AnimMontage_UpdateForcedPlayFlagsForMesh(
	const FGameplayAbilityRepAnimMontageForMesh& OutRepAnimMontageInfo)
{
	FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(
		OutRepAnimMontageInfo.Mesh);
}

void UGSAbilitySystemComponent::OnRep_ReplicatedAnimMontageForMesh()
{
	for (FGameplayAbilityRepAnimMontageForMesh& NewRepMontageInfoForMesh : RepAnimMontageInfoForMeshes)
	{
		FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(
			NewRepMontageInfoForMesh.Mesh);

		const UWorld* World = GetWorld();

		if (NewRepMontageInfoForMesh.RepMontageInfo.bSkipPlayRate)
		{
			NewRepMontageInfoForMesh.RepMontageInfo.PlayRate = 1.0f;
		}

		const bool bIsPlayingReplay = World && World->IsPlayingReplay();

		const float MONTAGE_REP_POS_ERR_THRESH = bIsPlayingReplay
			                                         ? CVarReplayMontageErrorThreshold.GetValueOnGameThread()
			                                         : 0.1f;

		UAnimInstance* AnimInstance = IsValid(NewRepMontageInfoForMesh.Mesh) && NewRepMontageInfoForMesh.Mesh->
		                              GetOwner()
		                              == AbilityActorInfo->AvatarActor
			                              ? NewRepMontageInfoForMesh.Mesh->GetAnimInstance()
			                              : nullptr;
		
		if (AnimInstance == nullptr || !IsReadyForReplicatedMontageForMesh())
		{
			bPendingMontageRep = true;
			return;
		}
		
		bPendingMontageRep = false;

		if (!AbilityActorInfo->IsLocallyControlled())
		{
			static const auto CVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("net.Montage.Debug"));
			bool DebugMontage = (CVar && CVar->GetValueOnGameThread() == 1);
			if (DebugMontage)
			{
				ABILITY_LOG(Warning, TEXT("\n\nOnRep_ReplicatedAnimMontage, %s"), *GetNameSafe(this));
				ABILITY_LOG(
					Warning,
					TEXT(
						"\tAnimMontage: %s\n\tPlayRate: %f\n\tPosition: %f\n\tBlendTime: %f\n\tNextSectionID: %d\n\tIsStopped: %d"
					),
					*GetNameSafe(NewRepMontageInfoForMesh.RepMontageInfo.AnimMontage),
					NewRepMontageInfoForMesh.RepMontageInfo.PlayRate,
					NewRepMontageInfoForMesh.RepMontageInfo.Position,
					NewRepMontageInfoForMesh.RepMontageInfo.BlendTime,
					NewRepMontageInfoForMesh.RepMontageInfo.NextSectionID,
					NewRepMontageInfoForMesh.RepMontageInfo.IsStopped);
				ABILITY_LOG(Warning, TEXT("\tLocalAnimMontageInfo.AnimMontage: %s\n\tPosition: %f"),
				            *GetNameSafe(AnimMontageInfo.LocalMontageInfo.AnimMontage),
				            AnimInstance->Montage_GetPosition(AnimMontageInfo.LocalMontageInfo.AnimMontage));
			}

			if (NewRepMontageInfoForMesh.RepMontageInfo.AnimMontage)
			{
				if ((AnimMontageInfo.LocalMontageInfo.AnimMontage != NewRepMontageInfoForMesh.RepMontageInfo.
					AnimMontage))
				{
					PlayMontageSimulatedForMesh(NewRepMontageInfoForMesh.Mesh,
					                            NewRepMontageInfoForMesh.RepMontageInfo.AnimMontage,
					                            NewRepMontageInfoForMesh.RepMontageInfo.PlayRate);
				}

				if (AnimMontageInfo.LocalMontageInfo.AnimMontage == nullptr)
				{
					ABILITY_LOG(
						Warning,
						TEXT("OnRep_ReplicatedAnimMontage: PlayMontageSimulated failed. Name: %s, AnimMontage: %s"),
						*GetNameSafe(this), *GetNameSafe(NewRepMontageInfoForMesh.RepMontageInfo.AnimMontage));
					return;
				}
				
				if (AnimInstance->Montage_GetPlayRate(AnimMontageInfo.LocalMontageInfo.AnimMontage) !=
					NewRepMontageInfoForMesh.RepMontageInfo.PlayRate)
				{
					AnimInstance->Montage_SetPlayRate(AnimMontageInfo.LocalMontageInfo.AnimMontage,
					                                  NewRepMontageInfoForMesh.RepMontageInfo.PlayRate);
				}
				
				const bool bIsStopped = AnimInstance->
					Montage_GetIsStopped(AnimMontageInfo.LocalMontageInfo.AnimMontage);
				const bool bReplicatedIsStopped = static_cast<bool>(NewRepMontageInfoForMesh.RepMontageInfo.IsStopped);
				
				if (bReplicatedIsStopped)
				{
					if (!bIsStopped)
					{
						CurrentMontageStopForMesh(NewRepMontageInfoForMesh.Mesh,
						                          NewRepMontageInfoForMesh.RepMontageInfo.BlendTime);
					}
				}
				else if (!NewRepMontageInfoForMesh.RepMontageInfo.SkipPositionCorrection)
				{
					const int32 RepSectionID = AnimMontageInfo.LocalMontageInfo.AnimMontage->
					                                           GetSectionIndexFromPosition(
						                                           NewRepMontageInfoForMesh.RepMontageInfo.Position);
					const int32 RepNextSectionID = static_cast<int32>(NewRepMontageInfoForMesh.RepMontageInfo.
						NextSectionID) - 1;
					
					if (RepSectionID != INDEX_NONE)
					{
						const int32 NextSectionID = AnimInstance->Montage_GetNextSectionID(
							AnimMontageInfo.LocalMontageInfo.AnimMontage, RepSectionID);
						
						if (NextSectionID != RepNextSectionID)
						{
							AnimInstance->Montage_SetNextSection(
								AnimMontageInfo.LocalMontageInfo.AnimMontage->GetSectionName(RepSectionID),
								AnimMontageInfo.LocalMontageInfo.AnimMontage->GetSectionName(RepNextSectionID),
								AnimMontageInfo.LocalMontageInfo.AnimMontage);
						}
						
						const int32 CurrentSectionID = AnimMontageInfo.LocalMontageInfo.AnimMontage->
						                                               GetSectionIndexFromPosition(
							                                               AnimInstance->Montage_GetPosition(
								                                               AnimMontageInfo.LocalMontageInfo.
								                                               AnimMontage));
						if ((CurrentSectionID != RepSectionID) && (CurrentSectionID != RepNextSectionID))
						{
							const float SectionStartTime = AnimMontageInfo.LocalMontageInfo.AnimMontage->
							                                               GetAnimCompositeSection(RepSectionID).
							                                               GetTime();
							AnimInstance->Montage_SetPosition(AnimMontageInfo.LocalMontageInfo.AnimMontage,
							                                  SectionStartTime);
						}
					}
					
					const float CurrentPosition = AnimInstance->Montage_GetPosition(
						AnimMontageInfo.LocalMontageInfo.AnimMontage);
					const int32 CurrentSectionID = AnimMontageInfo.LocalMontageInfo.AnimMontage->
					                                               GetSectionIndexFromPosition(CurrentPosition);
					const float DeltaPosition = NewRepMontageInfoForMesh.RepMontageInfo.Position - CurrentPosition;
					
					if ((CurrentSectionID == RepSectionID) && (FMath::Abs(DeltaPosition) > MONTAGE_REP_POS_ERR_THRESH)
						&& (NewRepMontageInfoForMesh.RepMontageInfo.IsStopped == 0))
					{
						if (FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(
							NewRepMontageInfoForMesh.RepMontageInfo.AnimMontage))
						{
							const float DeltaTime =
								!FMath::IsNearlyZero(NewRepMontageInfoForMesh.RepMontageInfo.PlayRate)
									? (DeltaPosition / NewRepMontageInfoForMesh.RepMontageInfo.PlayRate)
									: 0.0f;
							if (DeltaTime >= 0.0f)
							{
								MontageInstance->UpdateWeight(DeltaTime);
								MontageInstance->HandleEvents(CurrentPosition,
								                              NewRepMontageInfoForMesh.RepMontageInfo.Position,
								                              nullptr);
								AnimInstance->TriggerAnimNotifies(DeltaTime);
							}
						}
						AnimInstance->Montage_SetPosition(AnimMontageInfo.LocalMontageInfo.AnimMontage,
						                                  NewRepMontageInfoForMesh.RepMontageInfo.Position);
					}
				}
			}
		}
	}
}

bool UGSAbilitySystemComponent::IsReadyForReplicatedMontageForMesh()
{
	return true;
}

void UGSAbilitySystemComponent::ServerCurrentMontageSetNextSectionNameForMesh_Implementation(
	USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, float ClientPosition, FName SectionName,
	FName NextSectionName)
{
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	const FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);

	if (AnimInstance)
	{
		const UAnimMontage* CurrentAnimMontage = AnimMontageInfo.LocalMontageInfo.AnimMontage;
		if (ClientAnimMontage == CurrentAnimMontage)
		{
			AnimInstance->Montage_SetNextSection(SectionName, NextSectionName, CurrentAnimMontage);
			
			const float CurrentPosition = AnimInstance->Montage_GetPosition(CurrentAnimMontage);
			const int32 CurrentSectionID = CurrentAnimMontage->GetSectionIndexFromPosition(CurrentPosition);
			const FName CurrentSectionName = CurrentAnimMontage->GetSectionName(CurrentSectionID);

			const int32 ClientSectionID = CurrentAnimMontage->GetSectionIndexFromPosition(ClientPosition);
			const FName ClientCurrentSectionName = CurrentAnimMontage->GetSectionName(ClientSectionID);
			
			if ((CurrentSectionName != ClientCurrentSectionName) || (CurrentSectionName != SectionName))
			{
				AnimInstance->Montage_SetPosition(CurrentAnimMontage, ClientPosition);
			}
			
			if (IsOwnerActorAuthoritative())
			{
				AnimMontage_UpdateReplicatedDataForMesh(InMesh);
			}
		}
	}
}

bool UGSAbilitySystemComponent::ServerCurrentMontageSetNextSectionNameForMesh_Validate(USkeletalMeshComponent* InMesh,
	UAnimMontage* ClientAnimMontage, float ClientPosition, FName SectionName, FName NextSectionName)
{
	return true;
}

void UGSAbilitySystemComponent::ServerCurrentMontageJumpToSectionNameForMesh_Implementation(
	USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, FName SectionName)
{
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	const FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);

	if (AnimInstance)
	{
		const UAnimMontage* CurrentAnimMontage = AnimMontageInfo.LocalMontageInfo.AnimMontage;
		if (ClientAnimMontage == CurrentAnimMontage)
		{
			AnimInstance->Montage_JumpToSection(SectionName, CurrentAnimMontage);
			
			if (IsOwnerActorAuthoritative())
			{
				AnimMontage_UpdateReplicatedDataForMesh(InMesh);
			}
		}
	}
}

bool UGSAbilitySystemComponent::ServerCurrentMontageJumpToSectionNameForMesh_Validate(USkeletalMeshComponent* InMesh,
	UAnimMontage* ClientAnimMontage, FName SectionName)
{
	return true;
}

void UGSAbilitySystemComponent::ServerCurrentMontageSetPlayRateForMesh_Implementation(USkeletalMeshComponent* InMesh,
	UAnimMontage* ClientAnimMontage, float InPlayRate)
{
	UAnimInstance* AnimInstance = IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor
		                              ? InMesh->GetAnimInstance()
		                              : nullptr;
	const FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);

	if (AnimInstance)
	{
		const UAnimMontage* CurrentAnimMontage = AnimMontageInfo.LocalMontageInfo.AnimMontage;
		if (ClientAnimMontage == CurrentAnimMontage)
		{
			AnimInstance->Montage_SetPlayRate(AnimMontageInfo.LocalMontageInfo.AnimMontage, InPlayRate);
			
			if (IsOwnerActorAuthoritative())
			{
				AnimMontage_UpdateReplicatedDataForMesh(InMesh);
			}
		}
	}
}

bool UGSAbilitySystemComponent::ServerCurrentMontageSetPlayRateForMesh_Validate(USkeletalMeshComponent* InMesh,
	UAnimMontage* ClientAnimMontage, float InPlayRate)
{
	return true;
}
