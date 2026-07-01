#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/Viewport.h"
#include "GameFramework/Actor.h"
#include "GreenhouseInventoryWidget.generated.h"

class UBorder;
class UCanvasPanel;
class UHorizontalBox;
class UOverlay;
class USizeBox;
class UTextBlock;
class UVerticalBox;
class UPointLightComponent;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EGreenhouseInventoryItem : uint8
{
	None,
	Lily,
	WateringCan,
	EmptyPot,
	SoilBag,
	FertilizerBag,
	Trowel,
	Secateur
};

UCLASS()
class THEISOLATEDGREENHOUSE_API AGreenhouseItemPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	AGreenhouseItemPreviewActor();
	void SetPreviewItem(EGreenhouseInventoryItem InItem);

private:
	UPROPERTY()
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY()
	TObjectPtr<UPointLightComponent> KeyLightComponent;

	UPROPERTY()
	TObjectPtr<UPointLightComponent> FillLightComponent;

	UPROPERTY()
	TObjectPtr<UStaticMesh> LilyMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> WateringCanMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> EmptyPotMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> SoilBagMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> FertilizerBagMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> TrowelMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> SecateurMesh;
};

UCLASS()
class THEISOLATEDGREENHOUSE_API UGreenhouseItemIconWidget : public UViewport
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	void SetItem(EGreenhouseInventoryItem InItem);

private:
	EGreenhouseInventoryItem Item = EGreenhouseInventoryItem::None;

	UPROPERTY()
	TObjectPtr<AGreenhouseItemPreviewActor> PreviewActor;

	void ConfigurePreviewScene();
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
	void OpenOnlineShop();
	void CloseOnlineShop();
	bool IsShopOpen() const { return bShopOpen; }
	void HandleSlotClicked(int32 SlotIndex);
	void SelectHotbarSlot(int32 SlotIndex);
	EGreenhouseInventoryItem GetSelectedHotbarItem() const;
	bool ConsumeSelectedHotbarItem(EGreenhouseInventoryItem Item);
	void AddItem(EGreenhouseInventoryItem Item);
	void SetWateringCanLiters(float CurrentLiters, float MaxLiters);

private:
	enum class EGreenhouseShopPage : uint8
	{
		Home,
		BuyPlants,
		SellPlants,
		GardenSupplies
	};

	static constexpr int32 HotbarSlotCount = 5;
	static constexpr int32 InventoryColumnCount = 9;
	static constexpr int32 InventoryRowCount = 3;
	static constexpr int32 MaxStackCount = 10;
	static constexpr int32 TotalSlotCount = HotbarSlotCount + InventoryColumnCount * InventoryRowCount;

	UPROPERTY()
	TObjectPtr<UCanvasPanel> RootCanvas;

	UPROPERTY()
	TObjectPtr<UVerticalBox> InventoryPanel;

	UPROPERTY()
	TObjectPtr<UBorder> InventoryFrame;

	UPROPERTY()
	TObjectPtr<UBorder> DebugFrame;

	UPROPERTY()
	TObjectPtr<UBorder> ShopFrame;

	UPROPERTY()
	TObjectPtr<UVerticalBox> ShopPanel;

	UPROPERTY()
	TObjectPtr<UVerticalBox> ShopContentPanel;

	UPROPERTY()
	TObjectPtr<UTextBlock> ShopTitleText;

	UPROPERTY()
	TObjectPtr<UTextBlock> ShopMoneyText;

	UPROPERTY()
	TObjectPtr<UTextBlock> PlayerMoneyText;

	UPROPERTY()
	TObjectPtr<UTextBlock> ShopStatusText;

	UPROPERTY()
	TObjectPtr<UTextBlock> PlantBuyQuantityText;

	UPROPERTY()
	TObjectPtr<USizeBox> CrosshairDot;

	UPROPERTY()
	TObjectPtr<USizeBox> CursorPreview;

	UPROPERTY()
	TObjectPtr<UGreenhouseItemIconWidget> CursorIcon;

	UPROPERTY()
	TObjectPtr<UTextBlock> CursorStackText;

	UPROPERTY()
	TObjectPtr<USizeBox> WaterGaugeFillBox;

	UPROPERTY()
	TObjectPtr<UTextBlock> WaterGaugeText;

	UPROPERTY()
	TArray<TObjectPtr<UGreenhouseItemIconWidget>> SlotIcons;

	UPROPERTY()
	TArray<TObjectPtr<UTextBlock>> SlotStackTexts;

	UPROPERTY()
	TArray<TObjectPtr<UBorder>> SlotBackgrounds;

	UPROPERTY()
	TArray<EGreenhouseInventoryItem> Slots;

	UPROPERTY()
	TArray<int32> SlotStacks;

	EGreenhouseInventoryItem HeldItem = EGreenhouseInventoryItem::None;
	int32 HeldItemStack = 0;
	int32 SelectedHotbarSlot = 0;
	int32 Money = 120;
	int32 PlantBuyQuantity = 1;
	EGreenhouseShopPage CurrentShopPage = EGreenhouseShopPage::Home;
	bool bInventoryOpen = false;
	bool bShopOpen = false;
	bool bBuilt = false;

	void BuildInterface();
	UGreenhouseInventorySlotButton* CreateSlotButton(int32 SlotIndex, const FVector2D& Size);
	USizeBox* CreateSlotFrame(int32 SlotIndex, const FVector2D& Size);
	UTextBlock* CreateLabel(const FText& Text, const FLinearColor& Color, int32 FontSize, ETextJustify::Type Justification = ETextJustify::Center);
	UButton* CreateDebugButton(const FName Name, const FText& Label, UVerticalBox* ParentPanel);
	UButton* CreateShopButton(const FName Name, const FText& Label, UVerticalBox* ParentPanel);
	bool HasItem(EGreenhouseInventoryItem Item) const;
	int32 CountItem(EGreenhouseInventoryItem Item) const;
	bool RemoveItem(EGreenhouseInventoryItem Item, int32 Count);
	void RefreshSlots();
	void RefreshCursorPreview();
	void RefreshShopHeader();
	void ShowShopHome();
	void ShowPlantBuyPage();
	void ShowPlantSellPage();
	void ShowGardenSuppliesPage();
	void SetShopStatus(const FText& StatusText, const FLinearColor& Color);
	bool TrySpendMoney(int32 Cost);
	FText GetStackText(EGreenhouseInventoryItem Item, int32 StackCount) const;
	FLinearColor GetSlotColor(int32 SlotIndex) const;
	FLinearColor GetSlotInnerColor(int32 SlotIndex) const;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION()
	void GiveLily();

	UFUNCTION()
	void GiveWateringCan();

	UFUNCTION()
	void GiveEmptyPot();

	UFUNCTION()
	void GiveSoilBag();

	UFUNCTION()
	void GiveFertilizerBag();

	UFUNCTION()
	void GiveTrowel();

	UFUNCTION()
	void GiveSecateur();

	UFUNCTION()
	void HandleShopBack();

	UFUNCTION()
	void HandleOpenPlantBuyPage();

	UFUNCTION()
	void HandleOpenPlantSellPage();

	UFUNCTION()
	void HandleOpenGardenSuppliesPage();

	UFUNCTION()
	void IncreasePlantBuyQuantity();

	UFUNCTION()
	void DecreasePlantBuyQuantity();

	UFUNCTION()
	void BuyPlants();

	UFUNCTION()
	void SellLily();

	UFUNCTION()
	void BuyEmptyPot();

	UFUNCTION()
	void BuySoilBag();

	UFUNCTION()
	void BuyFertilizerBag();

	UFUNCTION()
	void BuyWateringCan();
};
