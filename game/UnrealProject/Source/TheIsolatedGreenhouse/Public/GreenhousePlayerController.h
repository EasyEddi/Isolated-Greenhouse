#pragma once

#include "CoreMinimal.h"
#include "GreenhouseInventoryWidget.h"
#include "GameFramework/PlayerController.h"
#include "GreenhousePlayerController.generated.h"

class AGreenhouseHeldItemActor;
class AGreenhousePlantingPlotActor;
class AStaticMeshActor;
class UGreenhouseInventoryWidget;
class UInputMappingContext;
class USceneComponent;

UCLASS()
class THEISOLATEDGREENHOUSE_API AGreenhousePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGreenhousePlayerController();

	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

private:
	static constexpr float SprintSpeedMultiplier = 1.5f;
	static constexpr float InteractionTraceDistance = 520.0f;
	static constexpr float MaxWateringCanLiters = 10.0f;
	static constexpr float WateringCanFillDuration = 7.0f;
	static constexpr float WateringLitersPerSecond = 1.15f;
	static constexpr float FillingCanFloorOffsetZ = 12.0f;

	UPROPERTY()
	TObjectPtr<UGreenhouseInventoryWidget> InventoryWidget;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> MouseLookMappingContext;

	UPROPERTY()
	TObjectPtr<AGreenhouseHeldItemActor> HeldItemActor;

	UPROPERTY()
	TObjectPtr<AStaticMeshActor> FillingWaterStreamActor;

	UPROPERTY()
	TArray<TObjectPtr<AStaticMeshActor>> FillingWaterFlowActors;

	UPROPERTY()
	TArray<TObjectPtr<AStaticMeshActor>> PlacedPotActors;

	TArray<int32> OccupiedPotSlotIndices;

	EGreenhouseInventoryItem LastDisplayedHeldItem = EGreenhouseInventoryItem::None;
	float DefaultWalkSpeed = 0.0f;
	float WateringCanLiters = 0.0f;
	float WateringCanFillSeconds = 0.0f;
	float WateringCanFillStartLiters = 0.0f;
	float PouringVisualSecondsRemaining = 0.0f;
	FTransform WateringCanFillingTransform = FTransform::Identity;
	FVector FillingWaterStartLocation = FVector::ZeroVector;
	FVector FillingWaterEndLocation = FVector::ZeroVector;
	TWeakObjectPtr<AGreenhousePlantingPlotActor> CurrentWateringPlot;
	bool bIsSprinting = false;
	bool bIsFillingWateringCan = false;
	bool bIsPouringWater = false;
	bool bStopPouringWhenVisualEnds = false;

	void ToggleInventory();
	void HandleInteractOrInventory();
	bool TryPlaceSelectedItem();
	bool TryPlantSelectedLily();
	bool FindItemPlacementSlot(EGreenhouseInventoryItem SelectedItem, const FHitResult& InteractionHit, FTransform& OutPlacementTransform, int32& OutSlotIndex) const;
	bool IsPotSlotOccupied(int32 SlotIndex) const;
	void HandleComputerShopPressed();
	void HandleWateringCanFillPressed();
	bool TryStartFillingSelectedWateringCan();
	void HandleWateringCanPressed();
	void HandleWateringCanReleased();
	void ApplyInventoryInputMode(bool bInventoryOpen);
	void RegisterInputMappingContexts();
	void ApplyInitialSpawnView();
	void ConfigurePlayerCollision();
	void SpawnHeldItemActor();
	void SpawnFillingWaterStreamActor();
	USceneComponent* FindFirstPersonCameraComponent() const;
	void UpdateHeldItemActor();
	void UpdateWateringCanState(float DeltaTime);
	bool HasSelectedWateringCan() const;
	bool TraceForInteraction(FHitResult& OutHit) const;
	bool IsFaucetHit(const FHitResult& Hit) const;
	bool IsComputerScreenHit(const FHitResult& Hit) const;
	void StartFillingWateringCan(const FHitResult& FaucetHit);
	void FinishFillingWateringCan();
	void StartPouringWater(const FHitResult& InteractionHit);
	void StopPouringWater();
	AGreenhousePlantingPlotActor* FindWateringPlot(const FHitResult& InteractionHit) const;
	FTransform BuildFillingCanTransform(const FHitResult& FaucetHit) const;
	FVector FindFaucetWaterStart(const FHitResult& FaucetHit) const;
	FVector FindGroundedCanLocation(const FVector& DesiredLocation, const FHitResult& FaucetHit) const;
	void UpdateFillingWaterStream();
	void HideFillingWaterStream();
	void SelectHotbarSlot(int32 SlotIndex);
	void SelectHotbarSlotOne();
	void SelectHotbarSlotTwo();
	void SelectHotbarSlotThree();
	void SelectHotbarSlotFour();
	void SelectHotbarSlotFive();
	void StartSprint();
	void StopSprint();
	void ApplySprintSpeed();
};
