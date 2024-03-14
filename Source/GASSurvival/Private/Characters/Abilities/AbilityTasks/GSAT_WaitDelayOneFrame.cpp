// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Abilities/AbilityTasks/GSAT_WaitDelayOneFrame.h"

UGSAT_WaitDelayOneFrame::UGSAT_WaitDelayOneFrame(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UGSAT_WaitDelayOneFrame::Activate()
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UGSAT_WaitDelayOneFrame::OnDelayFinish);
}

UGSAT_WaitDelayOneFrame* UGSAT_WaitDelayOneFrame::WaitDelayOneFrame(UGameplayAbility* OwningAbility)
{
	UGSAT_WaitDelayOneFrame* Task = NewAbilityTask<UGSAT_WaitDelayOneFrame>(OwningAbility);
	return Task;
}

void UGSAT_WaitDelayOneFrame::OnDelayFinish()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnFinish.Broadcast();
	}
	
	EndTask();
}
