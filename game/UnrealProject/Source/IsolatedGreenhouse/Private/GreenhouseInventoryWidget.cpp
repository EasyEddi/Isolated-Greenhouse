#include "GreenhouseInventoryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SizeBox.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Widget.h"
#include "Styling/SlateTypes.h"
#include "Widgets/SWidget.h"

namespace
{
UStaticMesh* LoadFirstAvailableInventoryMesh(const TCHAR* PrimaryPath, const TCHAR* FallbackPath = nullptr)
{
	if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, PrimaryPath))
	{
		return Mesh;
	}

	return FallbackPath ? LoadObject<UStaticMesh>(nullptr, FallbackPath) : nullptr;
}

bool IsStackableInventoryItem(EGreenhouseInventoryItem Item)
{
	return Item == EGreenhouseInventoryItem::Lily
		|| Item == EGreenhouseInventoryItem::EmptyPot
		|| Item == EGreenhouseInventoryItem::SoilBag
		|| Item == EGreenhouseInventoryItem::FertilizerBag;
}

bool IsUniqueInventoryItem(EGreenhouseInventoryItem Item)
{
	return Item == EGreenhouseInventoryItem::WateringCan
		|| Item == EGreenhouseInventoryItem::Trowel
		|| Item == EGreenhouseInventoryItem::Secateur;
}

constexpr float InventoryPreviewFillWorldSize = 88.0f;
const FVector InventoryPreviewCameraLocation(-145.0f, 0.0f, 8.0f);

FRotator GetInventoryPreviewRotation(EGreenhouseInventoryItem Item)
{
	switch (Item)
	{
	case EGreenhouseInventoryItem::Lily:
		return FRotator(0.0f, 180.0f, 0.0f);
	case EGreenhouseInventoryItem::WateringCan:
		return FRotator(0.0f, 35.0f, 0.0f);
	case EGreenhouseInventoryItem::EmptyPot:
		return FRotator(0.0f, 180.0f, 0.0f);
	case EGreenhouseInventoryItem::SoilBag:
	case EGreenhouseInventoryItem::FertilizerBag:
		return FRotator(0.0f, 90.0f, 0.0f);
	case EGreenhouseInventoryItem::Trowel:
		return FRotator(0.0f, 135.0f, -18.0f);
	case EGreenhouseInventoryItem::Secateur:
		return FRotator(0.0f, 130.0f, -8.0f);
	default:
		return FRotator::ZeroRotator;
	}
}

FBox BuildRotatedMeshBox(const FBox& LocalBox, const FRotator& Rotation)
{
	FBox RotatedBox(ForceInit);
	for (int32 X = 0; X < 2; ++X)
	{
		for (int32 Y = 0; Y < 2; ++Y)
		{
			for (int32 Z = 0; Z < 2; ++Z)
			{
				const FVector Corner(
					X == 0 ? LocalBox.Min.X : LocalBox.Max.X,
					Y == 0 ? LocalBox.Min.Y : LocalBox.Max.Y,
					Z == 0 ? LocalBox.Min.Z : LocalBox.Max.Z);
				RotatedBox += Rotation.RotateVector(Corner);
			}
		}
	}

	return RotatedBox;
}
}

AGreenhouseItemPreviewActor::AGreenhouseItemPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	MeshComponent->SetupAttachment(SceneRoot);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetCastShadow(false);

	KeyLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("PreviewKeyLight"));
	KeyLightComponent->SetupAttachment(SceneRoot);
	KeyLightComponent->SetRelativeLocation(FVector(-96.0f, -44.0f, 46.0f));
	KeyLightComponent->SetIntensity(250.0f);
	KeyLightComponent->SetAttenuationRadius(380.0f);
	KeyLightComponent->SetCastShadows(false);
	KeyLightComponent->SetLightColor(FLinearColor(1.0f, 0.96f, 0.88f));

	FillLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("PreviewFillLight"));
	FillLightComponent->SetupAttachment(SceneRoot);
	FillLightComponent->SetRelativeLocation(FVector(-92.0f, 58.0f, 18.0f));
	FillLightComponent->SetIntensity(190.0f);
	FillLightComponent->SetAttenuationRadius(380.0f);
	FillLightComponent->SetCastShadows(false);
	FillLightComponent->SetLightColor(FLinearColor(0.72f, 0.82f, 1.0f));

	LilyMesh = LoadFirstAvailableInventoryMesh(TEXT("/Game/models/Plants/Lily/lily.lily"));
	WateringCanMesh = LoadFirstAvailableInventoryMesh(
		TEXT("/Game/models/equipment/Watering_Can/watering_can.watering_can"),
		TEXT("/Game/models/furniture/Watering_Can/watering_can.watering_can"));
	EmptyPotMesh = LoadFirstAvailableInventoryMesh(
		TEXT("/Game/models/equipment/pots/ornament_plants/empty/ornament_plants_pot_empty.ornament_plants_pot_empty"),
		TEXT("/Game/models/equipment/pots/ornament plants/empty/ornament_plants_pot_empty.ornament_plants_pot_empty"));
	SoilBagMesh = LoadFirstAvailableInventoryMesh(
		TEXT("/Game/models/equipment/soil/ornament_plants/ornament_plants_soil.ornament_plants_soil"),
		TEXT("/Game/models/equipment/soil/ornament plants/ornament_plants_soil.ornament_plants_soil"));
	FertilizerBagMesh = LoadFirstAvailableInventoryMesh(
		TEXT("/Game/models/equipment/fertilizer/ornament_plants/ornament_plants_fertilizer.ornament_plants_fertilizer"));
	TrowelMesh = LoadFirstAvailableInventoryMesh(TEXT("/Game/models/equipment/Trowel/trowel.trowel"));
	SecateurMesh = LoadFirstAvailableInventoryMesh(TEXT("/Game/models/equipment/Secateur/Secateur/secateur.secateur"));

	SetPreviewItem(EGreenhouseInventoryItem::None);
}

void AGreenhouseItemPreviewActor::SetPreviewItem(EGreenhouseInventoryItem InItem)
{
	if (!MeshComponent)
	{
		return;
	}

	UStaticMesh* Mesh = nullptr;

	if (InItem == EGreenhouseInventoryItem::Lily)
	{
		Mesh = LilyMesh;
	}
	else if (InItem == EGreenhouseInventoryItem::WateringCan)
	{
		Mesh = WateringCanMesh;
	}
	else if (InItem == EGreenhouseInventoryItem::EmptyPot)
	{
		Mesh = EmptyPotMesh;
	}
	else if (InItem == EGreenhouseInventoryItem::SoilBag)
	{
		Mesh = SoilBagMesh;
	}
	else if (InItem == EGreenhouseInventoryItem::FertilizerBag)
	{
		Mesh = FertilizerBagMesh;
	}
	else if (InItem == EGreenhouseInventoryItem::Trowel)
	{
		Mesh = TrowelMesh;
	}
	else if (InItem == EGreenhouseInventoryItem::Secateur)
	{
		Mesh = SecateurMesh;
	}

	MeshComponent->SetStaticMesh(Mesh);
	const FRotator PreviewRotation = GetInventoryPreviewRotation(InItem);
	MeshComponent->SetRelativeRotation(PreviewRotation);
	MeshComponent->SetRelativeScale3D(FVector::OneVector);

	if (Mesh)
	{
		const FBox RotatedBox = BuildRotatedMeshBox(Mesh->GetBoundingBox(), PreviewRotation);
		const FVector RotatedCenter = RotatedBox.GetCenter();
		const FVector RotatedExtent = RotatedBox.GetExtent();
		const float VisibleWidth = static_cast<float>(RotatedExtent.Y * 2.0);
		const float VisibleHeight = static_cast<float>(RotatedExtent.Z * 2.0);
		const float VisibleSize = FMath::Max3(VisibleWidth, VisibleHeight, 1.0f);
		const float PreviewScale = InventoryPreviewFillWorldSize / VisibleSize;
		MeshComponent->SetRelativeScale3D(FVector(PreviewScale));
		MeshComponent->SetRelativeLocation(-RotatedCenter * PreviewScale);
	}
	else
	{
		MeshComponent->SetRelativeLocation(FVector::ZeroVector);
	}

	SetActorHiddenInGame(InItem == EGreenhouseInventoryItem::None || Mesh == nullptr);
}

TSharedRef<SWidget> UGreenhouseItemIconWidget::RebuildWidget()
{
	TSharedRef<SWidget> Widget = Super::RebuildWidget();
	ConfigurePreviewScene();
	return Widget;
}

void UGreenhouseItemIconWidget::SetItem(EGreenhouseInventoryItem InItem)
{
	Item = InItem;
	ConfigurePreviewScene();
}

void UGreenhouseItemIconWidget::ConfigurePreviewScene()
{
	SetVisibility(Item == EGreenhouseInventoryItem::None ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
	if (!GetViewportWorld())
	{
		return;
	}

	SetBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));
	SetEnableAdvancedFeatures(false);
	SetLightIntensity(0.0f);
	SetSkyIntensity(1.25f);
	SetViewLocation(InventoryPreviewCameraLocation);
	SetViewRotation(FRotator(0.0f, 0.0f, 0.0f));
	SetShowFlag(TEXT("Grid"), false);
	SetShowFlag(TEXT("Selection"), false);
	SetShowFlag(TEXT("AntiAliasing"), true);
	SetShowFlag(TEXT("Lighting"), true);
	SetShowFlag(TEXT("DirectionalLights"), false);
	SetShowFlag(TEXT("PointLights"), true);
	SetShowFlag(TEXT("SkyLighting"), true);
	SetShowFlag(TEXT("DynamicShadows"), false);
	SetShowFlag(TEXT("AmbientOcclusion"), false);
	SetShowFlag(TEXT("PostProcessing"), false);
	SetShowFlag(TEXT("Atmosphere"), false);
	SetShowFlag(TEXT("Fog"), false);
	SetShowFlag(TEXT("MotionBlur"), false);

	if (!PreviewActor)
	{
		PreviewActor = Cast<AGreenhouseItemPreviewActor>(Spawn(AGreenhouseItemPreviewActor::StaticClass()));
	}

	if (PreviewActor)
	{
		PreviewActor->SetPreviewItem(Item);
	}
}

