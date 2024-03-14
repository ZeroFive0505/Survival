#pragma once

#include "CoreMinimal.h"
#include "Characters/GSCharacter.h"
#include "GameplayEffectTypes.h"
#include "Characters/Abilities/GSInteractable.h"
#include "GSSurvivor.generated.h"

class AGSWeapon;
class UGameplayEffect;
class UCameraComponent;
class USpringArmComponent;
class UGSFloatingStatusBarWidget;
class UInputAction;
class UGSAmmoAttributeSet;
class UWidgetComponent;

UENUM(BlueprintType)
enum class EGSSurvivorWeaponState : uint8
{
	Rifle					UMETA(DisplayName = "Rifle"),
	RifleAiming				UMETA(DisplayName = "Rifle Aiming"),

	MagicGun				UMETA(DisplayName = "Magic gun"),
	
	Shotgun					UMETA(DisplayName = "Shotgun"),
	ShotgunAiming			UMETA(DisplayName = "Shotgun Aiming"),
	
	RocketLauncher			UMETA(DisplayName = "Rocket Launcher"),
	RocketLauncherAiming	UMETA(DisplayName = "Rocket Launcher Aiming"),

	GrenadeLauncher			UMETA(DisplayName = "Grenade Launcher"),
	GrenadeLauncherAiming	UMETA(DisplayName = "Grenade Launcher Aiming"),
	
	Sniper					UMETA(DisplayName = "Sniper"),
	SniperAiming			UMETA(DisplayName = "Sniper Aiming"),
	
	Pistol					UMETA(DisplayName = "Pistol"),
	PistolAiming			UMETA(DisplayName = "Pistol Aiming"),
	
	None					UMETA(DisplayName = "No Weapon")
};

USTRUCT()
struct GASSURVIVAL_API FGSSurvivorInventory
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<AGSWeapon*> Weapons;
};

