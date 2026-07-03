#include "GreenhousePlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GreenhouseHeldItemActor.h"
#include "GreenhouseInventoryWidget.h"
#include "GreenhousePlantingPlotActor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
bool IsGardenFaucetMesh(const UStaticMesh* Mesh)
{
	if (!Mesh)
	{
		return false;
	}

	const FString MeshPath = Mesh->GetPathName();
	return MeshPath.Contains(TEXT("Garden_Faucet"), ESearchCase::IgnoreCase)
		|| MeshPath.Contains(TEXT("garden_faucet"), ESearchCase::IgnoreCase);
}

FVector GetGardenFaucetOutletLocal()
{
	// Keep the front axis centered, then nudge depth so the side view sits under the faucet outlet.
	return FVector(0.0f, 38.0f, 54.0f);
}

FVector GetWateringCanFillOpeningActorOffset()
{
	const FVector WateringCanMeshLocation(0.0f, 0.0f, -12.0f);
	const FRotator WateringCanMeshRotation(0.0f, 145.0f, 0.0f);
	constexpr float WateringCanMeshScale = 0.32f;
	const FVector FillOpeningLocal = FVector(0.05f, 0.0f, 1.205f) * 100.0f;

	return WateringCanMeshLocation + WateringCanMeshRotation.RotateVector(FillOpeningLocal * WateringCanMeshScale);
}

float GetFillingCanYawDegrees()
{
	// Watering can mesh has a 145 degree relative yaw; 125 flips the visible nozzle to world-left.
	constexpr float FixedLeftFacingYawDegrees = 125.0f;
	return FixedLeftFacingYawDegrees;
}

const FVector GreenhousePotSlotLocalCenters[] = {
	FVector(-2139.1f, -1216.5f, 911.6f),
	FVector(-1347.1f, -1216.5f, 911.6f),
	FVector(-555.1f, -1216.5f, 911.6f),
	FVector(236.9f, -1216.5f, 911.6f),
	FVector(1028.9f, -1216.5f, 911.6f),
	FVector(-2139.1f, 1180.9f, 911.6f),
	FVector(-1347.1f, 1180.9f, 911.6f),
	FVector(-555.1f, 1180.9f, 911.6f),
	FVector(236.9f, 1180.9f, 911.6f),
	FVector(1028.9f, 1180.9f, 911.6f)
};

const FVector StorageShelfPotSlotLocalCenters[] = {
	FVector(5.4f, 2.0f, 262.75f),
	FVector(62.4f, 2.0f, 262.75f),
	FVector(119.4f, 2.0f, 262.75f)
};

struct FStorageShelfPlacementSlotDefinition
{
	FVector InteractionLocalCenter;
	FVector PlacementLocalLocation;
};

const FStorageShelfPlacementSlotDefinition StorageShelfSoilBagSlotDefinitions[] = {
	{ FVector(-126.0f, 2.0f, 41.5f), FVector(-126.0f, 41.0f, 44.5f) },
	{ FVector(-42.0f, 2.0f, 41.5f), FVector(-42.0f, 41.0f, 44.5f) },
	{ FVector(42.0f, 2.0f, 41.5f), FVector(42.0f, 41.0f, 44.5f) },
	{ FVector(126.0f, 2.0f, 41.5f), FVector(126.0f, 41.0f, 44.5f) }
};

const FStorageShelfPlacementSlotDefinition StorageShelfFertilizerBagSlotDefinitions[] = {
	{ FVector(-126.0f, 2.0f, 151.5f), FVector(-126.0f, 41.0f, 154.5f) },
	{ FVector(-42.0f, 2.0f, 151.5f), FVector(-42.0f, 41.0f, 154.5f) },
	{ FVector(42.0f, 2.0f, 151.5f), FVector(42.0f, 41.0f, 154.5f) },
	{ FVector(126.0f, 2.0f, 151.5f), FVector(126.0f, 41.0f, 154.5f) }
};

const FStorageShelfPlacementSlotDefinition StorageShelfToolSlotDefinitions[] = {
	{ FVector(-116.0f, 2.0f, 262.75f), FVector(-116.0f, 2.0f, 266.0f) },
	{ FVector(-72.0f, 2.0f, 262.75f), FVector(-72.0f, 2.0f, 266.0f) }
};

constexpr float PotPlacementScale = 0.32f;
constexpr float StorageShelfPotPlacementScale = 0.22f;
constexpr float PotPlacementSnapRadiusCm = 58.0f;
constexpr float PotPlacementMaxVerticalOffsetCm = 45.0f;
constexpr float PotPlacementBenchFrontOffsetCm = 6.0f;
constexpr float PotPlacementLeftBenchBackCorrectionCm = 6.0f;
constexpr float PotPlacementSurfaceLiftCm = 1.0f;
constexpr int32 StorageShelfPotSlotIndexOffset = 100;
constexpr int32 StorageShelfSoilBagSlotIndexOffset = 200;
constexpr int32 StorageShelfFertilizerBagSlotIndexOffset = 220;
constexpr int32 StorageShelfToolSlotIndexOffset = 240;
constexpr float StorageShelfPotPlacementSnapRadiusCm = 42.0f;
constexpr float StorageShelfPotPlacementMaxVerticalOffsetCm = 36.0f;
constexpr float StorageShelfBagPlacementSnapRadiusCm = 54.0f;
constexpr float StorageShelfBagPlacementMaxVerticalOffsetCm = 42.0f;
constexpr float StorageShelfToolPlacementSnapRadiusCm = 46.0f;
constexpr float StorageShelfToolPlacementMaxVerticalOffsetCm = 34.0f;
constexpr uint32 GreenhousePlacementSlotKeyBase = 1;
constexpr uint32 StorageShelfPlacementSlotKeySalt = 0x5A000000;

bool IsGreenhouseMesh(const UStaticMesh* Mesh)
{
	if (!Mesh)
	{
		return false;
	}

	return Mesh->GetPathName().Contains(TEXT("greenhouse_5x3m"), ESearchCase::IgnoreCase);
}

bool IsStorageShelfMesh(const UStaticMesh* Mesh)
{
	if (!Mesh)
	{
		return false;
	}

	const FString MeshPath = Mesh->GetPathName();
	return MeshPath.Contains(TEXT("Storage_Shelf"), ESearchCase::IgnoreCase)
		|| MeshPath.Contains(TEXT("storage_shelf"), ESearchCase::IgnoreCase)
		|| MeshPath.Contains(TEXT("storge_shelf"), ESearchCase::IgnoreCase);
}

bool ContainsStorageShelfName(const FString& Name)
{
	return Name.Contains(TEXT("Storage_Shelf"), ESearchCase::IgnoreCase)
		|| Name.Contains(TEXT("storage_shelf"), ESearchCase::IgnoreCase)
		|| Name.Contains(TEXT("storge_shelf"), ESearchCase::IgnoreCase)
		|| Name.Contains(TEXT("StorageShelf"), ESearchCase::IgnoreCase);
}

bool IsStorageShelfHit(const FHitResult& Hit, const UStaticMesh* Mesh)
{
	if (IsStorageShelfMesh(Mesh))
	{
		return true;
	}

	const AActor* HitActor = Hit.GetActor();
	const UActorComponent* HitComponent = Hit.GetComponent();
	return (HitActor && ContainsStorageShelfName(HitActor->GetName()))
		|| (HitComponent && ContainsStorageShelfName(HitComponent->GetName()));
}

