// Fill out your copyright notice in the Description page of Project Settings.

#include "BiciReplicationComponent.h"
#include "UnrealNetwork.h"
#include "BiciMovementComponent.h"


// Sets default values for this component's properties
UBiciReplicationComponent::UBiciReplicationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);

}


// Called when the game starts
void UBiciReplicationComponent::BeginPlay()
{
	Super::BeginPlay();

	MovementComp = GetOwner()->FindComponentByClass<UBiciMovementComponent>();
	
}


// Called every frame
void UBiciReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ensure(MovementComp)) return;

	FBiciMoves LastMove = MovementComp->GetLastMove();

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		UnacknowledgedMoves.Add(LastMove);
		Server_SendMove(LastMove);
	}

	//if Server and in control of the Pawn
	if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		UpdateServerState(LastMove);
	}

	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		ClientTick(DeltaTime);
	}

}

void UBiciReplicationComponent::UpdateServerState(const FBiciMoves& Move)
{
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComp->GetVelocity();
	ServerState.LastMove = Move;
}

void UBiciReplicationComponent::ClientTick(float DeltaTime)
{
	ClientTimeSinceUpdate += DeltaTime;

	if (ClientTimeBetweenLastUpdate < KINDA_SMALL_NUMBER) return;
	if (!ensure(MovementComp)) return;

	FVector TargetLocation = ServerState.Transform.GetLocation();
	float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdate;
	FVector StartLocation = ClientStartTransform.GetLocation();
	float VelocityToDerivative = ClientTimeBetweenLastUpdate * 100;
	FVector StartDerivative = ClientStartVelocity * VelocityToDerivative;
	FVector TargetDerivative = ServerState.Velocity * VelocityToDerivative;

	//FVector NewLocation = FMath::LerpStable(StartLocation, TargetLocation, LerpRatio); Good for some games
	FVector NewLocation = FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	GetOwner()->SetActorLocation(NewLocation);

	FVector NewDerivative = FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	FVector NewVelocity = NewDerivative / VelocityToDerivative;
	MovementComp->SetVelocity(NewVelocity);

	FQuat TargetRotation = ServerState.Transform.GetRotation();
	FQuat StartRotation = ClientStartTransform.GetRotation();

	FQuat NewRotation = FQuat::Slerp(StartRotation, TargetRotation, LerpRatio);
	GetOwner()->SetActorRotation(NewRotation);
}



void UBiciReplicationComponent::ClearAcknowledgeMoves(FBiciMoves LastMove)
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

void UBiciReplicationComponent::Server_SendMove_Implementation(FBiciMoves Move)
{
	if (!ensure(MovementComp)) return;

	MovementComp->SimulateMove(Move);

	UpdateServerState(Move);

}

bool UBiciReplicationComponent::Server_SendMove_Validate(FBiciMoves Move)
{
	return true;
}

void UBiciReplicationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBiciReplicationComponent, ServerState);
}

void UBiciReplicationComponent::OnRep_ServerState()
{
	switch (GetOwnerRole())
	{
	case ROLE_AutonomousProxy:
		AuthonomousProxy_OnRep_ServerState();
		break;
	case ROLE_SimulatedProxy:
		SimulatedProxy_OnRep_ServerState();
		break;
	default:
		break;
	}
}

void UBiciReplicationComponent::AuthonomousProxy_OnRep_ServerState()
{
	if (!ensure(MovementComp)) return;

	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComp->SetVelocity(ServerState.Velocity);

	ClearAcknowledgeMoves(ServerState.LastMove);
	for (const FBiciMoves& Move : UnacknowledgedMoves)
	{
		MovementComp->SimulateMove(Move);
	}
}

void UBiciReplicationComponent::SimulatedProxy_OnRep_ServerState() 
{
	if (!ensure(MovementComp)) return;

	ClientTimeBetweenLastUpdate = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0.0f;
	ClientStartTransform = GetOwner()->GetActorTransform();
	ClientStartVelocity = MovementComp->GetVelocity();
}
