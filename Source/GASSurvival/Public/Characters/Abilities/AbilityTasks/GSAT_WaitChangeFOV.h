﻿#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GSAT_WaitChangeFOV.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FChangeFOVDelegate);

class UCameraComponent;

UCLASS()
class GASSURVIVAL_API UGSAT_WaitChangeFOV : public UAbilityTask
{
	GENERATED_BODY()

public:
	UGSAT_WaitChangeFOV(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(BlueprintAssignable)
	FChangeFOVDelegate OnTargetFOVReached;
	
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGSAT_WaitChangeFOV* WaitChangeFOV(UGameplayAbility* OwningAbility, FName TaskInstanceName, class UCameraComponent* CameraComponent, float TargetFOV, float Duration, UCurveFloat* OptionalInterpolationCurve);

	virtual void Activate() override;
	
	virtual void TickTask(float DeltaTime) override;

	virtual void OnDestroy(bool AbilityIsEnding) override;

protected:
	bool bIsFinished;

	float StartFOV;

	float TargetFOV;

	float Duration;

	float TimeChangeStarted;

	float TimeChangeWillEnd;

	UPROPERTY()
	UCameraComponent* CameraComponent;

	UPROPERTY()
	UCurveFloat* LerpCurve;
};
