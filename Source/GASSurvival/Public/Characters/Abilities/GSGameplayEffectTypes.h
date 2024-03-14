#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "UObject/Object.h"
#include "GSGameplayEffectTypes.generated.h"


USTRUCT()
struct GASSURVIVAL_API FGSGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()
	
	virtual FGameplayAbilityTargetDataHandle GetTargetData()
	{
		return TargetData;
	}

	virtual void AddTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
	{
		TargetData.Append(TargetDataHandle);
	}
	
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGSGameplayEffectContext::StaticStruct();
	}

	virtual FGSGameplayEffectContext* Duplicate() const override
	{
		FGSGameplayEffectContext* NewContext = new FGSGameplayEffectContext();
		*NewContext = *this;
		NewContext->AddActors(Actors);
		if (GetHitResult())
		{
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		
		NewContext->TargetData.Append(TargetData);
		return NewContext;
	}

	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;

protected:
	FGameplayAbilityTargetDataHandle TargetData;
};

template<>
struct TStructOpsTypeTraits<FGSGameplayEffectContext> : TStructOpsTypeTraitsBase2<FGSGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

