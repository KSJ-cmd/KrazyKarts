// Copyright Epic Games, Inc. All Rights Reserved.

#include "KrazyKartsWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UKrazyKartsWheelFront::UKrazyKartsWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}