#include "Characters/Survivors/GSSurvivor.h"
#include "AIController.h"
#include "GSBlueprintFunctionLibrary.h"
#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "Camera/CameraComponent.h"
#include "Characters/Abilities/GSAbilitySystemComponent.h"
#include "Characters/Abilities/AttributeSets/GSAmmoAttributeSet.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GASSurvival/GASSurvival.h"
#include "GASSurvival/GASSurvivalGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Player/GSPlayerController.h"
#include "Player/GSPlayerState.h"
#include "Sound/SoundCue.h"
#include "UI/GSFloatingStatusBarWidget.h"
#include "Weapons/GSWeapon.h"

AGSSurvivor::AGSSurvivor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;
	bStartInFirstPersonPerspective = true;
	bIsFirstPersonPerspective = false;
	bWasInFirstPersonPerspectiveWhenKnockedDown = false;
	bASCInputBound = false;
	bChangedWeaponLocally = false;
	Default1PFOV = 90.0f;
	Default3PFOV = 80.0f;
	NoWeaponTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.None"));
	WeaponChangingDelayReplicationTag = FGameplayTag::RequestGameplayTag(
		FName("Ability.Weapon.IsChangingDelayReplication"));
	WeaponAmmoTypeNoneTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.None"));
	WeaponAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon"));
	CurrentWeaponTag = NoWeaponTag;
	Inventory = FGSSurvivorInventory();
	ReviveDuration = 4.0f;

	ThirdPersonCameraBoom = CreateDefaultSubobject<USpringArmComponent>(FName("CameraBoom"));
	ThirdPersonCameraBoom->SetupAttachment(RootComponent);
	ThirdPersonCameraBoom->bUsePawnControlRotation = true;
	ThirdPersonCameraBoom->SetRelativeLocation(FVector(0, 50, 68.492264));

	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(FName("FollowCamera"));
	ThirdPersonCamera->SetupAttachment(ThirdPersonCameraBoom);
	ThirdPersonCamera->FieldOfView = Default3PFOV;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(FName("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(RootComponent);
	FirstPersonCamera->bUsePawnControlRotation = true;

	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(FirstPersonCamera);
	FirstPersonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	FirstPersonMesh->bReceivesDecals = false;
	FirstPersonMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
	FirstPersonMesh->CastShadow = false;
	FirstPersonMesh->SetVisibility(false, true);

	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(FName("NoCollision"));
	GetMesh()->SetCollisionResponseToChannel(COLLISION_INTERACTABLE, ECollisionResponse::ECR_Overlap);
	GetMesh()->bCastHiddenShadow = true;
	GetMesh()->bReceivesDecals = false;
	
	UIFloatingStatusBarClass = StaticLoadClass(UObject::StaticClass(), nullptr,
	                                           TEXT(
		                                           "/Game/Survival/Blueprints/UI/WBP_SurvivorFloatingStatusBar.WBP_SurvivorFloatingStatusBar_C"));
	if (!UIFloatingStatusBarClass)
	{
		UE_LOG(LogTemp, Error,
		       TEXT(
			       "%s() Failed to find UIFloatingStatusBarClass. If it was moved, please update the reference location in C++."
		       ), *FString(__FUNCTION__));
	}

	AutoPossessAI = EAutoPossessAI::PlacedInWorld;
	AIControllerClass = AAIController::StaticClass();

	KnockedDownTag = FGameplayTag::RequestGameplayTag("State.KnockedDown");
	InteractingTag = FGameplayTag::RequestGameplayTag("State.Interacting");
}

void AGSSurvivor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGSSurvivor, Inventory);
	
	DOREPLIFETIME_CONDITION(AGSSurvivor, CurrentWeapon, COND_SimulatedOnly);
}

void AGSSurvivor::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGSSurvivor::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGSSurvivor::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &AGSSurvivor::LookUp);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AGSSurvivor::LookUpRate);
	PlayerInputComponent->BindAxis("Turn", this, &AGSSurvivor::Turn);
	PlayerInputComponent->BindAxis("TurnRate", this, &AGSSurvivor::TurnRate);

	PlayerInputComponent->BindAction("TogglePerspective", IE_Pressed, this, &AGSSurvivor::TogglePerspective);
	
	BindASCInput();
}

