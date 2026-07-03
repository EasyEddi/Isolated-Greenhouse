#include "GreenhouseGameMode.h"

#include "GreenhousePlantingPlotActor.h"
#include "GreenhousePlayerController.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AGreenhouseGameMode::AGreenhouseGameMode()
{
	PlayerControllerClass = AGreenhousePlayerController::StaticClass();

	static ConstructorHelpers::FClassFinder<APawn> FirstPersonPawnClass(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	if (FirstPersonPawnClass.Succeeded())
	{
		DefaultPawnClass = FirstPersonPawnClass.Class;
	}
}

void AGreenhouseGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!GetWorld())
	{
		return;
	}

	RepairWallMaterials();

	TActorIterator<AGreenhousePlantingPlotActor> ExistingPlotIt(GetWorld());
	if (ExistingPlotIt)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AGreenhousePlantingPlotActor* Plot = GetWorld()->SpawnActor<AGreenhousePlantingPlotActor>(
		AGreenhousePlantingPlotActor::StaticClass(),
		FVector(820.0f, 1180.0f, 2.0f),
		FRotator::ZeroRotator,
		SpawnParameters);

	if (Plot)
	{
#if WITH_EDITOR
		Plot->SetActorLabel(TEXT("PlantingPlot_GreenhouseZone_01"));
#endif
	}
}

void AGreenhouseGameMode::RepairWallMaterials()
{
#if WITH_EDITOR
	if (!GetWorld())
	{
		return;
	}

	const TMap<FString, const TCHAR*> WallMaterials = {
		{ TEXT("Hall_Wall_bottom_green_750"), TEXT("/Game/Art/Materials/M_Hall_Brick_Wall_750.M_Hall_Brick_Wall_750") },
		{ TEXT("Hall_Wall_bottom_red_2350"), TEXT("/Game/Art/Materials/M_Hall_Brick_Wall_2350.M_Hall_Brick_Wall_2350") },
		{ TEXT("Hall_Wall_right_brown_1500"), TEXT("/Game/Art/Materials/M_Hall_Brick_Wall_1500.M_Hall_Brick_Wall_1500") },
		{ TEXT("Hall_Wall_top_red_2350"), TEXT("/Game/Art/Materials/M_Hall_Brick_Wall_2350.M_Hall_Brick_Wall_2350") },
		{ TEXT("Hall_Wall_cyan_500"), TEXT("/Game/Art/Materials/M_Hall_Brick_Wall_500.M_Hall_Brick_Wall_500") },
		{ TEXT("Hall_Wall_top_green_750"), TEXT("/Game/Art/Materials/M_Hall_Brick_Wall_750.M_Hall_Brick_Wall_750") },
		{ TEXT("Hall_Wall_left_blue_1000"), TEXT("/Game/Art/Materials/M_Hall_Brick_Wall_1000.M_Hall_Brick_Wall_1000") },
	};

	for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
	{
		AStaticMeshActor* MeshActor = *It;
		if (!MeshActor)
		{
			continue;
		}

		const TCHAR* const* MaterialPath = WallMaterials.Find(MeshActor->GetActorLabel());
		if (!MaterialPath)
		{
			continue;
		}

		UStaticMeshComponent* MeshComponent = MeshActor->GetStaticMeshComponent();
		UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);
		if (MeshComponent && Material)
		{
			MeshComponent->SetMaterial(0, Material);
		}
	}
#endif
}