uint32 MakeGreenhousePlacementSlotKey(int32 SlotIndex)
{
	return GreenhousePlacementSlotKeyBase + static_cast<uint32>(SlotIndex);
}

uint32 MakeStorageShelfPlacementSlotKey(const AActor* ShelfActor, int32 SlotIndex)
{
	const uint32 ActorHash = ShelfActor ? GetTypeHash(ShelfActor) : 0u;
	return HashCombine(StorageShelfPlacementSlotKeySalt ^ ActorHash, static_cast<uint32>(SlotIndex));
}

UStaticMesh* LoadEmptyPotPlacementMesh()
{
	if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/models/equipment/pots/ornament_plants/empty/ornament_plants_pot_empty.ornament_plants_pot_empty")))
	{
		return Mesh;
	}

	return LoadObject<UStaticMesh>(nullptr, TEXT("/Game/models/equipment/pots/ornament plants/empty/ornament_plants_pot_empty.ornament_plants_pot_empty"));
}

UStaticMesh* LoadPlacementMeshForItem(EGreenhouseInventoryItem Item)
{
	switch (Item)
	{
	case EGreenhouseInventoryItem::EmptyPot:
		return LoadEmptyPotPlacementMesh();
	case EGreenhouseInventoryItem::SoilBag:
		return LoadObject<UStaticMesh>(nullptr, TEXT("/Game/models/equipment/soil/ornament_plants/ornament_plants_soil.ornament_plants_soil"));
	case EGreenhouseInventoryItem::FertilizerBag:
		return LoadObject<UStaticMesh>(nullptr, TEXT("/Game/models/equipment/fertilizer/ornament_plants/ornament_plants_fertilizer.ornament_plants_fertilizer"));
	case EGreenhouseInventoryItem::Trowel:
		return LoadObject<UStaticMesh>(nullptr, TEXT("/Game/models/equipment/Trowel/trowel.trowel"));
	case EGreenhouseInventoryItem::Secateur:
		return LoadObject<UStaticMesh>(nullptr, TEXT("/Game/models/equipment/Secateur/Secateur/secateur.secateur"));
	default:
		return nullptr;
	}
}

bool IsPlaceableInventoryItem(EGreenhouseInventoryItem Item)
{
	return Item == EGreenhouseInventoryItem::EmptyPot
		|| Item == EGreenhouseInventoryItem::SoilBag
		|| Item == EGreenhouseInventoryItem::FertilizerBag
		|| Item == EGreenhouseInventoryItem::Trowel
		|| Item == EGreenhouseInventoryItem::Secateur;
}

float GetPlacementScaleForItem(EGreenhouseInventoryItem Item)
{
	switch (Item)
	{
	case EGreenhouseInventoryItem::SoilBag:
	case EGreenhouseInventoryItem::FertilizerBag:
		return 0.26f;
	case EGreenhouseInventoryItem::Trowel:
		return 0.34f;
	case EGreenhouseInventoryItem::Secateur:
		return 0.42f;
	default:
		return PotPlacementScale;
	}
}

FRotator GetStorageShelfPlacementRotation(EGreenhouseInventoryItem Item, float ShelfYawDegrees)
{
	switch (Item)
	{
	case EGreenhouseInventoryItem::SoilBag:
	case EGreenhouseInventoryItem::FertilizerBag:
		return FRotator(0.0f, ShelfYawDegrees, -90.0f);
	case EGreenhouseInventoryItem::Trowel:
		return FRotator(0.0f, ShelfYawDegrees + 90.0f, -72.0f);
	case EGreenhouseInventoryItem::Secateur:
		return FRotator(0.0f, ShelfYawDegrees + 90.0f, -66.0f);
	default:
		return FRotator(0.0f, ShelfYawDegrees, 0.0f);
	}
}
}

AGreenhousePlayerController::AGreenhousePlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultMappingContextFinder(TEXT("/Game/Input/IMC_Default"));
	if (DefaultMappingContextFinder.Succeeded())
	{
		DefaultMappingContext = DefaultMappingContextFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MouseLookMappingContextFinder(TEXT("/Game/Input/IMC_MouseLook"));
	if (MouseLookMappingContextFinder.Succeeded())
	{
		MouseLookMappingContext = MouseLookMappingContextFinder.Object;
	}
}

void AGreenhousePlayerController::BeginPlay()
{
	Super::BeginPlay();

	RegisterInputMappingContexts();
	ApplyInitialSpawnView();
	ConfigurePlayerCollision();

	InventoryWidget = CreateWidget<UGreenhouseInventoryWidget>(this, UGreenhouseInventoryWidget::StaticClass());
	if (InventoryWidget)
	{
		InventoryWidget->AddToViewport();
		ApplyInventoryInputMode(false);
	}

	SpawnHeldItemActor();
	SpawnFillingWaterStreamActor();
	UpdateHeldItemActor();
}

void AGreenhousePlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	UpdateWateringCanState(DeltaTime);
	if (InventoryWidget)
	{
		InventoryWidget->SetWateringCanLiters(WateringCanLiters, MaxWateringCanLiters);
	}
	UpdateHeldItemActor();
}

void AGreenhousePlayerController::RegisterInputMappingContexts()
{
	if (!GetLocalPlayer())
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!InputSubsystem)
	{
		return;
	}

	if (DefaultMappingContext)
	{
		InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	if (MouseLookMappingContext)
	{
		InputSubsystem->AddMappingContext(MouseLookMappingContext, 1);
	}
}

void AGreenhousePlayerController::ApplyInitialSpawnView()
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	FRotator SpawnViewRotation = ControlledPawn->GetActorRotation();
	SpawnViewRotation.Pitch = 0.0f;
	SpawnViewRotation.Roll = 0.0f;
	SetControlRotation(SpawnViewRotation);
}

void AGreenhousePlayerController::ConfigurePlayerCollision()
{
	ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn());
	if (!ControlledCharacter)
	{
		return;
	}

	if (UCapsuleComponent* CapsuleComponent = ControlledCharacter->GetCapsuleComponent())
	{
		constexpr float DoorFriendlyRadiusCm = 24.0f;
		constexpr float FirstPersonHalfHeightCm = 90.0f;
		CapsuleComponent->SetCapsuleSize(DoorFriendlyRadiusCm, FirstPersonHalfHeightCm, true);
	}

	if (UCameraComponent* CameraComponent = ControlledCharacter->FindComponentByClass<UCameraComponent>())
	{
		constexpr float EyeHeightCm = 170.0f;
		constexpr float FirstPersonHalfHeightCm = 90.0f;
		CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, EyeHeightCm - FirstPersonHalfHeightCm));
	}

	if (UCharacterMovementComponent* MovementComponent = ControlledCharacter->GetCharacterMovement())
	{
		MovementComponent->MaxStepHeight = 90.0f;
		MovementComponent->PerchAdditionalHeight = 45.0f;
		MovementComponent->PerchRadiusThreshold = 0.0f;
		MovementComponent->SetWalkableFloorAngle(60.0f);
		MovementComponent->bUseFlatBaseForFloorChecks = false;
	}
}

void AGreenhousePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	check(InputComponent);
	InputComponent->BindKey(EKeys::E, IE_Pressed, this, &AGreenhousePlayerController::HandleInteractOrInventory);
	InputComponent->BindKey(EKeys::F, IE_Pressed, this, &AGreenhousePlayerController::HandleWateringCanFillPressed);
	InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AGreenhousePlayerController::SelectHotbarSlotOne);
	InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AGreenhousePlayerController::SelectHotbarSlotTwo);
	InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AGreenhousePlayerController::SelectHotbarSlotThree);
	InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &AGreenhousePlayerController::SelectHotbarSlotFour);
	InputComponent->BindKey(EKeys::Five, IE_Pressed, this, &AGreenhousePlayerController::SelectHotbarSlotFive);
	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AGreenhousePlayerController::HandleComputerTerminalExitPressed);
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AGreenhousePlayerController::HandleWateringCanPressed);
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &AGreenhousePlayerController::HandleWateringCanReleased);
	InputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &AGreenhousePlayerController::StartSprint);
	InputComponent->BindKey(EKeys::LeftShift, IE_Released, this, &AGreenhousePlayerController::StopSprint);
	InputComponent->BindKey(EKeys::RightShift, IE_Pressed, this, &AGreenhousePlayerController::StartSprint);
	InputComponent->BindKey(EKeys::RightShift, IE_Released, this, &AGreenhousePlayerController::StopSprint);
}

void AGreenhousePlayerController::ToggleInventory()
{
	if (!InventoryWidget)
	{
		return;
	}

	InventoryWidget->ToggleInventory();
	ApplyInventoryInputMode(InventoryWidget->IsInventoryOpen());
}

void AGreenhousePlayerController::HandleInteractOrInventory()
{
	if (bComputerTerminalOpen || (InventoryWidget && InventoryWidget->IsShopOpen()))
	{
		return;
	}

	if (InventoryWidget && !InventoryWidget->IsInventoryOpen() && TryPlantSelectedLily())
	{
		return;
	}

	ToggleInventory();
}

bool AGreenhousePlayerController::TryPlaceSelectedItem()
{
	if (!InventoryWidget || !GetWorld())
	{
		return false;
	}

	const EGreenhouseInventoryItem SelectedItem = InventoryWidget->GetSelectedHotbarItem();
	if (!IsPlaceableInventoryItem(SelectedItem))
	{
		return false;
	}

	FHitResult InteractionHit;
	if (!TraceForInteraction(InteractionHit) || !InteractionHit.bBlockingHit)
	{
		return false;
	}

	FTransform PlacementTransform;
	uint32 SlotKey = 0;
	if (!FindItemPlacementSlot(SelectedItem, InteractionHit, PlacementTransform, SlotKey))
	{
		return false;
	}

	UStaticMesh* PlacementMesh = LoadPlacementMeshForItem(SelectedItem);
	if (!PlacementMesh || !InventoryWidget->ConsumeSelectedHotbarItem(SelectedItem))
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AStaticMeshActor* PlacedActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), PlacementTransform, SpawnParameters);
	if (!PlacedActor)
	{
		InventoryWidget->AddItem(SelectedItem);
		return false;
	}

	if (UStaticMeshComponent* MeshComponent = PlacedActor->GetStaticMeshComponent())
	{
		MeshComponent->SetMobility(EComponentMobility::Movable);
		MeshComponent->SetStaticMesh(PlacementMesh);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->SetCastShadow(true);
	}

	PlacedPotActors.Add(PlacedActor);
	OccupiedPlacementSlotKeys.Add(SlotKey);
	UpdateHeldItemActor();
	return true;
}

bool AGreenhousePlayerController::TryPlantSelectedLily()
{
	if (!InventoryWidget || InventoryWidget->GetSelectedHotbarItem() != EGreenhouseInventoryItem::Lily)
	{
		return false;
	}

	const APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || !GetWorld())
	{
		return false;
	}

	AGreenhousePlantingPlotActor* ClosestPlot = nullptr;
	float ClosestDistanceSq = TNumericLimits<float>::Max();
	const FVector PlayerLocation = ControlledPawn->GetActorLocation();

	for (TActorIterator<AGreenhousePlantingPlotActor> PlotIt(GetWorld()); PlotIt; ++PlotIt)
	{
		AGreenhousePlantingPlotActor* Plot = *PlotIt;
		if (!Plot || !Plot->CanPlantAt(PlayerLocation))
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared2D(PlayerLocation, Plot->GetActorLocation());
		if (DistanceSq < ClosestDistanceSq)
		{
			ClosestDistanceSq = DistanceSq;
			ClosestPlot = Plot;
		}
	}

	if (!ClosestPlot || !InventoryWidget->ConsumeSelectedHotbarItem(EGreenhouseInventoryItem::Lily))
	{
		return false;
	}

	const bool bPlanted = ClosestPlot->PlantLily();
	if (!bPlanted)
	{
		InventoryWidget->AddItem(EGreenhouseInventoryItem::Lily);
	}

	UpdateHeldItemActor();
	return bPlanted;
}