void AGSSurvivor::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AGSPlayerState* PS = GetPlayerState<AGSPlayerState>();
	if (PS)
	{
		AbilitySystemComponent = Cast<UGSAbilitySystemComponent>(PS->GetAbilitySystemComponent());
		
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);

		WeaponChangingDelayReplicationTagChangedDelegateHandle = AbilitySystemComponent->RegisterGameplayTagEvent(
				WeaponChangingDelayReplicationTag)
			.AddUObject(this, &AGSSurvivor::WeaponChangingDelayReplicationTagChanged);
		
		AttributeSetBase = PS->GetAttributeSetBase();

		AmmoAttributeSet = PS->GetAmmoAttributeSet();
		
		InitializeAttributes();

		AddStartupEffects();

		AddCharacterAbilities();

		AGSPlayerController* PC = Cast<AGSPlayerController>(GetController());
		
		if (PC)
		{
			PC->CreateHUD();
		}

		if (AbilitySystemComponent->GetTagCount(DeadTag) > 0)
		{
			SetHealth(GetMaxHealth());
			SetMana(GetMaxMana());
			SetStamina(GetMaxStamina());
			SetShield(GetMaxShield());
		}
		
		AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(DeadTag));

		InitializeFloatingStatusBar();
		
		if (IsLocallyControlled() && IsPlayerControlled() && UIFloatingStatusBarComponent && UIFloatingStatusBar)
		{
			UIFloatingStatusBarComponent->SetVisibility(false, true);
		}
	}

	SetupStartupPerspective();
}

UGSFloatingStatusBarWidget* AGSSurvivor::GetFloatingStatusBar() const
{
	return UIFloatingStatusBar;
}

void AGSSurvivor::KnockDown()
{
	if (!HasAuthority())
	{
		return;
	}

	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->CancelAllAbilities();

		FGameplayTagContainer EffectTagsToRemove;
		EffectTagsToRemove.AddTag(EffectRemoveOnDeathTag);
		int32 NumEffectsRemoved = AbilitySystemComponent->RemoveActiveEffectsWithTags(EffectTagsToRemove);

		AbilitySystemComponent->ApplyGameplayEffectToSelf(Cast<UGameplayEffect>(KnockDownEffect->GetDefaultObject()),
		                                                  1.0f, AbilitySystemComponent->MakeEffectContext());
	}

	SetHealth(GetMaxHealth());
	SetShield(0.0f);
}

void AGSSurvivor::PlayKnockDownEffects()
{
	bWasInFirstPersonPerspectiveWhenKnockedDown = IsInFirstPersonPerspective();

	SetPerspective(false);
	
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}

	if (AbilitySystemComponent)
	{
		FGameplayCueParameters GCParameters;
		GCParameters.Location = GetActorLocation();
		AbilitySystemComponent->ExecuteGameplayCueLocal(
			FGameplayTag::RequestGameplayTag("GameplayCue.Hero.KnockedDown"), GCParameters);
	}
}

void AGSSurvivor::PlayReviveEffects()
{
	SetPerspective(bWasInFirstPersonPerspectiveWhenKnockedDown);
	
	if (AbilitySystemComponent)
	{
		FGameplayCueParameters GCParameters;
		GCParameters.Location = GetActorLocation();
		AbilitySystemComponent->ExecuteGameplayCueLocal(FGameplayTag::RequestGameplayTag("GameplayCue.Hero.Revived"),
		                                                GCParameters);
	}
}

void AGSSurvivor::FinishDying()
{
	if (!HasAuthority())
	{
		return;
	}

	RemoveAllWeaponsFromInventory();

	AbilitySystemComponent->RegisterGameplayTagEvent(WeaponChangingDelayReplicationTag).Remove(
		WeaponChangingDelayReplicationTagChangedDelegateHandle);

	AGASSurvivalGameModeBase* GameMode = Cast<AGASSurvivalGameModeBase>(GetWorld()->GetAuthGameMode());

	if (GameMode)
	{
		GameMode->Died(GetController());
	}

	RemoveCharacterAbilities();

	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->CancelAllAbilities();

		FGameplayTagContainer EffectTagsToRemove;
		EffectTagsToRemove.AddTag(EffectRemoveOnDeathTag);
		int32 NumEffectsRemoved = AbilitySystemComponent->RemoveActiveEffectsWithTags(EffectTagsToRemove);

		AbilitySystemComponent->ApplyGameplayEffectToSelf(Cast<UGameplayEffect>(DeathEffect->GetDefaultObject()), 1.0f,
		                                                  AbilitySystemComponent->MakeEffectContext());
	}

	OnCharacterDied.Broadcast(this);
	SpawnDeathPawn();
	
	Super::FinishDying();
}

bool AGSSurvivor::IsInFirstPersonPerspective() const
{
	return bIsFirstPersonPerspective;
}

USkeletalMeshComponent* AGSSurvivor::GetFirstPersonMesh() const
{
	return FirstPersonMesh;
}

USkeletalMeshComponent* AGSSurvivor::GetThirdPersonMesh() const
{
	return GetMesh();
}

AGSWeapon* AGSSurvivor::GetCurrentWeapon() const
{
	return CurrentWeapon;
}

