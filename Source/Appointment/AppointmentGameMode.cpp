// Copyright Epic Games, Inc. All Rights Reserved.

#include "AppointmentGameMode.h"
#include "AppointmentPlayerController.h"
#include "AppointmentCharacter.h"
#include "UObject/ConstructorHelpers.h"

AAppointmentGameMode::AAppointmentGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AAppointmentPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownPlayerController"));
	if(PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}

void AAppointmentGameMode::StartPlay()
{
	Super::StartPlay();

	check(GEngine != nullptr);

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("This is Appointment Game Mode."));
	// throw std::logic_error("The method or operation is not implemented.");
}
