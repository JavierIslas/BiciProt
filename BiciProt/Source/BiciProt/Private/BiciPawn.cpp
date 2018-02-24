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

	bReplicates = true;

	/*if (HasAuthority())
	{
		NetUpdateFrequency = 1.0f;
	}*/

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

	if (Role == ROLE_AutonomousProxy)
	{
		FBiciMoves Move = CreateMove(DeltaTime);
		SimulateMove(Move);
		UnacknowledgedMoves.Add(Move);
		Server_SendMove(Move);
	}

	//if Server and in control of the Pawn
	if (Role == ROLE_Authority && GetRemoteRole() == ROLE_SimulatedProxy)
	{
		FBiciMoves Move = CreateMove(DeltaTime);
		Server_SendMove(Move);
	}

	if (Role == ROLE_SimulatedProxy)
	{
		SimulateMove(ServerState.LastMove);
	}

	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(Role), this, FColor::Red, DeltaTime);

}

void ABiciPawn::ApplyRotation(float DeltaTime, float SteeringThrow)
{
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime;
	float RotationAngle = (DeltaLocation / MinTurningRadius) * SteeringThrow;
	FQuat RotationDelta(GetActorUpVector(), RotationAngle);
	Velocity = RotationDelta.RotateVector(Velocity);
	AddActorWorldRotation(RotationDelta);
}

FVector ABiciPawn::GetAirResistance()
{
	return - Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector ABiciPawn::GetRollingResistance()
{
	float AccelerationDueGravity = - GetWorld()->GetGravityZ() / 100;
	NormalForce = Mass * AccelerationDueGravity;
	return - Velocity.GetSafeNormal() * RollingResistanceCoefficient * NormalForce;
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

FBiciMoves ABiciPawn::CreateMove(float DeltaTime)
{
	FBiciMoves Move;
	Move.DeltaTime = DeltaTime;
	Move.SteeringThrow = SteeringThrow;
	Move.Throttle = Throttle;
	Move.Time = GetWorld()->TimeSeconds;

	return Move;
}

void ABiciPawn::SimulateMove(FBiciMoves Move)
{
	Force = GetActorForwardVector() * MaxDrivingForce * Move.Throttle;
	Force += GetAirResistance();
	Force += GetRollingResistance();

	Acceleration = Force / Mass;

	Velocity += Acceleration * Move.DeltaTime;

	ApplyRotation(Move.DeltaTime, Move.SteeringThrow);

	UpdateLocationFromVelocity(Move.DeltaTime);
}

void ABiciPawn::ClearAcknowledgeMoves(FBiciMoves LastMove)
{
	TArray<FBiciMoves> NewMoves;

	for (const FBiciMoves& Move : UnacknowledgedMoves)
	{
		if (Move.Time > LastMove.Time)
		{
			NewMoves.Add(Move);
		}
	}

	UnacknowledgedMoves = NewMoves;
}

void ABiciPawn::MoveForward(float Value)
{
	Throttle = Value;
}

void ABiciPawn::MoveRight(float Value)
{
	SteeringThrow = Value;
}

void ABiciPawn::Server_SendMove_Implementation(FBiciMoves Move)
{
	SimulateMove(Move);

	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = Velocity;
	ServerState.LastMove = Move;

}

bool ABiciPawn::Server_SendMove_Validate(FBiciMoves Move)
{
	return true;
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

void ABiciPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABiciPawn, ServerState);
	DOREPLIFETIME(ABiciPawn, Throttle);
	DOREPLIFETIME(ABiciPawn, SteeringThrow);
}

void ABiciPawn::OnRep_ServerState()
{
	SetActorTransform(ServerState.Transform);
	Velocity = ServerState.Velocity;

	ClearAcknowledgeMoves(ServerState.LastMove);
	for (const FBiciMoves& Move : UnacknowledgedMoves)
	{
		SimulateMove(Move);
	}
}