bool AGreenhousePlayerController::FindItemPlacementSlot(EGreenhouseInventoryItem SelectedItem, const FHitResult& InteractionHit, FTransform& OutPlacementTransform, uint32& OutSlotKey) const
{
	const UStaticMeshComponent* HitMeshComponent = Cast<UStaticMeshComponent>(InteractionHit.GetComponent());
	if (!HitMeshComponent)
	{
		return false;
	}

	const UStaticMesh* HitMesh = HitMeshComponent->GetStaticMesh();
	const FTransform ComponentTransform = HitMeshComponent->GetComponentTransform();
	const FVector HitLocation = InteractionHit.ImpactPoint;

	if (IsGreenhouseMesh(HitMesh))
	{
		if (SelectedItem != EGreenhouseInventoryItem::EmptyPot)
		{
			return false;
		}

		const float MaxDistanceSq = FMath::Square(PotPlacementSnapRadiusCm);

		int32 ClosestSlotIndex = INDEX_NONE;
		float ClosestDistanceSq = TNumericLimits<float>::Max();
		FVector ClosestSlotLocation = FVector::ZeroVector;
		for (int32 SlotIndex = 0; SlotIndex < UE_ARRAY_COUNT(GreenhousePotSlotLocalCenters); ++SlotIndex)
		{
			if (IsPlacementSlotOccupied(MakeGreenhousePlacementSlotKey(SlotIndex)))
			{
				continue;
			}

			const FVector SlotLocation = ComponentTransform.TransformPosition(GreenhousePotSlotLocalCenters[SlotIndex]);
			const float VerticalOffset = FMath::Abs(HitLocation.Z - SlotLocation.Z);
			if (VerticalOffset > PotPlacementMaxVerticalOffsetCm)
			{
				continue;
			}

			const float DistanceSq = FVector::DistSquared2D(HitLocation, SlotLocation);
			if (DistanceSq < ClosestDistanceSq)
			{
				ClosestDistanceSq = DistanceSq;
				ClosestSlotIndex = SlotIndex;
				ClosestSlotLocation = SlotLocation;
			}
		}

		if (ClosestSlotIndex == INDEX_NONE || ClosestDistanceSq > MaxDistanceSq)
		{
			return false;
		}

		const FRotator SlotRotation(0.0f, HitMeshComponent->GetComponentRotation().Yaw, 0.0f);
		const bool bFirstBenchRow = ClosestSlotIndex < 5;
		const FVector LocalBenchFrontDirection = bFirstBenchRow ? FVector::YAxisVector : -FVector::YAxisVector;
		const FVector BenchFrontDirection = ComponentTransform.TransformVectorNoScale(LocalBenchFrontDirection).GetSafeNormal();
		const float BenchFrontOffset = bFirstBenchRow
			? PotPlacementBenchFrontOffsetCm
			: FMath::Max(0.0f, PotPlacementBenchFrontOffsetCm - PotPlacementLeftBenchBackCorrectionCm);
		OutPlacementTransform = FTransform(
			SlotRotation,
			ClosestSlotLocation + BenchFrontDirection * BenchFrontOffset + FVector(0.0f, 0.0f, PotPlacementSurfaceLiftCm),
			FVector(PotPlacementScale));
		OutSlotKey = MakeGreenhousePlacementSlotKey(ClosestSlotIndex);
		return true;
	}

	if (IsStorageShelfHit(InteractionHit, HitMesh))
	{
		const AActor* ShelfActor = InteractionHit.GetActor();
		if (SelectedItem == EGreenhouseInventoryItem::EmptyPot)
		{
			const float MaxDistanceSq = FMath::Square(StorageShelfPotPlacementSnapRadiusCm);

			int32 ClosestShelfSlotIndex = INDEX_NONE;
			float ClosestDistanceSq = TNumericLimits<float>::Max();
			FVector ClosestSlotLocation = FVector::ZeroVector;
			for (int32 SlotIndex = 0; SlotIndex < UE_ARRAY_COUNT(StorageShelfPotSlotLocalCenters); ++SlotIndex)
			{
				const int32 SlotKeyIndex = StorageShelfPotSlotIndexOffset + SlotIndex;
				if (IsPlacementSlotOccupied(MakeStorageShelfPlacementSlotKey(ShelfActor, SlotKeyIndex)))
				{
					continue;
				}

				const FVector SlotLocation = ComponentTransform.TransformPosition(StorageShelfPotSlotLocalCenters[SlotIndex]);
				const float VerticalOffset = FMath::Abs(HitLocation.Z - SlotLocation.Z);
				if (VerticalOffset > StorageShelfPotPlacementMaxVerticalOffsetCm)
				{
					continue;
				}

				const float DistanceSq = FVector::DistSquared2D(HitLocation, SlotLocation);
				if (DistanceSq < ClosestDistanceSq)
				{
					ClosestDistanceSq = DistanceSq;
					ClosestShelfSlotIndex = SlotIndex;
					ClosestSlotLocation = SlotLocation;
				}
			}

			if (ClosestShelfSlotIndex == INDEX_NONE || ClosestDistanceSq > MaxDistanceSq)
			{
				return false;
			}

			const FRotator SlotRotation(0.0f, HitMeshComponent->GetComponentRotation().Yaw, 0.0f);
			OutPlacementTransform = FTransform(
				SlotRotation,
				ClosestSlotLocation + FVector(0.0f, 0.0f, PotPlacementSurfaceLiftCm),
				FVector(StorageShelfPotPlacementScale));
			OutSlotKey = MakeStorageShelfPlacementSlotKey(ShelfActor, StorageShelfPotSlotIndexOffset + ClosestShelfSlotIndex);
			return true;
		}

		const FStorageShelfPlacementSlotDefinition* SlotDefinitions = nullptr;
		int32 SlotCount = 0;
		int32 SlotIndexOffset = INDEX_NONE;
		float SnapRadiusCm = 0.0f;
		float MaxVerticalOffsetCm = 0.0f;

		switch (SelectedItem)
		{
		case EGreenhouseInventoryItem::SoilBag:
			SlotDefinitions = StorageShelfSoilBagSlotDefinitions;
			SlotCount = UE_ARRAY_COUNT(StorageShelfSoilBagSlotDefinitions);
			SlotIndexOffset = StorageShelfSoilBagSlotIndexOffset;
			SnapRadiusCm = StorageShelfBagPlacementSnapRadiusCm;
			MaxVerticalOffsetCm = StorageShelfBagPlacementMaxVerticalOffsetCm;
			break;
		case EGreenhouseInventoryItem::FertilizerBag:
			SlotDefinitions = StorageShelfFertilizerBagSlotDefinitions;
			SlotCount = UE_ARRAY_COUNT(StorageShelfFertilizerBagSlotDefinitions);
			SlotIndexOffset = StorageShelfFertilizerBagSlotIndexOffset;
			SnapRadiusCm = StorageShelfBagPlacementSnapRadiusCm;
			MaxVerticalOffsetCm = StorageShelfBagPlacementMaxVerticalOffsetCm;
			break;
		case EGreenhouseInventoryItem::Trowel:
		case EGreenhouseInventoryItem::Secateur:
			SlotDefinitions = StorageShelfToolSlotDefinitions;
			SlotCount = UE_ARRAY_COUNT(StorageShelfToolSlotDefinitions);
			SlotIndexOffset = StorageShelfToolSlotIndexOffset;
			SnapRadiusCm = StorageShelfToolPlacementSnapRadiusCm;
			MaxVerticalOffsetCm = StorageShelfToolPlacementMaxVerticalOffsetCm;
			break;
		default:
			return false;
		}

		int32 ClosestShelfSlotIndex = INDEX_NONE;
		float ClosestDistanceSq = TNumericLimits<float>::Max();
		FVector ClosestPlacementLocation = FVector::ZeroVector;
		for (int32 SlotIndex = 0; SlotIndex < SlotCount; ++SlotIndex)
		{
			const int32 SlotKeyIndex = SlotIndexOffset + SlotIndex;
			if (IsPlacementSlotOccupied(MakeStorageShelfPlacementSlotKey(ShelfActor, SlotKeyIndex)))
			{
				continue;
			}

			const FVector InteractionSlotLocation = ComponentTransform.TransformPosition(SlotDefinitions[SlotIndex].InteractionLocalCenter);
			const float VerticalOffset = FMath::Abs(HitLocation.Z - InteractionSlotLocation.Z);
			if (VerticalOffset > MaxVerticalOffsetCm)
			{
				continue;
			}

			const float DistanceSq = FVector::DistSquared2D(HitLocation, InteractionSlotLocation);
			if (DistanceSq < ClosestDistanceSq)
			{
				ClosestDistanceSq = DistanceSq;
				ClosestShelfSlotIndex = SlotIndex;
				ClosestPlacementLocation = ComponentTransform.TransformPosition(SlotDefinitions[SlotIndex].PlacementLocalLocation);
			}
		}

		if (ClosestShelfSlotIndex == INDEX_NONE || ClosestDistanceSq > FMath::Square(SnapRadiusCm))
		{
			return false;
		}

		const FRotator SlotRotation = GetStorageShelfPlacementRotation(SelectedItem, HitMeshComponent->GetComponentRotation().Yaw);
		OutPlacementTransform = FTransform(
			SlotRotation,
			ClosestPlacementLocation + FVector(0.0f, 0.0f, PotPlacementSurfaceLiftCm),
			FVector(GetPlacementScaleForItem(SelectedItem)));
		OutSlotKey = MakeStorageShelfPlacementSlotKey(ShelfActor, SlotIndexOffset + ClosestShelfSlotIndex);
		return true;
	}

	return false;
}

