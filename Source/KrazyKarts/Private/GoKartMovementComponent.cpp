// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKartMovementComponent.h"

#include "GameFramework/GameStateBase.h"

// Sets default values for this component's properties
UGoKartMovementComponent::UGoKartMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGoKartMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGoKartMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UGoKartMovementComponent::SimulateMove(const FGoKartMove& Move)
{
	const float DeltaTime = Move.DeltaTime;
	const FVector Force =
		GetOwner()->GetActorForwardVector() * MaxDrivingForce * Move.Throttle +
		GetAirResistance() + GetRollingResistance();
	const FVector Acceleration = Force / Mass;
	Velocity = Velocity + Acceleration * DeltaTime;
	ApplyRotation(DeltaTime, Move.SteeringThrow);

	UpdateLocationFromVelocity(DeltaTime);

}


FGoKartMove UGoKartMovementComponent::CreateMove(float deltaTime)
{
	FGoKartMove Move;
	Move.DeltaTime = deltaTime;
	Move.SteeringThrow = SteeringThrow;
	Move.Throttle = Throttle;
	Move.Time = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	return Move;
}

FVector UGoKartMovementComponent::GetVelocity() const
{
	return Velocity;
}

void UGoKartMovementComponent::SetVelocity(FVector newVelocity)
{
	Velocity = newVelocity;
}

void UGoKartMovementComponent::SetThrottle(float newThrottle)
{
	Throttle = newThrottle;
}

void UGoKartMovementComponent::SetSteeringThrow(float newSteeringThrow)
{
	SteeringThrow = newSteeringThrow;
}

FVector UGoKartMovementComponent::GetAirResistance() const
{
	return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector UGoKartMovementComponent::GetRollingResistance() const
{
	const float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100;
	const float NormalForce = Mass * AccelerationDueToGravity;
	return -Velocity.GetSafeNormal() * RollingResistanceCoefficient * NormalForce;
}


void UGoKartMovementComponent::ApplyRotation(float DeltaTime, float mSteeringThrow)
{

	const float DeltaLocation = FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity) * DeltaTime;
	const float RotationAngle = DeltaLocation / MinTurningRadius * mSteeringThrow;
	const FQuat RotationDelta(GetOwner()->GetActorUpVector(), RotationAngle);

	Velocity = RotationDelta.RotateVector(Velocity);

	GetOwner()->AddActorWorldRotation(RotationDelta);
}
void UGoKartMovementComponent::UpdateLocationFromVelocity(float DeltaTime)
{
	const FVector Translation = Velocity * DeltaTime * 100;

	FHitResult HitResult;
	GetOwner()->AddActorWorldOffset(Translation, true, &HitResult);
	if (HitResult.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}
