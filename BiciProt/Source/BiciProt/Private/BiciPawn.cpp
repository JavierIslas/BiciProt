// Fill out your copyright notice in the Description page of Project Settings.

#include "BiciPawn.h"
#include "Components/InputComponent.h"

// Sets default values
ABiciPawn::ABiciPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABiciPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABiciPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Force = GetActorForwardVector() * MaxDrivingForce * Throttle;
	Force += GetResistance();

	Acceleration = Force / Mass;

	Velocity = Velocity + Acceleration * DeltaTime;

	ApplyRotation(DeltaTime);
	
	UpdateLocationFromVelocity(DeltaTime);
}

void ABiciPawn::ApplyRotation(float DeltaTime)
{
	float RotationAngle = MaxDegreesPerSec * DeltaTime * SteeringThrow;
	FQuat RotationDelta(GetActorUpVector(), FMath::DegreesToRadians(RotationAngle));
	Velocity = RotationDelta.RotateVector(Velocity);
	AddActorWorldRotation(RotationDelta);
}

FVector ABiciPawn::GetResistance()
{
	return - Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

void ABiciPawn::UpdateLocationFromVelocity(float DeltaTime)
{
	Translation = Velocity * 100 * DeltaTime;
	FHitResult Hit;
	AddActorWorldOffset(Translation, true, &Hit);
	if (Hit.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

void ABiciPawn::MoveForward(float Value)
{
	Throttle = Value ;

}

void ABiciPawn::MoveRight(float Value)
{
	SteeringThrow = Value;
}

// Called to bind functionality to input
void ABiciPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABiciPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABiciPawn::MoveRight);
	PlayerInputComponent->BindAxis("LookUp");
	PlayerInputComponent->BindAxis("LookRight");

}

