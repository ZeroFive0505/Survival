#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GSInteractable.generated.h"

UINTERFACE(MinimalAPI)
class UGSInteractable : public UInterface
{
	GENERATED_BODY()
};

class GASSURVIVAL_API IGSInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	bool IsAvailableForInteraction(UPrimitiveComponent* InteractionComponent) const;
	virtual bool IsAvailableForInteraction_Implementation(UPrimitiveComponent* InteractionComponent) const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	float GetInteractionDuration(UPrimitiveComponent* InteractionComponent) const;
	virtual float GetInteractionDuration_Implementation(UPrimitiveComponent* InteractionComponent) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	FText GetInteractionDescription(UPrimitiveComponent* InteractionComponent) const;
	virtual FText GetInteractionDescription_Implementation(UPrimitiveComponent* InteractionComponent) const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void GetPreInteractSyncType(bool& bShouldSync, EAbilityTaskNetSyncType& Type,
	                            UPrimitiveComponent* InteractionComponent) const;
	virtual void GetPreInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type,
	                                                   UPrimitiveComponent* InteractionComponent) const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void GetPostInteractSyncType(bool& bShouldSync, EAbilityTaskNetSyncType& Type,
	                             UPrimitiveComponent* InteractionComponent) const;
	void GetPostInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type,
	                                            UPrimitiveComponent* InteractionComponent) const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void PreInteract(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent);

	virtual void PreInteract_Implementation(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void PostInteract(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent);

	virtual void PostInteract_Implementation(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void CancelInteraction(UPrimitiveComponent* InteractionComponent);

	virtual void CancelInteraction_Implementation(UPrimitiveComponent* InteractionComponent);

	virtual FSimpleMulticastDelegate* GetTargetCancelInteractionDelegate(UPrimitiveComponent* InteractionComponent);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable|Do Not Override")
	void RegisterInteracter(UPrimitiveComponent* InteractionComponent, AActor* InteractingActor);
	void RegisterInteracter_Implementation(UPrimitiveComponent* InteractionComponent, AActor* InteractingActor);

	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable|Do Not Override")
	void UnregisterInteracter(UPrimitiveComponent* InteractionComponent, AActor* InteractingActor);
	void UnregisterInteracter_Implementation(UPrimitiveComponent* InteractionComponent, AActor* InteractingActor);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable|Do Not Override")
	void InteractableCancelInteraction(UPrimitiveComponent* InteractionComponent);
	void InteractableCancelInteraction_Implementation(UPrimitiveComponent* InteractionComponent);

protected:
	TMap<UPrimitiveComponent*, TArray<AActor*>> Interacters;
};
