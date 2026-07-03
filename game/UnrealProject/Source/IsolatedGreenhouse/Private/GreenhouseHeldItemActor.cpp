#include "GreenhouseHeldItemActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
UStaticMesh* LoadFirstAvailableMesh(const TCHAR* PrimaryPath, const TCHAR* FallbackPath = nullptr)
{
	if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, PrimaryPath))
	{
		return Mesh;
	}

	return FallbackPath ? LoadObject<UStaticMesh>(nullptr, FallbackPath) : nullptr;
}

UMaterialInterface* LoadWaterMaterial()
{
	return LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/models/equipment/Watering_Can/Water_In_The_Watering_Can.Water_In_The_Watering_Can"));
}

void ApplyWaterMaterial(UStaticMeshComponent* MeshComponent, UMaterialInterface* WaterMaterial)
{
	if (!MeshComponent)
	{
		return;
	}

	if (WaterMaterial)
	{
		MeshComponent->SetMaterial(0, WaterMaterial);
		return;
	}

	if (UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0))
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.18f, 0.58f, 1.0f, 0.82f));
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.18f, 0.58f, 1.0f, 0.82f));
		DynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), 0.74f);
	}
}

FVector GetWateringCanNozzleOutletLocal()
{
	// Matches the black shower-head material bounds in watering_can.fbx.
	return FVector(-1.98f, 0.0f, 1.315f) * 100.0f;
}

FVector GetWateringCanSprayDirectionLocal()
{
	return FVector(-1.0f, 0.0f, 0.0f).GetSafeNormal();
}

void ConfigureHeldMesh(UStaticMeshComponent* MeshComponent, UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation, float Scale)
{
	if (!MeshComponent)
	{
		return;
	}

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetCastShadow(false);
	MeshComponent->SetStaticMesh(Mesh);
	MeshComponent->SetRelativeLocation(Location);
	MeshComponent->SetRelativeRotation(Rotation);
	MeshComponent->SetRelativeScale3D(FVector(Scale));
}
}

