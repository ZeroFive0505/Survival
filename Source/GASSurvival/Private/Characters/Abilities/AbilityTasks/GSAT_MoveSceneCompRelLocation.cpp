

#include "Characters/Abilities/AbilityTasks/GSAT_MoveSceneCompRelLocation.h"
#include "Curves/CurveVector.h"

UGSAT_MoveSceneCompRelLocation::UGSAT_MoveSceneCompRelLocation(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bTickingTask = true;
	bIsFinished = false;
}

UGSAT_MoveSceneCompRelLocation* UGSAT_MoveSceneCompRelLocation::MoveSceneComponentRelativeLocation(
	UGameplayAbility* OwningAbility, FName TaskInstanceName, USceneComponent* SceneComponent, FVector Location,
	float Duration, UCurveFloat* OptionalInterpolationCurve, UCurveVector* OptionalVectorInterpolationCurve)
{
	UGSAT_MoveSceneCompRelLocation* Task = NewAbilityTask<UGSAT_MoveSceneCompRelLocation>(OwningAbility, TaskInstanceName);

	Task->Component = SceneComponent;
	Task->StartLocation = SceneComponent->GetRelativeLocation();
	Task->TargetLocation = Location;
	Task->DurationOfMovement = FMath::Max(Duration, 0.001f);		
	Task->TimeMoveStarted = Task->GetWorld()->GetTimeSeconds();
	Task->TimeMoveWillEnd = Task->TimeMoveStarted + Task->DurationOfMovement;
	Task->LerpCurve = OptionalInterpolationCurve;
	Task->LerpCurveVector = OptionalVectorInterpolationCurve;

	return Task;
}

void UGSAT_MoveSceneCompRelLocation::Activate()
{
	Super::Activate();
}

void UGSAT_MoveSceneCompRelLocation::TickTask(float DeltaTime)
{
	if (bIsFinished)
	{
		return;
	}

	Super::TickTask(DeltaTime);
	AActor* MyActor = GetAvatarActor();
	if (MyActor)
	{
		if (!IsValid(Component))
		{
			EndTask();
		}

		float CurrentTime = GetWorld()->GetTimeSeconds();

		if (CurrentTime >= TimeMoveWillEnd)
		{
			bIsFinished = true;

			Component->SetRelativeLocation(TargetLocation);

			if (!bIsSimulating)
			{
				if (ShouldBroadcastAbilityTaskDelegates())
				{
					OnFinishMove.Broadcast();
				}
				EndTask();
			}
		}
		else
		{
			FVector NewLocation;
			float MoveFraction = (CurrentTime - TimeMoveStarted) / DurationOfMovement;
			if (LerpCurveVector)
			{
				const FVector ComponentInterpolationFraction = LerpCurveVector->GetVectorValue(MoveFraction);
				NewLocation = FMath::Lerp<FVector, FVector>(StartLocation, TargetLocation, ComponentInterpolationFraction);
			}
			else
			{
				if (LerpCurve)
				{
					MoveFraction = LerpCurve->GetFloatValue(MoveFraction);
				}
				NewLocation = FMath::Lerp<FVector, float>(StartLocation, TargetLocation, MoveFraction);
			}
			Component->SetRelativeLocation(NewLocation);
		}
	}
	else
	{
		bIsFinished = true;
		EndTask();
	}
}

void UGSAT_MoveSceneCompRelLocation::OnDestroy(bool AbilityIsEnding)
{
	Super::OnDestroy(AbilityIsEnding);
}
