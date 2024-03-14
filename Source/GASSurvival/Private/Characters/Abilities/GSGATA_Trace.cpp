#include "GASSurvival/Public/Characters/Abilities/GSGATA_Trace.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "Abilities/GameplayAbility.h"

AGSGATA_Trace::AGSGATA_Trace()
{
	bDestroyOnConfirmation = false;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	MaxHitResultsPerTrace = 1;
	NumberOfTraces = 1;
	bIgnoreBlockingHits = false;
	bTraceAffectsAimPitch = true;
	bTraceFromPlayerViewPoint = false;
	MaxRange = 999999.0f;
	bUseAimingSpreadMod = false;
	BaseSpread = 0.0f;
	AimingSpreadMod = 0.0f;
	TargetingSpreadIncrement = 0.0f;
	TargetingSpreadMax = 0.0f;
	CurrentTargetingSpread = 0.0f;
	bUsePersistentHitResults = false;
}

void AGSGATA_Trace::ResetSpread()
{
	bUseAimingSpreadMod = false;
	BaseSpread = 0.0f;
	AimingSpreadMod = 0.0f;
	TargetingSpreadIncrement = 0.0f;
	TargetingSpreadMax = 0.0f;
	CurrentTargetingSpread = 0.0f;
}

float AGSGATA_Trace::GetCurrentSpread() const
{
	float FinalSpread = BaseSpread + CurrentTargetingSpread;

	if (bUseAimingSpreadMod && AimingTag.IsValid() && AimingRemovalTag.IsValid())
	{
		const UAbilitySystemComponent* ASC = OwningAbility->GetCurrentActorInfo()->AbilitySystemComponent.Get();
		if (ASC && (ASC->GetTagCount(AimingTag) > ASC->GetTagCount(AimingRemovalTag)))
		{
			FinalSpread *= AimingSpreadMod;
		}
	}

	return FinalSpread;
}

void AGSGATA_Trace::SetStartLocation(const FGameplayAbilityTargetingLocationInfo& InStartLocation)
{
	StartLocation = InStartLocation;
}

void AGSGATA_Trace::SetShouldProduceTargetDataOnServer(bool bInShouldProduceTargetDataOnServer)
{
	ShouldProduceTargetDataOnServer = bInShouldProduceTargetDataOnServer;
}

void AGSGATA_Trace::SetDestroyOnConfirmation(bool bInDestroyOnConfirmation)
{
	bDestroyOnConfirmation = bInDestroyOnConfirmation;
}

void AGSGATA_Trace::StartTargeting(UGameplayAbility* Ability)
{
	SetActorTickEnabled(true);

	OwningAbility = Ability;
	SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	
	DestroyReticleActors();

	if (ReticleClass)
	{
		for (int32 i = 0; i < MaxHitResultsPerTrace * NumberOfTraces; i++)
		{
			SpawnReticleActor(GetActorLocation(), GetActorRotation());
		}
	}

	if (bUsePersistentHitResults)
	{
		PersistentHitResults.Empty();
	}
}

void AGSGATA_Trace::ConfirmTargetingAndContinue()
{
	check(ShouldProduceTargetData());
	if (SourceActor)
	{
		TArray<FHitResult> HitResults = PerformTrace(SourceActor);
		const FGameplayAbilityTargetDataHandle Handle = MakeTargetData(HitResults);
		TargetDataReadyDelegate.Broadcast(Handle);

#if ENABLE_DRAW_DEBUG
		if (bDebug)
		{
			ShowDebugTrace(HitResults, EDrawDebugTrace::Type::ForDuration, 2.0f);
		}
#endif
	}

	if (bUsePersistentHitResults)
	{
		PersistentHitResults.Empty();
	}
}

void AGSGATA_Trace::CancelTargeting()
{
	const FGameplayAbilityActorInfo* ActorInfo = (OwningAbility ? OwningAbility->GetCurrentActorInfo() : nullptr);
	UAbilitySystemComponent* ASC = (ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr);
	if (ASC)
	{
		ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GenericCancel,
		                                    OwningAbility->GetCurrentAbilitySpecHandle(),
		                                    OwningAbility->GetCurrentActivationInfo().GetActivationPredictionKey()).
		     Remove(GenericCancelHandle);
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("AGameplayAbilityTargetActor::CancelTargeting called with null ASC! Actor %s"),
		            *GetName());
	}

	CanceledDelegate.Broadcast(FGameplayAbilityTargetDataHandle());

	SetActorTickEnabled(false);

	if (bUsePersistentHitResults)
	{
		PersistentHitResults.Empty();
	}
}

void AGSGATA_Trace::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyReticleActors();

	Super::EndPlay(EndPlayReason);
}

