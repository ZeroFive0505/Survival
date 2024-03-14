#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "GASSurvival/GASSurvival.h"
#include "GSWeapon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponAmmoChangedDelegate, int32, OldValue, int32, NewValue);

class AGSSurvivor;
class UGSHUDReticle;
class UCapsuleComponent;
class AGSGATA_LineTrace;
class AGSGATA_SphereTrace;
class UAnimMontage;
class UGSAbilitySystemComponent;
class UGSGameplayAbility;
class UPaperSprite;
class USkeletalMeshComponent;

UENUM(Blueprintable)
enum EWeaponType
{
	Pistol,
	Rifle,
	Shotgun,
	RocketLauncher,
	Sniper,
	GrenadeLauncher,
	None
};

UCLASS()
class GASSURVIVAL_API AGSWeapon : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:	
	AGSWeapon();
	
	UPROPERTY(BlueprintReadWrite)
	bool bSpawnWithCollision;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASSurvival|GSWeapon")
	FGameplayTag WeaponTag;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASSurvival|GSWeapon")
	FGameplayTagContainer RestrictedPickupTags;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASSurvival|UI")
	UPaperSprite* PrimaryIcon;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASSurvival|UI")
	UPaperSprite* SecondaryIcon;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASSurvival|UI")
	UPaperSprite* PrimaryClipIcon;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASSurvival|UI")
	UPaperSprite* SecondaryClipIcon;

	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "GASSurvival|GSWeapon")
	FGameplayTag FireMode;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASSurvival|GSWeapon")
	FGameplayTag PrimaryAmmoType;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASSurvival|GSWeapon")
	FGameplayTag SecondaryAmmoType;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASSurvival|GSWeapon")
	TEnumAsByte<EWeaponType> WeaponType;
	
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "GASSurvival|GSWeapon")
	FText StatusText;

	UPROPERTY(BlueprintAssignable, Category = "GASSurvival|GSWeapon")
	FOnWeaponAmmoChangedDelegate OnPrimaryClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "GASSurvival|GSWeapon")
	FOnWeaponAmmoChangedDelegate OnMaxPrimaryClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "GASSurvival|GSWeapon")
	FOnWeaponAmmoChangedDelegate OnSecondaryClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "GASSurvival|GSWeapon")
	FOnWeaponAmmoChangedDelegate OnMaxSecondaryClipAmmoChanged;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GASSurvival|GSWeapon")
	virtual USkeletalMeshComponent* GetWeaponMesh1P() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GASSurvival|GSWeapon")
	virtual USkeletalMeshComponent* GetWeaponMesh3P() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	void SetOwningCharacter(AGSSurvivor* InOwningCharacter);
	
	virtual void NotifyActorBeginOverlap(AActor* Other) override;
	
	virtual void Equip();
	
	virtual void UnEquip();

	virtual void AddAbilities();

	virtual void RemoveAbilities();

	virtual int32 GetAbilityLevel(EGSAbilityInputID AbilityID);
	
	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSWeapon")
	virtual void ResetWeapon();

	UFUNCTION(NetMulticast, Reliable)
	void OnDropped(FVector NewLocation);
	virtual void OnDropped_Implementation(FVector NewLocation);
	virtual bool OnDropped_Validate(FVector NewLocation);

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSWeapon")
	virtual int32 GetPrimaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSWeapon")
	virtual int32 GetMaxPrimaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSWeapon")
	virtual int32 GetSecondaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSWeapon")
	virtual int32 GetMaxSecondaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSWeapon")
	virtual void SetPrimaryClipAmmo(int32 NewPrimaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSWeapon")
	virtual void SetMaxPrimaryClipAmmo(int32 NewMaxPrimaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSWeapon")
	virtual void SetSecondaryClipAmmo(int32 NewSecondaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSWeapon")
	virtual void SetMaxSecondaryClipAmmo(int32 NewMaxSecondaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSWeapon")
	TSubclassOf<UGSHUDReticle> GetPrimaryHUDReticleClass() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSWeapon")
	virtual bool HasInfiniteAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|Animation")
	UAnimMontage* GetEquip1PMontage() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|Animation")
	UAnimMontage* GetEquip3PMontage() const;
	
	UFUNCTION(BlueprintCallable, Category = "GASSurvival|Audio")
	USoundCue* GetPickupSound() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSWeapon")
	FText GetDefaultStatusText() const;
	
	UFUNCTION(BlueprintCallable, Category = "GASSurvival|Targeting")
	AGSGATA_LineTrace* GetLineTraceTargetActor();
	
	UFUNCTION(BlueprintCallable, Category = "GASSurvival|Targeting")
	AGSGATA_SphereTrace* GetSphereTraceTargetActor();

protected:
	UPROPERTY()
	UGSAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_PrimaryClipAmmo, Category = "GASSurvival|GSWeapon|Ammo")
	int32 PrimaryClipAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_MaxPrimaryClipAmmo, Category = "GASSurvival|GSWeapon|Ammo")
	int32 MaxPrimaryClipAmmo;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_SecondaryClipAmmo, Category = "GASSurvival|GSWeapon|Ammo")
	int32 SecondaryClipAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_MaxSecondaryClipAmmo, Category = "GASSurvival|GSWeapon|Ammo")
	int32 MaxSecondaryClipAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASSurvival|GSWeapon|Ammo")
	bool bInfiniteAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASSurvival|UI")
	TSubclassOf<UGSHUDReticle> PrimaryHUDReticleClass;

	UPROPERTY()
	AGSGATA_LineTrace* LineTraceTargetActor;

	UPROPERTY()
	AGSGATA_SphereTrace* SphereTraceTargetActor;
	
	UPROPERTY(VisibleAnywhere)
	UCapsuleComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, Category = "GASSurvival|GSWeapon")
	USkeletalMeshComponent* WeaponMesh1P;

	UPROPERTY(VisibleAnywhere, Category = "GASSurvival|GSWeapon")
	USkeletalMeshComponent* WeaponMesh3P;
	
	UPROPERTY(EditDefaultsOnly, Category = "GASSurvival|GSWeapon")
	FVector WeaponMesh3PickupRelativeLocation;
	
	UPROPERTY(EditDefaultsOnly, Category = "GASSurvival|GSWeapon")
	FVector WeaponMesh1PEquippedRelativeLocation;

	UPROPERTY(EditDefaultsOnly, Category = "GASSurvival|GSWeapon")
	FVector WeaponMesh3PEquippedRelativeLocation;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "GASSurvival|GSWeapon")
	AGSSurvivor* OwningCharacter;

	UPROPERTY(EditAnywhere, Category = "GASSurvival|GSWeapon")
	TArray<TSubclassOf<UGSGameplayAbility>> Abilities;

	UPROPERTY(BlueprintReadOnly, Category = "GASSurvival|GSWeapon")
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GASSurvival|GSWeapon")
	FGameplayTag DefaultFireMode;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASSurvival|GSWeapon")
	FText DefaultStatusText;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASSurvival|Animation")
	UAnimMontage* Equip1PMontage;

	UPROPERTY(BlueprintReadonly, EditAnywhere, Category = "GASSurvival|Animation")
	UAnimMontage* Equip3PMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "GASSurvival|Audio")
	USoundCue* PickupSound;

	FGameplayTag WeaponPrimaryInstantAbilityTag;
	FGameplayTag WeaponSecondaryInstantAbilityTag;
	FGameplayTag WeaponAlternateInstantAbilityTag;
	FGameplayTag WeaponIsFiringTag;

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	
	virtual void PickUpOnTouch(AGSSurvivor* InCharacter);

	UFUNCTION()
	virtual void OnRep_PrimaryClipAmmo(int32 OldPrimaryClipAmmo);

	UFUNCTION()
	virtual void OnRep_MaxPrimaryClipAmmo(int32 OldMaxPrimaryClipAmmo);

	UFUNCTION()
	virtual void OnRep_SecondaryClipAmmo(int32 OldSecondaryClipAmmo);

	UFUNCTION()
	virtual void OnRep_MaxSecondaryClipAmmo(int32 OldMaxSecondaryClipAmmo);
};
