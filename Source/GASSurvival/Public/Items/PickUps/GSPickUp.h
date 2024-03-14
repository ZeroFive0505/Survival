#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "GSPickUp.generated.h"

class AGSCharacter;
class USoundCue;
class UGSGameplayAbility;
class UGameplayEffect;

UCLASS()
class GASSURVIVAL_API AGSPickUp : public AActor
{
	GENERATED_BODY()

public:	
	AGSPickUp();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void NotifyActorBeginOverlap(class AActor* Other) override;
	
	virtual bool CanBePickedUp(AGSCharacter* TestCharacter) const;

	UFUNCTION(BlueprintNativeEvent, Meta = (DisplayName = "CanBePickedUp"))
	bool BP_CanBePickedUp(AGSCharacter* TestCharacter) const;
	virtual bool BP_CanBePickedUp_Implementation(AGSCharacter* TestCharacter) const;

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "GSPickup")
	class UCapsuleComponent* CollisionComp;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsActive, Category = "GSPickup")
	bool bIsActive;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GSPickup")
	bool bCanRespawn;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GSPickup")
	float RespawnTime;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GSPickup")
	FGameplayTagContainer RestrictedPickupTags;
	
	UPROPERTY(EditDefaultsOnly, Category = "GSPickup")
	USoundCue* PickupSound;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GSPickup")
	TArray<TSubclassOf<UGSGameplayAbility>> AbilityClasses;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GSPickup")
	TArray<TSubclassOf<UGameplayEffect>> EffectClasses;
	
	UPROPERTY(BlueprintReadOnly, Replicated)
	AGSCharacter* PickedUpBy;

	FTimerHandle TimerHandle_RespawnPickup;

	void PickupOnTouch(AGSCharacter* Pawn);

	virtual void GivePickupTo(AGSCharacter* Pawn);
	
	virtual void OnPickedUp();
	
	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "OnPickedUp"))
	void BP_OnPickedUp();

	virtual void RespawnPickup();
	
	virtual void OnRespawned();
	
	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "OnRespawned"))
	void BP_OnRespawned();

	UFUNCTION()
	virtual void OnRep_IsActive();
};
