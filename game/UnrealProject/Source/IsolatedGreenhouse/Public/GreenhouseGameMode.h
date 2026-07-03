#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GreenhouseGameMode.generated.h"

UCLASS()
class ISOLATEDGREENHOUSE_API AGreenhouseGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGreenhouseGameMode();

	virtual void BeginPlay() override;

private:
	void RepairWallMaterials();
};
