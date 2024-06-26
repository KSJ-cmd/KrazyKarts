// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKartMovementReplicator.h"
#include "GoKartMovementComponent.h"
#include "GameFramework/GameStateBase.h"
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
static FString GetEnumNetMode(ENetMode mode)
{
	switch (mode) {
	case NM_Standalone:
		return "NM_Standalone";
		break;
	case NM_DedicatedServer:
		return "NM_DedicatedServer";
		break;
	case NM_ListenServer:
		return "NM_ListenServer";
		break;
	case NM_Client:
		return "NM_Client";
		break;
	case NM_MAX:
		return "NM_MAX";
		break;
		default:
			return "";
	}

	
}

// Called every frame
void UGoKartMovementReplicator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Movement == nullptr) return;
	if (GetOwner()->GetInstigatorController() != nullptr) {

		const auto Move = Movement->GetLastMove(); //Cast<APawn>(GetOwner())->IsLocallyControlled()

		if (GetOwnerRole() == ROLE_AutonomousProxy )
		{
			Server_SendMove(Move);

		}

		// We are the server and in control of the pawn.
		if (Cast<APawn>(GetOwner())->IsLocallyControlled())
		{
			//UE_LOG(LogTemp, Warning, TEXT("%s is GetOwner()->GetInstigatorController()->GetRemoteRole() == ROLE_SimulatedProxy == UpdateServerState"), *GetOwner()->GetName());
			UpdateServerState(Move);
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("GetEnumNetMode : %s = %s is OnRep_ServerState UnacknowledgedMoves : %d "),*GetEnumNetMode(GetNetMode()), *GetOwner()->GetName(), UnacknowledgedMoves.Num());
	if (GetOwnerRole()==ROLE_SimulatedProxy)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s is ROLE_SimulatedProxy"), *GetOwner()->GetName());
		//Movement->SimulateMove(ServerState.LastMove);
		ClientTick(DeltaTime);
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
	switch (GetOwnerRole()) {
	
	case ROLE_SimulatedProxy:
		SimulatedProxy_OnRep_ServerState();
		break;
	case ROLE_AutonomousProxy:
		AutonomousProxy_OnRep_ServerState();
		break;
	default:
		return;
	}
}

void UGoKartMovementReplicator::AutonomousProxy_OnRep_ServerState()
{
	if (Movement == nullptr) return;
	//UE_LOG(LogTemp, Warning, TEXT("%s is OnRep_ServerState deltaTime : %f, SteeringThrow : %f, Throttle : %f, Time : %f"), *GetOwner()->GetName(),ServerState.LastMove.DeltaTime, ServerState.LastMove.SteeringThrow, ServerState.LastMove.Throttle, ServerState.LastMove.Time);
	GetOwner()->SetActorTransform(ServerState.Transform);
	Movement->SetVelocity(ServerState.Velocity);
	ClearAcknowledgedMoves(ServerState.LastMove);
	//UE_LOG(LogTemp, Warning, TEXT("%s is OnRep_ServerState UnacknowledgedMoves : %d "), *GetOwner()->GetName(), UnacknowledgedMoves.Num());

	for (const auto& move : UnacknowledgedMoves) {
		Movement->SimulateMove(move);
	}
}

void UGoKartMovementReplicator::SimulatedProxy_OnRep_ServerState()
{
	if (Movement == nullptr) return;
	ClientTimeBetweenLastUpdates = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0;

	if (MeshOffsetRoot != nullptr)
	{
		ClientStartTransform.SetLocation(MeshOffsetRoot->GetComponentLocation());
		ClientStartTransform.SetRotation(MeshOffsetRoot->GetComponentQuat());
	}
	ClientStartVelocity = Movement->GetVelocity();
	GetOwner()->SetActorTransform(ServerState.Transform);
}

void UGoKartMovementReplicator::UpdateServerState(const FGoKartMove& Move)
{
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetTransform();
	ServerState.Velocity = Movement->GetVelocity();
}

float UGoKartMovementReplicator::VelocityDerivative() const
{
	return ClientTimeBetweenLastUpdates * 100;
}

FHermiteCubicSpline UGoKartMovementReplicator::CreateSpline() const
{
	FHermiteCubicSpline Spline;

	Spline.StartLocation = ClientStartTransform.GetLocation();
	Spline.TargetLocation = ServerState.Transform.GetLocation();
	Spline.StartDerivative = ClientStartVelocity * VelocityDerivative();
	Spline.TargetDerivative = ServerState.Velocity * VelocityDerivative();
	return Spline;
}

void UGoKartMovementReplicator::InterpLocation(const FHermiteCubicSpline& Spline, float LerpRatio) const
{
	const auto newVector = Spline.InterpLocation(LerpRatio);
	if (MeshOffsetRoot != nullptr)
	{
		MeshOffsetRoot->SetWorldLocation(newVector);
	}
}

void UGoKartMovementReplicator::InterpRotation(const float LerpRatio) const
{
	const auto TargetRot = ServerState.Transform.GetRotation();
	const auto StartRot = ClientStartTransform.GetRotation();

	const auto NewRotation = FQuat::Slerp(StartRot, TargetRot, LerpRatio);

	if (MeshOffsetRoot != nullptr)
	{
		MeshOffsetRoot->SetWorldRotation(NewRotation);
	}
}

void UGoKartMovementReplicator::InterpVelocity(const FHermiteCubicSpline& Spline, float LerpRatio) const
{
	const FVector NewDerivative = Spline.InterpDerivative(LerpRatio);
	const FVector NewVelocity = NewDerivative / VelocityDerivative();

	Movement->SetVelocity(NewVelocity);
}

float UGoKartMovementReplicator::LerpRatio() const
{
	return ClientTimeSinceUpdate / ClientTimeBetweenLastUpdates;
}

void UGoKartMovementReplicator::ClientTick(float DeltaTime)
{
	ClientTimeSinceUpdate += DeltaTime;

	if (ClientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER) return;
	if (Movement == nullptr) return;

	const FHermiteCubicSpline Spline = CreateSpline();
	const auto Ratio = LerpRatio();

	InterpLocation(Spline, Ratio);

	InterpVelocity(Spline, Ratio);

	InterpRotation(Ratio);
}

void UGoKartMovementReplicator::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (Movement == nullptr) return;
	ClientSimulatedTimes += Move.DeltaTime;
	Movement->SimulateMove(Move);
	UpdateServerState(Move);
}

bool UGoKartMovementReplicator::Server_SendMove_Validate(FGoKartMove Move)
{

	float ProposeedTime = ClientSimulatedTimes += Move.DeltaTime;
	const bool ClientNotRunningAhead = ProposeedTime < GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	if(!ClientNotRunningAhead)
	{
		UE_LOG(LogTemp, Error, TEXT("Server_SendMove_Validate ClientNotRunningAhead"));
		return false;
	}

	if(!Move.IsVaild())
	{
		UE_LOG(LogTemp, Error, TEXT("Server_SendMove_Validate Move inVaild"));
		return false;
	}
	return Move.IsVaild();
}
