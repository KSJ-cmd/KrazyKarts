// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.h"
#include "GoKartMovementReplicator.generated.h"


USTRUCT()
struct FGoKartState
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform Transform;
	UPROPERTY()
	FVector Velocity;
	UPROPERTY()
	FGoKartMove LastMove;
};

USTRUCT()
struct FHermiteCubicSpline
{
	GENERATED_BODY()

	FVector StartLocation, TargetLocation, StartDerivative, TargetDerivative;

	FVector InterpLocation(float LerpRatio) const
	{
		return FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}
	FVector InterpDerivative(float LerpRatio) const
	{
		return FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	} 
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KRAZYKARTS_API UGoKartMovementReplicator : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGoKartMovementReplicator();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


private:

	//serverSend
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoKartMove Move);

	void ClearAcknowledgedMoves(FGoKartMove lastMove);

	UFUNCTION()
	void OnRep_ServerState();

	void AutonomousProxy_OnRep_ServerState();
	void SimulatedProxy_OnRep_ServerState();
	void UpdateServerState(const FGoKartMove& Move);
	float VelocityDerivative() const;
	FHermiteCubicSpline CreateSpline() const;
	void InterpRotation(float LerpRatio) const;
	void InterpVelocity(const FHermiteCubicSpline& Spline, float LerpRatio) const;
	void InterpLocation(const FHermiteCubicSpline& Spline, float LerpRatio) const;
	float LerpRatio() const;
	void ClientTick(float DeltaTime);

private:

	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FGoKartState ServerState;

	TArray<FGoKartMove> UnacknowledgedMoves;
	FTransform ClientStartTransform;
	FVector ClientStartVelocity;
	float ClientTimeSinceUpdate;
	float ClientTimeBetweenLastUpdates;
	UPROPERTY(VisibleAnywhere)
	UGoKartMovementComponent* Movement;
};
