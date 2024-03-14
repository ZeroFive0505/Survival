// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Abilities/AttributeSets/GSAmmoAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UGSAmmoAttributeSet::UGSAmmoAttributeSet()
{
	RifleAmmoTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Rifle"));
	RocketAmmoTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Rocket"));
	ShotgunAmmoTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Shotgun"));
	SniperAmmoTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Sniper"));
	InfinitePistolAmmoTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.InfinitePistol"));
	GrenadeAmmoTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Grenade"));
}

void UGSAmmoAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void UGSAmmoAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetRifleReserveAmmoAttribute())
	{
		const float Ammo = GetRifleReserveAmmo();
		SetRifleReserveAmmo(FMath::Clamp<float>(Ammo, 0, GetMaxRifleReserveAmmo()));
	}
	else if (Data.EvaluatedData.Attribute == GetRocketReserveAmmoAttribute())
	{
		const float Ammo = GetRocketReserveAmmo();
		SetRocketReserveAmmo(FMath::Clamp<float>(Ammo, 0, GetMaxRocketReserveAmmo()));
	}
	else if (Data.EvaluatedData.Attribute == GetShotgunReserveAmmoAttribute())
	{
		const float Ammo = GetShotgunReserveAmmo();
		SetShotgunReserveAmmo(FMath::Clamp<float>(Ammo, 0, GetMaxShotgunReserveAmmo()));
	}
	else if(Data.EvaluatedData.Attribute == GetSniperReserveAmmoAttribute())
	{
		const float Ammo = GetSniperReserveAmmo();
		SetSniperReserveAmmo(FMath::Clamp<float>(Ammo, 0, GetMaxSniperReserveAmmo()));
	}
	else if(Data.EvaluatedData.Attribute == GetInfinitePistolReserveAmmoAttribute())
	{
		const float Ammo = GetInfinitePistolReserveAmmo();
		SetInfinitePistolReserveAmmo(FMath::Clamp<float>(Ammo, 0, GetMaxInfinitePistolReserveAmmo()));
	}
	else if(Data.EvaluatedData.Attribute == GetGrenadeReserveAmmoAttribute())
	{
		const float Ammo = GetGrenadeReserveAmmo();
		SetGrenadeReserveAmmo(FMath::Clamp<float>(Ammo, 0, GetMaxGrenadeReserveAmmo()));
	}
}

void UGSAmmoAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAmmoAttributeSet, RifleReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAmmoAttributeSet, MaxRifleReserveAmmo, COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAmmoAttributeSet, RocketReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAmmoAttributeSet, MaxRocketReserveAmmo, COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAmmoAttributeSet, ShotgunReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAmmoAttributeSet, MaxShotgunReserveAmmo, COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAmmoAttributeSet, SniperReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAmmoAttributeSet, MaxSniperReserveAmmo, COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAmmoAttributeSet, InfinitePistolReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAmmoAttributeSet, MaxInfinitePistolReserveAmmo, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UGSAmmoAttributeSet, GrenadeReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSAmmoAttributeSet, MaxGrenadeReserveAmmo, COND_None, REPNOTIFY_Always);
}