bool AGSSurvivor::AddWeaponToInventory(AGSWeapon* NewWeapon, bool bEquipWeapon)
{
	if (DoesWeaponExistInInventory(NewWeapon))
	{
		USoundCue* PickupSound = NewWeapon->GetPickupSound();

		if (PickupSound && IsLocallyControlled())
		{
			UGameplayStatics::SpawnSoundAttached(PickupSound, GetRootComponent());
		}

		if (GetLocalRole() < ROLE_Authority)
		{
			return false;
		}
		
		UGameplayEffect* GEAmmo = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("Ammo")));
		GEAmmo->DurationPolicy = EGameplayEffectDurationType::Instant;

		if (NewWeapon->PrimaryAmmoType != WeaponAmmoTypeNoneTag)
		{
			int32 Idx = GEAmmo->Modifiers.Num();
			GEAmmo->Modifiers.SetNum(Idx + 1);

			FGameplayModifierInfo& InfoPrimaryAmmo = GEAmmo->Modifiers[Idx];
			InfoPrimaryAmmo.ModifierMagnitude = FScalableFloat(NewWeapon->GetPrimaryClipAmmo());
			InfoPrimaryAmmo.ModifierOp = EGameplayModOp::Additive;
			InfoPrimaryAmmo.Attribute = UGSAmmoAttributeSet::GetReserveAmmoAttributeFromTag(NewWeapon->PrimaryAmmoType);
		}

		if (NewWeapon->SecondaryAmmoType != WeaponAmmoTypeNoneTag)
		{
			int32 Idx = GEAmmo->Modifiers.Num();
			GEAmmo->Modifiers.SetNum(Idx + 1);

			FGameplayModifierInfo& InfoSecondaryAmmo = GEAmmo->Modifiers[Idx];
			InfoSecondaryAmmo.ModifierMagnitude = FScalableFloat(NewWeapon->GetSecondaryClipAmmo());
			InfoSecondaryAmmo.ModifierOp = EGameplayModOp::Additive;
			InfoSecondaryAmmo.Attribute = UGSAmmoAttributeSet::GetReserveAmmoAttributeFromTag(
				NewWeapon->SecondaryAmmoType);
		}

		if (GEAmmo->Modifiers.Num() > 0)
		{
			AbilitySystemComponent->
				ApplyGameplayEffectToSelf(GEAmmo, 1.0f, AbilitySystemComponent->MakeEffectContext());
		}

		NewWeapon->Destroy();

		return false;
	}

	if (GetLocalRole() < ROLE_Authority)
	{
		return false;
	}

	Inventory.Weapons.Add(NewWeapon);
	NewWeapon->SetOwningCharacter(this);
	NewWeapon->AddAbilities();

	if (bEquipWeapon)
	{
		EquipWeapon(NewWeapon);
		ClientSyncCurrentWeapon(CurrentWeapon);
	}

	return true;
}

bool AGSSurvivor::RemoveWeaponFromInventory(AGSWeapon* WeaponToRemove)
{
	if (DoesWeaponExistInInventory(WeaponToRemove))
	{
		if (WeaponToRemove == CurrentWeapon)
		{
			UnEquipCurrentWeapon();
		}

		Inventory.Weapons.Remove(WeaponToRemove);
		WeaponToRemove->RemoveAbilities();
		WeaponToRemove->SetOwningCharacter(nullptr);
		WeaponToRemove->ResetWeapon();
		
		return true;
	}

	return false;
}

void AGSSurvivor::RemoveAllWeaponsFromInventory()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		return;
	}

	UnEquipCurrentWeapon();

	float Radius = 50.0f;
	float NumWeapons = Inventory.Weapons.Num();

	for (int32 i = Inventory.Weapons.Num() - 1; i >= 0; i--)
	{
		AGSWeapon* Weapon = Inventory.Weapons[i];
		RemoveWeaponFromInventory(Weapon);
		
		float OffsetX = Radius * FMath::Cos((i / NumWeapons) * 2.0f * PI);
		float OffsetY = Radius * FMath::Sin((i / NumWeapons) * 2.0f * PI);
		Weapon->OnDropped(GetActorLocation() + FVector(OffsetX, OffsetY, 0.0f));
	}
}

void AGSSurvivor::EquipWeapon(AGSWeapon* NewWeapon)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerEquipWeapon(NewWeapon);
		SetCurrentWeapon(NewWeapon, CurrentWeapon);
		bChangedWeaponLocally = true;
	}
	else
	{
		SetCurrentWeapon(NewWeapon, CurrentWeapon);
	}
}

void AGSSurvivor::ServerEquipWeapon_Implementation(AGSWeapon* NewWeapon)
{
	EquipWeapon(NewWeapon);
}

