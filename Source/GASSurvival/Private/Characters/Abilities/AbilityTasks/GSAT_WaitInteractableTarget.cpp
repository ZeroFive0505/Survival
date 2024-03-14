#include "Characters/Abilities/AbilityTasks/GSAT_WaitInteractableTarget.h"

#include "GSBlueprintFunctionLibrary.h"
#include "Characters/Survivors/GSSurvivor.h"
#include "GASSurvival/GASSurvival.h"

UGSAT_WaitInteractableTarget::UGSAT_WaitInteractableTarget(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer)
{
	bTraceAffectsAimPitch = true;
}

UGSAT_WaitInteractableTarget* UGSAT_WaitInteractableTarget::WaitForInteractableTarget(UGameplayAbility* OwningAbility,
	FName TaskInstanceName, FCollisionProfileName TraceProfile, float MaxRange, float TimerPeriod, bool bShowDebug)
{
	UGSAT_WaitInteractableTarget* Task = NewAbilityTask<UGSAT_WaitInteractableTarget>(OwningAbility, TaskInstanceName);
	Task->TraceProfile = TraceProfile;
	Task->MaxRange = MaxRange;
	Task->TimerPeriod = TimerPeriod;
	Task->bShowDebug = bShowDebug;

	const AGSSurvivor* Survivor = Cast<AGSSurvivor>(OwningAbility->GetCurrentActorInfo()->AvatarActor);

	Task->StartLocation1P = FGameplayAbilityTargetingLocationInfo();
	Task->StartLocation1P.LocationType = EGameplayAbilityTargetingLocationType::SocketTransform;
	Task->StartLocation1P.SourceComponent = Survivor->GetFirstPersonMesh();
	Task->StartLocation1P.SourceSocketName = "WeaponPoint";
	Task->StartLocation1P.SourceAbility = OwningAbility;

	Task->StartLocation3P = FGameplayAbilityTargetingLocationInfo();
	Task->StartLocation3P.LocationType = EGameplayAbilityTargetingLocationType::SocketTransform;
	Task->StartLocation3P.SourceComponent = Survivor->GetThirdPersonMesh();
	Task->StartLocation3P.SourceSocketName = "Head";
	Task->StartLocation3P.SourceAbility = OwningAbility;

	return Task;
}

void UGSAT_WaitInteractableTarget::Activate()
{
	const UWorld* World = GetWorld();
	World->GetTimerManager().SetTimer(TraceTimerHandle, this, &UGSAT_WaitInteractableTarget::PerformTrace, TimerPeriod,
	                                  true);
}

void UGSAT_WaitInteractableTarget::OnDestroy(bool AbilityEnded)
{
	const UWorld* World = GetWorld();
	World->GetTimerManager().ClearTimer(TraceTimerHandle);

	Super::OnDestroy(AbilityEnded);
}

void UGSAT_WaitInteractableTarget::LineTrace(FHitResult& OutHitResult, const UWorld* World, const FVector& Start,
                                             const FVector& End, FName ProfileName, const FCollisionQueryParams Params,
                                             bool bLookForInteractableActor) const
{
	check(World);

	TArray<FHitResult> HitResults;
	World->LineTraceMultiByProfile(HitResults, Start, End, ProfileName, Params);

	OutHitResult.TraceStart = Start;
	OutHitResult.TraceEnd = End;

	for (int32 HitIdx = 0; HitIdx < HitResults.Num(); HitIdx++)
	{
		const FHitResult& Hit = HitResults[HitIdx];

		if (!Hit.GetActor() || Hit.GetActor() != Ability->GetCurrentActorInfo()->AvatarActor.Get())
		{
			if (bLookForInteractableActor && Hit.GetActor())
			{
				if (Hit.Component.IsValid() && Hit.Component.Get()->GetCollisionResponseToChannel(
						COLLISION_INTERACTABLE)
					== ECollisionResponse::ECR_Overlap)
				{
					bool bIsInteractable = Hit.GetActor()->Implements<UGSInteractable>();

					if (bIsInteractable && IGSInteractable::Execute_IsAvailableForInteraction(
						Hit.GetActor(), Hit.Component.Get()))
					{
						OutHitResult = Hit;
						OutHitResult.bBlockingHit = true;
						return;
					}
				}

				OutHitResult.TraceEnd = Hit.Location;
				OutHitResult.bBlockingHit = false;
				return;
			}
			
			OutHitResult = Hit;
			OutHitResult.bBlockingHit = true;
			return;
		}
	}
}