AGreenhouseHeldItemActor::AGreenhouseHeldItemActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	UStaticMesh* LilyMesh = LoadFirstAvailableMesh(TEXT("/Game/models/Plants/Lily/lily.lily"));
	UStaticMesh* WateringCanMesh = LoadFirstAvailableMesh(
		TEXT("/Game/models/equipment/Watering_Can/watering_can.watering_can"),
		TEXT("/Game/models/furniture/Watering_Can/watering_can.watering_can"));
	UStaticMesh* EmptyPotMesh = LoadFirstAvailableMesh(
		TEXT("/Game/models/equipment/pots/ornament_plants/empty/ornament_plants_pot_empty.ornament_plants_pot_empty"),
		TEXT("/Game/models/equipment/pots/ornament plants/empty/ornament_plants_pot_empty.ornament_plants_pot_empty"));
	UStaticMesh* SoilBagMesh = LoadFirstAvailableMesh(
		TEXT("/Game/models/equipment/soil/ornament_plants/ornament_plants_soil.ornament_plants_soil"),
		TEXT("/Game/models/equipment/soil/ornament plants/ornament_plants_soil.ornament_plants_soil"));
	UStaticMesh* FertilizerBagMesh = LoadFirstAvailableMesh(
		TEXT("/Game/models/equipment/fertilizer/ornament_plants/ornament_plants_fertilizer.ornament_plants_fertilizer"));
	UStaticMesh* TrowelMesh = LoadFirstAvailableMesh(TEXT("/Game/models/equipment/Trowel/trowel.trowel"));
	UStaticMesh* SecateurMesh = LoadFirstAvailableMesh(TEXT("/Game/models/equipment/Secateur/Secateur/secateur.secateur"));
	UStaticMesh* CylinderMesh = LoadFirstAvailableMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* DropletMesh = LoadFirstAvailableMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UMaterialInterface* WaterMaterial = LoadWaterMaterial();

	LilyMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LilyMesh"));
	LilyMeshComponent->SetupAttachment(SceneRoot);
	ConfigureHeldMesh(LilyMeshComponent, LilyMesh, FVector(0.0f, 0.0f, 6.0f), FRotator(0.0f, 170.0f, 0.0f), 0.2f);

	WateringCanMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WateringCanMesh"));
	WateringCanMeshComponent->SetupAttachment(SceneRoot);
	ConfigureHeldMesh(WateringCanMeshComponent, WateringCanMesh, FVector(0.0f, 0.0f, -12.0f), FRotator(0.0f, 145.0f, 0.0f), 0.32f);

	EmptyPotMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EmptyPotMesh"));
	EmptyPotMeshComponent->SetupAttachment(SceneRoot);
	ConfigureHeldMesh(EmptyPotMeshComponent, EmptyPotMesh, FVector(0.0f, 0.0f, -12.0f), FRotator(0.0f, 150.0f, 0.0f), 0.30f);

	SoilBagMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SoilBagMesh"));
	SoilBagMeshComponent->SetupAttachment(SceneRoot);
	ConfigureHeldMesh(SoilBagMeshComponent, SoilBagMesh, FVector(0.0f, 0.0f, -14.0f), FRotator(0.0f, 148.0f, 0.0f), 0.25f);

	FertilizerBagMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FertilizerBagMesh"));
	FertilizerBagMeshComponent->SetupAttachment(SceneRoot);
	ConfigureHeldMesh(FertilizerBagMeshComponent, FertilizerBagMesh, FVector(0.0f, 0.0f, -14.0f), FRotator(0.0f, 148.0f, 0.0f), 0.25f);

	TrowelMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrowelMesh"));
	TrowelMeshComponent->SetupAttachment(SceneRoot);
	ConfigureHeldMesh(TrowelMeshComponent, TrowelMesh, FVector(4.0f, 0.0f, -10.0f), FRotator(0.0f, 132.0f, -24.0f), 0.34f);

	SecateurMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SecateurMesh"));
	SecateurMeshComponent->SetupAttachment(SceneRoot);
	ConfigureHeldMesh(SecateurMeshComponent, SecateurMesh, FVector(4.0f, 0.0f, -9.0f), FRotator(0.0f, 128.0f, -12.0f), 0.42f);

	WaterStreamMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterStreamMesh"));
	WaterStreamMeshComponent->SetupAttachment(WateringCanMeshComponent);
	WaterStreamMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WaterStreamMeshComponent->SetCastShadow(false);
	WaterStreamMeshComponent->SetStaticMesh(CylinderMesh);
	WaterStreamMeshComponent->SetRelativeScale3D(FVector(0.018f, 0.018f, 0.62f));
	WaterStreamMeshComponent->SetVisibility(false, true);
	ApplyWaterMaterial(WaterStreamMeshComponent, WaterMaterial);

	constexpr int32 SprayDropCount = 96;
	WaterSprayComponents.Reserve(SprayDropCount);
	for (int32 DropIndex = 0; DropIndex < SprayDropCount; ++DropIndex)
	{
		const FName ComponentName(*FString::Printf(TEXT("WaterSprayDrop_%02d"), DropIndex));
		UStaticMeshComponent* SprayDrop = CreateDefaultSubobject<UStaticMeshComponent>(ComponentName);
		SprayDrop->SetupAttachment(WateringCanMeshComponent);
		SprayDrop->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SprayDrop->SetCastShadow(false);
		SprayDrop->SetStaticMesh(DropletMesh ? DropletMesh : CylinderMesh);
		SprayDrop->SetVisibility(false, true);
		ApplyWaterMaterial(SprayDrop, WaterMaterial);
		WaterSprayComponents.Add(SprayDrop);
	}

	SetHeldItem(EGreenhouseInventoryItem::None);
	SetActorTickEnabled(false);
}

void AGreenhouseHeldItemActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	WaterVisualTime += DeltaSeconds;
	if (CurrentItem == EGreenhouseInventoryItem::WateringCan && CurrentWaterEffect == EGreenhouseHeldWaterEffect::Pouring)
	{
		UpdateWaterSpray();
	}
}

void AGreenhouseHeldItemActor::SetHeldItem(EGreenhouseInventoryItem Item)
{
	CurrentItem = Item;
	if (LilyMeshComponent)
	{
		LilyMeshComponent->SetVisibility(Item == EGreenhouseInventoryItem::Lily, true);
	}

	if (WateringCanMeshComponent)
	{
		WateringCanMeshComponent->SetVisibility(Item == EGreenhouseInventoryItem::WateringCan, true);
	}

	if (EmptyPotMeshComponent)
	{
		EmptyPotMeshComponent->SetVisibility(Item == EGreenhouseInventoryItem::EmptyPot, true);
	}

	if (SoilBagMeshComponent)
	{
		SoilBagMeshComponent->SetVisibility(Item == EGreenhouseInventoryItem::SoilBag, true);
	}

	if (FertilizerBagMeshComponent)
	{
		FertilizerBagMeshComponent->SetVisibility(Item == EGreenhouseInventoryItem::FertilizerBag, true);
	}

	if (TrowelMeshComponent)
	{
		TrowelMeshComponent->SetVisibility(Item == EGreenhouseInventoryItem::Trowel, true);
	}

	if (SecateurMeshComponent)
	{
		SecateurMeshComponent->SetVisibility(Item == EGreenhouseInventoryItem::Secateur, true);
	}

	if (Item != EGreenhouseInventoryItem::WateringCan)
	{
		SetWaterEffect(EGreenhouseHeldWaterEffect::None);
	}

	SetActorHiddenInGame(Item == EGreenhouseInventoryItem::None);
}

