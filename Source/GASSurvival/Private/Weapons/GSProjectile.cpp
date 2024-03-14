#include "Weapons/GSProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

AGSProjectile::AGSProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovement"));
	ProjectileMovement->ProjectileGravityScale = 0;
	ProjectileMovement->InitialSpeed = 7000.0f;

	bReplicates = true;
	
	NetUpdateFrequency = 100.0f;
}