bool AGreenhousePlayerController::IsPlacementSlotOccupied(uint32 SlotKey) const
{
	return OccupiedPlacementSlotKeys.Contains(SlotKey);
}

void AGreenhousePlayerController::HandleComputerShopPressed()
{
	if (!InventoryWidget)
	{
		return;
	}

	if (bComputerTerminalOpen || InventoryWidget->IsShopOpen())
	{
		CloseComputerTerminal();
		return;
	}

	if (InventoryWidget->IsInventoryOpen())
	{
		return;
	}

	FHitResult InteractionHit;
	if (!TraceForInteraction(InteractionHit) || !InteractionHit.bBlockingHit || !IsComputerScreenHit(InteractionHit))
	{
		return;
	}

	OpenComputerTerminal(InteractionHit);
}

void AGreenhousePlayerController::HandleComputerTerminalExitPressed()
{
	if (bComputerTerminalOpen || (InventoryWidget && InventoryWidget->IsShopOpen()))
	{
		CloseComputerTerminal();
	}
}

void AGreenhousePlayerController::OpenComputerTerminal(const FHitResult& ScreenHit)
{
	if (!InventoryWidget || !GetWorld())
	{
		return;
	}

	if (!ComputerTerminalCameraActor)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ComputerTerminalCameraActor = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), BuildComputerTerminalCameraTransform(ScreenHit), SpawnParameters);
	}
	else
	{
		ComputerTerminalCameraActor->SetActorTransform(BuildComputerTerminalCameraTransform(ScreenHit));
	}

	if (!ComputerTerminalCameraActor)
	{
		return;
	}

	StopPouringWater();
	if (bIsFillingWateringCan)
	{
		FinishFillingWateringCan();
	}

	bComputerTerminalOpen = true;
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);

	if (ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn()))
	{
		if (UCharacterMovementComponent* MovementComponent = ControlledCharacter->GetCharacterMovement())
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->DisableMovement();
		}
	}

	SetViewTargetWithBlend(ComputerTerminalCameraActor, 0.45f, VTBlend_Cubic, 0.0f, false);
	InventoryWidget->OpenOnlineShop();
	ApplyInventoryInputMode(true);
}

void AGreenhousePlayerController::CloseComputerTerminal()
{
	bComputerTerminalOpen = false;

	if (InventoryWidget)
	{
		InventoryWidget->CloseOnlineShop();
	}

	ApplyInventoryInputMode(false);
	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);

	if (ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn()))
	{
		if (UCharacterMovementComponent* MovementComponent = ControlledCharacter->GetCharacterMovement())
		{
			MovementComponent->SetMovementMode(MOVE_Walking);
		}

		SetViewTargetWithBlend(ControlledCharacter, 0.35f, VTBlend_Cubic, 0.0f, false);
	}
}

FTransform AGreenhousePlayerController::BuildComputerTerminalCameraTransform(const FHitResult& ScreenHit) const
{
	const FVector ScreenPoint = !ScreenHit.ImpactPoint.IsNearlyZero() ? ScreenHit.ImpactPoint : ScreenHit.Location;
	FVector ScreenNormal = ScreenHit.ImpactNormal.GetSafeNormal();

	if (ScreenNormal.IsNearlyZero())
	{
		ScreenNormal = PlayerCameraManager ? -PlayerCameraManager->GetCameraRotation().Vector() : FVector::BackwardVector;
	}

	if (PlayerCameraManager)
	{
		const FVector CurrentCameraToScreen = PlayerCameraManager->GetCameraLocation() - ScreenPoint;
		if (FVector::DotProduct(ScreenNormal, CurrentCameraToScreen) < 0.0f)
		{
			ScreenNormal *= -1.0f;
		}
	}

	const FVector CameraLocation = ScreenPoint + ScreenNormal * 92.0f + FVector(0.0f, 0.0f, 8.0f);
	FRotator CameraRotation = (ScreenPoint - CameraLocation).Rotation();
	CameraRotation.Roll = 0.0f;

	return FTransform(CameraRotation, CameraLocation);
}

void AGreenhousePlayerController::HandleWateringCanFillPressed()
{
	if (bComputerTerminalOpen || (InventoryWidget && InventoryWidget->IsShopOpen()))
	{
		CloseComputerTerminal();
		return;
	}

	if (InventoryWidget && InventoryWidget->IsInventoryOpen())
	{
		return;
	}

	FHitResult InteractionHit;
	if (TraceForInteraction(InteractionHit) && InteractionHit.bBlockingHit && IsComputerScreenHit(InteractionHit))
	{
		OpenComputerTerminal(InteractionHit);
		return;
	}

	if (TryPlaceSelectedItem())
	{
		return;
	}

	TryStartFillingSelectedWateringCan();
}

bool AGreenhousePlayerController::TryStartFillingSelectedWateringCan()
{
	if (!HasSelectedWateringCan() || bIsFillingWateringCan || WateringCanLiters >= MaxWateringCanLiters - 0.05f)
	{
		return false;
	}

	FHitResult InteractionHit;
	if (!TraceForInteraction(InteractionHit) || !InteractionHit.bBlockingHit || !IsFaucetHit(InteractionHit))
	{
		return false;
	}

	StartFillingWateringCan(InteractionHit);
	return true;
}

void AGreenhousePlayerController::HandleWateringCanPressed()
{
	if (InventoryWidget && (InventoryWidget->IsInventoryOpen() || InventoryWidget->IsShopOpen()))
	{
		return;
	}

	if (!HasSelectedWateringCan() || bIsFillingWateringCan)
	{
		return;
	}

	FHitResult InteractionHit;
	TraceForInteraction(InteractionHit);
	StartPouringWater(InteractionHit);
}

void AGreenhousePlayerController::HandleWateringCanReleased()
{
	StopPouringWater();
}

void AGreenhousePlayerController::ApplyInventoryInputMode(bool bInventoryOpen)
{
	bShowMouseCursor = bInventoryOpen;

	if (bInventoryOpen)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		if (InventoryWidget)
		{
			InputMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
		}
		SetInputMode(InputMode);
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
	}
}