FGameplayAttribute UGSAmmoAttributeSet::GetReserveAmmoAttributeFromTag(const FGameplayTag& PrimaryAmmoTag)
{
	if (PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Rifle")))
	{
		return GetRifleReserveAmmoAttribute();
	}
	
	if (PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Rocket")))
	{
		return GetRocketReserveAmmoAttribute();
	}
	
	if (PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Shotgun")))
	{
		return GetShotgunReserveAmmoAttribute();
	}

	if(PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Sniper")))
	{
		return GetSniperReserveAmmoAttribute();
	}

	if(PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.InfinitePistol")))
	{
		return GetInfinitePistolReserveAmmoAttribute();
	}

	if(PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Grenade")))
	{
		return GetGrenadeReserveAmmoAttribute();
	}

	return FGameplayAttribute();
}

FGameplayAttribute UGSAmmoAttributeSet::GetMaxReserveAmmoAttributeFromTag(const FGameplayTag& PrimaryAmmoTag)
{
	if (PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Rifle")))
	{
		return GetMaxRifleReserveAmmoAttribute();
	}

	if (PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Rocket")))
	{
		return GetMaxRocketReserveAmmoAttribute();
	}

	if (PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Shotgun")))
	{
		return GetMaxShotgunReserveAmmoAttribute();
	}

	if(PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Sniper")))
	{
		return GetMaxSniperReserveAmmoAttribute();
	}

	if(PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.InfinitePistol")))
	{
		return GetMaxInfinitePistolReserveAmmoAttribute();
	}

	if(PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Grenade")))
	{
		return GetMaxGrenadeReserveAmmoAttribute();
	}

	return FGameplayAttribute();
}

void UGSAmmoAttributeSet::AdjustAttributeForMaxChange(const FGameplayAttributeData& AffectedAttribute,
                                                      const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty) const
{
	UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilityComp)
	{
		const float CurrentValue = AffectedAttribute.GetCurrentValue();
		const float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

		AbilityComp->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}

void UGSAmmoAttributeSet::OnRep_RifleReserveAmmo(const FGameplayAttributeData& OldRifleReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAmmoAttributeSet, RifleReserveAmmo, OldRifleReserveAmmo);
}

void UGSAmmoAttributeSet::OnRep_MaxRifleReserveAmmo(const FGameplayAttributeData& OldMaxRifleReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAmmoAttributeSet, MaxRifleReserveAmmo, OldMaxRifleReserveAmmo);
}

void UGSAmmoAttributeSet::OnRep_InfinitePistolReserveAmmo(const FGameplayAttributeData& OldInfinitePistolReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAmmoAttributeSet, InfinitePistolReserveAmmo, OldInfinitePistolReserveAmmo);
}

void UGSAmmoAttributeSet::OnRep_MaxInfinitePistolReserveAmmo(const FGameplayAttributeData& OldMaxInfinitePistolReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAmmoAttributeSet, MaxInfinitePistolReserveAmmo, OldMaxInfinitePistolReserveAmmo);
}

void UGSAmmoAttributeSet::OnRep_RocketReserveAmmo(const FGameplayAttributeData& OldRocketReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAmmoAttributeSet, RocketReserveAmmo, OldRocketReserveAmmo);
}

void UGSAmmoAttributeSet::OnRep_MaxRocketReserveAmmo(const FGameplayAttributeData& OldMaxRocketReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAmmoAttributeSet, MaxRocketReserveAmmo, OldMaxRocketReserveAmmo);
}

void UGSAmmoAttributeSet::OnRep_ShotgunReserveAmmo(const FGameplayAttributeData& OldShotgunReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAmmoAttributeSet, ShotgunReserveAmmo, OldShotgunReserveAmmo);
}

void UGSAmmoAttributeSet::OnRep_MaxShotgunReserveAmmo(const FGameplayAttributeData& OldMaxShotgunReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAmmoAttributeSet, MaxShotgunReserveAmmo, OldMaxShotgunReserveAmmo);
}

void UGSAmmoAttributeSet::OnRep_SniperReserveAmmo(const FGameplayAttributeData& OldSniperReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAmmoAttributeSet, SniperReserveAmmo, OldSniperReserveAmmo);
}

void UGSAmmoAttributeSet::OnRep_MaxSniperReserveAmmo(const FGameplayAttributeData& OldMaxSniperReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAmmoAttributeSet, MaxSniperReserveAmmo, OldMaxSniperReserveAmmo);
}

void UGSAmmoAttributeSet::OnRep_GrenadeReserveAmmo(const FGameplayAttributeData& OldGrenadeReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAmmoAttributeSet, GrenadeReserveAmmo, OldGrenadeReserveAmmo);
}

void UGSAmmoAttributeSet::OnRep_MaxGrenadeReserveAmmo(const FGameplayAttributeData& OldMaxGrenadeReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGSAmmoAttributeSet, MaxGrenadeReserveAmmo, OldMaxGrenadeReserveAmmo);
}

