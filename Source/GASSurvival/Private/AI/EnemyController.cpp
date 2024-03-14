#include "AI/EnemyController.h"
#include "Characters/GSCharacter.h"
#include "Characters/Abilities/GSAbilitySystemComponent.h"
#include "Characters/Abilities/AttributeSets/GSAttributeSetBase.h"


AEnemyController::AEnemyController()
{
	PrimaryActorTick.bCanEverTick = true;
	
	AbilitySystemComponent = CreateDefaultSubobject<UGSAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSetBase = CreateDefaultSubobject<UGSAttributeSetBase>(TEXT("AttributeSetBase"));
	DeadTag = FGameplayTag::RequestGameplayTag("State.Dead");
}

void AEnemyController::BeginPlay()
{
	Super::BeginPlay();

	HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetHealthAttribute()).AddUObject(this,  &AEnemyController::HealthChanged);
}

void AEnemyController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UAbilitySystemComponent* AEnemyController::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UGSAttributeSetBase* AEnemyController::GetAttributeSetBase() const
{
	return AttributeSetBase;
}

void AEnemyController::HealthChanged(const FOnAttributeChangeData& Data)
{
	AGSCharacter* EnemyCharacter = Cast<AGSCharacter>(GetPawn());

	if(IsValid(EnemyCharacter) && !EnemyCharacter->IsAlive() && !AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	{
		EnemyCharacter->Die();
	}
}

