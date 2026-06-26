#include "GreenhouseHeldItemActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
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
}

AGreenhouseHeldItemActor::AGreenhouseHeldItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	UStaticMesh* LilyMesh = LoadFirstAvailableMesh(TEXT("/Game/models/Plants/Lily/lily.lily"));
	UStaticMesh* WateringCanMesh = LoadFirstAvailableMesh(
		TEXT("/Game/models/equipment/Watering_Can/watering_can.watering_can"),
		TEXT("/Game/models/furniture/Watering_Can/watering_can.watering_can"));

	LilyMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LilyMesh"));
	LilyMeshComponent->SetupAttachment(SceneRoot);
	LilyMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LilyMeshComponent->SetCastShadow(false);
	LilyMeshComponent->SetStaticMesh(LilyMesh);
	LilyMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 6.0f));
	LilyMeshComponent->SetRelativeRotation(FRotator(0.0f, 170.0f, 0.0f));
	LilyMeshComponent->SetRelativeScale3D(FVector(0.2f));

	WateringCanMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WateringCanMesh"));
	WateringCanMeshComponent->SetupAttachment(SceneRoot);
	WateringCanMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WateringCanMeshComponent->SetCastShadow(false);
	WateringCanMeshComponent->SetStaticMesh(WateringCanMesh);
	WateringCanMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -12.0f));
	WateringCanMeshComponent->SetRelativeRotation(FRotator(0.0f, 145.0f, 0.0f));
	WateringCanMeshComponent->SetRelativeScale3D(FVector(0.32f));

	SetHeldItem(EGreenhouseInventoryItem::None);
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

	SetActorHiddenInGame(Item == EGreenhouseInventoryItem::None);
}
