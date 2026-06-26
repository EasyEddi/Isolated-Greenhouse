#include "GreenhousePlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GreenhouseHeldItemActor.h"
#include "GreenhouseInventoryWidget.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "Components/SceneComponent.h"
#include "UObject/ConstructorHelpers.h"

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

	InventoryWidget = CreateWidget<UGreenhouseInventoryWidget>(this, UGreenhouseInventoryWidget::StaticClass());
	if (InventoryWidget)
	{
		InventoryWidget->AddToViewport();
		ApplyInventoryInputMode(false);
	}

	SpawnHeldItemActor();
	UpdateHeldItemActor();
}

void AGreenhousePlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

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

void AGreenhousePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	check(InputComponent);
	InputComponent->BindKey(EKeys::E, IE_Pressed, this, &AGreenhousePlayerController::ToggleInventory);
	InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AGreenhousePlayerController::SelectHotbarSlotOne);
	InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AGreenhousePlayerController::SelectHotbarSlotTwo);
	InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AGreenhousePlayerController::SelectHotbarSlotThree);
	InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &AGreenhousePlayerController::SelectHotbarSlotFour);
	InputComponent->BindKey(EKeys::Five, IE_Pressed, this, &AGreenhousePlayerController::SelectHotbarSlotFive);
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

USceneComponent* AGreenhousePlayerController::FindFirstPersonCameraComponent() const
{
	const APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return nullptr;
	}

	return ControlledPawn->FindComponentByClass<UCameraComponent>();
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

	const EGreenhouseInventoryItem SelectedItem = InventoryWidget->GetSelectedHotbarItem();
	if (SelectedItem != LastDisplayedHeldItem)
	{
		HeldItemActor->SetHeldItem(SelectedItem);
		LastDisplayedHeldItem = SelectedItem;
	}

	if (USceneComponent* CameraComponent = FindFirstPersonCameraComponent())
	{
		if (HeldItemActor->GetRootComponent()->GetAttachParent() != CameraComponent)
		{
			HeldItemActor->AttachToComponent(CameraComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}

		HeldItemActor->SetActorRelativeLocation(FVector(58.0f, 25.0f, -32.0f));
		HeldItemActor->SetActorRelativeRotation(FRotator(-8.0f, 18.0f, 8.0f));
		return;
	}

	const FRotator CameraRotation = PlayerCameraManager->GetCameraRotation();
	const FVector CameraLocation = PlayerCameraManager->GetCameraLocation();
	const FVector HandOffset = CameraRotation.RotateVector(FVector(58.0f, 25.0f, -32.0f));
	const FRotator ItemRotation = CameraRotation + FRotator(-8.0f, 18.0f, 8.0f);
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