void AGSGATA_Trace::LineTraceWithFilter(TArray<FHitResult>& OutHitResults, const UWorld* World,
                                        const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start,
                                        const FVector& End, FName ProfileName,
                                        const FCollisionQueryParams Params)
{
	check(World);

	TArray<FHitResult> HitResults;
	World->LineTraceMultiByProfile(HitResults, Start, End, ProfileName, Params);

	TArray<FHitResult> FilteredHitResults;
	
	const FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();

	for (int32 HitIdx = 0; HitIdx < HitResults.Num(); ++HitIdx)
	{
		FHitResult& Hit = HitResults[HitIdx];

		if (!Hit.GetActor() || FilterHandle.FilterPassesForActor(Hit.GetActor()))
		{
			Hit.TraceStart = TraceStart;
			Hit.TraceEnd = End;

			FilteredHitResults.Add(Hit);
		}
	}

	OutHitResults = FilteredHitResults;
}

void AGSGATA_Trace::AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params,
                                            const FVector& TraceStart, FVector& OutTraceEnd, bool bIgnorePitch)
{
	if (!OwningAbility)
	{
		return;
	}
	
	FVector ViewStart = TraceStart;
	FRotator ViewRot = StartLocation.GetTargetingTransform().GetRotation().Rotator();

	if (PrimaryPC)
	{
		PrimaryPC->GetPlayerViewPoint(ViewStart, ViewRot);
	}

	const FVector ViewDir = ViewRot.Vector();
	FVector ViewEnd = ViewStart + (ViewDir * MaxRange);

	ClipCameraRayToAbilityRange(ViewStart, ViewDir, TraceStart, MaxRange, ViewEnd);
	
	TArray<FHitResult> HitResults;
	LineTraceWithFilter(HitResults, InSourceActor->GetWorld(), Filter, ViewStart, ViewEnd, TraceProfile.Name, Params);

	CurrentTargetingSpread = FMath::Min(TargetingSpreadMax, CurrentTargetingSpread + TargetingSpreadIncrement);

	const bool bUseTraceResult = HitResults.Num() > 0 && (FVector::DistSquared(TraceStart, HitResults[0].Location) <= (
		MaxRange * MaxRange));

	const FVector AdjustedEnd = (bUseTraceResult) ? HitResults[0].Location : ViewEnd;

	FVector AdjustedAimDir = (AdjustedEnd - TraceStart).GetSafeNormal();
	if (AdjustedAimDir.IsZero())
	{
		AdjustedAimDir = ViewDir;
	}

	if (!bTraceAffectsAimPitch && bUseTraceResult)
	{
		FVector OriginalAimDir = (ViewEnd - TraceStart).GetSafeNormal();

		if (!OriginalAimDir.IsZero())
		{
			const FRotator OriginalAimRot = OriginalAimDir.Rotation();

			FRotator AdjustedAimRot = AdjustedAimDir.Rotation();
			AdjustedAimRot.Pitch = OriginalAimRot.Pitch;

			AdjustedAimDir = AdjustedAimRot.Vector();
		}
	}

	const float CurrentSpread = GetCurrentSpread();

	const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);
	const int32 RandomSeed = FMath::Rand();
	FRandomStream WeaponRandomStream(RandomSeed);
	const FVector ShootDir = WeaponRandomStream.VRandCone(AdjustedAimDir, ConeHalfAngle, ConeHalfAngle);

	OutTraceEnd = TraceStart + (ShootDir * MaxRange);
}

bool AGSGATA_Trace::ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter,
                                                float AbilityRange, FVector& ClippedPosition)
{
	const FVector CameraToCenter = AbilityCenter - CameraLocation;
	const float DotToCenter = FVector::DotProduct(CameraToCenter, CameraDirection);
	if (DotToCenter >= 0.0f)
	{
		const float DistanceSquared = CameraToCenter.SizeSquared() - (DotToCenter * DotToCenter);
		const float RadiusSquared = (AbilityRange * AbilityRange);
		if (DistanceSquared <= RadiusSquared)
		{
			const float DistanceFromCamera = FMath::Sqrt(RadiusSquared - DistanceSquared);
			const float DistanceAlongRay = DotToCenter + DistanceFromCamera;
			ClippedPosition = CameraLocation + (DistanceAlongRay * CameraDirection);
			return true;
		}
	}

	return false;
}

void AGSGATA_Trace::StopTargeting()
{
	SetActorTickEnabled(false);

	DestroyReticleActors();
	
	TargetDataReadyDelegate.Clear();
	CanceledDelegate.Clear();

	if (GenericDelegateBoundASC)
	{
		GenericDelegateBoundASC->GenericLocalConfirmCallbacks.RemoveDynamic(
			this, &AGameplayAbilityTargetActor::ConfirmTargeting);
		GenericDelegateBoundASC->GenericLocalCancelCallbacks.RemoveDynamic(
			this, &AGameplayAbilityTargetActor::CancelTargeting);
		GenericDelegateBoundASC = nullptr;
	}
}