UCLASS()
class GASSURVIVAL_API AGSSurvivor : public AGSCharacter, public IGSInteractable
{
	GENERATED_BODY()
	
public:
	AGSSurvivor(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GASSurvival|GSSurvivor")
	bool bStartInFirstPersonPerspective;

	FGameplayTag CurrentWeaponTag;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	virtual void PossessedBy(AController* NewController) override;

	UGSFloatingStatusBarWidget* GetFloatingStatusBar() const;
	
	virtual void KnockDown();
	
	virtual void PlayKnockDownEffects();
	
	virtual void PlayReviveEffects();

	virtual void FinishDying() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "GASSurvival|GSSurvivor")
	void SpawnDeathPawn();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "GASSurvival|GSSurvivor")
	void HitBy(const AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|GSSurvivor")
	virtual bool IsInFirstPersonPerspective() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GASSurvival|GSSurvivor")
	USkeletalMeshComponent* GetFirstPersonMesh() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GASSurvival|GSSurvivor")
	USkeletalMeshComponent* GetThirdPersonMesh() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|Inventory")
	AGSWeapon* GetCurrentWeapon() const;
	
	UFUNCTION(BlueprintCallable, Category = "GASSurvival|Inventory")
	bool AddWeaponToInventory(AGSWeapon* NewWeapon, bool bEquipWeapon = false);
	
	UFUNCTION(BlueprintCallable, Category = "GASSurvival|Inventory")
	bool RemoveWeaponFromInventory(AGSWeapon* WeaponToRemove);

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|Inventory")
	void RemoveAllWeaponsFromInventory();

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|Inventory")
	void EquipWeapon(AGSWeapon* NewWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipWeapon(AGSWeapon* NewWeapon);
	void ServerEquipWeapon_Implementation(AGSWeapon* NewWeapon);
	bool ServerEquipWeapon_Validate(AGSWeapon* NewWeapon);

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|Inventory")
	virtual void NextWeapon();

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|Inventory")
	virtual void PreviousWeapon();

	FName GetWeaponAttachPoint();

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|Inventory")
	int32 GetPrimaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|Inventory")
	int32 GetMaxPrimaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|Inventory")
	int32 GetPrimaryReserveAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|Inventory")
	int32 GetSecondaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|Inventory")
	int32 GetMaxSecondaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|Inventory")
	int32 GetSecondaryReserveAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASSurvival|Inventory")
	int32 GetNumWeapons() const;

	virtual bool IsAvailableForInteraction_Implementation(UPrimitiveComponent* InteractionComponent) const override;

	virtual float GetInteractionDuration_Implementation(UPrimitiveComponent* InteractionComponent) const override;
	
	virtual void PreInteract_Implementation(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent) override;
	
	virtual void PostInteract_Implementation(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent) override;
	
	virtual void GetPreInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type, UPrimitiveComponent* InteractionComponent) const override;
	
	virtual void CancelInteraction_Implementation(UPrimitiveComponent* InteractionComponent) override;
	
	virtual FSimpleMulticastDelegate* GetTargetCancelInteractionDelegate(UPrimitiveComponent* InteractionComponent) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "GASSurvival|GSSurvivor")
	FVector StartingThirdPersonMeshLocation;

	UPROPERTY(BlueprintReadOnly, Category = "GASSurvival|GSSurvivor")
	FVector StartingFirstPersonMeshLocation;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASSurvival|Abilities")
	float ReviveDuration;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GASSurvival|Camera")
	float BaseTurnRate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GASSurvival|Camera")
	float BaseLookUpRate;

	UPROPERTY(BlueprintReadOnly, Category = "GASSurvival|Camera")
	float StartingThirdPersonCameraBoomArmLength;

	UPROPERTY(BlueprintReadOnly, Category = "GASSurvival|Camera")
	FVector StartingThirdPersonCameraBoomLocation;

	UPROPERTY(BlueprintReadOnly, Category = "GASSurvival|Camera")
	bool bIsFirstPersonPerspective;

	UPROPERTY(BlueprintReadOnly, Category = "GASSurvival|GASSurvivor")
	bool bWasInFirstPersonPerspectiveWhenKnockedDown;

	bool bASCInputBound;
	
	bool bChangedWeaponLocally;

	UPROPERTY(BlueprintReadOnly, Category = "GASSurvival|Camera")
	float Default1PFOV;

	UPROPERTY(BlueprintReadOnly, Category = "GASSurvival|Camera")
	float Default3PFOV;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASSurvival|GSSurvivor")
	FName WeaponAttachPoint;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "GASSurvival|Camera")
	USpringArmComponent* ThirdPersonCameraBoom;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "GASSurvival|Camera")
	UCameraComponent* ThirdPersonCamera;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "GASSurvival|Camera")
	UCameraComponent* FirstPersonCamera;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	USkeletalMeshComponent* FirstPersonMesh;
	
	UPROPERTY(ReplicatedUsing = OnRep_Inventory)
	FGSSurvivorInventory Inventory;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASSurvival|Inventory")
	TArray<TSubclassOf<AGSWeapon>> DefaultInventoryWeaponClasses;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon)
	AGSWeapon* CurrentWeapon;

	UPROPERTY()
	UGSAmmoAttributeSet* AmmoAttributeSet;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASSurvival|GSSurvivor")
	TSubclassOf<UGameplayEffect> KnockDownEffect;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASSurvival|GSSurvivor")
	TSubclassOf<UGameplayEffect> ReviveEffect;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASSurvival|GSSurvivor")
	TSubclassOf<UGameplayEffect> DeathEffect;

	FSimpleMulticastDelegate InteractionCanceledDelegate;
	
	FGameplayTag NoWeaponTag;
	FGameplayTag WeaponChangingDelayReplicationTag;
	FGameplayTag WeaponAmmoTypeNoneTag;
	FGameplayTag WeaponAbilityTag;
	FGameplayTag KnockedDownTag;
	FGameplayTag InteractingTag;
	
	FDelegateHandle PrimaryReserveAmmoChangedDelegateHandle;
	FDelegateHandle SecondaryReserveAmmoChangedDelegateHandle;
	
	FDelegateHandle WeaponChangingDelayReplicationTagChangedDelegateHandle;
	
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PostInitializeComponents() override;
	
	void LookUp(float Value);

	void LookUpRate(float Value);
	
	void Turn(float Value);
	
	void TurnRate(float Value);
	
	void MoveForward(float Value);

	void MoveRight(float Value);
	
	void TogglePerspective();
	
	void SetPerspective(bool InIsFirstPersonPerspective);
	
	UFUNCTION()
	virtual void InitializeFloatingStatusBar() override;

	virtual void OnRep_PlayerState() override;
	virtual void OnRep_Controller() override;
	
	void BindASCInput();
	
	void SpawnDefaultInventory();

	void SetupStartupPerspective();

	bool DoesWeaponExistInInventory(const AGSWeapon* InWeapon);

	void SetCurrentWeapon(AGSWeapon* NewWeapon, AGSWeapon* LastWeapon);
	
	void UnEquipWeapon(AGSWeapon* WeaponToUnEquip);
	
	void UnEquipCurrentWeapon();

	UFUNCTION()
	virtual void CurrentWeaponPrimaryClipAmmoChanged(int32 OldPrimaryClipAmmo, int32 NewPrimaryClipAmmo);

	UFUNCTION()
	virtual void CurrentWeaponSecondaryClipAmmoChanged(int32 OldSecondaryClipAmmo, int32 NewSecondaryClipAmmo);
	
	virtual void CurrentWeaponPrimaryReserveAmmoChanged(const FOnAttributeChangeData& Data);
	virtual void CurrentWeaponSecondaryReserveAmmoChanged(const FOnAttributeChangeData& Data);
	
	virtual void WeaponChangingDelayReplicationTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	UFUNCTION()
	void OnRep_CurrentWeapon(AGSWeapon* LastWeapon);

	UFUNCTION()
	void OnRep_Inventory();

	void OnAbilityActivationFailed(const UGameplayAbility* FailedAbility, const FGameplayTagContainer& FailTags);
	
	UFUNCTION(Server, Reliable)
	void ServerSyncCurrentWeapon();
	void ServerSyncCurrentWeapon_Implementation();
	bool ServerSyncCurrentWeapon_Validate();
	
	UFUNCTION(Client, Reliable)
	void ClientSyncCurrentWeapon(AGSWeapon* InWeapon);
	void ClientSyncCurrentWeapon_Implementation(AGSWeapon* InWeapon);
	bool ClientSyncCurrentWeapon_Validate(AGSWeapon* InWeapon);
};
