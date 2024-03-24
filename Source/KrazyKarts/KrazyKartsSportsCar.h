// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "KrazyKartsPawn.h"
#include "KrazyKartsSportsCar.generated.h"

/**
 *  Sports car wheeled vehicle implementation
 */
UCLASS(abstract)
class KRAZYKARTS_API AKrazyKartsSportsCar : public AKrazyKartsPawn
{
	GENERATED_BODY()
	
public:

	AKrazyKartsSportsCar();
};