void AGreenhousePlayerController::SpawnHeldItemActor()
{
	if (HeldItemActor || !GetWorld())
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	HeldItemActor = GetWorld()->SpawnActor<AGreenhouseHeldItemActor>(AGreenhouseHeldItemActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	if (HeldItemActor)
	{
		HeldItemActor->SetActorHiddenInGame(true);
	}
}

void AGreenhousePlayerController::SpawnFillingWaterStreamActor()
{
	if (FillingWaterStreamActor || !GetWorld())
	{
		return;
	}

	UStaticMesh* CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (!CylinderMesh)
	{
		return;
	}

	UMaterialInterface* WaterMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/models/equipment/Watering_Can/Water_In_The_Watering_Can.Water_In_The_Watering_Can"));
	const auto ConfigureWaterActor = [CylinderMesh, WaterMaterial](AStaticMeshActor* WaterActor)
	{
		if (!WaterActor)
		{
			return;
		}

		UStaticMeshComponent* MeshComponent = WaterActor->GetStaticMeshComponent();
		MeshComponent->SetStaticMesh(CylinderMesh);
		MeshComponent->SetMobility(EComponentMobility::Movable);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->SetCastShadow(false);
		if (WaterMaterial)
		{
			MeshComponent->SetMaterial(0, WaterMaterial);
		}
		else if (UMaterialInstanceDynamic* DynamicWaterMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0))
		{
			DynamicWaterMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.18f, 0.55f, 1.0f, 0.76f));
			DynamicWaterMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.18f, 0.55f, 1.0f, 0.76f));
			DynamicWaterMaterial->SetScalarParameterValue(TEXT("Opacity"), 0.70f);
		}
	};

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FillingWaterStreamActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	if (!FillingWaterStreamActor)
	{
		return;
	}

	ConfigureWaterActor(FillingWaterStreamActor);
	FillingWaterStreamActor->SetActorHiddenInGame(true);

	FillingWaterFlowActors.Reset();
	constexpr int32 FlowSegmentCount = 4;
	FillingWaterFlowActors.Reserve(FlowSegmentCount);
	for (int32 SegmentIndex = 0; SegmentIndex < FlowSegmentCount; ++SegmentIndex)
	{
		AStaticMeshActor* FlowActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
		if (!FlowActor)
		{
			continue;
		}

		ConfigureWaterActor(FlowActor);
		FlowActor->SetActorHiddenInGame(true);
		FillingWaterFlowActors.Add(FlowActor);
	}
}

USceneComponent* AGreenhousePlayerController::FindFirstPersonCameraComponent() const
{
	const APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return nullptr;
	}

	return ControlledPawn->FindComponentByClass<UCameraComponent>();
}

void AGreenhousePlayerController::UpdateWateringCanState(float DeltaTime)
{
	if (bIsFillingWateringCan)
	{
		WateringCanFillSeconds = FMath::Min(WateringCanFillSeconds + DeltaTime, WateringCanFillDuration);
		const float FillAlpha = WateringCanFillDuration > 0.0f ? WateringCanFillSeconds / WateringCanFillDuration : 1.0f;
		WateringCanLiters = FMath::Lerp(WateringCanFillStartLiters, MaxWateringCanLiters, FillAlpha);

		if (FillAlpha >= 1.0f)
		{
			FinishFillingWateringCan();
		}
		return;
	}

	if (!bIsPouringWater)
	{
		return;
	}

	PouringVisualSecondsRemaining = FMath::Max(0.0f, PouringVisualSecondsRemaining - DeltaTime);
	WateringCanLiters = FMath::Max(0.0f, WateringCanLiters - DeltaTime * WateringLitersPerSecond);
	if (CurrentWateringPlot.IsValid())
	{
		CurrentWateringPlot->WaterPlant(DeltaTime);
	}

	if (WateringCanLiters <= 0.0f)
	{
		PouringVisualSecondsRemaining = 0.0f;
		bStopPouringWhenVisualEnds = false;
		StopPouringWater();
	}
	else if (bStopPouringWhenVisualEnds && PouringVisualSecondsRemaining <= 0.0f)
	{
		StopPouringWater();
	}
}

bool AGreenhousePlayerController::HasSelectedWateringCan() const
{
	return InventoryWidget && InventoryWidget->GetSelectedHotbarItem() == EGreenhouseInventoryItem::WateringCan;
}

bool AGreenhousePlayerController::TraceForInteraction(FHitResult& OutHit) const
{
	if (!PlayerCameraManager || !GetWorld())
	{
		return false;
	}

	const FVector TraceStart = PlayerCameraManager->GetCameraLocation();
	const FVector TraceEnd = TraceStart + PlayerCameraManager->GetCameraRotation().Vector() * InteractionTraceDistance;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GreenhouseWateringInteraction), false);
	if (const APawn* ControlledPawn = GetPawn())
	{
		QueryParams.AddIgnoredActor(ControlledPawn);
	}
	if (HeldItemActor)
	{
		QueryParams.AddIgnoredActor(HeldItemActor);
	}
	for (const AStaticMeshActor* PlacedPotActor : PlacedPotActors)
	{
		if (PlacedPotActor)
		{
			QueryParams.AddIgnoredActor(PlacedPotActor);
		}
	}

	return GetWorld()->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
}

bool AGreenhousePlayerController::IsFaucetHit(const FHitResult& Hit) const
{
	const AActor* HitActor = Hit.GetActor();
	const UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(Hit.GetComponent());
	const UStaticMesh* Mesh = MeshComponent ? MeshComponent->GetStaticMesh() : nullptr;

	const FString ActorName = HitActor ? HitActor->GetName() : FString();
	const FString ComponentName = MeshComponent ? MeshComponent->GetName() : FString();
	const FString MeshPath = Mesh ? Mesh->GetPathName() : FString();

	return ActorName.Contains(TEXT("Faucet"), ESearchCase::IgnoreCase)
		|| ActorName.Contains(TEXT("WaterTap"), ESearchCase::IgnoreCase)
		|| ActorName.Contains(TEXT("Wasserhahn"), ESearchCase::IgnoreCase)
		|| ComponentName.Contains(TEXT("Faucet"), ESearchCase::IgnoreCase)
		|| IsGardenFaucetMesh(Mesh)
		|| MeshPath.Contains(TEXT("WaterTap"), ESearchCase::IgnoreCase);
}

bool AGreenhousePlayerController::IsComputerScreenHit(const FHitResult& Hit) const
{
	const AActor* HitActor = Hit.GetActor();
	const UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(Hit.GetComponent());
	const UStaticMesh* Mesh = MeshComponent ? MeshComponent->GetStaticMesh() : nullptr;

	const FString ActorName = HitActor ? HitActor->GetName() : FString();
	const FString ComponentName = MeshComponent ? MeshComponent->GetName() : FString();
	const FString MeshPath = Mesh ? Mesh->GetPathName() : FString();
	if (ActorName.Contains(TEXT("Monitor"), ESearchCase::IgnoreCase)
		|| ActorName.Contains(TEXT("Computer"), ESearchCase::IgnoreCase)
		|| ActorName.Contains(TEXT("Screen"), ESearchCase::IgnoreCase)
		|| ActorName.Contains(TEXT("Office_PC"), ESearchCase::IgnoreCase)
		|| ComponentName.Contains(TEXT("Monitor"), ESearchCase::IgnoreCase)
		|| ComponentName.Contains(TEXT("Screen"), ESearchCase::IgnoreCase)
		|| MeshPath.Contains(TEXT("desk+setup"), ESearchCase::IgnoreCase)
		|| MeshPath.Contains(TEXT("Monitor"), ESearchCase::IgnoreCase)
		|| MeshPath.Contains(TEXT("Office_PC"), ESearchCase::IgnoreCase))
	{
		return true;
	}

	if (MeshComponent)
	{
		const int32 MaterialCount = MeshComponent->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
		{
			const UMaterialInterface* Material = MeshComponent->GetMaterial(MaterialIndex);
			const FString MaterialPath = Material ? Material->GetPathName() : FString();
			if (MaterialPath.Contains(TEXT("Monitor_Dark_Glass"), ESearchCase::IgnoreCase)
				|| MaterialPath.Contains(TEXT("Old_Office_PC"), ESearchCase::IgnoreCase))
			{
				return true;
			}
		}
	}

	return false;
}

