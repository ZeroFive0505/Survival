#include "Characters/Abilities/AbilityTasks/GSAT_WaitChangeFOV.h"

#include "Camera/CameraComponent.h"

UGSAT_WaitChangeFOV::UGSAT_WaitChangeFOV(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
	bIsFinished = false;
}

UGSAT_WaitChangeFOV* UGSAT_WaitChangeFOV::WaitChangeFOV(UGameplayAbility* OwningAbility, FName TaskInstanceName,
	UCameraComponent* CameraComponent, float TargetFOV, float Duration, UCurveFloat* OptionalInterpolationCurve)
{
	UGSAT_WaitChangeFOV* Task = NewAbilityTask<UGSAT_WaitChangeFOV>(OwningAbility, TaskInstanceName);

	Task->CameraComponent = CameraComponent;
	if (CameraComponent != nullptr)
	{
		Task->StartFOV = Task->CameraComponent->FieldOfView;
	}

	Task->TargetFOV = TargetFOV;
	Task->Duration = FMath::Max(Duration, 0.001f);
	Task->TimeChangeStarted = Task->GetWorld()->GetTimeSeconds();
	Task->TimeChangeWillEnd = Task->TimeChangeStarted + Task->Duration;
	Task->LerpCurve = OptionalInterpolationCurve;

	return Task;
}

void UGSAT_WaitChangeFOV::Activate()
{
	Super::Activate();
}

void UGSAT_WaitChangeFOV::TickTask(float DeltaTime)
{
	if (bIsFinished)
	{
		return;
	}

	Super::TickTask(DeltaTime);

	if (CameraComponent)
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();

		if (CurrentTime >= TimeChangeWillEnd)
		{
			bIsFinished = true;

			CameraComponent->SetFieldOfView(TargetFOV);
			
			if (ShouldBroadcastAbilityTaskDelegates())
			{
				OnTargetFOVReached.Broadcast();
			}
			
			EndTask();
		}
		else
		{
			float NewFOV;

			float MoveFraction = (CurrentTime - TimeChangeStarted) / Duration;
			if (LerpCurve)
			{
				MoveFraction = LerpCurve->GetFloatValue(MoveFraction);
			}

			NewFOV = FMath::Lerp<float, float>(StartFOV, TargetFOV, MoveFraction);

			CameraComponent->SetFieldOfView(NewFOV);
		}
	}
	else
	{
		bIsFinished = true;
		EndTask();
	}
}

void UGSAT_WaitChangeFOV::OnDestroy(bool AbilityIsEnding)
{
	Super::OnDestroy(AbilityIsEnding);
}
