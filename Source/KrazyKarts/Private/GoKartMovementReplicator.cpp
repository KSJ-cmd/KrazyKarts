// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKartMovementReplicator.h"
#include "GoKartMovementComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UGoKartMovementReplicator::UGoKartMovementReplicator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	SetIsReplicatedByDefault(true);
	// ...
}


// Called when the game starts
void UGoKartMovementReplicator::BeginPlay()
{
	Super::BeginPlay();
	
	// ...
	
	Movement = GetOwner()->FindComponentByClass<UGoKartMovementComponent>();
	
}


// Called every frame
void UGoKartMovementReplicator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Movement == nullptr) return;
	const auto Move = Movement->GetLastMove(); //Cast<APawn>(GetOwner())->IsLocallyControlled()
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s is ROLE_AutonomousProxy"), *GetOwner()->GetName());
		UnacknowledgedMoves.Add(Move);
		Server_SendMove(Move);
	}
	
	// We are the server and in control of the pawn.
	if (GetOwner()->GetRemoteRole() == ROLE_AutonomousProxy)
	{
		UpdateServerState(Move);
	}
	
	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s is ROLE_SimulatedProxy"), *GetOwner()->GetName());
		Movement->SimulateMove(ServerState.LastMove);
	}
	// ...
}

void UGoKartMovementReplicator::ClearAcknowledgedMoves(FGoKartMove lastMove)
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


void UGoKartMovementReplicator::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGoKartMovementReplicator, ServerState);
}

void UGoKartMovementReplicator::OnRep_ServerState()
{
	if (Movement == nullptr) return;
	GetOwner()->SetActorTransform(ServerState.Transform);
	Movement->SetVelocity(ServerState.Velocity);
	ClearAcknowledgedMoves(ServerState.LastMove);

	for (const auto& move : UnacknowledgedMoves) {
		Movement->SimulateMove(move);
	}
}

void UGoKartMovementReplicator::UpdateServerState(const FGoKartMove& Move)
{
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetTransform();
	ServerState.Velocity = Movement->GetVelocity();
}

void UGoKartMovementReplicator::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (Movement == nullptr) return;

	Movement->SimulateMove(Move);
	UpdateServerState(Move);
}

bool UGoKartMovementReplicator::Server_SendMove_Validate(FGoKartMove Move)
{
	return true;
}
