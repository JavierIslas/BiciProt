// Fill out your copyright notice in the Description page of Project Settings.

#include "BiciReplicationComponent.h"
#include "UnrealNetwork.h"
#include "BiciMovementComponent.h"
#include "GameFramework/Actor.h"


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
	
	FCubicSpline Spline = CreateSpline();
	float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdate;


	InterpolateLocation(Spline, LerpRatio);

	InterpolateVelocity(Spline, LerpRatio);

	InterpolateRotation(LerpRatio);
}

float UBiciReplicationComponent::VelocityToDerivative()
{
	return ClientTimeBetweenLastUpdate * 100;
}

void UBiciReplicationComponent::InterpolateRotation(float LerpRatio)
{
	FQuat TargetRotation = ServerState.Transform.GetRotation();
	FQuat StartRotation = ClientStartTransform.GetRotation();
	FQuat NewRotation = FQuat::Slerp(StartRotation, TargetRotation, LerpRatio);
	if (MeshOffSetRoot)
	{
		MeshOffSetRoot->SetWorldRotation(NewRotation);
	}

}

void UBiciReplicationComponent::InterpolateVelocity(const FCubicSpline &Spline, float LerpRatio)
{
	FVector NewDerivative = Spline.InterpolateDerivative(LerpRatio);
	FVector NewVelocity = NewDerivative / VelocityToDerivative();
	MovementComp->SetVelocity(NewVelocity);
}

void UBiciReplicationComponent::InterpolateLocation(const FCubicSpline &Spline, float LerpRatio)
{
	//FVector NewLocation = FMath::LerpStable(StartLocation, TargetLocation, LerpRatio); Good for some games
	FVector NewLocation = Spline.InterpolateLocation(LerpRatio);
	if (MeshOffSetRoot)
	{
		MeshOffSetRoot->SetWorldLocation(NewLocation);
	}

}

FCubicSpline UBiciReplicationComponent::CreateSpline()
{
	FCubicSpline Spline;
	Spline.TargetLocation = ServerState.Transform.GetLocation();
	Spline.StartLocation = ClientStartTransform.GetLocation();
	Spline.StartDerivative = ClientStartVelocity * VelocityToDerivative();
	Spline.TargetDerivative = ServerState.Velocity * VelocityToDerivative();
	
	return Spline;
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

	ClientSimulatedTime += Move.DeltaTime;
	MovementComp->SimulateMove(Move);

	UpdateServerState(Move);

}

bool UBiciReplicationComponent::Server_SendMove_Validate(FBiciMoves Move)
{
	float ProposedTime = ClientSimulatedTime + Move.DeltaTime;
	bool ClientNotRunningAhead = ProposedTime < GetWorld()->TimeSeconds;
	if (!ClientNotRunningAhead)
	{
		return false; //Client Movement Time != Servern Time
	}
	return Move.IsValid(); //Invalid Scale of Throtting or SteeringThrow = false

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

	if (MeshOffSetRoot)
	{
		ClientStartTransform.SetLocation(MeshOffSetRoot->GetComponentLocation());
		ClientStartTransform.SetRotation(MeshOffSetRoot->GetComponentQuat());
	}
	ClientStartVelocity = MovementComp->GetVelocity();

	GetOwner()->SetActorTransform(ServerState.Transform);
}
