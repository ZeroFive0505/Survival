#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GASSurvival/GASSurvival.h"
#include "Characters/Abilities/GSAbilityTypes.h"
#include "GSGameplayAbility.generated.h"

class USkeletalMeshComponent;

USTRUCT()
struct GASSURVIVAL_API FAbilityMeshMontage
{
	GENERATED_BODY()

public:
	UPROPERTY()
	USkeletalMeshComponent* Mesh;

	UPROPERTY()
	UAnimMontage* Montage;

	FAbilityMeshMontage() : Mesh(nullptr), Montage(nullptr)
	{
	}

	FAbilityMeshMontage(class USkeletalMeshComponent* InMesh, class UAnimMontage* InMontage) 
		: Mesh(InMesh), Montage(InMontage)
	{
	}
};

UCLASS()
class GASSURVIVAL_API UGSGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGSGameplayAbility();
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	EGSAbilityInputID AbilityInputID = EGSAbilityInputID::None;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	EGSAbilityInputID AbilityID = EGSAbilityInputID::None;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	bool bActivateAbilityOnGranted;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	bool bActivateOnInput;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	bool bSourceObjectMustEqualCurrentWeaponToActivate;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	bool bCannotActivateWhileInteracting;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEffects")
	TMap<FGameplayTag, FGSGameplayEffectContainer> EffectContainerMap;
	
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	UFUNCTION(BlueprintCallable, Category = "Ability")
	FGameplayAbilityTargetDataHandle MakeGameplayAbilityTargetDataHandleFromActorArray(const TArray<AActor*> TargetActors);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	FGameplayAbilityTargetDataHandle MakeGameplayAbilityTargetDataHandleFromHitResults(const TArray<FHitResult> HitResults);
	
	UFUNCTION(BlueprintCallable, Category = Ability, meta = (AutoCreateRefTerm = "EventData"))
	virtual FGSGameplayEffectContainerSpec MakeEffectContainerSpecFromContainer(const FGSGameplayEffectContainer& Container, const FGameplayEventData& EventData, int32 OverrideGameplayLevel = -1);
	
	UFUNCTION(BlueprintCallable, Category = Ability, meta = (AutoCreateRefTerm = "EventData"))
	virtual FGSGameplayEffectContainerSpec MakeEffectContainerSpec(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel = -1);
	
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual TArray<FActiveGameplayEffectHandle> ApplyEffectContainerSpec(const FGSGameplayEffectContainerSpec& ContainerSpec);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability", meta = (DisplayName = "Get Source Object"))
	UObject* BP_GetSourceObject(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const;
	
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual bool BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle, bool EndAbilityImmediately);
	
	virtual void ExternalEndAbility();
	
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual FString GetCurrentPredictionKeyStatus();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability")
	virtual bool IsPredictionKeyValidForMorePrediction() const;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability")
	bool GSCheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const;
	virtual bool GSCheckCost_Implementation(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const;

	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability")
	void GSApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const;
	virtual void GSApplyCost_Implementation(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const {};

	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void SetHUDReticle(TSubclassOf<UGSHUDReticle> ReticleClass);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void ResetHUDReticle();
	
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void SendTargetDataToServer(const FGameplayAbilityTargetDataHandle& TargetData);
	
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual bool IsInputPressed() const;
	
	UFUNCTION(BlueprintCallable, Category = Animation)
	UAnimMontage* GetCurrentMontageForMesh(USkeletalMeshComponent* InMesh);

	virtual void SetCurrentMontageForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* InCurrentMontage);

protected:
	FGameplayTag InteractingTag;
	FGameplayTag InteractingRemovalTag;
	
	UPROPERTY()
	TArray<FAbilityMeshMontage> CurrentAbilityMeshMontages;

	bool FindAbilityMeshMontage(const USkeletalMeshComponent* InMesh, FAbilityMeshMontage& InAbilityMontage);
	
	UFUNCTION(BlueprintCallable, Category = "Ability|Animation")
	void MontageJumpToSectionForMesh(USkeletalMeshComponent* InMesh, FName SectionName);
	
	UFUNCTION(BlueprintCallable, Category = "Ability|Animation")
	void MontageSetNextSectionNameForMesh(USkeletalMeshComponent* InMesh, FName FromSectionName, FName ToSectionName);
	
	UFUNCTION(BlueprintCallable, Category = "Ability|Animation", Meta = (AdvancedDisplay = "OverrideBlendOutTime"))
	void MontageStopForMesh(USkeletalMeshComponent* InMesh, float OverrideBlendOutTime = -1.0f);
	
	UFUNCTION(BlueprintCallable, Category = "Ability|Animation", Meta = (AdvancedDisplay = "OverrideBlendOutTime"))
	void MontageStopForAllMeshes(float OverrideBlendOutTime = -1.0f);
};
