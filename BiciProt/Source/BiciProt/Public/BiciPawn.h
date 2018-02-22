// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BiciPawn.generated.h"

UCLASS()
class BICIPROT_API ABiciPawn : public APawn
{
	GENERATED_BODY()

private:

	FVector Velocity;

	float Speed;

	FVector Translation;

	float Throttle;

	float SteeringThrow;

	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 1000.0;

	UPROPERTY(EditAnywhere)
	float MaxDegreesPerSec = 90.0;

	FVector Force;

	FVector Acceleration;

	//Mass of the Pawn
	UPROPERTY(EditAnywhere)
	float Mass = 1000.0;

	void MoveForward(float Value);

	void MoveRight(float Value);



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void UpdateLocationFromVelocity(float DeltaTime);

	void ApplyRotation(float DeltaTime);

	FVector GetResistance();

	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16;

public:	

	// Sets default values for this pawn's properties
	ABiciPawn();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