void UGreenhouseInventorySlotButton::Setup(UGreenhouseInventoryWidget* InInventoryWidget, int32 InSlotIndex)
{
	InventoryWidget = InInventoryWidget;
	SlotIndex = InSlotIndex;
	OnClicked.AddUniqueDynamic(this, &UGreenhouseInventorySlotButton::HandleClicked);
}

void UGreenhouseInventorySlotButton::HandleClicked()
{
	if (InventoryWidget)
	{
		InventoryWidget->HandleSlotClicked(SlotIndex);
	}
}

TSharedRef<SWidget> UGreenhouseInventoryWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}

	if (!bBuilt)
	{
		BuildInterface();
		bBuilt = true;
	}

	return Super::RebuildWidget();
}

void UGreenhouseInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Slots.Num() != TotalSlotCount)
	{
		Slots.Init(EGreenhouseInventoryItem::None, TotalSlotCount);
	}
	if (SlotStacks.Num() != TotalSlotCount)
	{
		SlotStacks.Init(0, TotalSlotCount);
	}

	SetInventoryOpen(false);
	SetWateringCanLiters(0.0f, 10.0f);
	RefreshShopHeader();
	RefreshSlots();
}

void UGreenhouseInventoryWidget::BuildInterface()
{
	Slots.Init(EGreenhouseInventoryItem::None, TotalSlotCount);
	SlotStacks.Init(0, TotalSlotCount);
	SlotIcons.SetNum(TotalSlotCount);
	SlotStackTexts.SetNum(TotalSlotCount);
	SlotBackgrounds.SetNum(TotalSlotCount);

	RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("InventoryRoot"));
	WidgetTree->RootWidget = RootCanvas;

	CrosshairDot = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("CrosshairDot"));
	CrosshairDot->SetWidthOverride(5.0f);
	CrosshairDot->SetHeightOverride(5.0f);
	CrosshairDot->SetVisibility(ESlateVisibility::HitTestInvisible);
	UBorder* CrosshairFill = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("CrosshairFill"));
	CrosshairFill->SetBrushColor(FLinearColor(0.90f, 0.98f, 0.88f, 0.86f));
	CrosshairDot->AddChild(CrosshairFill);
	UCanvasPanelSlot* CrosshairSlot = RootCanvas->AddChildToCanvas(CrosshairDot);
	CrosshairSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	CrosshairSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	CrosshairSlot->SetPosition(FVector2D(0.0f, 0.0f));
	CrosshairSlot->SetAutoSize(true);
	CrosshairSlot->SetZOrder(50);

	UHorizontalBox* Hotbar = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Hotbar"));
	UCanvasPanelSlot* HotbarCanvasSlot = RootCanvas->AddChildToCanvas(Hotbar);
	HotbarCanvasSlot->SetAnchors(FAnchors(0.5f, 1.0f));
	HotbarCanvasSlot->SetAlignment(FVector2D(0.5f, 1.0f));
	HotbarCanvasSlot->SetPosition(FVector2D(0.0f, -30.0f));
	HotbarCanvasSlot->SetAutoSize(true);

	for (int32 SlotIndex = 0; SlotIndex < HotbarSlotCount; ++SlotIndex)
	{
		UGreenhouseInventorySlotButton* Button = CreateSlotButton(SlotIndex, FVector2D(78.0f, 78.0f));
		UHorizontalBoxSlot* BoxSlot = Hotbar->AddChildToHorizontalBox(Button);
		BoxSlot->SetPadding(FMargin(5.0f));
	}

	InventoryFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("InventoryFrame"));
	InventoryFrame->SetBrushColor(FLinearColor(0.16f, 0.19f, 0.15f, 0.96f));
	InventoryFrame->SetPadding(FMargin(26.0f, 22.0f, 26.0f, 24.0f));
	UCanvasPanelSlot* InventoryCanvasSlot = RootCanvas->AddChildToCanvas(InventoryFrame);
	InventoryCanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	InventoryCanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	InventoryCanvasSlot->SetPosition(FVector2D(0.0f, 0.0f));
	InventoryCanvasSlot->SetAutoSize(true);

	InventoryPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InventoryPanel"));
	InventoryFrame->SetContent(InventoryPanel);

	UTextBlock* InventoryTitle = CreateLabel(FText::FromString(TEXT("INVENTORY")), FLinearColor(0.90f, 0.86f, 0.70f, 1.0f), 20);
	UVerticalBoxSlot* TitleSlot = InventoryPanel->AddChildToVerticalBox(InventoryTitle);
	TitleSlot->SetPadding(FMargin(6.0f, 0.0f, 0.0f, 18.0f));

	for (int32 Row = 0; Row < InventoryRowCount; ++Row)
	{
		UHorizontalBox* RowBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UVerticalBoxSlot* RowSlot = InventoryPanel->AddChildToVerticalBox(RowBox);
		RowSlot->SetPadding(FMargin(0.0f, Row == 0 ? 0.0f : 8.0f, 0.0f, 0.0f));

		for (int32 Column = 0; Column < InventoryColumnCount; ++Column)
		{
			const int32 SlotIndex = HotbarSlotCount + Row * InventoryColumnCount + Column;
			UGreenhouseInventorySlotButton* Button = CreateSlotButton(SlotIndex, FVector2D(72.0f, 72.0f));
			UHorizontalBoxSlot* BoxSlot = RowBox->AddChildToHorizontalBox(Button);
			BoxSlot->SetPadding(FMargin(5.0f));
		}
	}

	UVerticalBox* TopRightHudPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TopRightHudPanel"));
	UCanvasPanelSlot* TopRightHudSlot = RootCanvas->AddChildToCanvas(TopRightHudPanel);
	TopRightHudSlot->SetAnchors(FAnchors(1.0f, 0.0f));
	TopRightHudSlot->SetAlignment(FVector2D(1.0f, 0.0f));
	TopRightHudSlot->SetPosition(FVector2D(-10.0f, 10.0f));
	TopRightHudSlot->SetAutoSize(true);
	TopRightHudSlot->SetZOrder(40);

	auto CreateMoneyField = [this](const FName FrameName, const FName RowName, const FName LeafName, UTextBlock*& OutMoneyText) -> UBorder*
	{
		UBorder* MoneyFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), FrameName);
		MoneyFrame->SetBrushColor(FLinearColor(0.035f, 0.075f, 0.052f, 0.94f));
		MoneyFrame->SetPadding(FMargin(14.0f, 8.0f, 10.0f, 8.0f));

		UHorizontalBox* MoneyRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), RowName);
		MoneyFrame->SetContent(MoneyRow);

		OutMoneyText = CreateLabel(FText::GetEmpty(), FLinearColor(0.80f, 1.0f, 0.73f, 1.0f), 23);
		UHorizontalBoxSlot* MoneyTextSlot = MoneyRow->AddChildToHorizontalBox(OutMoneyText);
		MoneyTextSlot->SetPadding(FMargin(0.0f, 4.0f, 9.0f, 0.0f));
		MoneyTextSlot->SetVerticalAlignment(VAlign_Center);

		UWidget* LeafIcon = CreateLeafIcon(LeafName, FVector2D(42.0f, 30.0f), FLinearColor(0.31f, 0.88f, 0.38f, 1.0f), FLinearColor(0.18f, 0.62f, 0.27f, 1.0f));
		UHorizontalBoxSlot* LeafSlot = MoneyRow->AddChildToHorizontalBox(LeafIcon);
		LeafSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
		LeafSlot->SetVerticalAlignment(VAlign_Center);

		return MoneyFrame;
	};

	UTextBlock* CreatedPlayerMoneyText = nullptr;
	UBorder* PlayerMoneyFrame = CreateMoneyField(TEXT("PlayerMoneyFrame"), TEXT("PlayerMoneyRow"), TEXT("PlayerMoneyLeaf"), CreatedPlayerMoneyText);
	PlayerMoneyText = CreatedPlayerMoneyText;
	UVerticalBoxSlot* PlayerMoneySlot = TopRightHudPanel->AddChildToVerticalBox(PlayerMoneyFrame);
	PlayerMoneySlot->SetPadding(FMargin(0.0f));
	PlayerMoneySlot->SetHorizontalAlignment(HAlign_Right);

	DebugFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DebugFrame"));
	DebugFrame->SetBrushColor(FLinearColor(0.025f, 0.030f, 0.032f, 0.82f));
	DebugFrame->SetPadding(FMargin(12.0f, 10.0f, 12.0f, 10.0f));
	UVerticalBoxSlot* DebugFrameSlot = TopRightHudPanel->AddChildToVerticalBox(DebugFrame);
	DebugFrameSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 0.0f));
	DebugFrameSlot->SetHorizontalAlignment(HAlign_Right);

	UVerticalBox* GivePanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("GivePanel"));
	DebugFrame->SetContent(GivePanel);

	UTextBlock* DebugTitle = CreateLabel(FText::FromString(TEXT("Debug Tools")), FLinearColor(0.82f, 0.87f, 0.79f, 1.0f), 13);
	UVerticalBoxSlot* DebugTitleSlot = GivePanel->AddChildToVerticalBox(DebugTitle);
	DebugTitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));

	UButton* LilyButton = CreateDebugButton(TEXT("GiveLilyButton"), FText::FromString(TEXT("Give lily")), GivePanel);
	LilyButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::GiveLily);

	UButton* WateringCanButton = CreateDebugButton(TEXT("GiveWateringCanButton"), FText::FromString(TEXT("Give watering can")), GivePanel);
	WateringCanButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::GiveWateringCan);

	UButton* EmptyPotButton = CreateDebugButton(TEXT("GiveEmptyPotButton"), FText::FromString(TEXT("Give empty pot")), GivePanel);
	EmptyPotButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::GiveEmptyPot);

	UButton* SoilBagButton = CreateDebugButton(TEXT("GiveSoilBagButton"), FText::FromString(TEXT("Give soil bag")), GivePanel);
	SoilBagButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::GiveSoilBag);

	UButton* FertilizerBagButton = CreateDebugButton(TEXT("GiveFertilizerBagButton"), FText::FromString(TEXT("Give fertilizer bag")), GivePanel);
	FertilizerBagButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::GiveFertilizerBag);

	UButton* TrowelButton = CreateDebugButton(TEXT("GiveTrowelButton"), FText::FromString(TEXT("Give trowel")), GivePanel);
	TrowelButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::GiveTrowel);

	UButton* SecateurButton = CreateDebugButton(TEXT("GiveSecateurButton"), FText::FromString(TEXT("Give secateur")), GivePanel);
	SecateurButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::GiveSecateur);

	ShopFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("OnlineShopFrame"));
	ShopFrame->SetBrushColor(FLinearColor(0.04f, 0.06f, 0.075f, 0.97f));
	ShopFrame->SetPadding(FMargin(38.0f, 32.0f, 38.0f, 34.0f));
	UCanvasPanelSlot* ShopCanvasSlot = RootCanvas->AddChildToCanvas(ShopFrame);
	ShopCanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	ShopCanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	ShopCanvasSlot->SetPosition(FVector2D(0.0f, 0.0f));
	ShopCanvasSlot->SetAutoSize(true);
	ShopCanvasSlot->SetZOrder(80);

	USizeBox* ShopSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("OnlineShopSize"));
	ShopSizeBox->SetWidthOverride(980.0f);
	ShopSizeBox->SetHeightOverride(620.0f);
	ShopFrame->SetContent(ShopSizeBox);

	ShopPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("OnlineShopPanel"));
	ShopSizeBox->AddChild(ShopPanel);

	USizeBox* ShopHeaderSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("OnlineShopHeaderSize"));
	ShopHeaderSize->SetHeightOverride(76.0f);
	UVerticalBoxSlot* ShopHeaderSlot = ShopPanel->AddChildToVerticalBox(ShopHeaderSize);
	ShopHeaderSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));
	ShopHeaderSlot->SetHorizontalAlignment(HAlign_Fill);

	UOverlay* ShopHeaderOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("OnlineShopHeader"));
	ShopHeaderSize->AddChild(ShopHeaderOverlay);

	UHorizontalBox* ShopTabRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("OnlineShopTabRow"));
	UOverlaySlot* ShopTabRowSlot = ShopHeaderOverlay->AddChildToOverlay(ShopTabRow);
	ShopTabRowSlot->SetHorizontalAlignment(HAlign_Left);
	ShopTabRowSlot->SetVerticalAlignment(VAlign_Top);

	UBorder* CreatedBuyTabBackground = nullptr;
	UButton* BuyTabButton = CreateShopTabButton(
		TEXT("ShopBuyTabButton"),
		CreateCartIcon(TEXT("ShopBuyTabCartIcon"), FVector2D(46.0f, 36.0f), FLinearColor(0.66f, 0.89f, 0.77f, 1.0f), FLinearColor(0.22f, 0.43f, 0.35f, 1.0f)),
		CreatedBuyTabBackground);
	ShopBuyTabBackground = CreatedBuyTabBackground;
	BuyTabButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::HandleOpenPlantBuyPage);
	UHorizontalBoxSlot* BuyTabSlot = ShopTabRow->AddChildToHorizontalBox(BuyTabButton);
	BuyTabSlot->SetPadding(FMargin(0.0f, 0.0f, 10.0f, 0.0f));

	UBorder* CreatedSellTabBackground = nullptr;
	UButton* SellTabButton = CreateShopTabButton(
		TEXT("ShopSellTabButton"),
		CreateLeafIcon(TEXT("ShopSellTabLeafIcon"), FVector2D(44.0f, 32.0f), FLinearColor(0.31f, 0.88f, 0.38f, 1.0f), FLinearColor(0.18f, 0.62f, 0.27f, 1.0f)),
		CreatedSellTabBackground);
	ShopSellTabBackground = CreatedSellTabBackground;
	SellTabButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::HandleOpenPlantSellPage);
	UHorizontalBoxSlot* SellTabSlot = ShopTabRow->AddChildToHorizontalBox(SellTabButton);
	SellTabSlot->SetPadding(FMargin(0.0f));

	ShopContentPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("OnlineShopContent"));
	UVerticalBoxSlot* ShopContentSlot = ShopPanel->AddChildToVerticalBox(ShopContentPanel);
	ShopContentSlot->SetPadding(FMargin(0.0f));
	ShopContentSlot->SetHorizontalAlignment(HAlign_Fill);

	ShopStatusText = CreateLabel(FText::GetEmpty(), FLinearColor(0.84f, 0.91f, 0.86f, 1.0f), 15);
	UVerticalBoxSlot* ShopStatusSlot = ShopPanel->AddChildToVerticalBox(ShopStatusText);
	ShopStatusSlot->SetPadding(FMargin(0.0f, 18.0f, 0.0f, 0.0f));
	ShopFrame->SetVisibility(ESlateVisibility::Collapsed);

	USizeBox* WaterGaugeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("WaterGaugeBox"));
	WaterGaugeBox->SetWidthOverride(46.0f);
	WaterGaugeBox->SetHeightOverride(176.0f);
	UCanvasPanelSlot* WaterGaugeCanvasSlot = RootCanvas->AddChildToCanvas(WaterGaugeBox);
	WaterGaugeCanvasSlot->SetAnchors(FAnchors(1.0f, 1.0f));
	WaterGaugeCanvasSlot->SetAlignment(FVector2D(1.0f, 1.0f));
	WaterGaugeCanvasSlot->SetPosition(FVector2D(-30.0f, -124.0f));
	WaterGaugeCanvasSlot->SetAutoSize(true);

	UBorder* WaterGaugeFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("WaterGaugeFrame"));
	WaterGaugeFrame->SetBrushColor(FLinearColor(0.03f, 0.045f, 0.05f, 0.86f));
	WaterGaugeFrame->SetPadding(FMargin(5.0f));
	WaterGaugeBox->AddChild(WaterGaugeFrame);

	UOverlay* WaterGaugeOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("WaterGaugeOverlay"));
	WaterGaugeFrame->SetContent(WaterGaugeOverlay);

	WaterGaugeFillBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("WaterGaugeFill"));
	WaterGaugeFillBox->SetWidthOverride(30.0f);
	WaterGaugeFillBox->SetHeightOverride(0.0f);
	UOverlaySlot* WaterFillSlot = WaterGaugeOverlay->AddChildToOverlay(WaterGaugeFillBox);
	WaterFillSlot->SetHorizontalAlignment(HAlign_Center);
	WaterFillSlot->SetVerticalAlignment(VAlign_Bottom);

	UBorder* WaterFillBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("WaterGaugeFillBorder"));
	WaterFillBorder->SetBrushColor(FLinearColor(0.10f, 0.48f, 0.95f, 0.88f));
	WaterGaugeFillBox->AddChild(WaterFillBorder);

	WaterGaugeText = CreateLabel(FText::FromString(TEXT("0L")), FLinearColor(0.86f, 0.95f, 1.0f, 1.0f), 11);
	UOverlaySlot* WaterTextSlot = WaterGaugeOverlay->AddChildToOverlay(WaterGaugeText);
	WaterTextSlot->SetHorizontalAlignment(HAlign_Center);
	WaterTextSlot->SetVerticalAlignment(VAlign_Center);

	CursorPreview = CreateSlotFrame(INDEX_NONE, FVector2D(70.0f, 70.0f));
	CursorPreview->SetVisibility(ESlateVisibility::HitTestInvisible);
	CursorPreview->SetRenderOpacity(0.92f);
	UCanvasPanelSlot* CursorSlot = RootCanvas->AddChildToCanvas(CursorPreview);
	CursorSlot->SetAutoSize(true);
	CursorSlot->SetZOrder(100);
	CursorSlot->SetPosition(FVector2D(-1000.0f, -1000.0f));
}