bool AGSSurvivor::ServerEquipWeapon_Validate(AGSWeapon* NewWeapon)
{
	return true;
}

void AGSSurvivor::NextWeapon()
{
	if (Inventory.Weapons.Num() < 2)
	{
		return;
	}

	int32 CurrentWeaponIndex = Inventory.Weapons.Find(CurrentWeapon);
	UnEquipCurrentWeapon();

	if (CurrentWeaponIndex == INDEX_NONE)
	{
		EquipWeapon(Inventory.Weapons[0]);
	}
	else
	{
		EquipWeapon(Inventory.Weapons[(CurrentWeaponIndex + 1) % Inventory.Weapons.Num()]);
	}
}

void AGSSurvivor::PreviousWeapon()
{
	if (Inventory.Weapons.Num() < 2)
	{
		return;
	}

	int32 CurrentWeaponIndex = Inventory.Weapons.Find(CurrentWeapon);
	UnEquipCurrentWeapon();

	if (CurrentWeaponIndex == INDEX_NONE)
	{
		EquipWeapon(Inventory.Weapons[0]);
	}
	else
	{
		int32 IndexOfPrevWeapon = FMath::Abs(CurrentWeaponIndex - 1 + Inventory.Weapons.Num()) % Inventory.Weapons.
			Num();
		EquipWeapon(Inventory.Weapons[IndexOfPrevWeapon]);
	}
}

FName AGSSurvivor::GetWeaponAttachPoint()
{
	return WeaponAttachPoint;
}

int32 AGSSurvivor::GetPrimaryClipAmmo() const
{
	if (CurrentWeapon)
	{
		return CurrentWeapon->GetPrimaryClipAmmo();
	}

	return 0;
}

int32 AGSSurvivor::GetMaxPrimaryClipAmmo() const
{
	if (CurrentWeapon)
	{
		return CurrentWeapon->GetMaxPrimaryClipAmmo();
	}

	return 0;
}

int32 AGSSurvivor::GetPrimaryReserveAmmo() const
{
	if (CurrentWeapon && AmmoAttributeSet)
	{
		const FGameplayAttribute Attribute = AmmoAttributeSet->GetReserveAmmoAttributeFromTag(CurrentWeapon->PrimaryAmmoType);
		
		if (Attribute.IsValid())
		{
			return AbilitySystemComponent->GetNumericAttribute(Attribute);
		}
	}

	return 0;
}

int32 AGSSurvivor::GetSecondaryClipAmmo() const
{
	if (CurrentWeapon)
	{
		return CurrentWeapon->GetSecondaryClipAmmo();
	}

	return 0;
}

int32 AGSSurvivor::GetMaxSecondaryClipAmmo() const
{
	if (CurrentWeapon)
	{
		return CurrentWeapon->GetMaxSecondaryClipAmmo();
	}

	return 0;
}

int32 AGSSurvivor::GetSecondaryReserveAmmo() const
{
	if (CurrentWeapon)
	{
		const FGameplayAttribute Attribute = AmmoAttributeSet->GetReserveAmmoAttributeFromTag(
			CurrentWeapon->SecondaryAmmoType);
		
		if (Attribute.IsValid())
		{
			return AbilitySystemComponent->GetNumericAttribute(Attribute);
		}
	}

	return 0;
}

int32 AGSSurvivor::GetNumWeapons() const
{
	return Inventory.Weapons.Num();
}

bool AGSSurvivor::IsAvailableForInteraction_Implementation(UPrimitiveComponent* InteractionComponent) const
{
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag)
		&& !AbilitySystemComponent->HasMatchingGameplayTag(InteractingTag))
	{
		return true;
	}

	return IGSInteractable::IsAvailableForInteraction_Implementation(InteractionComponent);
}

float AGSSurvivor::GetInteractionDuration_Implementation(UPrimitiveComponent* InteractionComponent) const
{
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag))
	{
		return ReviveDuration;
	}

	return IGSInteractable::GetInteractionDuration_Implementation(InteractionComponent);
}

void AGSSurvivor::PreInteract_Implementation(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent)
{
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag) &&
		HasAuthority())
	{
		AbilitySystemComponent->TryActivateAbilitiesByTag(
			FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Ability.Revive")));
	}
}

void AGSSurvivor::PostInteract_Implementation(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent)
{
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag) &&
		HasAuthority())
	{
		AbilitySystemComponent->ApplyGameplayEffectToSelf(Cast<UGameplayEffect>(ReviveEffect->GetDefaultObject()), 1.0f,
		                                                  AbilitySystemComponent->MakeEffectContext());
	}
}

