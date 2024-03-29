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

	if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		const auto Move = Movement->CreateMove(DeltaTime);
		Movement->SimulateMove(Move);

		UnacknowledgedMoves.Add(Move);

		Server_SendMove(Move);

	}
	if (GetOwner()->GetLocalRole() == ROLE_Authority && Cast<APawn>(GetOwner())->IsLocallyControlled())
	{
		const auto Move = Movement->CreateMove(DeltaTime);
		Server_SendMove(Move);
	}

	if (GetOwner()->GetLocalRole() == ROLE_SimulatedProxy)
	{
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
void UGoKartMovementReplicator::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (Movement == nullptr) return;

	Movement->SimulateMove(Move);
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetTransform();
	ServerState.Velocity = Movement->GetVelocity();
}

bool UGoKartMovementReplicator::Server_SendMove_Validate(FGoKartMove Move)
{
	return true;
}
