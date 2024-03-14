#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GSFloatingStatusBarWidget.generated.h"


class AGSCharacter;

UCLASS()
class GASSURVIVAL_API UGSFloatingStatusBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
	AGSCharacter* OwningCharacter;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetHealthPercentage(float HealthPercentage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetManaPercentage(float ManaPercentage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetShieldPercentage(float ShieldPercentage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetCharacterName(const FText& NewName);
};