UGreenhouseInventorySlotButton* UGreenhouseInventoryWidget::CreateSlotButton(int32 SlotIndex, const FVector2D& Size)
{
	UGreenhouseInventorySlotButton* Button = WidgetTree->ConstructWidget<UGreenhouseInventorySlotButton>(UGreenhouseInventorySlotButton::StaticClass());
	Button->Setup(this, SlotIndex);
	Button->SetBackgroundColor(FLinearColor::Transparent);

	FButtonStyle ButtonStyle = Button->GetStyle();
	ButtonStyle.SetNormal(FSlateNoResource());
	ButtonStyle.SetHovered(FSlateNoResource());
	ButtonStyle.SetPressed(FSlateNoResource());
	ButtonStyle.SetDisabled(FSlateNoResource());
	ButtonStyle.NormalPadding = FMargin(0.0f);
	ButtonStyle.PressedPadding = FMargin(0.0f);
	Button->SetStyle(ButtonStyle);

	Button->AddChild(CreateSlotFrame(SlotIndex, Size));
	return Button;
}

USizeBox* UGreenhouseInventoryWidget::CreateSlotFrame(int32 SlotIndex, const FVector2D& Size)
{
	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	SizeBox->SetWidthOverride(Size.X);
	SizeBox->SetHeightOverride(Size.Y);

	UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Background->SetBrushColor(SlotIndex == INDEX_NONE ? FLinearColor(0.10f, 0.13f, 0.10f, 0.88f) : GetSlotColor(SlotIndex));
	Background->SetPadding(FMargin(3.0f));
	SizeBox->AddChild(Background);

	UBorder* Inner = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Inner->SetBrushColor(SlotIndex == INDEX_NONE ? FLinearColor(0.07f, 0.09f, 0.07f, 0.96f) : GetSlotInnerColor(SlotIndex));
	Inner->SetPadding(FMargin(0.0f));
	Background->SetContent(Inner);

	UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	Inner->SetContent(Overlay);

	UGreenhouseItemIconWidget* Icon = WidgetTree->ConstructWidget<UGreenhouseItemIconWidget>(UGreenhouseItemIconWidget::StaticClass());
	UOverlaySlot* IconSlot = Overlay->AddChildToOverlay(Icon);
	IconSlot->SetHorizontalAlignment(HAlign_Fill);
	IconSlot->SetVerticalAlignment(VAlign_Fill);
	IconSlot->SetPadding(FMargin(0.0f));

	UTextBlock* StackText = CreateLabel(FText::GetEmpty(), FLinearColor(0.94f, 0.93f, 0.82f, 1.0f), 11);
	UOverlaySlot* StackSlot = Overlay->AddChildToOverlay(StackText);
	StackSlot->SetHorizontalAlignment(HAlign_Right);
	StackSlot->SetVerticalAlignment(VAlign_Bottom);
	StackSlot->SetPadding(FMargin(0.0f, 0.0f, 3.0f, 1.0f));

	if (SlotIndex == INDEX_NONE)
	{
		CursorIcon = Icon;
		CursorStackText = StackText;
	}
	else
	{
		SlotIcons[SlotIndex] = Icon;
		SlotStackTexts[SlotIndex] = StackText;
		SlotBackgrounds[SlotIndex] = Background;
	}

	return SizeBox;
}

