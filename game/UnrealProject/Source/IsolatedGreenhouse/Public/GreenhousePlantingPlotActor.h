#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GreenhousePlantingPlotActor.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class ISOLATEDGREENHOUSE_API AGreenhousePlantingPlotActor : public AActor
{
	GENERATED_BODY()

public:
	AGreenhousePlantingPlotActor();

	virtual void Tick(float DeltaSeconds) override;

	bool CanPlantAt(const FVector& PlayerLocation) const;
	bool PlantLily();
	bool CanWaterAt(const FVector& PlayerLocation) const;
	bool WaterPlant(float WaterSeconds);

private:
	UPROPERTY()
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> PlotMeshComponent;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> LilyMeshComponent;

	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> PlotMaterial;

	float GrowthSeconds = 0.0f;
	float WetSecondsRemaining = 0.0f;
	bool bHasLily = false;
	bool bIsGrowing = false;

	static constexpr float InteractionRadius = 180.0f;
	static constexpr float GrowthDuration = 5.0f;
	static constexpr float InitialLilyScale = 0.04f;
	static constexpr float FullLilyScale = 0.45f;
	static constexpr float WetVisualDuration = 4.0f;

	void UpdatePlotMaterial();
};
