#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GSDamageTextWidgetComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GASSURVIVAL_API UGSDamageTextWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetDamageText(float Damage, const FGameplayTagContainer& Tags);
};