UTextBlock* UGreenhouseInventoryWidget::CreateLabel(const FText& Text, const FLinearColor& Color, int32 FontSize, ETextJustify::Type Justification)
{
	UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Label->SetText(Text);
	FSlateFontInfo Font = Label->GetFont();
	Font.Size = FontSize;
	Label->SetFont(Font);
	Label->SetColorAndOpacity(FSlateColor(Color));
	Label->SetJustification(Justification);
	return Label;
}

UButton* UGreenhouseInventoryWidget::CreateDebugButton(const FName Name, const FText& Label, UVerticalBox* ParentPanel)
{
	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	SizeBox->SetWidthOverride(172.0f);
	SizeBox->SetHeightOverride(34.0f);

	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);

	UBorder* ButtonBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	ButtonBackground->SetBrushColor(FLinearColor(0.58f, 0.66f, 0.51f, 0.92f));
	ButtonBackground->SetPadding(FMargin(8.0f, 6.0f, 8.0f, 6.0f));

	UTextBlock* LabelText = CreateLabel(Label, FLinearColor(0.03f, 0.04f, 0.035f, 1.0f), 12);
	ButtonBackground->SetContent(LabelText);
	Button->AddChild(ButtonBackground);
	SizeBox->AddChild(Button);

	UVerticalBoxSlot* ButtonSlot = ParentPanel->AddChildToVerticalBox(SizeBox);
	ButtonSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
	return Button;
}

UButton* UGreenhouseInventoryWidget::CreateShopButton(const FName Name, const FText& Label, UVerticalBox* ParentPanel)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	UBorder* ButtonBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	ButtonBackground->SetBrushColor(FLinearColor(0.16f, 0.32f, 0.23f, 0.96f));
	ButtonBackground->SetPadding(FMargin(16.0f, 11.0f, 16.0f, 11.0f));

	UTextBlock* LabelText = CreateLabel(Label, FLinearColor(0.92f, 0.98f, 0.91f, 1.0f), 17);
	ButtonBackground->SetContent(LabelText);
	Button->AddChild(ButtonBackground);

	if (ParentPanel)
	{
		USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		SizeBox->SetWidthOverride(420.0f);
		SizeBox->SetHeightOverride(56.0f);
		SizeBox->AddChild(Button);

		UVerticalBoxSlot* ButtonSlot = ParentPanel->AddChildToVerticalBox(SizeBox);
		ButtonSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));
		ButtonSlot->SetHorizontalAlignment(HAlign_Left);
	}

	return Button;
}

UButton* UGreenhouseInventoryWidget::CreateShopTabButton(const FName Name, UWidget* IconWidget, UBorder*& OutBackground)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	Button->SetBackgroundColor(FLinearColor::Transparent);

	FButtonStyle ButtonStyle = Button->GetStyle();
	ButtonStyle.SetNormal(FSlateNoResource());
	ButtonStyle.SetHovered(FSlateNoResource());
	ButtonStyle.SetPressed(FSlateNoResource());
	ButtonStyle.SetDisabled(FSlateNoResource());
	ButtonStyle.NormalPadding = FMargin(0.0f);
	ButtonStyle.PressedPadding = FMargin(0.0f);
	Button->SetStyle(ButtonStyle);

	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	SizeBox->SetWidthOverride(76.0f);
	SizeBox->SetHeightOverride(66.0f);

	OutBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	OutBackground->SetBrushColor(FLinearColor(0.36f, 0.38f, 0.36f, 0.92f));
	OutBackground->SetPadding(FMargin(3.0f));
	SizeBox->AddChild(OutBackground);

	UBorder* Inner = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Inner->SetBrushColor(FLinearColor(0.01f, 0.012f, 0.011f, 0.97f));
	Inner->SetPadding(FMargin(10.0f, 9.0f, 10.0f, 9.0f));
	OutBackground->SetContent(Inner);

	Inner->SetContent(IconWidget);
	Button->AddChild(SizeBox);
	return Button;
}

UButton* UGreenhouseInventoryWidget::CreateShopProductButton(const FName Name, EGreenhouseInventoryItem Item, int32 Price, UHorizontalBox* ParentRow)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	Button->SetBackgroundColor(FLinearColor::Transparent);

	FButtonStyle ButtonStyle = Button->GetStyle();
	ButtonStyle.SetNormal(FSlateNoResource());
	ButtonStyle.SetHovered(FSlateNoResource());
	ButtonStyle.SetPressed(FSlateNoResource());
	ButtonStyle.SetDisabled(FSlateNoResource());
	ButtonStyle.NormalPadding = FMargin(0.0f);
	ButtonStyle.PressedPadding = FMargin(0.0f);
	Button->SetStyle(ButtonStyle);

	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	SizeBox->SetWidthOverride(154.0f);
	SizeBox->SetHeightOverride(166.0f);

	UBorder* Outer = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Outer->SetBrushColor(FLinearColor(0.42f, 0.44f, 0.42f, 0.95f));
	Outer->SetPadding(FMargin(3.0f));
	SizeBox->AddChild(Outer);

	UBorder* Inner = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Inner->SetBrushColor(FLinearColor(0.008f, 0.010f, 0.008f, 0.98f));
	Inner->SetPadding(FMargin(0.0f));
	Outer->SetContent(Inner);

	UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	Inner->SetContent(Overlay);

	UGreenhouseItemIconWidget* Icon = WidgetTree->ConstructWidget<UGreenhouseItemIconWidget>(UGreenhouseItemIconWidget::StaticClass());
	Icon->SetItem(Item);
	UOverlaySlot* IconSlot = Overlay->AddChildToOverlay(Icon);
	IconSlot->SetHorizontalAlignment(HAlign_Fill);
	IconSlot->SetVerticalAlignment(VAlign_Fill);
	IconSlot->SetPadding(FMargin(12.0f, 8.0f, 12.0f, 34.0f));

	UBorder* PriceFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	PriceFrame->SetBrushColor(FLinearColor(0.035f, 0.075f, 0.052f, 0.92f));
	PriceFrame->SetPadding(FMargin(10.0f, 5.0f, 7.0f, 5.0f));
	UOverlaySlot* PriceFrameSlot = Overlay->AddChildToOverlay(PriceFrame);
	PriceFrameSlot->SetHorizontalAlignment(HAlign_Right);
	PriceFrameSlot->SetVerticalAlignment(VAlign_Bottom);
	PriceFrameSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 8.0f));

	UHorizontalBox* PriceRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	PriceFrame->SetContent(PriceRow);

	UTextBlock* PriceText = CreateLabel(FText::AsNumber(Price), FLinearColor(0.80f, 1.0f, 0.73f, 1.0f), 18);
	UHorizontalBoxSlot* PriceTextSlot = PriceRow->AddChildToHorizontalBox(PriceText);
	PriceTextSlot->SetVerticalAlignment(VAlign_Center);
	PriceTextSlot->SetPadding(FMargin(0.0f, 1.0f, 4.0f, 0.0f));

	UWidget* LeafIcon = CreateLeafIcon(*FString::Printf(TEXT("%sPriceLeaf"), *Name.ToString()), FVector2D(24.0f, 18.0f), FLinearColor(0.31f, 0.88f, 0.38f, 1.0f), FLinearColor(0.18f, 0.62f, 0.27f, 1.0f));
	UHorizontalBoxSlot* LeafSlot = PriceRow->AddChildToHorizontalBox(LeafIcon);
	LeafSlot->SetVerticalAlignment(VAlign_Center);

	Button->AddChild(SizeBox);

	if (ParentRow)
	{
		UHorizontalBoxSlot* ProductSlot = ParentRow->AddChildToHorizontalBox(Button);
		ProductSlot->SetPadding(FMargin(0.0f, 0.0f, 18.0f, 0.0f));
	}

	return Button;
}

