// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"

#include "EnhancedInputComponent.h"

#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	//static const ConstructorHelpers::FObjectFinder<UInputAction> ThrottleActionObject(TEXT(""))
}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		NetUpdateFrequency = 1;
	}
}

FVector AGoKart::GetAirResistance() const
{
	return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector AGoKart::GetRollingResistance() const
{
	const float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100;
	const float NormalForce = Mass * AccelerationDueToGravity;
	return -Velocity.GetSafeNormal() * RollingResistanceCoefficient * NormalForce;
}


void AGoKart::ApplyRotation(float DeltaTime)
{
	/*const auto RotationAngle = SteeringThrow * MaxDegreePerSecond * DeltaTime;
	const FQuat RotationDelta(GetActorUpVector(), FMath::DegreesToRadians(RotationAngle));
	*/

	const float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime;
	const float RotationAngle = DeltaLocation / MinTurningRadius * SteeringThrow;
	const FQuat RotationDelta(GetActorUpVector(), RotationAngle);

	Velocity = RotationDelta.RotateVector(Velocity);

	AddActorWorldRotation(RotationDelta);
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

void AGoKart::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGoKart, ReplicatedTranform);
	DOREPLIFETIME(AGoKart, Throttle);
	DOREPLIFETIME(AGoKart, SteeringThrow);
	DOREPLIFETIME(AGoKart, Velocity);

}

FString GetEnumText(ENetRole Role)
{
	switch (Role)
	{
	case ROLE_None:
		return "None";
	case ROLE_SimulatedProxy:
		return "SimulatedProxy";
	case ROLE_AutonomousProxy:
		return "AutonomousProxy";
	case ROLE_Authority:
		return "Authority";
	default:
		return "ERROR";
	}
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FVector Force =
		GetActorForwardVector() * MaxDrivingForce * Throttle +
		GetAirResistance() + GetRollingResistance();
	const FVector Acceleration = Force / Mass; 
	Velocity = Velocity + Acceleration * DeltaTime;
	ApplyRotation(DeltaTime);
	
	UpdateLocationFromVelocity(DeltaTime);

	if (HasAuthority())
	{
		ReplicatedTranform = GetActorTransform();
	}
	
	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(GetLocalRole()), this, FColor::White, DeltaTime);
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
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &AGoKart::MoveRight);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &AGoKart::MoveRight);

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
	Server_MoveForward(Value);
}
void AGoKart::Server_MoveForward_Implementation(const FInputActionValue& Value)
{
	Throttle = Value.Get<float>();
}

bool AGoKart::Server_MoveForward_Validate(const FInputActionValue& Value)
{
	return FMath::Abs(Value.Get<float>()) <= 1;
}
void AGoKart::Brake(const FInputActionValue& InputActionValue)
{
	Throttle = -1 * InputActionValue.Get<float>();
	Server_Brake(InputActionValue);
}

void AGoKart::Server_Brake_Implementation(const FInputActionValue& Value)
{
	Throttle = -1 * Value.Get<float>();
}

bool AGoKart::Server_Brake_Validate(const FInputActionValue& Value)
{
	return FMath::Abs(Value.Get<float>()) <= 1;
}

void AGoKart::StartBrake(const FInputActionValue& InputActionValue)
{
}

void AGoKart::StopBrake(const FInputActionValue& InputActionValue)
{
	Throttle = 0;
	Server_StopBrake(InputActionValue);
}

void AGoKart::Server_StopBrake_Implementation(const FInputActionValue& Value)
{
	Throttle = 0;
}

bool AGoKart::Server_StopBrake_Validate(const FInputActionValue& Value)
{
	return true;
}

void AGoKart::MoveRight(const FInputActionValue& Value)
{
	SteeringThrow = Value.Get<float>();
	Server_MoveRight(Value);
}

void AGoKart::OnRep_ReplicatedTranform()
{
	SetActorTransform(ReplicatedTranform);
}

void AGoKart::Server_MoveRight_Implementation(const FInputActionValue& Value)
{
	SteeringThrow = Value.Get<float>();
}

bool AGoKart::Server_MoveRight_Validate(const FInputActionValue& Value)
{
	return FMath::Abs(Value.Get<float>()) <= 1;
}


