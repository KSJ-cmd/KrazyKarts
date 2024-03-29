// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"

#include "EnhancedInputComponent.h"
#include "GoKartMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	//static const ConstructorHelpers::FObjectFinder<UInputAction> ThrottleActionObject(TEXT(""))
	Movement = CreateDefaultSubobject<UGoKartMovementComponent>(TEXT("Movement"));
	if(Movement==nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGoKartMovementComponent is nullptr"));
	}
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

void AGoKart::ClearAcknowledgedMoves(FGoKartMove lastMove)
{
	TArray<FGoKartMove> newMoves;

	for (const auto& move : UnacknowledgedMoves)
	{
		if (move.Time > lastMove.Time)
		{
			newMoves.Add(move);
		}
	}
	UnacknowledgedMoves = newMoves;
}


void AGoKart::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGoKart, ServerState);
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

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		const auto Move = Movement->CreateMove(DeltaTime);
		Movement->SimulateMove(Move);

		UnacknowledgedMoves.Add(Move);

		Server_SendMove(Move);

	}
	if (GetLocalRole() == ROLE_Authority && IsLocallyControlled())
	{
		const auto Move = Movement->CreateMove(DeltaTime);
		Server_SendMove(Move);
	}

	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		Movement->SimulateMove(ServerState.LastMove);
	}
	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(GetLocalRole()), this, FColor::White, DeltaTime);
	DrawDebugString(GetWorld(), FVector(0, 0, 200), GetEnumText(GetRemoteRole()), this, FColor::White, DeltaTime);
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
	
	Movement->SetThrottle(Value.Get<float>());
}

void AGoKart::Brake(const FInputActionValue& InputActionValue)
{
	Movement->SetThrottle( -1 * InputActionValue.Get<float>());
}


void AGoKart::StartBrake(const FInputActionValue& InputActionValue)
{
}

void AGoKart::StopBrake(const FInputActionValue& InputActionValue)
{
	
	Movement->SetThrottle(0);

}


void AGoKart::MoveRight(const FInputActionValue& Value)
{
	Movement->SetSteeringThrow(Value.Get<float>());
	
}

void AGoKart::OnRep_ServerState()
{
	SetActorTransform(ServerState.Transform);
	Movement->SetVelocity(ServerState.Velocity);
	ClearAcknowledgedMoves(ServerState.LastMove);

	for (const auto& move : UnacknowledgedMoves) {
		Movement->SimulateMove(move);
	}
}
void AGoKart::Server_SendMove_Implementation(FGoKartMove Move)
{
	Movement->SimulateMove(Move);
	ServerState.LastMove = Move;
	ServerState.Transform = GetTransform();
	ServerState.Velocity = Movement->GetVelocity();
}

bool AGoKart::Server_SendMove_Validate(FGoKartMove Move)
{
	return true;
}