UButton* UGreenhouseInventoryWidget::CreateIconOnlyShopButton(const FName Name, UWidget* IconWidget, const FVector2D& Size)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	Button->SetBackgroundColor(FLinearColor::Transparent);

	FButtonStyle ButtonStyle = Button->GetStyle();
	ButtonStyle.SetNormal(FSlateNoResource());
	ButtonStyle.SetHovered(FSlateNoResource());
	ButtonStyle.SetPressed(FSlateNoResource());
	ButtonStyle.SetDisabled(FSlateNoResource());
	ButtonStyle.NormalPadding = FMargin(0.0f);
	ButtonStyle.PressedPadding = FMargin(0.0f);
	Button->SetStyle(ButtonStyle);

	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	SizeBox->SetWidthOverride(Size.X);
	SizeBox->SetHeightOverride(Size.Y);

	UBorder* Outer = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Outer->SetBrushColor(FLinearColor(0.42f, 0.44f, 0.42f, 0.95f));
	Outer->SetPadding(FMargin(3.0f));
	SizeBox->AddChild(Outer);

	UBorder* Inner = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Inner->SetBrushColor(FLinearColor(0.008f, 0.010f, 0.008f, 0.98f));
	Inner->SetPadding(FMargin(8.0f));
	Outer->SetContent(Inner);

	Inner->SetContent(IconWidget);
	Button->AddChild(SizeBox);
	return Button;
}

UWidget* UGreenhouseInventoryWidget::CreateLeafIcon(const FName Name, const FVector2D& Size, const FLinearColor& MainColor, const FLinearColor& ShadowColor)
{
	USizeBox* IconBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), Name);
	IconBox->SetWidthOverride(Size.X);
	IconBox->SetHeightOverride(Size.Y);

	UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	IconBox->AddChild(Overlay);

	AddBoxPart(Overlay, *FString::Printf(TEXT("%sStem"), *Name.ToString()), FVector2D(Size.X * 0.22f, FMath::Max(2.0f, Size.Y * 0.13f)), FMargin(Size.X * 0.00f, Size.Y * 0.64f, 0.0f, 0.0f), ShadowColor, -10.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sBase"), *Name.ToString()), FVector2D(Size.X * 0.30f, Size.Y * 0.34f), FMargin(Size.X * 0.18f, Size.Y * 0.42f, 0.0f, 0.0f), MainColor, -10.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sCore"), *Name.ToString()), FVector2D(Size.X * 0.44f, Size.Y * 0.58f), FMargin(Size.X * 0.34f, Size.Y * 0.20f, 0.0f, 0.0f), MainColor, -10.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sTopFacet"), *Name.ToString()), FVector2D(Size.X * 0.34f, Size.Y * 0.22f), FMargin(Size.X * 0.42f, Size.Y * 0.12f, 0.0f, 0.0f), FLinearColor(0.43f, 0.98f, 0.50f, MainColor.A), -10.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sTip"), *Name.ToString()), FVector2D(Size.X * 0.24f, Size.Y * 0.38f), FMargin(Size.X * 0.72f, Size.Y * 0.28f, 0.0f, 0.0f), MainColor, -10.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sBottomFacet"), *Name.ToString()), FVector2D(Size.X * 0.30f, Size.Y * 0.20f), FMargin(Size.X * 0.45f, Size.Y * 0.60f, 0.0f, 0.0f), ShadowColor, -10.0f);

	return IconBox;
}

UWidget* UGreenhouseInventoryWidget::CreateCartIcon(const FName Name, const FVector2D& Size, const FLinearColor& MainColor, const FLinearColor& ShadowColor)
{
	USizeBox* IconBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), Name);
	IconBox->SetWidthOverride(Size.X);
	IconBox->SetHeightOverride(Size.Y);

	UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	IconBox->AddChild(Overlay);

	AddBoxPart(Overlay, *FString::Printf(TEXT("%sBasketShadow"), *Name.ToString()), FVector2D(Size.X * 0.62f, Size.Y * 0.34f), FMargin(Size.X * 0.20f, Size.Y * 0.36f, 0.0f, 0.0f), ShadowColor, 0.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sBasket"), *Name.ToString()), FVector2D(Size.X * 0.62f, Size.Y * 0.34f), FMargin(Size.X * 0.16f, Size.Y * 0.30f, 0.0f, 0.0f), MainColor, 0.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sHandle"), *Name.ToString()), FVector2D(Size.X * 0.40f, FMath::Max(2.0f, Size.Y * 0.10f)), FMargin(Size.X * 0.07f, Size.Y * 0.17f, 0.0f, 0.0f), MainColor, -18.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sFront"), *Name.ToString()), FVector2D(FMath::Max(3.0f, Size.X * 0.10f), Size.Y * 0.32f), FMargin(Size.X * 0.75f, Size.Y * 0.30f, 0.0f, 0.0f), MainColor, -10.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sWheelA"), *Name.ToString()), FVector2D(Size.X * 0.14f, Size.X * 0.14f), FMargin(Size.X * 0.26f, Size.Y * 0.72f, 0.0f, 0.0f), ShadowColor, 0.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sWheelB"), *Name.ToString()), FVector2D(Size.X * 0.14f, Size.X * 0.14f), FMargin(Size.X * 0.62f, Size.Y * 0.72f, 0.0f, 0.0f), ShadowColor, 0.0f);

	return IconBox;
}

UWidget* UGreenhouseInventoryWidget::CreateSeedPackIcon(const FName Name, const FVector2D& Size)
{
	USizeBox* IconBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), Name);
	IconBox->SetWidthOverride(Size.X);
	IconBox->SetHeightOverride(Size.Y);

	UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	IconBox->AddChild(Overlay);

	const FLinearColor Packet(0.70f, 0.78f, 0.66f, 1.0f);
	const FLinearColor PacketDark(0.35f, 0.44f, 0.36f, 1.0f);
	const FLinearColor Label(0.91f, 0.86f, 0.67f, 1.0f);
	const FLinearColor Seed(0.25f, 0.86f, 0.34f, 1.0f);

	AddBoxPart(Overlay, *FString::Printf(TEXT("%sBody"), *Name.ToString()), FVector2D(Size.X * 0.58f, Size.Y * 0.62f), FMargin(Size.X * 0.21f, Size.Y * 0.18f, 0.0f, 0.0f), Packet, -6.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sTop"), *Name.ToString()), FVector2D(Size.X * 0.44f, Size.Y * 0.12f), FMargin(Size.X * 0.28f, Size.Y * 0.18f, 0.0f, 0.0f), PacketDark, -6.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sLabel"), *Name.ToString()), FVector2D(Size.X * 0.34f, Size.Y * 0.22f), FMargin(Size.X * 0.34f, Size.Y * 0.42f, 0.0f, 0.0f), Label, -6.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sSeedA"), *Name.ToString()), FVector2D(Size.X * 0.10f, Size.Y * 0.10f), FMargin(Size.X * 0.39f, Size.Y * 0.47f, 0.0f, 0.0f), Seed, -6.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sSeedB"), *Name.ToString()), FVector2D(Size.X * 0.10f, Size.Y * 0.10f), FMargin(Size.X * 0.52f, Size.Y * 0.48f, 0.0f, 0.0f), Seed, -6.0f);

	return IconBox;
}

UWidget* UGreenhouseInventoryWidget::CreateSuppliesIcon(const FName Name, const FVector2D& Size)
{
	USizeBox* IconBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), Name);
	IconBox->SetWidthOverride(Size.X);
	IconBox->SetHeightOverride(Size.Y);

	UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	IconBox->AddChild(Overlay);

	const FLinearColor SoilBag(0.66f, 0.58f, 0.46f, 1.0f);
	const FLinearColor FertBag(0.63f, 0.86f, 0.72f, 1.0f);
	const FLinearColor Fold(0.31f, 0.36f, 0.30f, 1.0f);
	const FLinearColor Label(0.86f, 0.83f, 0.69f, 1.0f);

	AddBoxPart(Overlay, *FString::Printf(TEXT("%sSoil"), *Name.ToString()), FVector2D(Size.X * 0.38f, Size.Y * 0.52f), FMargin(Size.X * 0.15f, Size.Y * 0.32f, 0.0f, 0.0f), SoilBag, -4.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sSoilFold"), *Name.ToString()), FVector2D(Size.X * 0.26f, Size.Y * 0.10f), FMargin(Size.X * 0.21f, Size.Y * 0.32f, 0.0f, 0.0f), Fold, -4.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sSoilLabel"), *Name.ToString()), FVector2D(Size.X * 0.20f, Size.Y * 0.12f), FMargin(Size.X * 0.24f, Size.Y * 0.55f, 0.0f, 0.0f), Label, -4.0f);

	AddBoxPart(Overlay, *FString::Printf(TEXT("%sFertilizer"), *Name.ToString()), FVector2D(Size.X * 0.38f, Size.Y * 0.52f), FMargin(Size.X * 0.48f, Size.Y * 0.22f, 0.0f, 0.0f), FertBag, 5.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sFertilizerFold"), *Name.ToString()), FVector2D(Size.X * 0.26f, Size.Y * 0.10f), FMargin(Size.X * 0.54f, Size.Y * 0.22f, 0.0f, 0.0f), Fold, 5.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sFertilizerLabel"), *Name.ToString()), FVector2D(Size.X * 0.20f, Size.Y * 0.12f), FMargin(Size.X * 0.57f, Size.Y * 0.45f, 0.0f, 0.0f), Label, 5.0f);

	return IconBox;
}

