// Fill out your copyright notice in the Description page of Project Settings.

#include "BiciPawn.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "UnrealNetwork.h"

// Sets default values
ABiciPawn::ABiciPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Movement = CreateDefaultSubobject<UBiciMovementComponent>(TEXT("Movement Component"));

	Replicator = CreateDefaultSubobject<UBiciReplicationComponent>(TEXT("Replication Component"));

	bReplicates = true;
	bReplicateMovement = false;

	if (HasAuthority())
	{
		NetUpdateFrequency = 1.0f;
	}

}

// Called when the game starts or when spawned
void ABiciPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

FString GetEnumText(ENetRole Role)
{
	switch (Role)
	{
	case ROLE_None:
		return "None";
	case ROLE_SimulatedProxy:
		return "SimulatedProxy";
	case ROLE_AutonomousProxy:
		return "AutonomousProxy";
	case ROLE_Authority:
		return "Authority";
	default:
		return "Error";
	}
}

// Called every frame
void ABiciPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(Role), this, FColor::Red, DeltaTime);

}

void ABiciPawn::MoveForward(float Value)
{
	Movement->SetThrottle(Value);
}

void ABiciPawn::MoveRight(float Value)
{
	Movement->SetSteeringThrow(Value);
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