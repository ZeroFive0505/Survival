#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GSHUDReticle.generated.h"


UCLASS()
class GASSURVIVAL_API UGSHUDReticle : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetColor(FLinearColor Color);
};