FGameplayAbilityTargetDataHandle AGSGATA_Trace::MakeTargetData(const TArray<FHitResult>& HitResults) const
{
	FGameplayAbilityTargetDataHandle ReturnDataHandle;

	for (int32 i = 0; i < HitResults.Num(); i++)
	{
		FGameplayAbilityTargetData_SingleTargetHit* ReturnData = new FGameplayAbilityTargetData_SingleTargetHit();
		ReturnData->HitResult = HitResults[i];
		ReturnDataHandle.Add(ReturnData);
	}

	return ReturnDataHandle;
}

TArray<FHitResult> AGSGATA_Trace::PerformTrace(AActor* InSourceActor)
{
	bool bTraceComplex = false;
	TArray<AActor*> ActorsToIgnore;

	ActorsToIgnore.Add(InSourceActor);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AGSGATA_LineTrace), bTraceComplex);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActors(ActorsToIgnore);
	Params.bIgnoreBlocks = bIgnoreBlockingHits;

	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();
	FVector TraceEnd;

	if (PrimaryPC)
	{
		FVector ViewStart;
		FRotator ViewRot;
		PrimaryPC->GetPlayerViewPoint(ViewStart, ViewRot);

		TraceStart = bTraceFromPlayerViewPoint ? ViewStart : TraceStart;
	}

	if (bUsePersistentHitResults)
	{
		for (int32 i = PersistentHitResults.Num() - 1; i >= 0; i--)
		{
			FHitResult& HitResult = PersistentHitResults[i];

			if (HitResult.bBlockingHit || !HitResult.GetActor() || FVector::DistSquared(
				TraceStart, HitResult.GetActor()->GetActorLocation()) > (MaxRange * MaxRange))
			{
				PersistentHitResults.RemoveAt(i);
			}
		}
	}

	TArray<FHitResult> ReturnHitResults;

	for (int32 TraceIndex = 0; TraceIndex < NumberOfTraces; TraceIndex++)
	{
		AimWithPlayerController(InSourceActor, Params, TraceStart, TraceEnd);

		SetActorLocationAndRotation(TraceEnd, SourceActor->GetActorRotation());

		CurrentTraceEnd = TraceEnd;

		TArray<FHitResult> TraceHitResults;
		DoTrace(TraceHitResults, InSourceActor->GetWorld(), Filter, TraceStart, TraceEnd, TraceProfile.Name, Params);

		for (int32 j = TraceHitResults.Num() - 1; j >= 0; j--)
		{
			if (MaxHitResultsPerTrace >= 0 && j + 1 > MaxHitResultsPerTrace)
			{
				TraceHitResults.RemoveAt(j);
				continue;
			}

			FHitResult& HitResult = TraceHitResults[j];
			
			if (bUsePersistentHitResults)
			{
				if (HitResult.GetActor() && (!HitResult.bBlockingHit || PersistentHitResults.Num() < 1))
				{
					bool bActorAlreadyInPersistentHits = false;

					// Make sure PersistentHitResults doesn't have this hit actor already
					for (int32 k = 0; k < PersistentHitResults.Num(); k++)
					{
						FHitResult& PersistentHitResult = PersistentHitResults[k];

						if (PersistentHitResult.GetActor() == HitResult.GetActor())
						{
							bActorAlreadyInPersistentHits = true;
							break;
						}
					}

					if (bActorAlreadyInPersistentHits)
					{
						continue;
					}

					if (PersistentHitResults.Num() >= MaxHitResultsPerTrace)
					{
						PersistentHitResults.RemoveAt(0);
					}

					PersistentHitResults.Add(HitResult);
				}
			}
			else
			{
				int32 ReticleIndex = TraceIndex * MaxHitResultsPerTrace + j;
				
				if (ReticleIndex < ReticleActors.Num())
				{
					AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActors[ReticleIndex].Get();
					
					if (LocalReticleActor)
					{
						const bool bHitActor = HitResult.GetActor() != nullptr;

						if (bHitActor && !HitResult.bBlockingHit)
						{
							LocalReticleActor->SetActorHiddenInGame(false);

							const FVector ReticleLocation = (bHitActor && LocalReticleActor->bSnapToTargetedActor)
								                                ? HitResult.GetActor()->GetActorLocation()
								                                : HitResult.Location;

							LocalReticleActor->SetActorLocation(ReticleLocation);
							LocalReticleActor->SetIsTargetAnActor(bHitActor);
						}
						else
						{
							LocalReticleActor->SetActorHiddenInGame(true);
						}
					}
				}
			}
		}

		if (!bUsePersistentHitResults)
		{
			if (TraceHitResults.Num() < ReticleActors.Num())
			{
				for (int32 j = TraceHitResults.Num(); j < ReticleActors.Num(); j++)
				{
					AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActors[j].Get();
					
					if (LocalReticleActor)
					{
						LocalReticleActor->SetIsTargetAnActor(false);
						LocalReticleActor->SetActorHiddenInGame(true);
					}
				}
			}
		}

		if (TraceHitResults.Num() < 1)
		{
			FHitResult HitResult;
			HitResult.TraceStart = StartLocation.GetTargetingTransform().GetLocation();
			HitResult.TraceEnd = TraceEnd;
			HitResult.Location = TraceEnd;
			HitResult.ImpactPoint = TraceEnd;
			TraceHitResults.Add(HitResult);

			if (bUsePersistentHitResults && PersistentHitResults.Num() < 1)
			{
				PersistentHitResults.Add(HitResult);
			}
		}

		ReturnHitResults.Append(TraceHitResults);
	}
	
	if (bUsePersistentHitResults && MaxHitResultsPerTrace > 0)
	{
		for (int32 PersistentHitResultIndex = 0; PersistentHitResultIndex < PersistentHitResults.Num();
		     PersistentHitResultIndex++)
		{
			FHitResult& HitResult = PersistentHitResults[PersistentHitResultIndex];
			
			HitResult.TraceStart = StartLocation.GetTargetingTransform().GetLocation();

			AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActors[PersistentHitResultIndex].Get();
			
			if (LocalReticleActor)
			{
				const bool bHitActor = HitResult.GetActor() != nullptr;

				if (bHitActor && !HitResult.bBlockingHit)
				{
					LocalReticleActor->SetActorHiddenInGame(false);

					const FVector ReticleLocation = (bHitActor && LocalReticleActor->bSnapToTargetedActor)
						                                ? HitResult.GetActor()->GetActorLocation()
						                                : HitResult.Location;

					LocalReticleActor->SetActorLocation(ReticleLocation);
					LocalReticleActor->SetIsTargetAnActor(bHitActor);
				}
				else
				{
					LocalReticleActor->SetActorHiddenInGame(true);
				}
			}
		}

		if (PersistentHitResults.Num() < ReticleActors.Num())
		{
			for (int32 PersistentHitResultIndex = PersistentHitResults.Num(); PersistentHitResultIndex < ReticleActors.
			     Num(); PersistentHitResultIndex++)
			{
				AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActors[PersistentHitResultIndex].Get();
				
				if (LocalReticleActor)
				{
					LocalReticleActor->SetIsTargetAnActor(false);
					LocalReticleActor->SetActorHiddenInGame(true);
				}
			}
		}

		return PersistentHitResults;
	}

	return ReturnHitResults;
}

