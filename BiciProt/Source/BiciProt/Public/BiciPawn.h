// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BiciPawn.generated.h"

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

};

USTRUCT()
struct FBiciState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FTransform Transform;

	FBiciMoves LastMove;

};

UCLASS()
class BICIPROT_API ABiciPawn : public APawn
{
	GENERATED_BODY()

private:

	FBiciMoves CreateMove(float DeltaTime);

	void SimulateMove(FBiciMoves Move);

	void ClearAcknowledgeMoves(FBiciMoves LastMove);

	UPROPERTY()
	FVector Velocity;

	float Speed;

	FVector Translation;

	float Throttle;

	float SteeringThrow;

	//[N]
	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 20000.0;

	//[m]
	UPROPERTY(EditAnywhere)
	float MinTurningRadius = 10;

	float NormalForce;

	FVector Force;

	FVector Acceleration;

	//Mass of the Pawn [Kg]
	UPROPERTY(EditAnywhere)
	float Mass = 1000.0;

	TArray<FBiciMoves> UnacknowledgedMoves;

	void MoveForward(float Value);

	void MoveRight(float Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FBiciMoves Move);

	UFUNCTION()
	void OnRep_ServerState();

	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FBiciState ServerState;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void UpdateLocationFromVelocity(float DeltaTime);

	void ApplyRotation(float DeltaTime, float SteeringThrow);

	FVector GetAirResistance();

	FVector GetRollingResistance();

	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16;

	UPROPERTY(EditAnywhere)
	float RollingResistanceCoefficient = 0.015;

public:	

	// Sets default values for this pawn's properties
	ABiciPawn();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