void AGSSurvivor::GetPreInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type,
                                                        UPrimitiveComponent* InteractionComponent) const
{
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag))
	{
		bShouldSync = true;
		Type = EAbilityTaskNetSyncType::OnlyClientWait;
		return;
	}

	IGSInteractable::GetPreInteractSyncType_Implementation(bShouldSync, Type, InteractionComponent);
}

void AGSSurvivor::CancelInteraction_Implementation(UPrimitiveComponent* InteractionComponent)
{
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag) &&
		HasAuthority())
	{
		const FGameplayTagContainer CancelTags(FGameplayTag::RequestGameplayTag("Ability.Revive"));
		AbilitySystemComponent->CancelAbilities(&CancelTags);
	}
}

FSimpleMulticastDelegate* AGSSurvivor::GetTargetCancelInteractionDelegate(
	UPrimitiveComponent* InteractionComponent)
{
	return &InteractionCanceledDelegate;
}

void AGSSurvivor::BeginPlay()
{
	Super::BeginPlay();

	StartingFirstPersonMeshLocation = FirstPersonMesh->GetRelativeLocation();
	
	InitializeFloatingStatusBar();
	
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerSyncCurrentWeapon();
	}
}

void AGSSurvivor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Execute_InteractableCancelInteraction(this, GetThirdPersonMesh());
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(CurrentWeaponTag);
		CurrentWeaponTag = NoWeaponTag;
		AbilitySystemComponent->AddLooseGameplayTag(CurrentWeaponTag);
	}

	Super::EndPlay(EndPlayReason);
}

void AGSSurvivor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	StartingThirdPersonCameraBoomArmLength = ThirdPersonCameraBoom->TargetArmLength;
	StartingThirdPersonCameraBoomLocation = ThirdPersonCameraBoom->GetRelativeLocation();
	StartingThirdPersonMeshLocation = GetMesh()->GetRelativeLocation();

	GetWorldTimerManager().SetTimerForNextTick(this, &AGSSurvivor::SpawnDefaultInventory);
}

void AGSSurvivor::LookUp(float Value)
{
	if (IsAlive())
	{
		AddControllerPitchInput(Value);
	}
}

void AGSSurvivor::LookUpRate(float Value)
{
	if (IsAlive())
	{
		AddControllerPitchInput(Value * BaseLookUpRate * GetWorld()->DeltaTimeSeconds);
	}
}

void AGSSurvivor::Turn(float Value)
{
	if (IsAlive())
	{
		AddControllerYawInput(Value);
	}
}

void AGSSurvivor::TurnRate(float Value)
{
	if (IsAlive())
	{
		AddControllerYawInput(Value * BaseTurnRate * GetWorld()->DeltaTimeSeconds);
	}
}

void AGSSurvivor::MoveForward(float Value)
{
	if (IsAlive())
	{
		AddMovementInput(UKismetMathLibrary::GetForwardVector(FRotator(0, GetControlRotation().Yaw, 0)), Value);
	}
}

void AGSSurvivor::MoveRight(float Value)
{
	if (IsAlive())
	{
		AddMovementInput(UKismetMathLibrary::GetRightVector(FRotator(0, GetControlRotation().Yaw, 0)), Value);
	}
}

void AGSSurvivor::TogglePerspective()
{
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag))
	{
		return;
	}

	bIsFirstPersonPerspective = !bIsFirstPersonPerspective;
	SetPerspective(bIsFirstPersonPerspective);
}

void AGSSurvivor::SetPerspective(bool InIsFirstPersonPerspective)
{
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag) &&
		InIsFirstPersonPerspective)
	{
		return;
	}
	
	AGSPlayerController* PC = GetController<AGSPlayerController>();
	if (PC && PC->IsLocalPlayerController())
	{
		if (InIsFirstPersonPerspective)
		{
			ThirdPersonCamera->Deactivate();
			FirstPersonCamera->Activate();
			PC->SetViewTarget(this);

			GetMesh()->SetVisibility(false, true);
			FirstPersonMesh->SetVisibility(true, true);
			
			GetMesh()->SetRelativeLocation(StartingThirdPersonMeshLocation + FVector(-120.0f, 0.0f, 0.0f));
		}
		else
		{
			FirstPersonCamera->Deactivate();
			ThirdPersonCamera->Activate();
			PC->SetViewTarget(this);

			FirstPersonMesh->SetVisibility(false, true);
			GetMesh()->SetVisibility(true, true);
			
			GetMesh()->SetRelativeLocation(StartingThirdPersonMeshLocation);
		}
	}
}

