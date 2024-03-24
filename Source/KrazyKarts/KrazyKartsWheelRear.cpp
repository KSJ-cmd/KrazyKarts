// Copyright Epic Games, Inc. All Rights Reserved.

#include "KrazyKartsWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UKrazyKartsWheelRear::UKrazyKartsWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}