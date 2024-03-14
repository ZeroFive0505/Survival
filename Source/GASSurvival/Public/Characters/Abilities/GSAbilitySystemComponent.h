#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GSAbilitySystemComponent.generated.h"

class USkeletalMeshComponent;

USTRUCT()
struct GASSURVIVAL_API FGameplayAbilityLocalAnimMontageForMesh
{
	GENERATED_BODY()

	UPROPERTY()
	USkeletalMeshComponent* Mesh;

	UPROPERTY()
	FGameplayAbilityLocalAnimMontage LocalMontageInfo;

	FGameplayAbilityLocalAnimMontageForMesh() : Mesh(nullptr), LocalMontageInfo()
	{
	}

	FGameplayAbilityLocalAnimMontageForMesh(USkeletalMeshComponent* InMesh)
		: Mesh(InMesh), LocalMontageInfo()
	{
	}

	FGameplayAbilityLocalAnimMontageForMesh(USkeletalMeshComponent* InMesh,
	                                        const FGameplayAbilityLocalAnimMontage& InLocalMontageInfo)
		: Mesh(InMesh), LocalMontageInfo(InLocalMontageInfo)
	{
	}
};

USTRUCT()
struct GASSURVIVAL_API FGameplayAbilityRepAnimMontageForMesh
{
	GENERATED_BODY()

	UPROPERTY()
	USkeletalMeshComponent* Mesh;

	UPROPERTY()
	FGameplayAbilityRepAnimMontage RepMontageInfo;

	FGameplayAbilityRepAnimMontageForMesh() : Mesh(nullptr), RepMontageInfo()
	{
	}

	FGameplayAbilityRepAnimMontageForMesh(USkeletalMeshComponent* InMesh)
		: Mesh(InMesh), RepMontageInfo()
	{
	}
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GASSURVIVAL_API UGSAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UGSAbilitySystemComponent();