void AGreenhouseHeldItemActor::SetWaterEffect(EGreenhouseHeldWaterEffect Effect)
{
	const EGreenhouseHeldWaterEffect PreviousEffect = CurrentWaterEffect;
	CurrentWaterEffect = Effect;
	if (PreviousEffect != CurrentWaterEffect)
	{
		WaterVisualTime = 0.0f;
	}

	const bool bShowWater = CurrentItem == EGreenhouseInventoryItem::WateringCan && Effect != EGreenhouseHeldWaterEffect::None;
	const bool bShowPouring = bShowWater && Effect == EGreenhouseHeldWaterEffect::Pouring;
	if (WaterStreamMeshComponent)
	{
		WaterStreamMeshComponent->SetVisibility(false, true);
	}

	SetWaterSprayVisible(bShowPouring);
	SetActorTickEnabled(bShowPouring);
	if (bShowPouring)
	{
		UpdateWaterSpray();
	}
}

void AGreenhouseHeldItemActor::SetWaterSprayVisible(bool bVisible)
{
	for (UStaticMeshComponent* SprayDrop : WaterSprayComponents)
	{
		if (SprayDrop)
		{
			SprayDrop->SetVisibility(bVisible, true);
		}
	}
}

void AGreenhouseHeldItemActor::UpdateWaterSpray()
{
	if (!WateringCanMeshComponent)
	{
		return;
	}

	const FTransform MeshTransform = WateringCanMeshComponent->GetComponentTransform();
	const FVector SprayOrigin = MeshTransform.TransformPosition(GetWateringCanNozzleOutletLocal());
	const FVector NozzleDirection = MeshTransform.TransformVectorNoScale(GetWateringCanSprayDirectionLocal()).GetSafeNormal();
	const FVector SprayDirection = (NozzleDirection * 0.82f + FVector::DownVector * 0.34f).GetSafeNormal();
	FVector SpraySide = FVector::CrossProduct(SprayDirection, FVector::DownVector);
	if (SpraySide.IsNearlyZero())
	{
		SpraySide = MeshTransform.TransformVectorNoScale(FVector::UpVector);
	}
	SpraySide.Normalize();
	FVector SprayUp = FVector::CrossProduct(SpraySide, SprayDirection);
	if (SprayUp.IsNearlyZero())
	{
		SprayUp = FVector::UpVector;
	}
	SprayUp.Normalize();

	for (int32 DropIndex = 0; DropIndex < WaterSprayComponents.Num(); ++DropIndex)
	{
		UStaticMeshComponent* SprayDrop = WaterSprayComponents[DropIndex];
		if (!SprayDrop)
		{
			continue;
		}

		const float DropSeed = static_cast<float>(DropIndex);
		const int32 HoleIndex = DropIndex % 16;
		const int32 RingIndex = DropIndex / 16;
		const float HoleAngle = (static_cast<float>(HoleIndex) / 16.0f) * (2.0f * UE_PI) + static_cast<float>(RingIndex) * 0.24f;
		const float HoleCos = FMath::Cos(HoleAngle);
		const float HoleSin = FMath::Sin(HoleAngle);
		const float TravelAlpha = FMath::Frac(WaterVisualTime * (4.4f + static_cast<float>(RingIndex) * 0.13f) + DropSeed * 0.031f);
		const float ConeSide = 0.20f + static_cast<float>(RingIndex % 6) * 0.045f;
		const float ConeUp = 0.17f + static_cast<float>((RingIndex + 2) % 6) * 0.038f;
		const FVector HoleOffset = SpraySide * (HoleCos * 5.8f) + SprayUp * (HoleSin * 5.2f);
		const FVector DropDirection = (SprayDirection + SpraySide * (HoleCos * ConeSide) + SprayUp * (HoleSin * ConeUp)).GetSafeNormal();
		const FVector Position = SprayOrigin
			+ NozzleDirection * 2.5f
			+ HoleOffset
			+ DropDirection * (6.0f + TravelAlpha * 132.0f)
			+ FVector::DownVector * (TravelAlpha * TravelAlpha * 38.0f);

		const float DropScale = 0.021f + 0.012f * (1.0f - TravelAlpha) + 0.003f * FMath::Sin(DropSeed * 2.1f);
		SprayDrop->SetWorldLocation(Position);
		SprayDrop->SetWorldRotation(FRotator(0.0f, DropSeed * 31.0f, 0.0f));
		SprayDrop->SetWorldScale3D(FVector(DropScale, DropScale, DropScale * (1.05f + TravelAlpha * 0.55f)));
	}
}
