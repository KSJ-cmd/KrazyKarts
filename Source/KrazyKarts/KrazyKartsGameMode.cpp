// Copyright Epic Games, Inc. All Rights Reserved.

#include "KrazyKartsGameMode.h"
#include "KrazyKartsPlayerController.h"

AKrazyKartsGameMode::AKrazyKartsGameMode()
{
	PlayerControllerClass = AKrazyKartsPlayerController::StaticClass();
}