void AGreenhousePlayerController::StartFillingWateringCan(const FHitResult& FaucetHit)
{
	StopPouringWater();
	bIsFillingWateringCan = true;
	WateringCanFillSeconds = 0.0f;
	WateringCanFillStartLiters = WateringCanLiters;
	WateringCanFillingTransform = BuildFillingCanTransform(FaucetHit);

	const FVector FaucetWaterStart = FindFaucetWaterStart(FaucetHit);
	const FVector FillOpeningLocation = WateringCanFillingTransform.TransformPosition(GetWateringCanFillOpeningActorOffset());
	FillingWaterEndLocation = FillOpeningLocation + FVector(0.0f, 0.0f, 4.0f);
	FillingWaterStartLocation = FaucetWaterStart;

	if (HeldItemActor)
	{
		HeldItemActor->SetHeldItem(EGreenhouseInventoryItem::WateringCan);
		HeldItemActor->SetWaterEffect(EGreenhouseHeldWaterEffect::None);
	}
	UpdateFillingWaterStream();
}

void AGreenhousePlayerController::FinishFillingWateringCan()
{
	bIsFillingWateringCan = false;
	WateringCanFillSeconds = 0.0f;
	WateringCanFillStartLiters = MaxWateringCanLiters;
	WateringCanLiters = MaxWateringCanLiters;
	if (HeldItemActor)
	{
		HeldItemActor->SetWaterEffect(EGreenhouseHeldWaterEffect::None);
	}
	HideFillingWaterStream();
}

void AGreenhousePlayerController::StartPouringWater(const FHitResult& InteractionHit)
{
	if (WateringCanLiters <= 0.0f)
	{
		return;
	}

	bIsPouringWater = true;
	bStopPouringWhenVisualEnds = false;
	PouringVisualSecondsRemaining = 0.35f;
	CurrentWateringPlot = FindWateringPlot(InteractionHit);
	if (CurrentWateringPlot.IsValid())
	{
		CurrentWateringPlot->WaterPlant(0.0f);
	}

	if (HeldItemActor)
	{
		HeldItemActor->SetHeldItem(EGreenhouseInventoryItem::WateringCan);
		HeldItemActor->SetWaterEffect(EGreenhouseHeldWaterEffect::Pouring);
		LastDisplayedHeldItem = EGreenhouseInventoryItem::WateringCan;
	}
}

void AGreenhousePlayerController::StopPouringWater()
{
	if (bIsPouringWater && PouringVisualSecondsRemaining > 0.0f)
	{
		bStopPouringWhenVisualEnds = true;
		return;
	}

	bIsPouringWater = false;
	bStopPouringWhenVisualEnds = false;
	CurrentWateringPlot.Reset();
	if (HeldItemActor && !bIsFillingWateringCan)
	{
		HeldItemActor->SetWaterEffect(EGreenhouseHeldWaterEffect::None);
	}
}

AGreenhousePlantingPlotActor* AGreenhousePlayerController::FindWateringPlot(const FHitResult& InteractionHit) const
{
	const APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || !GetWorld())
	{
		return nullptr;
	}

	const FVector PlayerLocation = ControlledPawn->GetActorLocation();
	if (AGreenhousePlantingPlotActor* HitPlot = Cast<AGreenhousePlantingPlotActor>(InteractionHit.GetActor()))
	{
		if (HitPlot->CanWaterAt(PlayerLocation))
		{
			return HitPlot;
		}
	}

	AGreenhousePlantingPlotActor* ClosestPlot = nullptr;
	float ClosestDistanceSq = TNumericLimits<float>::Max();
	for (TActorIterator<AGreenhousePlantingPlotActor> PlotIt(GetWorld()); PlotIt; ++PlotIt)
	{
		AGreenhousePlantingPlotActor* Plot = *PlotIt;
		if (!Plot || !Plot->CanWaterAt(PlayerLocation))
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared2D(PlayerLocation, Plot->GetActorLocation());
		if (DistanceSq < ClosestDistanceSq)
		{
			ClosestDistanceSq = DistanceSq;
			ClosestPlot = Plot;
		}
	}

	return ClosestPlot;
}

FTransform AGreenhousePlayerController::BuildFillingCanTransform(const FHitResult& FaucetHit) const
{
	const FVector FaucetWaterStart = FindFaucetWaterStart(FaucetHit);
	const float Yaw = GetFillingCanYawDegrees();
	const FVector FillOpeningOffset = FRotator(0.0f, Yaw, 0.0f).RotateVector(GetWateringCanFillOpeningActorOffset());
	FVector DesiredCanLocation = FaucetWaterStart - FVector(FillOpeningOffset.X, FillOpeningOffset.Y, 0.0f);
	DesiredCanLocation.Z = FaucetWaterStart.Z - 88.0f;
	const FVector CanLocation = FindGroundedCanLocation(DesiredCanLocation, FaucetHit);

	return FTransform(FRotator(0.0f, Yaw, 0.0f), CanLocation, FVector(1.0f));
}

FVector AGreenhousePlayerController::FindFaucetWaterStart(const FHitResult& FaucetHit) const
{
	if (const UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(FaucetHit.GetComponent()))
	{
		if (IsGardenFaucetMesh(MeshComponent->GetStaticMesh()))
		{
			return MeshComponent->GetComponentTransform().TransformPosition(GetGardenFaucetOutletLocal());
		}
	}

	FVector WaterStart = FaucetHit.bBlockingHit ? FaucetHit.ImpactPoint : FVector::ZeroVector;
	if (const UPrimitiveComponent* HitComponent = FaucetHit.GetComponent())
	{
		const FBoxSphereBounds Bounds = HitComponent->Bounds;
		const float LowerFaucetZ = Bounds.Origin.Z - Bounds.BoxExtent.Z + 10.0f;
		WaterStart.Z = FMath::Min(WaterStart.Z - 4.0f, LowerFaucetZ);
	}
	else if (FaucetHit.bBlockingHit)
	{
		WaterStart.Z -= 4.0f;
	}

	return WaterStart;
}

FVector AGreenhousePlayerController::FindGroundedCanLocation(const FVector& DesiredLocation, const FHitResult& FaucetHit) const
{
	if (!GetWorld())
	{
		return DesiredLocation;
	}

	FVector GroundedLocation = DesiredLocation;
	FHitResult GroundHit;
	const FVector TraceStart(DesiredLocation.X, DesiredLocation.Y, DesiredLocation.Z + 260.0f);
	const FVector TraceEnd(DesiredLocation.X, DesiredLocation.Y, DesiredLocation.Z - 700.0f);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GreenhouseWateringCanGround), false);
	if (const APawn* ControlledPawn = GetPawn())
	{
		QueryParams.AddIgnoredActor(ControlledPawn);
	}
	if (HeldItemActor)
	{
		QueryParams.AddIgnoredActor(HeldItemActor);
	}
	if (AActor* FaucetActor = FaucetHit.GetActor())
	{
		QueryParams.AddIgnoredActor(FaucetActor);
	}

	if (GetWorld()->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams)
		|| GetWorld()->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		GroundedLocation.Z = GroundHit.ImpactPoint.Z + FillingCanFloorOffsetZ;
	}
	else if (const ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn()))
	{
		const UCapsuleComponent* CapsuleComponent = ControlledCharacter->GetCapsuleComponent();
		const float PlayerFeetZ = ControlledCharacter->GetActorLocation().Z - (CapsuleComponent ? CapsuleComponent->GetScaledCapsuleHalfHeight() : 90.0f);
		GroundedLocation.Z = PlayerFeetZ + FillingCanFloorOffsetZ;
	}

	return GroundedLocation;
}

