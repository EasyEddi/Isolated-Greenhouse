#include "GreenhousePlantingPlotActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AGreenhousePlantingPlotActor::AGreenhousePlantingPlotActor()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* LilyMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/models/Plants/Lily/lily.lily"));

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	PlotMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlotMesh"));
	PlotMeshComponent->SetupAttachment(SceneRoot);
	PlotMeshComponent->SetStaticMesh(CubeMeshFinder.Object);
	PlotMeshComponent->SetRelativeScale3D(FVector(0.55f, 0.55f, 0.035f));
	PlotMeshComponent->SetCollisionProfileName(TEXT("BlockAll"));

	PlotMaterial = PlotMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	UpdatePlotMaterial();

	LilyMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrowingLilyMesh"));
	LilyMeshComponent->SetupAttachment(SceneRoot);
	LilyMeshComponent->SetStaticMesh(LilyMesh);
	LilyMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LilyMeshComponent->SetCastShadow(true);
	LilyMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 8.0f));
	LilyMeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	LilyMeshComponent->SetRelativeScale3D(FVector(InitialLilyScale));
	LilyMeshComponent->SetVisibility(false, true);
}

void AGreenhousePlantingPlotActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (WetSecondsRemaining > 0.0f)
	{
		WetSecondsRemaining = FMath::Max(0.0f, WetSecondsRemaining - DeltaSeconds);
		UpdatePlotMaterial();
	}

	if (bIsGrowing && LilyMeshComponent)
	{
		const float GrowthRate = WetSecondsRemaining > 0.0f ? 1.55f : 1.0f;
		GrowthSeconds = FMath::Min(GrowthSeconds + DeltaSeconds * GrowthRate, GrowthDuration);
		const float Alpha = GrowthSeconds / GrowthDuration;
		const float Scale = FMath::Lerp(InitialLilyScale, FullLilyScale, Alpha);
		LilyMeshComponent->SetRelativeScale3D(FVector(Scale));

		if (GrowthSeconds >= GrowthDuration)
		{
			bIsGrowing = false;
		}
	}
}

bool AGreenhousePlantingPlotActor::CanPlantAt(const FVector& PlayerLocation) const
{
	if (bHasLily)
	{
		return false;
	}

	const FVector PlotLocation = GetActorLocation();
	const float Distance2D = FVector::Dist2D(PlayerLocation, PlotLocation);
	return Distance2D <= InteractionRadius;
}

bool AGreenhousePlantingPlotActor::PlantLily()
{
	if (bHasLily || !LilyMeshComponent)
	{
		return false;
	}

	bHasLily = true;
	bIsGrowing = true;
	GrowthSeconds = 0.0f;
	LilyMeshComponent->SetRelativeScale3D(FVector(InitialLilyScale));
	LilyMeshComponent->SetVisibility(true, true);
	return true;
}

bool AGreenhousePlantingPlotActor::CanWaterAt(const FVector& PlayerLocation) const
{
	if (!bHasLily)
	{
		return false;
	}

	const FVector PlotLocation = GetActorLocation();
	const float Distance2D = FVector::Dist2D(PlayerLocation, PlotLocation);
	return Distance2D <= InteractionRadius;
}

bool AGreenhousePlantingPlotActor::WaterPlant(float WaterSeconds)
{
	if (!bHasLily)
	{
		return false;
	}

	WetSecondsRemaining = FMath::Max(WetSecondsRemaining, WetVisualDuration);
	if (bIsGrowing)
	{
		GrowthSeconds = FMath::Min(GrowthSeconds + WaterSeconds * 0.35f, GrowthDuration);
	}
	UpdatePlotMaterial();
	return true;
}

void AGreenhousePlantingPlotActor::UpdatePlotMaterial()
{
	if (!PlotMaterial)
	{
		return;
	}

	const bool bWet = WetSecondsRemaining > 0.0f;
	const FLinearColor PlotColor = bWet
		? FLinearColor(0.07f, 0.12f, 0.09f, 1.0f)
		: FLinearColor(0.16f, 0.24f, 0.18f, 1.0f);
	PlotMaterial->SetVectorParameterValue(TEXT("BaseColor"), PlotColor);
	PlotMaterial->SetVectorParameterValue(TEXT("Color"), PlotColor);
	PlotMaterial->SetScalarParameterValue(TEXT("Roughness"), bWet ? 0.72f : 0.92f);
}
