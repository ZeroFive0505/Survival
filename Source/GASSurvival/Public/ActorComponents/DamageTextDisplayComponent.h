#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "DamageTextDisplayComponent.generated.h"

class UGSDamageTextWidgetComponent;

USTRUCT(BlueprintType)
struct GASSURVIVAL_API FGSDamageNumber
{
	GENERATED_USTRUCT_BODY()

	float DamageAmount;

	FGameplayTagContainer Tags;

	FGSDamageNumber() {}

	FGSDamageNumber(float InDamageAmount, FGameplayTagContainer InTags) : DamageAmount(InDamageAmount)
	{
		Tags.AppendTags(InTags);
	}
};

UCLASS(ClassGroup=(Custom), Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class GASSURVIVAL_API UDamageTextDisplayComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDamageTextDisplayComponent();

protected:
	virtual void BeginPlay() override;

	TArray<FGSDamageNumber> DamageNumberQueue;
	FTimerHandle DamageNumberTimer;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GASSurvival|UI")
	TSubclassOf<UGSDamageTextWidgetComponent> DamageNumberClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GASSurvival|UI")
	float OffsetRange;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void AddDamageNumber(float Damage, FGameplayTagContainer DamageNumberTags);
	
	void ShowDamageNumber();
};