void AGreenhousePlayerController::UpdateFillingWaterStream()
{
	SpawnFillingWaterStreamActor();
	if (!FillingWaterStreamActor)
	{
		return;
	}

	const FVector Start = FillingWaterStartLocation;
	const FVector End = FillingWaterEndLocation;
	const FVector WaterVector = End - Start;
	const float Length = WaterVector.Size();
	if (Length < 4.0f)
	{
		HideFillingWaterStream();
		return;
	}

	const FVector Direction = WaterVector / Length;
	const FQuat Rotation = FQuat::FindBetweenNormals(FVector::UpVector, Direction);
	const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	constexpr float StreamWidth = 0.034f;
	FillingWaterStreamActor->SetActorLocation(Start + WaterVector * 0.5f);
	FillingWaterStreamActor->SetActorRotation(Rotation);
	FillingWaterStreamActor->SetActorScale3D(FVector(StreamWidth, StreamWidth, Length / 100.0f));
	FillingWaterStreamActor->SetActorHiddenInGame(false);

	const float SegmentLength = FMath::Clamp(Length * 0.14f, 9.0f, 22.0f);
	const float TravelStart = SegmentLength * 0.5f + 2.0f;
	const float TravelEnd = FMath::Max(TravelStart, Length - SegmentLength * 0.5f - 2.0f);
	const int32 SegmentCount = FillingWaterFlowActors.Num();
	for (int32 SegmentIndex = 0; SegmentIndex < SegmentCount; ++SegmentIndex)
	{
		AStaticMeshActor* FlowActor = FillingWaterFlowActors[SegmentIndex];
		if (!FlowActor)
		{
			continue;
		}

		const float PhaseOffset = SegmentCount > 0 ? static_cast<float>(SegmentIndex) / static_cast<float>(SegmentCount) : 0.0f;
		const float Phase = FMath::Frac(TimeSeconds * 1.65f + PhaseOffset);
		const float SegmentDistance = FMath::Lerp(TravelStart, TravelEnd, Phase);
		const float SegmentWidth = StreamWidth * (1.18f + 0.08f * FMath::Sin(TimeSeconds * 9.0f + SegmentIndex));
		FlowActor->SetActorLocation(Start + Direction * SegmentDistance);
		FlowActor->SetActorRotation(Rotation);
		FlowActor->SetActorScale3D(FVector(SegmentWidth, SegmentWidth, SegmentLength / 100.0f));
		FlowActor->SetActorHiddenInGame(false);
	}
}

void AGreenhousePlayerController::HideFillingWaterStream()
{
	if (FillingWaterStreamActor)
	{
		FillingWaterStreamActor->SetActorHiddenInGame(true);
	}
	for (AStaticMeshActor* FlowActor : FillingWaterFlowActors)
	{
		if (FlowActor)
		{
			FlowActor->SetActorHiddenInGame(true);
		}
	}
}

void AGreenhousePlayerController::UpdateHeldItemActor()
{
	if (!InventoryWidget)
	{
		return;
	}

	SpawnHeldItemActor();
	if (!HeldItemActor || !PlayerCameraManager)
	{
		return;
	}

	if (bIsFillingWateringCan)
	{
		if (LastDisplayedHeldItem != EGreenhouseInventoryItem::WateringCan)
		{
			HeldItemActor->SetHeldItem(EGreenhouseInventoryItem::WateringCan);
			LastDisplayedHeldItem = EGreenhouseInventoryItem::WateringCan;
		}

		HeldItemActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		HeldItemActor->SetActorTransform(WateringCanFillingTransform);
		HeldItemActor->SetWaterEffect(EGreenhouseHeldWaterEffect::None);
		UpdateFillingWaterStream();
		return;
	}

	const EGreenhouseInventoryItem SelectedItem = InventoryWidget->GetSelectedHotbarItem();
	if (SelectedItem != LastDisplayedHeldItem)
	{
		HeldItemActor->SetHeldItem(SelectedItem);
		LastDisplayedHeldItem = SelectedItem;
	}
	HeldItemActor->SetWaterEffect(bIsPouringWater ? EGreenhouseHeldWaterEffect::Pouring : EGreenhouseHeldWaterEffect::None);

	const FVector HeldLocation = bIsPouringWater
		? FVector(60.0f, 27.0f, -34.0f)
		: FVector(58.0f, 25.0f, -32.0f);
	const FRotator HeldRotation = bIsPouringWater
		? FRotator(8.0f, 24.0f, -26.0f)
		: FRotator(-8.0f, 18.0f, 8.0f);

	if (USceneComponent* CameraComponent = FindFirstPersonCameraComponent())
	{
		if (HeldItemActor->GetRootComponent()->GetAttachParent() != CameraComponent)
		{
			HeldItemActor->AttachToComponent(CameraComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}

		HeldItemActor->SetActorRelativeLocation(HeldLocation);
		HeldItemActor->SetActorRelativeRotation(HeldRotation);
		return;
	}

	const FRotator CameraRotation = PlayerCameraManager->GetCameraRotation();
	const FVector CameraLocation = PlayerCameraManager->GetCameraLocation();
	const FVector HandOffset = CameraRotation.RotateVector(HeldLocation);
	const FRotator ItemRotation = CameraRotation + HeldRotation;
	HeldItemActor->SetActorLocationAndRotation(CameraLocation + HandOffset, ItemRotation);
}

void AGreenhousePlayerController::SelectHotbarSlot(int32 SlotIndex)
{
	if (!InventoryWidget)
	{
		return;
	}

	InventoryWidget->SelectHotbarSlot(SlotIndex);
	UpdateHeldItemActor();
}

void AGreenhousePlayerController::SelectHotbarSlotOne()
{
	SelectHotbarSlot(0);
}

void AGreenhousePlayerController::SelectHotbarSlotTwo()
{
	SelectHotbarSlot(1);
}

void AGreenhousePlayerController::SelectHotbarSlotThree()
{
	SelectHotbarSlot(2);
}

void AGreenhousePlayerController::SelectHotbarSlotFour()
{
	SelectHotbarSlot(3);
}

void AGreenhousePlayerController::SelectHotbarSlotFive()
{
	SelectHotbarSlot(4);
}

void AGreenhousePlayerController::StartSprint()
{
	bIsSprinting = true;
	ApplySprintSpeed();
}

void AGreenhousePlayerController::StopSprint()
{
	bIsSprinting = false;
	ApplySprintSpeed();
}

void AGreenhousePlayerController::ApplySprintSpeed()
{
	ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn());
	if (!ControlledCharacter)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = ControlledCharacter->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	if (DefaultWalkSpeed <= 0.0f)
	{
		DefaultWalkSpeed = MovementComponent->MaxWalkSpeed;
	}

	MovementComponent->MaxWalkSpeed = bIsSprinting ? DefaultWalkSpeed * SprintSpeedMultiplier : DefaultWalkSpeed;
}