UWidget* UGreenhouseInventoryWidget::CreateEquipmentIcon(const FName Name, const FVector2D& Size)
{
	USizeBox* IconBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), Name);
	IconBox->SetWidthOverride(Size.X);
	IconBox->SetHeightOverride(Size.Y);

	UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	IconBox->AddChild(Overlay);

	const FLinearColor Pot(0.72f, 0.47f, 0.36f, 1.0f);
	const FLinearColor Metal(0.70f, 0.82f, 0.79f, 1.0f);
	const FLinearColor Handle(0.26f, 0.48f, 0.36f, 1.0f);

	AddBoxPart(Overlay, *FString::Printf(TEXT("%sPotRim"), *Name.ToString()), FVector2D(Size.X * 0.44f, Size.Y * 0.12f), FMargin(Size.X * 0.13f, Size.Y * 0.40f, 0.0f, 0.0f), Pot, 0.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sPotBody"), *Name.ToString()), FVector2D(Size.X * 0.34f, Size.Y * 0.28f), FMargin(Size.X * 0.18f, Size.Y * 0.52f, 0.0f, 0.0f), Pot, 0.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sToolHandle"), *Name.ToString()), FVector2D(Size.X * 0.52f, Size.Y * 0.10f), FMargin(Size.X * 0.40f, Size.Y * 0.27f, 0.0f, 0.0f), Handle, -35.0f);
	AddBoxPart(Overlay, *FString::Printf(TEXT("%sToolHead"), *Name.ToString()), FVector2D(Size.X * 0.20f, Size.Y * 0.18f), FMargin(Size.X * 0.67f, Size.Y * 0.16f, 0.0f, 0.0f), Metal, -35.0f);

	return IconBox;
}

void UGreenhouseInventoryWidget::AddLeafPart(UOverlay* Overlay, const FName Name, const FVector2D& Size, const FMargin& InPadding, const FLinearColor& Color, float AngleDegrees)
{
	AddBoxPart(Overlay, Name, Size, InPadding, Color, AngleDegrees);
}

void UGreenhouseInventoryWidget::AddBoxPart(UOverlay* Overlay, const FName Name, const FVector2D& Size, const FMargin& InPadding, const FLinearColor& Color, float AngleDegrees)
{
	if (!Overlay)
	{
		return;
	}

	USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), Name);
	Box->SetWidthOverride(Size.X);
	Box->SetHeightOverride(Size.Y);
	Box->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	Box->SetRenderTransformAngle(AngleDegrees);

	UBorder* Part = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Part->SetBrushColor(Color);
	Box->AddChild(Part);

	UOverlaySlot* OverlaySlot = Overlay->AddChildToOverlay(Box);
	OverlaySlot->SetHorizontalAlignment(HAlign_Left);
	OverlaySlot->SetVerticalAlignment(VAlign_Top);
	OverlaySlot->SetPadding(InPadding);
}

void UGreenhouseInventoryWidget::OpenOnlineShop()
{
	if (!ShopFrame)
	{
		return;
	}

	SetInventoryOpen(false);
	bShopOpen = true;
	ShopFrame->SetVisibility(ESlateVisibility::Visible);
	CurrentShopBuyCategory = EGreenhouseShopBuyCategory::SeedPacks;
	ShowPlantBuyPage();
}

void UGreenhouseInventoryWidget::CloseOnlineShop()
{
	bShopOpen = false;
	if (ShopFrame)
	{
		ShopFrame->SetVisibility(ESlateVisibility::Collapsed);
	}
	SetShopStatus(FText::GetEmpty(), FLinearColor::White);
}

void UGreenhouseInventoryWidget::RefreshShopHeader()
{
	const FText MoneyText = FText::AsNumber(Money);
	if (PlayerMoneyText)
	{
		PlayerMoneyText->SetText(MoneyText);
	}
}

void UGreenhouseInventoryWidget::SetShopStatus(const FText& StatusText, const FLinearColor& Color)
{
	if (!ShopStatusText)
	{
		return;
	}

	ShopStatusText->SetText(StatusText);
	ShopStatusText->SetColorAndOpacity(FSlateColor(Color));
	ShopStatusText->SetVisibility(StatusText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
}

void UGreenhouseInventoryWidget::RefreshShopTabs()
{
	const bool bSellTabActive = CurrentShopPage == EGreenhouseShopPage::SellPlants;
	const FLinearColor ActiveOuterColor(0.62f, 0.64f, 0.62f, 1.0f);
	const FLinearColor InactiveOuterColor(0.36f, 0.38f, 0.36f, 0.92f);

	if (ShopBuyTabBackground)
	{
		ShopBuyTabBackground->SetBrushColor(bSellTabActive ? InactiveOuterColor : ActiveOuterColor);
	}
	if (ShopSellTabBackground)
	{
		ShopSellTabBackground->SetBrushColor(bSellTabActive ? ActiveOuterColor : InactiveOuterColor);
	}
}

void UGreenhouseInventoryWidget::RefreshShopCategoryTabs()
{
	const FLinearColor ActiveOuterColor(0.62f, 0.64f, 0.62f, 1.0f);
	const FLinearColor InactiveOuterColor(0.36f, 0.38f, 0.36f, 0.92f);

	if (ShopSeedPacksCategoryBackground)
	{
		ShopSeedPacksCategoryBackground->SetBrushColor(CurrentShopBuyCategory == EGreenhouseShopBuyCategory::SeedPacks ? ActiveOuterColor : InactiveOuterColor);
	}
	if (ShopSuppliesCategoryBackground)
	{
		ShopSuppliesCategoryBackground->SetBrushColor(CurrentShopBuyCategory == EGreenhouseShopBuyCategory::Supplies ? ActiveOuterColor : InactiveOuterColor);
	}
	if (ShopEquipmentCategoryBackground)
	{
		ShopEquipmentCategoryBackground->SetBrushColor(CurrentShopBuyCategory == EGreenhouseShopBuyCategory::Equipment ? ActiveOuterColor : InactiveOuterColor);
	}
}

void UGreenhouseInventoryWidget::ShowShopHome()
{
	ShowPlantBuyPage();
}

void UGreenhouseInventoryWidget::ShowPlantBuyPage()
{
	CurrentShopPage = EGreenhouseShopPage::BuyPlants;
	RefreshShopHeader();
	RefreshShopTabs();
	SetShopStatus(FText::GetEmpty(), FLinearColor::White);
	if (!ShopContentPanel)
	{
		return;
	}

	ShopContentPanel->ClearChildren();
	PlantBuyQuantity = 1;
	PlantBuyQuantityText = nullptr;

	UHorizontalBox* CategoryRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("BuyCategoryRow"));
	UVerticalBoxSlot* CategoryRowSlot = ShopContentPanel->AddChildToVerticalBox(CategoryRow);
	CategoryRowSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 28.0f));
	CategoryRowSlot->SetHorizontalAlignment(HAlign_Left);

	UBorder* CreatedSeedCategoryBackground = nullptr;
	UButton* SeedCategoryButton = CreateShopTabButton(
		TEXT("SeedPacksCategoryButton"),
		CreateSeedPackIcon(TEXT("SeedPacksCategoryIcon"), FVector2D(46.0f, 38.0f)),
		CreatedSeedCategoryBackground);
	ShopSeedPacksCategoryBackground = CreatedSeedCategoryBackground;
	SeedCategoryButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::HandleOpenSeedPacksCategory);
	UHorizontalBoxSlot* SeedCategorySlot = CategoryRow->AddChildToHorizontalBox(SeedCategoryButton);
	SeedCategorySlot->SetPadding(FMargin(0.0f, 0.0f, 10.0f, 0.0f));

	UBorder* CreatedSuppliesCategoryBackground = nullptr;
	UButton* SuppliesCategoryButton = CreateShopTabButton(
		TEXT("SuppliesCategoryButton"),
		CreateSuppliesIcon(TEXT("SuppliesCategoryIcon"), FVector2D(48.0f, 40.0f)),
		CreatedSuppliesCategoryBackground);
	ShopSuppliesCategoryBackground = CreatedSuppliesCategoryBackground;
	SuppliesCategoryButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::HandleOpenSuppliesCategory);
	UHorizontalBoxSlot* SuppliesCategorySlot = CategoryRow->AddChildToHorizontalBox(SuppliesCategoryButton);
	SuppliesCategorySlot->SetPadding(FMargin(0.0f, 0.0f, 10.0f, 0.0f));

	UBorder* CreatedEquipmentCategoryBackground = nullptr;
	UButton* EquipmentCategoryButton = CreateShopTabButton(
		TEXT("EquipmentCategoryButton"),
		CreateEquipmentIcon(TEXT("EquipmentCategoryIcon"), FVector2D(48.0f, 40.0f)),
		CreatedEquipmentCategoryBackground);
	ShopEquipmentCategoryBackground = CreatedEquipmentCategoryBackground;
	EquipmentCategoryButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::HandleOpenEquipmentCategory);
	UHorizontalBoxSlot* EquipmentCategorySlot = CategoryRow->AddChildToHorizontalBox(EquipmentCategoryButton);
	EquipmentCategorySlot->SetPadding(FMargin(0.0f));

	RefreshShopCategoryTabs();

	UHorizontalBox* ProductRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("BuyProductRow"));
	UVerticalBoxSlot* ProductRowSlot = ShopContentPanel->AddChildToVerticalBox(ProductRow);
	ProductRowSlot->SetPadding(FMargin(0.0f));
	ProductRowSlot->SetHorizontalAlignment(HAlign_Left);

	if (CurrentShopBuyCategory == EGreenhouseShopBuyCategory::SeedPacks)
	{
		UButton* LilyButton = CreateShopProductButton(TEXT("BuyLilyProductButton"), EGreenhouseInventoryItem::Lily, 15, ProductRow);
		LilyButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::BuyPlants);
		return;
	}

	if (CurrentShopBuyCategory == EGreenhouseShopBuyCategory::Supplies)
	{
		UButton* SoilButton = CreateShopProductButton(TEXT("BuySoilBagProductButton"), EGreenhouseInventoryItem::SoilBag, 10, ProductRow);
		SoilButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::BuySoilBag);

		UButton* FertilizerButton = CreateShopProductButton(TEXT("BuyFertilizerBagProductButton"), EGreenhouseInventoryItem::FertilizerBag, 14, ProductRow);
		FertilizerButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::BuyFertilizerBag);
		return;
	}

	UButton* PotButton = CreateShopProductButton(TEXT("BuyEmptyPotProductButton"), EGreenhouseInventoryItem::EmptyPot, 12, ProductRow);
	PotButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::BuyEmptyPot);

	UButton* WateringCanButton = CreateShopProductButton(TEXT("BuyWateringCanProductButton"), EGreenhouseInventoryItem::WateringCan, 25, ProductRow);
	WateringCanButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::BuyWateringCan);

	UButton* TrowelButton = CreateShopProductButton(TEXT("BuyTrowelProductButton"), EGreenhouseInventoryItem::Trowel, 18, ProductRow);
	TrowelButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::BuyTrowel);

	UButton* SecateurButton = CreateShopProductButton(TEXT("BuySecateurProductButton"), EGreenhouseInventoryItem::Secateur, 22, ProductRow);
	SecateurButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::BuySecateur);
}

