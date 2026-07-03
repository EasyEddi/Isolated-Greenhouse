#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GreenhouseInventoryWidget.h"
#include "GreenhouseHeldItemActor.generated.h"

class UStaticMeshComponent;

enum class EGreenhouseHeldWaterEffect : uint8
{
	None,
	Pouring,
	Filling
};

UCLASS()
class ISOLATEDGREENHOUSE_API AGreenhouseHeldItemActor : public AActor
{
	GENERATED_BODY()

public:
	AGreenhouseHeldItemActor();

	virtual void Tick(float DeltaSeconds) override;

	void SetHeldItem(EGreenhouseInventoryItem Item);
	void SetWaterEffect(EGreenhouseHeldWaterEffect Effect);

private:
	UPROPERTY()
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> LilyMeshComponent;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> WateringCanMeshComponent;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> EmptyPotMeshComponent;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> SoilBagMeshComponent;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> FertilizerBagMeshComponent;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> TrowelMeshComponent;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> SecateurMeshComponent;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> WaterStreamMeshComponent;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> WaterSprayComponents;

	EGreenhouseInventoryItem CurrentItem = EGreenhouseInventoryItem::None;
	EGreenhouseHeldWaterEffect CurrentWaterEffect = EGreenhouseHeldWaterEffect::None;
	float WaterVisualTime = 0.0f;

	void SetWaterSprayVisible(bool bVisible);
	void UpdateWaterSpray();
};
