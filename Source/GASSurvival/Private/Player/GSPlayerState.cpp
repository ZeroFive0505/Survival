#include "Player/GSPlayerState.h"
#include "Characters/Abilities/GSAbilitySystemComponent.h"
#include "Characters/Abilities/AttributeSets/GSAmmoAttributeSet.h"
#include "Characters/Abilities/AttributeSets/GSAttributeSetBase.h"
#include "Characters/Survivors/GSSurvivor.h"
#include "Player/GSPlayerController.h"
#include "UI/GSHUDWidget.h"
#include "Weapons/GSWeapon.h"

AGSPlayerState::AGSPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UGSAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);


	AttributeSetBase = CreateDefaultSubobject<UGSAttributeSetBase>(TEXT("AttributeSetBase"));

	AmmoAttributeSet = CreateDefaultSubobject<UGSAmmoAttributeSet>(TEXT("AmmoAttributeSet"));

	NetUpdateFrequency = 100.0f;

	DeadTag = FGameplayTag::RequestGameplayTag("State.Dead");
	KnockedDownTag = FGameplayTag::RequestGameplayTag("State.KnockedDown");
}

UAbilitySystemComponent* AGSPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UGSAttributeSetBase* AGSPlayerState::GetAttributeSetBase() const
{
	return AttributeSetBase;
}

UGSAmmoAttributeSet* AGSPlayerState::GetAmmoAttributeSet() const
{
	return AmmoAttributeSet;
}

bool AGSPlayerState::IsAlive() const
{
	return GetHealth() > 0.0f;
}

void AGSPlayerState::ShowAbilityConfirmPrompt(bool bShowPrompt)
{
	AGSPlayerController* PC = Cast<AGSPlayerController>(GetOwner());

	if (PC)
	{
		UGSHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->ShowAbilityConfirmPrompt(bShowPrompt);
		}
	}
}

void AGSPlayerState::ShowInteractionPrompt(float InteractionDuration)
{
	AGSPlayerController* PC = Cast<AGSPlayerController>(GetOwner());

	if (PC)
	{
		UGSHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->ShowInteractionPrompt(InteractionDuration);
		}
	}
}

void AGSPlayerState::SetInteractionDescription(FText Description)
{
	AGSPlayerController* PC = Cast<AGSPlayerController>(GetOwner());

	if (PC)
	{
		UGSHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->SetInteractionDescription(Description);
		}
	}
}

void AGSPlayerState::HideInteractionPrompt()
{
	AGSPlayerController* PC = Cast<AGSPlayerController>(GetOwner());

	if (PC)
	{
		UGSHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->HideInteractionPrompt();
		}
	}
}

void AGSPlayerState::StartInteractionTimer(float InteractionDuration)
{
	AGSPlayerController* PC = Cast<AGSPlayerController>(GetOwner());

	if (PC)
	{
		UGSHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->StartInteractionTimer(InteractionDuration);
		}
	}
}

void AGSPlayerState::StopInteractionTimer()
{
	AGSPlayerController* PC = Cast<AGSPlayerController>(GetOwner());

	if (PC)
	{
		UGSHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->StopInteractionTimer();
		}
	}
}

float AGSPlayerState::GetHealth() const
{
	return AttributeSetBase->GetHealth();
}

float AGSPlayerState::GetMaxHealth() const
{
	return AttributeSetBase->GetMaxHealth();
}

float AGSPlayerState::GetHealthRegenRate() const
{
	return AttributeSetBase->GetHealthRegenRate();
}

float AGSPlayerState::GetMana() const
{
	return AttributeSetBase->GetMana();
}

float AGSPlayerState::GetMaxMana() const
{
	return AttributeSetBase->GetMaxMana();
}

float AGSPlayerState::GetManaRegenRate() const
{
	return AttributeSetBase->GetManaRegenRate();
}

float AGSPlayerState::GetStamina() const
{
	return AttributeSetBase->GetStamina();
}

float AGSPlayerState::GetMaxStamina() const
{
	return AttributeSetBase->GetMaxStamina();
}

float AGSPlayerState::GetStaminaRegenRate() const
{
	return AttributeSetBase->GetStaminaRegenRate();
}

float AGSPlayerState::GetShield() const
{
	return AttributeSetBase->GetShield();
}

float AGSPlayerState::GetMaxShield() const
{
	return AttributeSetBase->GetMaxShield();
}

float AGSPlayerState::GetShieldRegenRate() const
{
	return AttributeSetBase->GetShieldRegenRate();
}

float AGSPlayerState::GetArmor() const
{
	return AttributeSetBase->GetArmor();
}

float AGSPlayerState::GetMoveSpeed() const
{
	return AttributeSetBase->GetMoveSpeed();
}

int32 AGSPlayerState::GetCharacterLevel() const
{
	return AttributeSetBase->GetCharacterLevel();
}

int32 AGSPlayerState::GetXP() const
{
	return AttributeSetBase->GetXP();
}

int32 AGSPlayerState::GetXPBounty() const
{
	return AttributeSetBase->GetXPBounty();
}

int32 AGSPlayerState::GetGold() const
{
	return AttributeSetBase->GetGold();
}

int32 AGSPlayerState::GetGoldBounty() const
{
	return AttributeSetBase->GetGoldBounty();
}

int32 AGSPlayerState::GetPrimaryClipAmmo() const
{
	const AGSSurvivor* Survivor = GetPawn<AGSSurvivor>();

	if (Survivor)
	{
		return Survivor->GetPrimaryClipAmmo();
	}

	return 0;
}

int32 AGSPlayerState::GetPrimaryReserveAmmo() const
{
	const AGSSurvivor* Survivor = GetPawn<AGSSurvivor>();
	if (Survivor && Survivor->GetCurrentWeapon() && AmmoAttributeSet)
	{
		const FGameplayAttribute Attribute = AmmoAttributeSet->GetReserveAmmoAttributeFromTag(
			Survivor->GetCurrentWeapon()->PrimaryAmmoType);

		if (Attribute.IsValid())
		{
			return AbilitySystemComponent->GetNumericAttribute(Attribute);
		}
	}

	return 0;
}

void AGSPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		HealthChangedDelegateHandle = AbilitySystemComponent->
		                              GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetHealthAttribute()).
		                              AddUObject(this, &AGSPlayerState::HealthChanged);
		
		AbilitySystemComponent->RegisterGameplayTagEvent(KnockedDownTag, EGameplayTagEventType::NewOrRemoved).
		                        AddUObject(this, &AGSPlayerState::KnockDownTagChanged);
	}
}

void AGSPlayerState::HealthChanged(const FOnAttributeChangeData& Data)
{
	AGSSurvivor* Survivor = Cast<AGSSurvivor>(GetPawn());
	if (IsValid(Survivor) && !IsAlive() && !AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	{
		if (Survivor)
		{
			// if (!AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag))
			// {
			// 	Survivor->KnockDown();
			// }
			// else
			// {
			// 	Survivor->FinishDying();
			// }

			Survivor->FinishDying();
		}
	}
}

void AGSPlayerState::KnockDownTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	AGSSurvivor* Survivor = Cast<AGSSurvivor>(GetPawn());
	if (!IsValid(Survivor))
	{
		return;
	}

	if (NewCount > 0)
	{
		Survivor->PlayKnockDownEffects();
	}
	else if (NewCount < 1 && !AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	{
		Survivor->PlayReviveEffects();
	}
}