void AGSSurvivor::InitializeFloatingStatusBar()
{
	if (UIFloatingStatusBar || !IsValid(AbilitySystemComponent))
	{
		return;
	}
	
	if (IsPlayerControlled() && IsLocallyControlled())
	{
		return;
	}
	
	if (!GetPlayerState())
	{
		return;
	}
	
	AGSPlayerController* PC = Cast<AGSPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PC && PC->IsLocalPlayerController())
	{
		if (UIFloatingStatusBarClass)
		{
			UIFloatingStatusBar = CreateWidget<UGSFloatingStatusBarWidget>(PC, UIFloatingStatusBarClass);
			
			if (UIFloatingStatusBar && UIFloatingStatusBarComponent)
			{
				UIFloatingStatusBar->OwningCharacter = this;
				UIFloatingStatusBarComponent->SetWidget(UIFloatingStatusBar);
				
				UIFloatingStatusBar->SetHealthPercentage(GetHealth() / GetMaxHealth());
				UIFloatingStatusBar->SetManaPercentage(GetMana() / GetMaxMana());
				UIFloatingStatusBar->SetShieldPercentage(GetShield() / GetMaxShield());
				UIFloatingStatusBar->SetCharacterName(CharacterName);
			}
		}
	}
}

void AGSSurvivor::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AGSPlayerState* PS = GetPlayerState<AGSPlayerState>();
	
	if (PS)
	{
		AbilitySystemComponent = Cast<UGSAbilitySystemComponent>(PS->GetAbilitySystemComponent());
		
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);
		
		BindASCInput();

		AbilitySystemComponent->AbilityFailedCallbacks.AddUObject(this, &AGSSurvivor::OnAbilityActivationFailed);
		
		AttributeSetBase = PS->GetAttributeSetBase();

		AmmoAttributeSet = PS->GetAmmoAttributeSet();
		
		InitializeAttributes();

		AGSPlayerController* PC = Cast<AGSPlayerController>(GetController());
		if (PC)
		{
			PC->CreateHUD();
		}

		if (CurrentWeapon)
		{
			AbilitySystemComponent->AddLooseGameplayTag(CurrentWeaponTag);
			CurrentWeapon->SetOwningCharacter(this);

			if (!PrimaryReserveAmmoChangedDelegateHandle.IsValid())
			{
				PrimaryReserveAmmoChangedDelegateHandle = AbilitySystemComponent->
				                                          GetGameplayAttributeValueChangeDelegate(
					                                          UGSAmmoAttributeSet::GetReserveAmmoAttributeFromTag(
						                                          CurrentWeapon->PrimaryAmmoType)).AddUObject(
					                                          this,
					                                          &AGSSurvivor::CurrentWeaponPrimaryReserveAmmoChanged);
			}
			if (!SecondaryReserveAmmoChangedDelegateHandle.IsValid())
			{
				SecondaryReserveAmmoChangedDelegateHandle = AbilitySystemComponent->
				                                            GetGameplayAttributeValueChangeDelegate(
					                                            UGSAmmoAttributeSet::GetReserveAmmoAttributeFromTag(
						                                            CurrentWeapon->SecondaryAmmoType)).AddUObject(
					                                            this,
					                                            &AGSSurvivor::CurrentWeaponSecondaryReserveAmmoChanged);
			}
		}

		if (AbilitySystemComponent->GetTagCount(DeadTag) > 0)
		{
			SetHealth(GetMaxHealth());
			SetMana(GetMaxMana());
			SetStamina(GetMaxStamina());
			SetShield(GetMaxShield());
		}
		
		InitializeFloatingStatusBar();
	}
}

void AGSSurvivor::OnRep_Controller()
{
	Super::OnRep_Controller();

	SetupStartupPerspective();
}

void AGSSurvivor::BindASCInput()
{
	if (!bASCInputBound && IsValid(AbilitySystemComponent) && IsValid(InputComponent))
	{
		const FTopLevelAssetPath AbilityEnumAssetPath = FTopLevelAssetPath(FName("/Script/GASSurvival"),
		                                                                   FName("EGSAbilityInputID"));
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds(
			                                                              FString("ConfirmTarget"),
			                                                              FString("CancelTarget"), AbilityEnumAssetPath,
			                                                              static_cast<int32>(
				                                                              EGSAbilityInputID::Confirm),
			                                                              static_cast<int32>(
				                                                              EGSAbilityInputID::Cancel)));

		bASCInputBound = true;
	}
}

void AGSSurvivor::SpawnDefaultInventory()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		return;
	}

	int32 NumWeaponClasses = DefaultInventoryWeaponClasses.Num();
	
	for (int32 i = 0; i < NumWeaponClasses; i++)
	{
		if (!DefaultInventoryWeaponClasses[i])
		{
			continue;
		}

		AGSWeapon* NewWeapon = GetWorld()->SpawnActorDeferred<AGSWeapon>(DefaultInventoryWeaponClasses[i],
		                                                                 FTransform::Identity, this, this,
		                                                                 ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		NewWeapon->bSpawnWithCollision = false;
		NewWeapon->FinishSpawning(FTransform::Identity);

		const bool bEquipFirstWeapon = i == 0;
		AddWeaponToInventory(NewWeapon, bEquipFirstWeapon);
	}
}

