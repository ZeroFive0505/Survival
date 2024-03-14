#include "Items/PickUps/GSPickUp.h"
#include "AbilitySystemComponent.h"
#include "Characters/Abilities/GSGameplayAbility.h"
#include "Characters/Survivors/GSSurvivor.h"
#include "Components/CapsuleComponent.h"
#include "GASSurvival/GASSurvival.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"

AGSPickUp::AGSPickUp()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bIsActive = true;
	bCanRespawn = true;
	RespawnTime = 5.0f;

	CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(FName("CollisionComp"));
	CollisionComp->InitCapsuleSize(40.0f, 50.0f);
	CollisionComp->SetCollisionObjectType(COLLISION_PICKUP);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = CollisionComp;

	RestrictedPickupTags.AddTag(FGameplayTag::RequestGameplayTag("State.Dead"));
	RestrictedPickupTags.AddTag(FGameplayTag::RequestGameplayTag("State.KnockedDown"));
}

void AGSPickUp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGSPickUp, bIsActive);
	DOREPLIFETIME(AGSPickUp, PickedUpBy);
}

void AGSPickUp::NotifyActorBeginOverlap(AActor* Other)
{
	if (GetLocalRole() == ROLE_Authority && Other && Other != this)
	{
		PickupOnTouch(Cast<AGSSurvivor>(Other));
	}
}

bool AGSPickUp::CanBePickedUp(AGSCharacter* TestCharacter) const
{
	return bIsActive && TestCharacter && TestCharacter->IsAlive() && IsValid(this) && !TestCharacter->
		GetAbilitySystemComponent()->HasAnyMatchingGameplayTags(RestrictedPickupTags) &&
		BP_CanBePickedUp(TestCharacter);
}

bool AGSPickUp::BP_CanBePickedUp_Implementation(AGSCharacter* TestCharacter) const
{
	return true;
}

void AGSPickUp::PickupOnTouch(AGSCharacter* Pawn)
{
	if (CanBePickedUp(Pawn))
	{
		GivePickupTo(Pawn);
		PickedUpBy = Pawn;
		bIsActive = false;
		OnPickedUp();

		if (bCanRespawn && RespawnTime > 0.0f)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_RespawnPickup, this, &AGSPickUp::RespawnPickup, RespawnTime, false);
		}
		else
		{
			Destroy();
		}
	}
}

void AGSPickUp::GivePickupTo(AGSCharacter* Pawn)
{
	UAbilitySystemComponent* ASC = Pawn->GetAbilitySystemComponent();

	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("%s Pawn's ASC is null."), *FString(__FUNCTION__));
		return;
	}

	for (TSubclassOf<UGSGameplayAbility> AbilityClass : AbilityClasses)
	{
		if (!AbilityClass)
		{
			continue;
		}
		
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1, static_cast<int32>(AbilityClass.GetDefaultObject()->AbilityInputID), this);
		ASC->GiveAbilityAndActivateOnce(AbilitySpec);
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	for (const TSubclassOf<UGameplayEffect> EffectClass : EffectClasses)
	{
		if (!EffectClass)
		{
			continue;
		}

		FGameplayEffectSpecHandle NewHandle = ASC->MakeOutgoingSpec(EffectClass, Pawn->GetCharacterLevel(), EffectContext);

		if (NewHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
		}
	}
}

void AGSPickUp::OnPickedUp()
{
	BP_OnPickedUp();

	if (PickupSound && PickedUpBy)
	{
		UGameplayStatics::SpawnSoundAttached(PickupSound, PickedUpBy->GetRootComponent());
	}
}

void AGSPickUp::RespawnPickup()
{
	bIsActive = true;
	PickedUpBy = nullptr;
	OnRespawned();

	TSet<AActor*> OverlappingPawns;
	GetOverlappingActors(OverlappingPawns, AGSCharacter::StaticClass());

	for (AActor* OverlappingPawn : OverlappingPawns)
	{
		PickupOnTouch(CastChecked<AGSCharacter>(OverlappingPawn));
	}
}

void AGSPickUp::OnRespawned()
{
	BP_OnRespawned();
}

void AGSPickUp::OnRep_IsActive()
{
	if (bIsActive)
	{
		OnRespawned();
	}
	else
	{
		OnPickedUp();
	}
}