void UGSAT_WaitInteractableTarget::AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params,
                                                           const FVector& TraceStart, FVector& OutTraceEnd,
                                                           bool bIgnorePitch) const
{
	if (!Ability)
	{
		return;
	}

	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	
	FVector ViewStart = TraceStart;
	FRotator ViewRot(0.0f);
	if (PC)
	{
		PC->GetPlayerViewPoint(ViewStart, ViewRot);
	}

	const FVector ViewDir = ViewRot.Vector();
	FVector ViewEnd = ViewStart + (ViewDir * MaxRange);

	ClipCameraRayToAbilityRange(ViewStart, ViewDir, TraceStart, MaxRange, ViewEnd);

	FHitResult HitResult;
	LineTrace(HitResult, InSourceActor->GetWorld(), ViewStart, ViewEnd, TraceProfile.Name, Params, false);

	const bool bUseTraceResult = HitResult.bBlockingHit && (FVector::DistSquared(TraceStart, HitResult.Location) <= (
		MaxRange * MaxRange));

	const FVector AdjustedEnd = (bUseTraceResult) ? HitResult.Location : ViewEnd;

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

	OutTraceEnd = TraceStart + (AdjustedAimDir * MaxRange);
}

bool UGSAT_WaitInteractableTarget::ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection,
                                                               FVector AbilityCenter, float AbilityRange,
                                                               FVector& ClippedPosition) const
{
	FVector CameraToCenter = AbilityCenter - CameraLocation;
	float DotToCenter = FVector::DotProduct(CameraToCenter, CameraDirection);
	if (DotToCenter >= 0.0f)
	{
		float DistanceSquared = CameraToCenter.SizeSquared() - (DotToCenter * DotToCenter);
		float RadiusSquared = (AbilityRange * AbilityRange);
		if (DistanceSquared <= RadiusSquared)
		{
			float DistanceFromCamera = FMath::Sqrt(RadiusSquared - DistanceSquared);
			float DistanceAlongRay = DotToCenter + DistanceFromCamera;
			ClippedPosition = CameraLocation + (DistanceAlongRay * CameraDirection);
			return true;
		}
	}
	return false;
}

void UGSAT_WaitInteractableTarget::PerformTrace()
{
	bool bTraceComplex = false;
	TArray<AActor*> ActorsToIgnore;

	AActor* SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	if (!SourceActor)
	{
		UE_LOG(LogTemp, Error, TEXT("%s %s SourceActor was null"), *FString(__FUNCTION__), *UGSBlueprintFunctionLibrary::GetPlayerEditorWindowRole(GetWorld()));
		return;
	}

	ActorsToIgnore.Add(SourceActor);
	
	AGSSurvivor* Hero = Cast<AGSSurvivor>(SourceActor);
	if (Hero->IsInFirstPersonPerspective())
	{
		StartLocation = StartLocation1P;
	}
	else
	{
		StartLocation = StartLocation3P;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AGameplayAbilityTargetActor_SingleLineTrace), bTraceComplex);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActors(ActorsToIgnore);
	
	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();
	FVector TraceEnd;
	AimWithPlayerController(SourceActor, Params, TraceStart, TraceEnd);
	
	FHitResult ReturnHitResult;
	LineTrace(ReturnHitResult, GetWorld(), TraceStart, TraceEnd, TraceProfile.Name, Params, true);
	
	if (!ReturnHitResult.bBlockingHit)
	{
		ReturnHitResult.Location = TraceEnd;
		if (TargetData.Num() > 0 && TargetData.Get(0)->GetHitResult()->GetActor())
		{
			LostInteractableTarget.Broadcast(TargetData);
		}

		TargetData = MakeTargetData(ReturnHitResult);
	}
	else
	{
		bool bBroadcastNewTarget = true;

		if (TargetData.Num() > 0)
		{
			const AActor* OldTarget = TargetData.Get(0)->GetHitResult()->GetActor();

			if (OldTarget == ReturnHitResult.GetActor())
			{
				bBroadcastNewTarget = false;
			}
			else if (OldTarget)
			{
				LostInteractableTarget.Broadcast(TargetData);
			}
		}

		if (bBroadcastNewTarget)
		{
			TargetData = MakeTargetData(ReturnHitResult);
			FoundNewInteractableTarget.Broadcast(TargetData);
		}
	}

#if ENABLE_DRAW_DEBUG
	if (bShowDebug)
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, false, TimerPeriod);

		if (ReturnHitResult.bBlockingHit)
		{
			DrawDebugSphere(GetWorld(), ReturnHitResult.Location, 20.0f, 16, FColor::Red, false, TimerPeriod);
		}
		else
		{
			DrawDebugSphere(GetWorld(), ReturnHitResult.TraceEnd, 20.0f, 16, FColor::Green, false, TimerPeriod);
		}
	}
#endif // ENABLE_DRAW_DEBUG
}

FGameplayAbilityTargetDataHandle UGSAT_WaitInteractableTarget::MakeTargetData(const FHitResult& HitResult) const
{
	return StartLocation.MakeTargetDataHandleFromHitResult(Ability, HitResult);
}
