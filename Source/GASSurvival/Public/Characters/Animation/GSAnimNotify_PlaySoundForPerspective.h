#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify_PlaySound.h"
#include "GSAnimNotify_PlaySoundForPerspective.generated.h"

UCLASS(meta = (DisplayName = "Play Sound For Perspective"))
class GASSURVIVAL_API UGSAnimNotify_PlaySoundForPerspective : public UAnimNotify_PlaySound
{
	GENERATED_BODY()

public:
	UGSAnimNotify_PlaySoundForPerspective();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimNotify", meta = (ExposeOnSpawn = true))
	bool bPlayForFirstPersonPerspective;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
