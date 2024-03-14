#include "ActorComponents/DamageTextDisplayComponent.h"

#include "Characters/GSCharacter.h"
#include "UI/GSDamageTextWidgetComponent.h"

UDamageTextDisplayComponent::UDamageTextDisplayComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	

	DamageNumberClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Game/Survival/Blueprints/UI/WC_DamageText.WC_DamageText_C"));
	if (!DamageNumberClass)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Failed to find DamageNumberClass. If it was moved, please update the reference location in C++."), *FString(__FUNCTION__));
	}
}


void UDamageTextDisplayComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


void UDamageTextDisplayComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UDamageTextDisplayComponent::AddDamageNumber(float Damage, FGameplayTagContainer DamageNumberTags)
{
	DamageNumberQueue.Add(FGSDamageNumber(Damage, DamageNumberTags));

	if (!GetWorld()->GetTimerManager().IsTimerActive(DamageNumberTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(DamageNumberTimer, this, &UDamageTextDisplayComponent::ShowDamageNumber, 0.1, true, 0.0f);
	}
}

void UDamageTextDisplayComponent::ShowDamageNumber()
{
	if (DamageNumberQueue.Num() > 0 && IsValid(this))
	{
		UGSDamageTextWidgetComponent* DamageText = NewObject<UGSDamageTextWidgetComponent>(this, DamageNumberClass);
		DamageText->RegisterComponent();
		DamageText->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		const float OffsetX = FMath::RandRange(-OffsetRange, OffsetRange);
		DamageText->SetRelativeLocation(DamageText->GetRelativeLocation() + FVector(OffsetX, 0.0f, 0.0f));
		DamageText->SetDamageText(DamageNumberQueue[0].DamageAmount, DamageNumberQueue[0].Tags);

		if (DamageNumberQueue.Num() < 1)
		{
			GetWorld()->GetTimerManager().ClearTimer(DamageNumberTimer);
		}

		DamageNumberQueue.RemoveAt(0);
	}
}

