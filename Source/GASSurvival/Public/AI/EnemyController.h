#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "EnemyController.generated.h"

class UGSAbilitySystemComponent;
class UGSAttributeSetBase;

UCLASS()
class GASSURVIVAL_API AEnemyController : public AAIController, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AEnemyController();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	TObjectPtr<UGSAttributeSetBase> AttributeSetBase;

	UGSAttributeSetBase* GetAttributeSetBase() const;

	FDelegateHandle HealthChangedDelegateHandle;

protected:
	FGameplayTag DeadTag;
	
	UPROPERTY()
	UGSAbilitySystemComponent* AbilitySystemComponent;
	
	virtual void HealthChanged(const FOnAttributeChangeData& Data);
};
