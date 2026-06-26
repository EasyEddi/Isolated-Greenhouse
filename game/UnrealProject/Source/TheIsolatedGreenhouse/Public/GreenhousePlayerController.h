#pragma once

#include "CoreMinimal.h"
#include "GreenhouseInventoryWidget.h"
#include "GameFramework/PlayerController.h"
#include "GreenhousePlayerController.generated.h"

class AGreenhouseHeldItemActor;
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
	UPROPERTY()
	TObjectPtr<UGreenhouseInventoryWidget> InventoryWidget;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> MouseLookMappingContext;

	UPROPERTY()
	TObjectPtr<AGreenhouseHeldItemActor> HeldItemActor;

	EGreenhouseInventoryItem LastDisplayedHeldItem = EGreenhouseInventoryItem::None;

	void ToggleInventory();
	void ApplyInventoryInputMode(bool bInventoryOpen);
	void RegisterInputMappingContexts();
	void ApplyInitialSpawnView();
	void SpawnHeldItemActor();
	USceneComponent* FindFirstPersonCameraComponent() const;
	void UpdateHeldItemActor();
	void SelectHotbarSlot(int32 SlotIndex);
	void SelectHotbarSlotOne();
	void SelectHotbarSlotTwo();
	void SelectHotbarSlotThree();
	void SelectHotbarSlotFour();
	void SelectHotbarSlotFive();
};
