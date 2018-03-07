// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BiciMovementComponent.generated.h"

USTRUCT()
struct FBiciMoves
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float SteeringThrow;

	UPROPERTY()
	float DeltaTime;

	UPROPERTY()
	float Time;

	bool IsValid() const
	{
		return (FMath::Abs(Throttle) <= 1 && FMath::Abs(SteeringThrow) <= 1);
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BICIPROT_API UBiciMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBiciMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SimulateMove(FBiciMoves Move);

	void SetVelocity(FVector Velocity);
	FVector GetVelocity() { return Velocity; };

	void SetThrottle(float Throttle);
	float GetThrottle() { return Throttle; };

	void SetSteeringThrow(float SteeringThrow);
	float GetSteeringThrow() { return SteeringThrow; };

	FBiciMoves GetLastMove() { return LastMove; };

private:

	FBiciMoves CreateMove(float DeltaTime);

	void UpdateLocationFromVelocity(float DeltaTime);

	void ApplyRotation(float DeltaTime, float SteeringThrow);

	FVector GetAirResistance();

	FVector GetRollingResistance();

	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16;

	UPROPERTY(EditAnywhere)
	float RollingResistanceCoefficient = 0.015;

	//[N]
	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000.0;

	//[m]
	UPROPERTY(EditAnywhere)
	float MinTurningRadius = 10;

	//float NormalForce;

	//Mass of the Pawn [Kg]
	UPROPERTY(EditAnywhere)
	float Mass = 1000.0;

	FVector Velocity;

	float Throttle;

	float SteeringThrow;
	
	FBiciMoves LastMove;
};
