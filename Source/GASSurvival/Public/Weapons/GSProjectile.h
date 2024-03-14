#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GSProjectile.generated.h"

class UProjectileMovementComponent;

UCLASS()
class GASSURVIVAL_API AGSProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AGSProjectile();

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "PBProjectile")
	UProjectileMovementComponent* ProjectileMovement;
};