AGameplayAbilityWorldReticle* AGSGATA_Trace::SpawnReticleActor(FVector Location, FRotator Rotation)
{
	if (ReticleClass)
	{
		AGameplayAbilityWorldReticle* SpawnedReticleActor = GetWorld()->SpawnActor<AGameplayAbilityWorldReticle>(
			ReticleClass, Location, Rotation);
		if (SpawnedReticleActor)
		{
			SpawnedReticleActor->InitializeReticle(this, PrimaryPC, ReticleParams);
			SpawnedReticleActor->SetActorHiddenInGame(true);
			ReticleActors.Add(SpawnedReticleActor);
			
			if (!ShouldProduceTargetDataOnServer)
			{
				SpawnedReticleActor->SetReplicates(false);
			}

			return SpawnedReticleActor;
		}
	}

	return nullptr;
}

void AGSGATA_Trace::DestroyReticleActors()
{
	for (int32 i = ReticleActors.Num() - 1; i >= 0; i--)
	{
		if (ReticleActors[i].IsValid())
		{
			ReticleActors[i].Get()->Destroy();
		}
	}

	ReticleActors.Empty();
}


void AGSGATA_Trace::BeginPlay()
{
	Super::BeginPlay();
	
	SetActorTickEnabled(false);
}

void AGSGATA_Trace::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TArray<FHitResult> HitResults;
	if (bDebug || bUsePersistentHitResults)
	{
		HitResults = PerformTrace(SourceActor);
	}

#if ENABLE_DRAW_DEBUG
	if (SourceActor && bDebug)
	{
		ShowDebugTrace(HitResults, EDrawDebugTrace::Type::ForOneFrame);
	}
#endif
}