void AGSSurvivor::SetupStartupPerspective()
{
	const APlayerController* PC = Cast<APlayerController>(GetController());

	if (PC && PC->IsLocalController())
	{
		bIsFirstPersonPerspective = bStartInFirstPersonPerspective;
		SetPerspective(bIsFirstPersonPerspective);
	}
}

bool AGSSurvivor::DoesWeaponExistInInventory(const AGSWeapon* InWeapon)
{
	for (const AGSWeapon* Weapon : Inventory.Weapons)
	{
		if (Weapon && InWeapon && Weapon->GetClass() == InWeapon->GetClass())
		{
			return true;
		}
	}

	return false;
}

void AGSSurvivor::SetCurrentWeapon(AGSWeapon* NewWeapon, AGSWeapon* LastWeapon)
{
	if (NewWeapon == LastWeapon)
	{
		return;
	}
	
	if (AbilitySystemComponent)
	{
		const FGameplayTagContainer AbilityTagsToCancel = FGameplayTagContainer(WeaponAbilityTag);
		AbilitySystemComponent->CancelAbilities(&AbilityTagsToCancel);
	}

	UnEquipWeapon(LastWeapon);

	if (NewWeapon)
	{
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(CurrentWeaponTag);
		}
		
		CurrentWeapon = NewWeapon;
		CurrentWeapon->SetOwningCharacter(this);
		CurrentWeapon->Equip();
		CurrentWeaponTag = CurrentWeapon->WeaponTag;

		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->AddLooseGameplayTag(CurrentWeaponTag);
		}

		AGSPlayerController* PC = GetController<AGSPlayerController>();
		
		if (PC && PC->IsLocalController())
		{
			PC->SetEquippedWeaponPrimaryIconFromSprite(CurrentWeapon->PrimaryIcon);
			PC->SetEquippedWeaponStatusText(CurrentWeapon->StatusText);
			PC->SetPrimaryClipAmmo(CurrentWeapon->GetPrimaryClipAmmo());
			PC->SetPrimaryReserveAmmo(GetPrimaryReserveAmmo());
			PC->SetHUDReticle(CurrentWeapon->GetPrimaryHUDReticleClass());
		}

		NewWeapon->OnPrimaryClipAmmoChanged.AddDynamic(this, &AGSSurvivor::CurrentWeaponPrimaryClipAmmoChanged);
		NewWeapon->OnSecondaryClipAmmoChanged.AddDynamic(this, &AGSSurvivor::CurrentWeaponSecondaryClipAmmoChanged);

		if (AbilitySystemComponent)
		{
			PrimaryReserveAmmoChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				UGSAmmoAttributeSet::GetReserveAmmoAttributeFromTag(CurrentWeapon->PrimaryAmmoType)).AddUObject(
				this, &AGSSurvivor::CurrentWeaponPrimaryReserveAmmoChanged);
			SecondaryReserveAmmoChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				UGSAmmoAttributeSet::GetReserveAmmoAttributeFromTag(CurrentWeapon->SecondaryAmmoType)).AddUObject(
				this, &AGSSurvivor::CurrentWeaponSecondaryReserveAmmoChanged);
		}

		UAnimMontage* Equip1PMontage = CurrentWeapon->GetEquip1PMontage();
		if (Equip1PMontage && GetFirstPersonMesh() && GetFirstPersonMesh()->GetAnimInstance())
		{
			GetFirstPersonMesh()->GetAnimInstance()->Montage_Play(Equip1PMontage);
		}

		UAnimMontage* Equip3PMontage = CurrentWeapon->GetEquip3PMontage();
		if (Equip3PMontage && GetThirdPersonMesh() && GetThirdPersonMesh()->GetAnimInstance())
		{
			GetThirdPersonMesh()->GetAnimInstance()->Montage_Play(Equip3PMontage);
		}
	}
	else
	{
		UnEquipCurrentWeapon();
	}
}

void AGSSurvivor::UnEquipWeapon(AGSWeapon* WeaponToUnEquip)
{
	if (WeaponToUnEquip)
	{
		WeaponToUnEquip->OnPrimaryClipAmmoChanged.
		                 RemoveDynamic(this, &AGSSurvivor::CurrentWeaponPrimaryClipAmmoChanged);
		WeaponToUnEquip->OnSecondaryClipAmmoChanged.RemoveDynamic(
			this, &AGSSurvivor::CurrentWeaponSecondaryClipAmmoChanged);

		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				UGSAmmoAttributeSet::GetReserveAmmoAttributeFromTag(WeaponToUnEquip->PrimaryAmmoType)).Remove(
				PrimaryReserveAmmoChangedDelegateHandle);
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				UGSAmmoAttributeSet::GetReserveAmmoAttributeFromTag(WeaponToUnEquip->SecondaryAmmoType)).Remove(
				SecondaryReserveAmmoChangedDelegateHandle);
		}

		WeaponToUnEquip->UnEquip();
	}
}

