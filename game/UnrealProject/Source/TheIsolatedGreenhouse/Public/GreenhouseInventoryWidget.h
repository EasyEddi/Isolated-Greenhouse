#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "GreenhouseInventoryWidget.generated.h"

class UBorder;
class UHorizontalBox;
class UTextBlock;
class UVerticalBox;

UENUM(BlueprintType)
enum class EGreenhouseInventoryItem : uint8
{
	None,
	Lily,
	WateringCan
};

UCLASS()
class THEISOLATEDGREENHOUSE_API UGreenhouseInventorySlotButton : public UButton
{
	GENERATED_BODY()

public:
	void Setup(class UGreenhouseInventoryWidget* InInventoryWidget, int32 InSlotIndex);

private:
	UPROPERTY()
	TObjectPtr<class UGreenhouseInventoryWidget> InventoryWidget;

	int32 SlotIndex = INDEX_NONE;

	UFUNCTION()
	void HandleClicked();
};

UCLASS()
class THEISOLATEDGREENHOUSE_API UGreenhouseInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	void ToggleInventory();
	void SetInventoryOpen(bool bOpen);
	bool IsInventoryOpen() const { return bInventoryOpen; }
	void HandleSlotClicked(int32 SlotIndex);
	void SelectHotbarSlot(int32 SlotIndex);
	EGreenhouseInventoryItem GetSelectedHotbarItem() const;

private:
	static constexpr int32 HotbarSlotCount = 5;
	static constexpr int32 InventoryColumnCount = 9;
	static constexpr int32 InventoryRowCount = 3;
	static constexpr int32 TotalSlotCount = HotbarSlotCount + InventoryColumnCount * InventoryRowCount;

	UPROPERTY()
	TObjectPtr<UVerticalBox> InventoryPanel;

	UPROPERTY()
	TObjectPtr<UTextBlock> HeldItemText;

	UPROPERTY()
	TArray<TObjectPtr<UTextBlock>> SlotTexts;

	UPROPERTY()
	TArray<TObjectPtr<UBorder>> SlotBackgrounds;

	UPROPERTY()
	TArray<EGreenhouseInventoryItem> Slots;

	EGreenhouseInventoryItem HeldItem = EGreenhouseInventoryItem::None;
	int32 SelectedHotbarSlot = 0;
	bool bInventoryOpen = false;
	bool bBuilt = false;

	void BuildInterface();
	UGreenhouseInventorySlotButton* CreateSlotButton(int32 SlotIndex, const FVector2D& Size);
	void AddItem(EGreenhouseInventoryItem Item);
	void RefreshSlots();
	FText GetItemText(EGreenhouseInventoryItem Item) const;
	FLinearColor GetSlotColor(int32 SlotIndex) const;

	UFUNCTION()
	void GiveLily();

	UFUNCTION()
	void GiveWateringCan();
};
