#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GSASCActor.generated.h"

UCLASS()
class GASSURVIVAL_API AGSASCActor : public AActor
{
	GENERATED_BODY()

public:
	AGSASCActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
