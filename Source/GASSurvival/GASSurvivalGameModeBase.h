#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GASSurvivalGameModeBase.generated.h"

class AGSSurvivor;

UCLASS()
class GASSURVIVAL_API AGASSurvivalGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGASSurvivalGameModeBase();

	void Died(AController* Controller);

protected:
	float RespawnDelay;

	TSubclassOf<AGSSurvivor> SurvivorClass;

	UPROPERTY()
	AActor* EnemySpawnPoint;

	virtual void BeginPlay() override;

	void RespawnSurvivor(AController* Controller);
};
