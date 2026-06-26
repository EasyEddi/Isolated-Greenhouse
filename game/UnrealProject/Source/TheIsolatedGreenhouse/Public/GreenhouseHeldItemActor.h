#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GreenhouseInventoryWidget.h"
#include "GreenhouseHeldItemActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class THEISOLATEDGREENHOUSE_API AGreenhouseHeldItemActor : public AActor
{
	GENERATED_BODY()

public:
	AGreenhouseHeldItemActor();

	void SetHeldItem(EGreenhouseInventoryItem Item);

private:
	UPROPERTY()
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> LilyMeshComponent;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> WateringCanMeshComponent;

	EGreenhouseInventoryItem CurrentItem = EGreenhouseInventoryItem::None;
};
