// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"

#include "EnhancedInputComponent.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//static const ConstructorHelpers::FObjectFinder<UInputAction> ThrottleActionObject(TEXT(""))
}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGoKart::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector Translation = Velocity *DeltaTime *100;

	FHitResult HitResult;
	AddActorWorldOffset(Translation,true, &HitResult);
	if(HitResult.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;

	}
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Force =GetActorForwardVector()* MaxDrivingForce * Throttle;
	FVector Acceleration = Force / Mass; 
	Velocity = Velocity + Acceleration * DeltaTime;
	UpdateLocationFromVelocity(DeltaTime);
}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{

		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &AGoKart::MoveForward);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &AGoKart::MoveForward);
		// steering 
		//EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &AKrazyKartsPawn::Steering);
		//EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &AKrazyKartsPawn::Steering);

		//// throttle 
		//EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &AKrazyKartsPawn::Throttle);
		//EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &AKrazyKartsPawn::Throttle);

		//// break 
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &AGoKart::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &AGoKart::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &AGoKart::StopBrake);

		//// handbrake 
		//EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &AKrazyKartsPawn::StartHandbrake);
		//EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &AKrazyKartsPawn::StopHandbrake);

		//// look around 
		//EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &AKrazyKartsPawn::LookAround);

		//// toggle camera 
		//EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &AKrazyKartsPawn::ToggleCamera);

		//// reset the vehicle 
		//EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &AKrazyKartsPawn::ResetVehicle);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AGoKart::MoveForward(const FInputActionValue& Value)
{
	Throttle = Value.Get<float>();
}

void AGoKart::Brake(const FInputActionValue& InputActionValue)
{
	Throttle = -1 * InputActionValue.Get<float>();
}

void AGoKart::StartBrake(const FInputActionValue& InputActionValue)
{
}

void AGoKart::StopBrake(const FInputActionValue& InputActionValue)
{
	Throttle = 0;
}

