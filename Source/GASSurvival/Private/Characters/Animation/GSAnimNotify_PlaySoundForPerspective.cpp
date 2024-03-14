// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Animation/GSAnimNotify_PlaySoundForPerspective.h"

#include "Characters/Survivors/GSSurvivor.h"
#include "Kismet/GameplayStatics.h"

UGSAnimNotify_PlaySoundForPerspective::UGSAnimNotify_PlaySoundForPerspective()
{
	bPlayForFirstPersonPerspective = true;
}

void UGSAnimNotify_PlaySoundForPerspective::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	USoundBase* SoundToPlay = Sound;

	if (SoundToPlay)
	{
		if (SoundToPlay->IsLooping())
		{
			UE_LOG(LogAudio, Warning, TEXT("PlaySound notify: Anim %s tried to spawn infinitely looping sound asset %s. Spawning suppressed."), *GetNameSafe(Animation), *GetNameSafe(SoundToPlay));
			return;
		}

		const AGSSurvivor* OwningSurvivor = Cast<AGSSurvivor>(MeshComp->GetOwner());

		if (!OwningSurvivor)
		{
			if (bFollow)
			{
				UGameplayStatics::SpawnSoundAttached(SoundToPlay, MeshComp, AttachName, FVector(ForceInit), EAttachLocation::SnapToTarget, false, VolumeMultiplier, PitchMultiplier);
			}
			else
			{
				UGameplayStatics::PlaySoundAtLocation(MeshComp->GetWorld(), SoundToPlay, MeshComp->GetComponentLocation(), VolumeMultiplier, PitchMultiplier);
			}

			return;
		}
		
		if (bPlayForFirstPersonPerspective && !OwningSurvivor->IsLocallyControlled() && !OwningSurvivor->IsPlayerControlled())
		{
			return;
		}
		
		if ((!bPlayForFirstPersonPerspective && !OwningSurvivor->IsLocallyControlled() && !OwningSurvivor->IsPlayerControlled())
			|| bPlayForFirstPersonPerspective == OwningSurvivor->IsInFirstPersonPerspective())
		{
			if (bFollow)
			{
				UGameplayStatics::SpawnSoundAttached(SoundToPlay, MeshComp, AttachName, FVector(ForceInit), EAttachLocation::SnapToTarget, false, VolumeMultiplier, PitchMultiplier);
			}
			else
			{
				UGameplayStatics::PlaySoundAtLocation(MeshComp->GetWorld(), SoundToPlay, MeshComp->GetComponentLocation(), VolumeMultiplier, PitchMultiplier);
			}
		}
	}
}
