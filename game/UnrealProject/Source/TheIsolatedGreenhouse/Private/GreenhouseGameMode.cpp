#include "GreenhouseGameMode.h"

#include "GreenhousePlayerController.h"
#include "GameFramework/Pawn.h"
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