void UGreenhouseInventoryWidget::ShowShopBuyCategory(EGreenhouseShopBuyCategory Category)
{
	CurrentShopBuyCategory = Category;
	ShowPlantBuyPage();
}

void UGreenhouseInventoryWidget::ShowPlantSellPage()
{
	CurrentShopPage = EGreenhouseShopPage::SellPlants;
	RefreshShopHeader();
	RefreshShopTabs();
	SetShopStatus(FText::GetEmpty(), FLinearColor::White);
	if (!ShopContentPanel)
	{
		return;
	}

	ShopContentPanel->ClearChildren();

	const int32 LilyCount = CountItem(EGreenhouseInventoryItem::Lily);
	UHorizontalBox* ProductRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("SellProductRow"));
	UVerticalBoxSlot* ProductRowSlot = ShopContentPanel->AddChildToVerticalBox(ProductRow);
	ProductRowSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	ProductRowSlot->SetHorizontalAlignment(HAlign_Left);

	UButton* SellLilyButton = CreateShopProductButton(TEXT("SellLilyProductButton"), EGreenhouseInventoryItem::Lily, 8, ProductRow);
	SellLilyButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::SellLily);
	SellLilyButton->SetIsEnabled(LilyCount > 0);
}

void UGreenhouseInventoryWidget::ShowGardenSuppliesPage()
{
	ShowPlantBuyPage();
}

void UGreenhouseInventoryWidget::ToggleInventory()
{
	SetInventoryOpen(!bInventoryOpen);
}

