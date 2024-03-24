// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKartPlayerController.h"
#include "GoKart.h"
#include "KrazyKarts//KrazyKartsUI.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosWheeledVehicleMovementComponent.h"

void AGoKartPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}

	// spawn the UI widget and add it to the viewport
	/*VehicleUI = CreateWidget<UKrazyKartsUI>(this, VehicleUIClass);

	check(VehicleUI);

	VehicleUI->AddToViewport();*/
}

void AGoKartPlayerController::Tick(float Delta)
{
	Super::Tick(Delta);

	/*if (IsValid(VehiclePawn) && IsValid(VehicleUI))
	{
		VehicleUI->UpdateSpeed(VehiclePawn->GetChaosVehicleMovement()->GetForwardSpeed());
		VehicleUI->UpdateGear(VehiclePawn->GetChaosVehicleMovement()->GetCurrentGear());
	}*/
}

void AGoKartPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// get a pointer to the controlled pawn
	VehiclePawn = CastChecked<AGoKart>(InPawn);
}
