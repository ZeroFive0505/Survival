// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "GSAmmoAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


UCLASS()
class GASSURVIVAL_API UGSAmmoAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UGSAmmoAttributeSet();

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_RifleReserveAmmo)
	FGameplayAttributeData RifleReserveAmmo;
	ATTRIBUTE_ACCESSORS(UGSAmmoAttributeSet, RifleReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_MaxRifleReserveAmmo)
	FGameplayAttributeData MaxRifleReserveAmmo;
	ATTRIBUTE_ACCESSORS(UGSAmmoAttributeSet, MaxRifleReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_RocketReserveAmmo)
	FGameplayAttributeData RocketReserveAmmo;
	ATTRIBUTE_ACCESSORS(UGSAmmoAttributeSet, RocketReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_MaxRocketReserveAmmo)
	FGameplayAttributeData MaxRocketReserveAmmo;
	ATTRIBUTE_ACCESSORS(UGSAmmoAttributeSet, MaxRocketReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_ShotgunReserveAmmo)
	FGameplayAttributeData ShotgunReserveAmmo;
	ATTRIBUTE_ACCESSORS(UGSAmmoAttributeSet, ShotgunReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_MaxShotgunReserveAmmo)
	FGameplayAttributeData MaxShotgunReserveAmmo;
	ATTRIBUTE_ACCESSORS(UGSAmmoAttributeSet, MaxShotgunReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_SniperReserveAmmo)
	FGameplayAttributeData SniperReserveAmmo;
	ATTRIBUTE_ACCESSORS(UGSAmmoAttributeSet, SniperReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_MaxSniperReserveAmmo)
	FGameplayAttributeData MaxSniperReserveAmmo;
	ATTRIBUTE_ACCESSORS(UGSAmmoAttributeSet, MaxSniperReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_InfinitePistolReserveAmmo)
	FGameplayAttributeData InfinitePistolReserveAmmo;
	ATTRIBUTE_ACCESSORS(UGSAmmoAttributeSet, InfinitePistolReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_MaxInfinitePistolReserveAmmo)
	FGameplayAttributeData MaxInfinitePistolReserveAmmo;
	ATTRIBUTE_ACCESSORS(UGSAmmoAttributeSet, MaxInfinitePistolReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_GrenadeReserveAmmo)
	FGameplayAttributeData GrenadeReserveAmmo;
	ATTRIBUTE_ACCESSORS(UGSAmmoAttributeSet, GrenadeReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_MaxGrenadeReserveAmmo)
	FGameplayAttributeData MaxGrenadeReserveAmmo;
	ATTRIBUTE_ACCESSORS(UGSAmmoAttributeSet, MaxGrenadeReserveAmmo)

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	static FGameplayAttribute GetReserveAmmoAttributeFromTag(const FGameplayTag& PrimaryAmmoTag);
	static FGameplayAttribute GetMaxReserveAmmoAttributeFromTag(const FGameplayTag& PrimaryAmmoTag);

protected:
	FGameplayTag InfinitePistolAmmoTag;
	FGameplayTag RifleAmmoTag;
	FGameplayTag RocketAmmoTag;
	FGameplayTag ShotgunAmmoTag;
	FGameplayTag SniperAmmoTag;
	FGameplayTag GrenadeAmmoTag;
	
	void AdjustAttributeForMaxChange(const FGameplayAttributeData& AffectedAttribute,
	                                 const FGameplayAttributeData& MaxAttribute, float NewMaxValue,
	                                 const FGameplayAttribute& AffectedAttributeProperty) const;
	
	UFUNCTION()
	virtual void OnRep_RifleReserveAmmo(const FGameplayAttributeData& OldRifleReserveAmmo);

	UFUNCTION()
	virtual void OnRep_MaxRifleReserveAmmo(const FGameplayAttributeData& OldMaxRifleReserveAmmo);

	UFUNCTION()
	virtual void OnRep_InfinitePistolReserveAmmo(const FGameplayAttributeData& OldInfinitePistolReserveAmmo);

	UFUNCTION()
	virtual void OnRep_MaxInfinitePistolReserveAmmo(const FGameplayAttributeData& OldMaxInfinitePistolReserveAmmo);

	UFUNCTION()
	virtual void OnRep_RocketReserveAmmo(const FGameplayAttributeData& OldRocketReserveAmmo);

	UFUNCTION()
	virtual void OnRep_MaxRocketReserveAmmo(const FGameplayAttributeData& OldMaxRocketReserveAmmo);

	UFUNCTION()
	virtual void OnRep_ShotgunReserveAmmo(const FGameplayAttributeData& OldShotgunReserveAmmo);

	UFUNCTION()
	virtual void OnRep_MaxShotgunReserveAmmo(const FGameplayAttributeData& OldMaxShotgunReserveAmmo);

	UFUNCTION()
	virtual void OnRep_SniperReserveAmmo(const FGameplayAttributeData& OldSniperReserveAmmo);

	UFUNCTION()
	virtual void OnRep_MaxSniperReserveAmmo(const FGameplayAttributeData& OldMaxSniperReserveAmmo);

	UFUNCTION()
	virtual void OnRep_GrenadeReserveAmmo(const FGameplayAttributeData& OldGrenadeReserveAmmo);

	UFUNCTION()
	virtual void OnRep_MaxGrenadeReserveAmmo(const FGameplayAttributeData& OldMaxGrenadeReserveAmmo);
};