void AGSSurvivor::UnEquipCurrentWeapon()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(CurrentWeaponTag);
		CurrentWeaponTag = NoWeaponTag;
		AbilitySystemComponent->AddLooseGameplayTag(CurrentWeaponTag);
	}

	UnEquipWeapon(CurrentWeapon);
	CurrentWeapon = nullptr;

	AGSPlayerController* PC = GetController<AGSPlayerController>();
	
	if (PC && PC->IsLocalController())
	{
		PC->SetEquippedWeaponPrimaryIconFromSprite(nullptr);
		PC->SetEquippedWeaponStatusText(FText());
		PC->SetPrimaryClipAmmo(0);
		PC->SetPrimaryReserveAmmo(0);
		PC->SetHUDReticle(nullptr);
	}
}

void AGSSurvivor::CurrentWeaponPrimaryClipAmmoChanged(int32 OldPrimaryClipAmmo, int32 NewPrimaryClipAmmo)
{
	AGSPlayerController* PC = GetController<AGSPlayerController>();
	
	if (PC && PC->IsLocalController())
	{
		PC->SetPrimaryClipAmmo(NewPrimaryClipAmmo);
	}
}

void AGSSurvivor::CurrentWeaponSecondaryClipAmmoChanged(int32 OldSecondaryClipAmmo, int32 NewSecondaryClipAmmo)
{
	AGSPlayerController* PC = GetController<AGSPlayerController>();
	
	if (PC && PC->IsLocalController())
	{
		PC->SetSecondaryClipAmmo(NewSecondaryClipAmmo);
	}
}

void AGSSurvivor::CurrentWeaponPrimaryReserveAmmoChanged(const FOnAttributeChangeData& Data)
{
	AGSPlayerController* PC = GetController<AGSPlayerController>();
	
	if (PC && PC->IsLocalController())
	{
		PC->SetPrimaryReserveAmmo(Data.NewValue);
	}
}

void AGSSurvivor::CurrentWeaponSecondaryReserveAmmoChanged(const FOnAttributeChangeData& Data)
{
	AGSPlayerController* PC = GetController<AGSPlayerController>();
	
	if (PC && PC->IsLocalController())
	{
		PC->SetSecondaryReserveAmmo(Data.NewValue);
	}
}

void AGSSurvivor::WeaponChangingDelayReplicationTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (CallbackTag == WeaponChangingDelayReplicationTag)
	{
		if (NewCount < 1)
		{
			ClientSyncCurrentWeapon(CurrentWeapon);
		}
	}
}

void AGSSurvivor::OnRep_CurrentWeapon(AGSWeapon* LastWeapon)
{
	bChangedWeaponLocally = false;
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
}

void AGSSurvivor::OnRep_Inventory()
{
	if (GetLocalRole() == ROLE_AutonomousProxy && Inventory.Weapons.Num() > 0 && !CurrentWeapon)
	{
		ServerSyncCurrentWeapon();
	}
}

void AGSSurvivor::OnAbilityActivationFailed(const UGameplayAbility* FailedAbility,
                                            const FGameplayTagContainer& FailTags)
{
	if (FailedAbility && FailedAbility->AbilityTags.HasTagExact(
		FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.IsChanging"))))
	{
		if (bChangedWeaponLocally)
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("%s Weapon Changing ability activation failed. Syncing CurrentWeapon. %s. %s"),
			       *FString(__FUNCTION__),
			       *UGSBlueprintFunctionLibrary::GetPlayerEditorWindowRole(GetWorld()), *FailTags.ToString());

			ServerSyncCurrentWeapon();
		}
	}
}

void AGSSurvivor::ServerSyncCurrentWeapon_Implementation()
{
	ClientSyncCurrentWeapon(CurrentWeapon);
}

bool AGSSurvivor::ServerSyncCurrentWeapon_Validate()
{
	return true;
}

void AGSSurvivor::ClientSyncCurrentWeapon_Implementation(AGSWeapon* InWeapon)
{
	AGSWeapon* LastWeapon = CurrentWeapon;
	CurrentWeapon = InWeapon;
	OnRep_CurrentWeapon(LastWeapon);
}

bool AGSSurvivor::ClientSyncCurrentWeapon_Validate(AGSWeapon* InWeapon)
{
	return true;
}
