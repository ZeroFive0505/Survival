#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GSCharacterMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GASSURVIVAL_API UGSCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

class FGSSavedMove : public FSavedMove_Character
	{
	public:

		typedef FSavedMove_Character Super;
		
		virtual void Clear() override;
		
		virtual uint8 GetCompressedFlags() const override;
		
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
		
		virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
		
		virtual void PrepMoveFor(class ACharacter* Character) override;
		
		uint8 SavedRequestToStartSprinting : 1;

		uint8 SavedRequestToStartADS : 1;
	};

	class FGSNetworkPredictionData_Client : public FNetworkPredictionData_Client_Character
	{
	public:
		FGSNetworkPredictionData_Client(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;
		
		virtual FSavedMovePtr AllocateNewMove() override;
	};

public:
	UGSCharacterMovementComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float SprintSpeedMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float ADSSpeedMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float KnockedDownSpeedMultiplier;

	uint8 RequestToStartSprinting : 1;
	uint8 RequestToStartADS : 1;

	FGameplayTag KnockedDownTag;
	FGameplayTag InteractingTag;
	FGameplayTag InteractingRemovalTag;

	virtual float GetMaxSpeed() const override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	UFUNCTION(BlueprintCallable, Category = "Sprint")
	void StartSprinting();
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	void StopSprinting();
	
	UFUNCTION(BlueprintCallable, Category = "Aim Down Sights")
	void StartAimDownSights();
	UFUNCTION(BlueprintCallable, Category = "Aim Down Sights")
	void StopAimDownSights();
};
