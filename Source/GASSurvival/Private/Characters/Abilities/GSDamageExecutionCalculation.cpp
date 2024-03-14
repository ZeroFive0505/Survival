#include "Characters/Abilities/GSDamageExecutionCalculation.h"

#include "Characters/GSCharacter.h"
#include "Characters/Abilities/AttributeSets/GSAttributeSetBase.h"
#include "Characters/Survivors/GSSurvivor.h"

struct GSDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);

	GSDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGSAttributeSetBase, Damage, Source, true);
		
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGSAttributeSetBase, Armor, Target, false);
	}
};

static const GSDamageStatics& DamageStatics()
{
	static GSDamageStatics DStatics;
	return DStatics;
}

UGSDamageExecutionCalculation::UGSDamageExecutionCalculation()
{
	HeadShotMultiplier = 1.5f;

	RelevantAttributesToCapture.Add(DamageStatics().DamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
}

void UGSDamageExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();
	const UAbilitySystemComponent* SourceAbilitySystemComponent = ExecutionParams.GetSourceAbilitySystemComponent();

	AActor* SourceActor = SourceAbilitySystemComponent ? SourceAbilitySystemComponent->GetAvatarActor() : nullptr;
	AActor* TargetActor = TargetAbilitySystemComponent ? TargetAbilitySystemComponent->GetAvatarActor() : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayTagContainer AssetTags;
	Spec.GetAllAssetTags(AssetTags);
	
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float Armor = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, Armor);
	Armor = FMath::Max<float>(Armor, 0.0f);

	float Damage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DamageDef, EvaluationParameters, Damage);
	Damage += FMath::Max<float>(Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), false, -1.0f), 0.0f);

	float UnmitigatedDamage = Damage;

	const float ForceAmount = FMath::Max<float>(Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.ForceAmount")), false, 0.0f), 0.0f);
	
	const FHitResult* Hit = Spec.GetContext().GetHitResult();
	
	if(Hit)
	{
		AGSCharacter* Character = Cast<AGSCharacter>(Hit->GetActor());
		AGSSurvivor* Survivor = Cast<AGSSurvivor>(Hit->GetActor());
		
		if(Character && !Character->IsAlive())
		{
			if(Hit->BoneName != NAME_None)
			{
				const FVector Direction = Character->GetMesh()->GetBoneLocation(Hit->BoneName) - Hit->TraceStart;
				Character->ApplyForceAtBone(Hit->BoneName, ForceAmount, Direction.GetSafeNormal());
			}
		}

		if(Survivor)
		{
			Survivor->HitBy(SourceActor);
		}
		
		if (AssetTags.HasTagExact(FGameplayTag::RequestGameplayTag(FName("Effect.Damage.CanHeadShot"))) && Character && Character->GetHeadBoneName() == Hit->BoneName)
		{
			UnmitigatedDamage *= HeadShotMultiplier;
			FGameplayEffectSpec* MutableSpec = ExecutionParams.GetOwningSpecForPreExecuteMod();
			MutableSpec->AddDynamicAssetTag(FGameplayTag::RequestGameplayTag(FName("Effect.Damage.HeadShot")));
		}
	}

	const float MitigatedDamage = (UnmitigatedDamage) * (100 / (100 + Armor));

	if (MitigatedDamage > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().DamageProperty, EGameplayModOp::Additive, MitigatedDamage));
	}
}
