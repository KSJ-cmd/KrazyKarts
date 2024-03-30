// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.generated.h"

USTRUCT()
struct FGoKartMove
{
	GENERATED_BODY()
	UPROPERTY()
	float Throttle;
	UPROPERTY()
	float SteeringThrow;

	UPROPERTY()
	float DeltaTime;
	UPROPERTY()
	float Time;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UGoKartMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoKartMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	void SimulateMove(const FGoKartMove& Move);

	

	FVector GetVelocity() const;

	void SetVelocity(FVector newVelocity);
	void SetThrottle(float newThrottle);
	void SetSteeringThrow(float newSteeringThrow);

	FGoKartMove GetLastMove() const;
private:
	FGoKartMove CreateMove(float deltaTime);

	FVector GetAirResistance() const;
	FVector GetRollingResistance() const;

	void ApplyRotation(float DeltaTime, float mSteeringThrow);

	void UpdateLocationFromVelocity(float DeltaTime);
private:
	//The Mass of the car (KG)
	UPROPERTY(EditAnywhere)
	float Mass = 1000;

	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000;

	// Minimum radius of the car turning circle at full lock (m).
	UPROPERTY(EditAnywhere)
	float MinTurningRadius = 10;



	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16;

	// Higher means more rolling resistance.
	UPROPERTY(EditAnywhere)
	float RollingResistanceCoefficient = 0.015;

	FVector Velocity;

	float Throttle;
	float SteeringThrow;

	FGoKartMove LastMove;
};
