// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "GridTutGameMode.h"
#include "GridTutPlayerController.h"
#include "GridTutCharacter.h"
#include "UObject/ConstructorHelpers.h"

AGridTutGameMode::AGridTutGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AGridTutPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}