	bool bCharacterAbilitiesGiven = false;
	bool bStartupEffectsApplied = false;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual bool GetShouldTick() const override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability,
	                                bool bWasCancelled) override;
	
	static UGSAbilitySystemComponent* GetAbilitySystemComponentFromActor(
		const AActor* Actor, const bool LookForComponent = false);
	
	virtual void AbilityLocalInputPressed(const int32 InputID) override;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Abilities",
		Meta = (DisplayName = "GetTagCount", ScriptName = "GetTagCount"))
	int32 BP_GetTagCount(const FGameplayTag TagToCheck) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Abilities")
	FGameplayAbilitySpecHandle FindAbilitySpecHandleForClass(const TSubclassOf<UGameplayAbility> AbilityClass,
	                                                         UObject* OptionalSourceObject = nullptr);
	
	virtual bool ShouldDoServerAbilityRPCBatch() const override { return true; }
	
	UFUNCTION(BlueprintCallable, Category = "Abilities", Meta = (DisplayName = "AddLooseGameplayTag"))
	void BP_AddLooseGameplayTag(const FGameplayTag& GameplayTag, int32 Count = 1);
	
	UFUNCTION(BlueprintCallable, Category = "Abilities", Meta = (DisplayName = "AddLooseGameplayTags"))
	void BP_AddLooseGameplayTags(const FGameplayTagContainer& GameplayTags, int32 Count = 1);
	
	UFUNCTION(BlueprintCallable, Category = "Abilities", Meta = (DisplayName = "RemoveLooseGameplayTag"))
	void BP_RemoveLooseGameplayTag(const FGameplayTag& GameplayTag, int32 Count = 1);
	
	UFUNCTION(BlueprintCallable, Category = "Abilities", Meta = (DisplayName = "RemoveLooseGameplayTags"))
	void BP_RemoveLooseGameplayTags(const FGameplayTagContainer& GameplayTags, int32 Count = 1);
	
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	virtual bool BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle, bool EndAbilityImmediately);

	UFUNCTION(BlueprintCallable, Category = "GameplayCue",
		Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	void ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag,
	                             const FGameplayCueParameters& GameplayCueParameters);

	UFUNCTION(BlueprintCallable, Category = "GameplayCue",
		Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	void AddGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

	UFUNCTION(BlueprintCallable, Category = "GameplayCue",
		Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	void RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual FString GetCurrentPredictionKeyStatus();


	UFUNCTION(BlueprintCallable, Category = "GameplayEffects",
		Meta = (DisplayName = "ApplyGameplayEffectToSelfWithPrediction"))
	FActiveGameplayEffectHandle BP_ApplyGameplayEffectToSelfWithPrediction(
		const TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level,
		FGameplayEffectContextHandle EffectContext);
	
	UFUNCTION(BlueprintCallable, Category = "GameplayEffects",
		Meta = (DisplayName = "ApplyGameplayEffectToTargetWithPrediction"))
	FActiveGameplayEffectHandle BP_ApplyGameplayEffectToTargetWithPrediction(
		const TSubclassOf<UGameplayEffect> GameplayEffectClass, UAbilitySystemComponent* Target, float Level,
		FGameplayEffectContextHandle Context);
	
	
	virtual float PlayMontageForMesh(UGameplayAbility* InAnimatingAbility, USkeletalMeshComponent* InMesh,
	                                 FGameplayAbilityActivationInfo ActivationInfo, UAnimMontage* NewAnimMontage,
	                                 float InPlayRate, FName StartSectionName = NAME_None,
	                                 bool bReplicateMontage = true);
	
	virtual float PlayMontageSimulatedForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* NewAnimMontage,
	                                          float InPlayRate,
	                                          FName StartSectionName = NAME_None);
	
	virtual void CurrentMontageStopForMesh(USkeletalMeshComponent* InMesh, float OverrideBlendOutTime = -1.0f);
	
	virtual void StopAllCurrentMontages(float OverrideBlendOutTime = -1.0f);
	
	virtual void StopMontageIfCurrentForMesh(USkeletalMeshComponent* InMesh, const UAnimMontage& Montage,
	                                         float OverrideBlendOutTime = -1.0f);
	
	virtual void ClearAnimatingAbilityForAllMeshes(UGameplayAbility* Ability);
	
	virtual void CurrentMontageJumpToSectionForMesh(USkeletalMeshComponent* InMesh, FName SectionName);
	
	virtual void CurrentMontageSetNextSectionNameForMesh(USkeletalMeshComponent* InMesh, FName FromSectionName,
	                                                     FName ToSectionName);
	
	virtual void CurrentMontageSetPlayRateForMesh(USkeletalMeshComponent* InMesh, float InPlayRate);
	
	bool IsAnimatingAbilityForAnyMesh(const UGameplayAbility* InAbility) const;
	
	UGameplayAbility* GetAnimatingAbilityFromAnyMesh();
	
	TArray<UAnimMontage*> GetCurrentMontages() const;
	
	UAnimMontage* GetCurrentMontageForMesh(USkeletalMeshComponent* InMesh);
	
	int32 GetCurrentMontageSectionIDForMesh(USkeletalMeshComponent* InMesh);
	
	FName GetCurrentMontageSectionNameForMesh(USkeletalMeshComponent* InMesh);
	
	float GetCurrentMontageSectionLengthForMesh(USkeletalMeshComponent* InMesh);
	
	float GetCurrentMontageSectionTimeLeftForMesh(USkeletalMeshComponent* InMesh);

protected:
	UPROPERTY()
	bool bPendingMontageRepForMesh;
	
	UPROPERTY()
	TArray<FGameplayAbilityLocalAnimMontageForMesh> LocalAnimMontageInfoForMeshes;
	
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedAnimMontageForMesh)
	TArray<FGameplayAbilityRepAnimMontageForMesh> RepAnimMontageInfoForMeshes;
	
	FGameplayAbilityLocalAnimMontageForMesh& GetLocalAnimMontageInfoForMesh(USkeletalMeshComponent* InMesh);
	
	FGameplayAbilityRepAnimMontageForMesh& GetGameplayAbilityRepAnimMontageForMesh(USkeletalMeshComponent* InMesh);
	
	void OnPredictiveMontageRejectedForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* PredictiveMontage);
	
	void AnimMontage_UpdateReplicatedDataForMesh(USkeletalMeshComponent* InMesh);
	void AnimMontage_UpdateReplicatedDataForMesh(FGameplayAbilityRepAnimMontageForMesh& OutRepAnimMontageInfo);
	
	void AnimMontage_UpdateForcedPlayFlagsForMesh(const FGameplayAbilityRepAnimMontageForMesh& OutRepAnimMontageInfo);

	UFUNCTION()
	virtual void OnRep_ReplicatedAnimMontageForMesh();
	
	virtual bool IsReadyForReplicatedMontageForMesh();
	
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCurrentMontageSetNextSectionNameForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage,
	                                                   float ClientPosition, FName SectionName, FName NextSectionName);
	void ServerCurrentMontageSetNextSectionNameForMesh_Implementation(USkeletalMeshComponent* InMesh,
	                                                                  UAnimMontage* ClientAnimMontage,
	                                                                  float ClientPosition, FName SectionName,
	                                                                  FName NextSectionName);
	bool ServerCurrentMontageSetNextSectionNameForMesh_Validate(USkeletalMeshComponent* InMesh,
	                                                            UAnimMontage* ClientAnimMontage, float ClientPosition,
	                                                            FName SectionName, FName NextSectionName);
	
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCurrentMontageJumpToSectionNameForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage,
	                                                  FName SectionName);
	void ServerCurrentMontageJumpToSectionNameForMesh_Implementation(USkeletalMeshComponent* InMesh,
	                                                                 UAnimMontage* ClientAnimMontage,
	                                                                 FName SectionName);
	bool ServerCurrentMontageJumpToSectionNameForMesh_Validate(USkeletalMeshComponent* InMesh,
	                                                           UAnimMontage* ClientAnimMontage, FName SectionName);
	
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCurrentMontageSetPlayRateForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage,
	                                            float InPlayRate);
	void ServerCurrentMontageSetPlayRateForMesh_Implementation(USkeletalMeshComponent* InMesh,
	                                                           UAnimMontage* ClientAnimMontage, float InPlayRate);
	bool ServerCurrentMontageSetPlayRateForMesh_Validate(USkeletalMeshComponent* InMesh,
	                                                     UAnimMontage* ClientAnimMontage, float InPlayRate);
};