void UGreenhouseInventoryWidget::SetInventoryOpen(bool bOpen)
{
	bInventoryOpen = bOpen;
	if (InventoryPanel)
	{
		InventoryPanel->SetVisibility(bInventoryOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	if (InventoryFrame)
	{
		InventoryFrame->SetVisibility(bInventoryOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	RefreshCursorPreview();
}

void UGreenhouseInventoryWidget::HandleSlotClicked(int32 SlotIndex)
{
	if (!Slots.IsValidIndex(SlotIndex))
	{
		return;
	}

	if (HeldItem == EGreenhouseInventoryItem::None)
	{
		HeldItem = Slots[SlotIndex];
		HeldItemStack = SlotStacks.IsValidIndex(SlotIndex) ? SlotStacks[SlotIndex] : 0;
		Slots[SlotIndex] = EGreenhouseInventoryItem::None;
		SlotStacks[SlotIndex] = 0;
	}
	else
	{
		Swap(HeldItem, Slots[SlotIndex]);
		Swap(HeldItemStack, SlotStacks[SlotIndex]);
	}

	RefreshSlots();
}

void UGreenhouseInventoryWidget::SelectHotbarSlot(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= HotbarSlotCount)
	{
		return;
	}

	SelectedHotbarSlot = SlotIndex;
	RefreshSlots();
}

EGreenhouseInventoryItem UGreenhouseInventoryWidget::GetSelectedHotbarItem() const
{
	return Slots.IsValidIndex(SelectedHotbarSlot) ? Slots[SelectedHotbarSlot] : EGreenhouseInventoryItem::None;
}

bool UGreenhouseInventoryWidget::ConsumeSelectedHotbarItem(EGreenhouseInventoryItem Item)
{
	if (!Slots.IsValidIndex(SelectedHotbarSlot) || Slots[SelectedHotbarSlot] != Item)
	{
		return false;
	}

	if (SlotStacks[SelectedHotbarSlot] > 1)
	{
		--SlotStacks[SelectedHotbarSlot];
	}
	else
	{
		Slots[SelectedHotbarSlot] = EGreenhouseInventoryItem::None;
		SlotStacks[SelectedHotbarSlot] = 0;
	}

	RefreshSlots();
	return true;
}

void UGreenhouseInventoryWidget::AddItem(EGreenhouseInventoryItem Item)
{
	if (IsUniqueInventoryItem(Item) && HasItem(Item))
	{
		return;
	}

	if (Item == EGreenhouseInventoryItem::None)
	{
		return;
	}

	if (IsStackableInventoryItem(Item))
	{
		for (int32 SlotIndex = 0; SlotIndex < Slots.Num(); ++SlotIndex)
		{
			if (Slots[SlotIndex] == Item && SlotStacks[SlotIndex] < MaxStackCount)
			{
				++SlotStacks[SlotIndex];
				RefreshSlots();
				return;
			}
		}
	}

	for (int32 SlotIndex = 0; SlotIndex < Slots.Num(); ++SlotIndex)
	{
		if (Slots[SlotIndex] == EGreenhouseInventoryItem::None)
		{
			Slots[SlotIndex] = Item;
			SlotStacks[SlotIndex] = 1;
			RefreshSlots();
			return;
		}
	}
}

void UGreenhouseInventoryWidget::SetWateringCanLiters(float CurrentLiters, float MaxLiters)
{
	constexpr float MaxGaugeHeight = 148.0f;
	const float SafeMaxLiters = FMath::Max(MaxLiters, 0.01f);
	const float ClampedLiters = FMath::Clamp(CurrentLiters, 0.0f, SafeMaxLiters);
	const float FillAlpha = ClampedLiters / SafeMaxLiters;

	if (WaterGaugeFillBox)
	{
		WaterGaugeFillBox->SetHeightOverride(MaxGaugeHeight * FillAlpha);
	}
	if (WaterGaugeText)
	{
		WaterGaugeText->SetText(FText::FromString(FString::Printf(TEXT("%.1fL"), ClampedLiters)));
	}
}

int32 UGreenhouseInventoryWidget::CountItem(EGreenhouseInventoryItem Item) const
{
	int32 Count = HeldItem == Item ? HeldItemStack : 0;
	for (int32 SlotIndex = 0; SlotIndex < Slots.Num(); ++SlotIndex)
	{
		if (Slots[SlotIndex] == Item)
		{
			Count += SlotStacks.IsValidIndex(SlotIndex) ? SlotStacks[SlotIndex] : 0;
		}
	}

	return Count;
}

bool UGreenhouseInventoryWidget::RemoveItem(EGreenhouseInventoryItem Item, int32 Count)
{
	if (Item == EGreenhouseInventoryItem::None || Count <= 0 || CountItem(Item) < Count)
	{
		return false;
	}

	if (HeldItem == Item)
	{
		const int32 RemovedFromHeld = FMath::Min(HeldItemStack, Count);
		HeldItemStack -= RemovedFromHeld;
		Count -= RemovedFromHeld;
		if (HeldItemStack <= 0)
		{
			HeldItem = EGreenhouseInventoryItem::None;
			HeldItemStack = 0;
		}
	}

	for (int32 SlotIndex = 0; SlotIndex < Slots.Num() && Count > 0; ++SlotIndex)
	{
		if (Slots[SlotIndex] != Item)
		{
			continue;
		}

		const int32 RemovedFromSlot = FMath::Min(SlotStacks[SlotIndex], Count);
		SlotStacks[SlotIndex] -= RemovedFromSlot;
		Count -= RemovedFromSlot;
		if (SlotStacks[SlotIndex] <= 0)
		{
			Slots[SlotIndex] = EGreenhouseInventoryItem::None;
			SlotStacks[SlotIndex] = 0;
		}
	}

	RefreshSlots();
	return true;
}

bool UGreenhouseInventoryWidget::TrySpendMoney(int32 Cost)
{
	if (Cost <= 0)
	{
		return true;
	}

	if (Money < Cost)
	{
		SetShopStatus(FText::FromString(TEXT("Not enough money.")), FLinearColor(0.95f, 0.38f, 0.32f, 1.0f));
		return false;
	}

	Money -= Cost;
	RefreshShopHeader();
	return true;
}

void UGreenhouseInventoryWidget::HandleShopBack()
{
	if (CurrentShopPage == EGreenhouseShopPage::Home)
	{
		CloseOnlineShop();
		return;
	}

	ShowShopHome();
}

void UGreenhouseInventoryWidget::HandleOpenPlantBuyPage()
{
	CurrentShopBuyCategory = EGreenhouseShopBuyCategory::SeedPacks;
	ShowPlantBuyPage();
}

void UGreenhouseInventoryWidget::HandleOpenPlantSellPage()
{
	ShowPlantSellPage();
}

void UGreenhouseInventoryWidget::HandleOpenGardenSuppliesPage()
{
	ShowGardenSuppliesPage();
}

void UGreenhouseInventoryWidget::HandleOpenSeedPacksCategory()
{
	ShowShopBuyCategory(EGreenhouseShopBuyCategory::SeedPacks);
}

void UGreenhouseInventoryWidget::HandleOpenSuppliesCategory()
{
	ShowShopBuyCategory(EGreenhouseShopBuyCategory::Supplies);
}

void UGreenhouseInventoryWidget::HandleOpenEquipmentCategory()
{
	ShowShopBuyCategory(EGreenhouseShopBuyCategory::Equipment);
}

void UGreenhouseInventoryWidget::IncreasePlantBuyQuantity()
{
	PlantBuyQuantity = FMath::Clamp(PlantBuyQuantity + 1, 1, 99);
	if (PlantBuyQuantityText)
	{
		PlantBuyQuantityText->SetText(FText::FromString(FString::Printf(TEXT("Quantity: %d"), PlantBuyQuantity)));
	}
}

void UGreenhouseInventoryWidget::DecreasePlantBuyQuantity()
{
	PlantBuyQuantity = FMath::Clamp(PlantBuyQuantity - 1, 1, 99);
	if (PlantBuyQuantityText)
	{
		PlantBuyQuantityText->SetText(FText::FromString(FString::Printf(TEXT("Quantity: %d"), PlantBuyQuantity)));
	}
}

void UGreenhouseInventoryWidget::BuyPlants()
{
	constexpr int32 LilyPrice = 15;
	const int32 TotalCost = PlantBuyQuantity * LilyPrice;
	if (!TrySpendMoney(TotalCost))
	{
		return;
	}

	for (int32 ItemIndex = 0; ItemIndex < PlantBuyQuantity; ++ItemIndex)
	{
		AddItem(EGreenhouseInventoryItem::Lily);
	}
	SetShopStatus(FText::FromString(FString::Printf(TEXT("Bought %d lilies for %d."), PlantBuyQuantity, TotalCost)), FLinearColor(0.65f, 0.96f, 0.66f, 1.0f));
}

void UGreenhouseInventoryWidget::SellLily()
{
	if (!RemoveItem(EGreenhouseInventoryItem::Lily, 1))
	{
		SetShopStatus(FText::FromString(TEXT("No lilies to sell.")), FLinearColor(0.95f, 0.38f, 0.32f, 1.0f));
		ShowPlantSellPage();
		return;
	}

	Money += 8;
	RefreshShopHeader();
	ShowPlantSellPage();
	SetShopStatus(FText::FromString(TEXT("Sold 1 lily for 8.")), FLinearColor(0.65f, 0.96f, 0.66f, 1.0f));
}

void UGreenhouseInventoryWidget::BuyEmptyPot()
{
	if (!TrySpendMoney(12))
	{
		return;
	}

	AddItem(EGreenhouseInventoryItem::EmptyPot);
	SetShopStatus(FText::FromString(TEXT("Bought 1 empty pot.")), FLinearColor(0.65f, 0.96f, 0.66f, 1.0f));
}

void UGreenhouseInventoryWidget::BuySoilBag()
{
	if (!TrySpendMoney(10))
	{
		return;
	}

	AddItem(EGreenhouseInventoryItem::SoilBag);
	SetShopStatus(FText::FromString(TEXT("Bought 1 soil bag.")), FLinearColor(0.65f, 0.96f, 0.66f, 1.0f));
}

void UGreenhouseInventoryWidget::BuyFertilizerBag()
{
	if (!TrySpendMoney(14))
	{
		return;
	}

	AddItem(EGreenhouseInventoryItem::FertilizerBag);
	SetShopStatus(FText::FromString(TEXT("Bought 1 fertilizer bag.")), FLinearColor(0.65f, 0.96f, 0.66f, 1.0f));
}

void UGreenhouseInventoryWidget::BuyWateringCan()
{
	if (HasItem(EGreenhouseInventoryItem::WateringCan))
	{
		SetShopStatus(FText::FromString(TEXT("You already have a watering can.")), FLinearColor(0.95f, 0.76f, 0.36f, 1.0f));
		return;
	}

	if (!TrySpendMoney(25))
	{
		return;
	}

	AddItem(EGreenhouseInventoryItem::WateringCan);
	SetShopStatus(FText::FromString(TEXT("Bought 1 watering can.")), FLinearColor(0.65f, 0.96f, 0.66f, 1.0f));
}

void UGreenhouseInventoryWidget::BuyTrowel()
{
	if (HasItem(EGreenhouseInventoryItem::Trowel))
	{
		SetShopStatus(FText::FromString(TEXT("You already have a trowel.")), FLinearColor(0.95f, 0.76f, 0.36f, 1.0f));
		return;
	}

	if (!TrySpendMoney(18))
	{
		return;
	}

	AddItem(EGreenhouseInventoryItem::Trowel);
	SetShopStatus(FText::FromString(TEXT("Bought 1 trowel.")), FLinearColor(0.65f, 0.96f, 0.66f, 1.0f));
}

void UGreenhouseInventoryWidget::BuySecateur()
{
	if (HasItem(EGreenhouseInventoryItem::Secateur))
	{
		SetShopStatus(FText::FromString(TEXT("You already have a secateur.")), FLinearColor(0.95f, 0.76f, 0.36f, 1.0f));
		return;
	}

	if (!TrySpendMoney(22))
	{
		return;
	}

	AddItem(EGreenhouseInventoryItem::Secateur);
	SetShopStatus(FText::FromString(TEXT("Bought 1 secateur.")), FLinearColor(0.65f, 0.96f, 0.66f, 1.0f));
}

bool UGreenhouseInventoryWidget::HasItem(EGreenhouseInventoryItem Item) const
{
	if (HeldItem == Item)
	{
		return true;
	}

	for (EGreenhouseInventoryItem SlotItem : Slots)
	{
		if (SlotItem == Item)
		{
			return true;
		}
	}

	return false;
}

void UGreenhouseInventoryWidget::RefreshSlots()
{
	for (int32 SlotIndex = 0; SlotIndex < SlotIcons.Num(); ++SlotIndex)
	{
		const EGreenhouseInventoryItem Item = Slots.IsValidIndex(SlotIndex) ? Slots[SlotIndex] : EGreenhouseInventoryItem::None;
		const int32 StackCount = SlotStacks.IsValidIndex(SlotIndex) ? SlotStacks[SlotIndex] : 0;

		if (SlotIcons[SlotIndex])
		{
			SlotIcons[SlotIndex]->SetItem(Item);
		}
		if (SlotStackTexts[SlotIndex])
		{
			SlotStackTexts[SlotIndex]->SetText(GetStackText(Item, StackCount));
		}
		if (SlotBackgrounds[SlotIndex])
		{
			SlotBackgrounds[SlotIndex]->SetBrushColor(GetSlotColor(SlotIndex));
		}
	}

	RefreshCursorPreview();
}

void UGreenhouseInventoryWidget::RefreshCursorPreview()
{
	const bool bShowCursorItem = bInventoryOpen && HeldItem != EGreenhouseInventoryItem::None;
	if (CursorPreview)
	{
		CursorPreview->SetVisibility(bShowCursorItem ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (CursorIcon)
	{
		CursorIcon->SetItem(bShowCursorItem ? HeldItem : EGreenhouseInventoryItem::None);
	}
	if (CursorStackText)
	{
		CursorStackText->SetText(bShowCursorItem ? GetStackText(HeldItem, HeldItemStack) : FText::GetEmpty());
	}
}

FText UGreenhouseInventoryWidget::GetStackText(EGreenhouseInventoryItem Item, int32 StackCount) const
{
	if (Item != EGreenhouseInventoryItem::None && StackCount > 1)
	{
		return FText::AsNumber(StackCount);
	}

	return FText::GetEmpty();
}

FLinearColor UGreenhouseInventoryWidget::GetSlotColor(int32 SlotIndex) const
{
	const bool bHotbarSlot = SlotIndex < HotbarSlotCount;
	if (bHotbarSlot && SlotIndex == SelectedHotbarSlot)
	{
		return FLinearColor(0.62f, 0.64f, 0.62f, 1.0f);
	}

	return FLinearColor(0.42f, 0.44f, 0.42f, 1.0f);
}

FLinearColor UGreenhouseInventoryWidget::GetSlotInnerColor(int32 SlotIndex) const
{
	const bool bHotbarSlot = SlotIndex < HotbarSlotCount;
	if (bHotbarSlot && SlotIndex == SelectedHotbarSlot)
	{
		return FLinearColor(0.005f, 0.005f, 0.005f, 1.0f);
	}

	return FLinearColor(0.005f, 0.005f, 0.005f, 1.0f);
}

void UGreenhouseInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CursorPreview || CursorPreview->GetVisibility() == ESlateVisibility::Collapsed)
	{
		return;
	}

	FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
	if (UCanvasPanelSlot* CursorSlot = Cast<UCanvasPanelSlot>(CursorPreview->Slot))
	{
		CursorSlot->SetPosition(MousePosition + FVector2D(14.0f, 14.0f));
	}
}

void UGreenhouseInventoryWidget::GiveLily()
{
	AddItem(EGreenhouseInventoryItem::Lily);
}

void UGreenhouseInventoryWidget::GiveWateringCan()
{
	AddItem(EGreenhouseInventoryItem::WateringCan);
}

void UGreenhouseInventoryWidget::GiveEmptyPot()
{
	AddItem(EGreenhouseInventoryItem::EmptyPot);
}

void UGreenhouseInventoryWidget::GiveSoilBag()
{
	AddItem(EGreenhouseInventoryItem::SoilBag);
}

void UGreenhouseInventoryWidget::GiveFertilizerBag()
{
	AddItem(EGreenhouseInventoryItem::FertilizerBag);
}

void UGreenhouseInventoryWidget::GiveTrowel()
{
	AddItem(EGreenhouseInventoryItem::Trowel);
}

void UGreenhouseInventoryWidget::GiveSecateur()
{
	AddItem(EGreenhouseInventoryItem::Secateur);
}
